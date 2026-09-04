"""Apply an approved Phase 3 LA->AQL rename batch for one project.

  python rebrand/tools/run_batch.py <project>

Reads rebrand/phase3_<project>_APPROVED.csv (columns old,new,...). Does:
  1. git mv every LA*-named file in src/<project>/  ->  AQL*
  2. tree-wide word-boundary identifier replace using the approved map,
     which MUST already include every renamed file's stem
  3. vcxproj/.filters filename pass: rewrite Include="...\LA<X>.<ext>" paths for
     renamed files case-insensitively (handles PayOff vs Payoff casing drift)
  4. verify: no broken AQL*.h includes, no missing vcxproj refs (case-insensitive),
     no AQAQL / AQLL artifacts
Aborts (nonzero exit) without writing if any file is locked - caller reverts.
"""
import glob, re, csv, os, subprocess, time, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
os.chdir(ROOT)
proj = sys.argv[1]
approved = "rebrand/phase3_%s_APPROVED.csv" % proj

pairs = [(r["old"], r["new"]) for r in csv.DictReader(open(approved, encoding="utf-8")) if r.get("new")]
rmap = dict(pairs)

# --- 1. git mv files ---
# ordered longest-prefix-first so LWO wins over LB/LA
FILE_PREFIX = [("LWO", "AQO"), ("LA", "AQL"), ("LB", "AQL")]
def new_base(base):
    for old, new in FILE_PREFIX:
        if base[:len(old)] == old:
            return new + base[len(old):]
    return None

renamed = []   # (old_basename, new_basename)
for p in subprocess.check_output(["git", "ls-files", "src/%s" % proj], text=True).splitlines():
    base = p.rsplit("/", 1)[-1]
    nb = new_base(base)
    if nb:
        subprocess.check_call(["git", "mv", p, p.rsplit("/", 1)[0] + "/" + nb])
        renamed.append((base, nb))
print("git mv: %d files" % len(renamed))

# every renamed stem must be in the map
stem_map = {b.rsplit(".", 1)[0]: nb.rsplit(".", 1)[0] for b, nb in renamed}
missing_stems = [s for s in stem_map if s not in rmap]
if missing_stems:
    print("ABORT: %d renamed file stems are not in the approved map:" % len(missing_stems))
    for s in missing_stems[:20]:
        print("   ", s)
    sys.exit(2)

# --- 1b. merge vcxproj/.filters filename casings into the map ---
# a file's git-stored casing can differ from the casing recorded in the .vcxproj
# (e.g. Payoff vs PayOff on case-insensitive Windows). Any LA*.<ext> token in a
# project file whose stem case-insensitively matches a renamed file gets its own
# map entry so the plain word-boundary replace fixes it.
new_stem_ci = {b.rsplit(".", 1)[0].lower(): nb.rsplit(".", 1)[0]
               for b, nb in renamed}
for pf in glob.glob("projects/*.vcxproj") + glob.glob("projects/*.vcxproj.filters"):
    for tok in re.findall(r"\b((?:LWO|LA|LB)[A-Za-z0-9_]+)\.(?:h|hpp|cpp|cxx)\b",
                          open(pf, encoding="utf-8", errors="replace").read()):
        if tok not in rmap and tok.lower() in new_stem_ci:
            rmap[tok] = new_stem_ci[tok.lower()]   # normalise to git's stored casing
            print("  vcxproj-casing merge: %s -> %s" % (tok, rmap[tok]))

# --- 2. tree-wide identifier replace ---
keys = sorted(rmap, key=len, reverse=True)
one = re.compile("|".join(r"\b" + re.escape(k) + r"\b" for k in keys))
targets = [f for f in
           glob.glob("src/**/*.cpp", recursive=True) + glob.glob("src/**/*.h", recursive=True) +
           glob.glob("src/**/*.i", recursive=True) +
           glob.glob("projects/*.vcxproj") + glob.glob("projects/*.vcxproj.filters") +
           ["Visualizer.natvis"]
           if not f.endswith(("_wrap.cpp", "_wrap.cxx"))]

# --- 3. also prepare a case-insensitive filename map for vcxproj/filters ---
ci_file = {b.lower(): nb for b, nb in renamed}
inc_re = re.compile(r'(Include=")([^"]*?)([^"\\/]+\.(?:h|hpp|cpp|cxx))(")')
def fix_inc(m):
    pre, path, fn, post = m.groups()
    return pre + path + ci_file.get(fn.lower(), fn) + post

new_toks = set(rmap.values())
strlit = re.compile(r'"(?:[^"\\]|\\.)*"')
in_string = []   # (file, lineno, text) - replacements that landed inside a "..." literal

pending, total = {}, 0
for fp in targets:
    if not os.path.exists(fp):
        continue
    b = open(fp, "rb").read()
    try:
        t = b.decode("utf-8"); enc = "utf-8"
    except UnicodeDecodeError:
        t = b.decode("latin-1"); enc = "latin-1"
    nt, n = one.subn(lambda m: rmap[m.group(0)], t)
    if fp.startswith("projects/"):
        nt = inc_re.sub(fix_inc, nt)
    if nt != t:
        data = nt.encode(enc)
        if b.startswith(b"\xef\xbb\xbf") and not data.startswith(b"\xef\xbb\xbf"):
            data = b"\xef\xbb\xbf" + data
        pending[fp] = data
        total += n
        # flag renamed tokens now sitting inside a string literal (not an #include)
        if not fp.startswith("projects/"):
            for i, ln in enumerate(nt.splitlines(), 1):
                if ln.lstrip().startswith("#include"):
                    continue
                for lit in strlit.findall(ln):
                    if any(re.search(r"\b" + re.escape(k) + r"\b", lit) for k in new_toks):
                        in_string.append((fp, i, ln.strip()[:140]))

failed = []
for fp, data in pending.items():
    for at in range(6):
        try:
            open(fp, "wb").write(data); break
        except PermissionError:
            if at == 5: failed.append(fp)
            else: time.sleep(0.8)
if failed:
    print("LOCKED: " + ", ".join(failed[:8])); sys.exit(3)
print("content: %d files, %d identifier replacements (+ vcxproj filename fixups)" % (len(pending), total))
if in_string:
    print("\n!! %d renamed tokens landed INSIDE string literals - REVIEW (may be paths / serialization keys):" % len(in_string))
    for fp, i, ln in in_string[:60]:
        print("   %s:%d  %s" % (fp, i, ln))
    if len(in_string) > 60:
        print("   ... +%d more" % (len(in_string) - 60))

# --- 4. verify ---
tracked = subprocess.check_output(["git", "ls-files", "src"], text=True).splitlines()
hdrs_ci = {p.rsplit("/", 1)[-1].lower() for p in tracked if p.endswith(".h")}
badinc = set()
for f in glob.glob("src/**/*.cpp", recursive=True) + glob.glob("src/**/*.h", recursive=True):
    if f.endswith(("_wrap.cpp", "_wrap.cxx")):
        continue
    for m in re.finditer(r'#include\s+"((?:AQL|AQO)[A-Za-z0-9_]+\.h)"', open(f, encoding="utf-8", errors="replace").read()):
        if m.group(1).lower() not in hdrs_ci:
            badinc.add(m.group(1))
badvcx = []
for vp in glob.glob("projects/*.vcxproj"):
    tx = open(vp, encoding="utf-8", errors="replace").read()
    for r in re.findall(r'(?:ClCompile|ClInclude)\s+Include="([^"]+)"', tx):
        disk = os.path.join("projects", r.replace("\\", "/"))
        if not os.path.exists(disk):   # NTFS is case-insensitive; this is a real miss
            badvcx.append((os.path.basename(vp), r))
art = subprocess.run(["grep", "-rlE", r"AQAQ[LO]|AQ[LO]AQ[LO]|\bAQLL[A-Z]|AQOObject", "src"], capture_output=True, text=True).stdout.split()
print()
print("VERIFY broken AQL*/AQO*.h includes :", sorted(badinc) or "NONE")
print("VERIFY missing vcxproj refs        :", badvcx or "NONE")
print("VERIFY AQAQ*/AQLL/AQOObject arts   :", art or "NONE")
if badinc or badvcx or art:
    sys.exit(4)
print("\nOK")
