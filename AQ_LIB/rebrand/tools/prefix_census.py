"""LA/MA/MB/LB prefix census for a project batch of the AlgoQuantLib rebrand.

  python rebrand/tools/prefix_census.py <project>

Writes rebrand/phase3_<project>_symbols.csv. Two sources are unioned:
  * every LA*-named source file STEM under src/<project>/  (authoritative - the
    file WILL be git-mv'd, so its stem MUST be in the rename map)
  * every LA/MA/MB/LB token that appears in src/<project>/ content AND is
    DEFINED there (class/struct/enum/#define/typedef/namespace or its own .h)

Columns: source(file|token|both), token, owned, proposed, refs, english, kind
Review, then feed the APPROVED csv to run_batch.py.
"""
import glob, re, sys, os, collections

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
os.chdir(ROOT)
proj = sys.argv[1]
srcdir = "src/%s" % proj
files = [f.replace("\\", "/") for f in
         glob.glob(srcdir + "/**/*.cpp", recursive=True) +
         glob.glob(srcdir + "/**/*.h", recursive=True)
         if not f.endswith(("_wrap.cpp", "_wrap.cxx"))]
if not files:
    sys.exit("no source under " + srcdir)

file_stems = {f.split("/")[-1].rsplit(".", 1)[0] for f in files
              if f.split("/")[-1][:2] == "LA"}

TOK = re.compile(r"\b(?:LA|LB|MA|MB)[A-Za-z0-9_]+")
counts = collections.Counter()
blob_parts = []
for f in files:
    t = open(f, encoding="utf-8", errors="replace").read()
    blob_parts.append(t)
    for m in TOK.finditer(t):
        counts[m.group(0)] += 1
blob = "\n".join(blob_parts)

# one-pass extraction of everything that is DEFINED in this project's source
def_class = set(re.findall(r"\b(?:class|struct)\s+((?:LA|LB|MA|MB)[A-Za-z0-9_]+)", blob))
def_ns    = set(re.findall(r"\bnamespace\s+((?:LA|LB|MA|MB)[A-Za-z0-9_]+)", blob))
def_macro = set(re.findall(r"#\s*define\s+((?:LA|LB|MA|MB)[A-Za-z0-9_]+)", blob))
def_enum  = set(re.findall(r"\benum\s+(?:class\s+)?((?:LA|LB|MA|MB)[A-Za-z0-9_]+)", blob))
def_tdef  = set(re.findall(r"\btypedef\b[^;\n]*?\b((?:LA|LB|MA|MB)[A-Za-z0-9_]+)\s*;", blob))
DEFINED = def_class | def_ns | def_macro | def_enum | def_tdef

def owned(tok):
    return tok in file_stems or tok in DEFINED

def kind(tok):
    k = []
    if tok in def_class: k.append("class")
    if tok in def_ns:    k.append("namespace")
    if tok in def_macro: k.append("macro")
    if tok in def_enum:  k.append("enum")
    if tok in def_tdef:  k.append("typedef")
    if tok in file_stems: k.append("file")
    return ",".join(k) or "?"

ENGLISH = re.compile(r"^(?:MA(?:X\b|X_|XIMUM|XIT|SK|TRIX|TCH|NAGER\b|NUAL\b|RGIN|RGINAL|RKET|RQUARDT|TURITY|SS\b)"
                     r"|LA(?:BEL\b|ST\b|RGE\b|YER\b|TER\b|PACK\b|MBDA\b|UNCH))")

def bucket(t):
    if re.match(r"^(?:LA|LB|MA|MB)[A-Z][a-z]", t): return "A_convention"
    if re.match(r"^(?:LA|LB|MA|MB)_", t):          return "C_underscore"
    if re.match(r"^(?:LA|LB|MA|MB)[A-Z][A-Z0-9]", t): return "B_acronym"
    return "D_other"

all_tokens = set(counts) | file_stems
rows = []
for t in sorted(all_tokens, key=lambda x: -counts.get(x, 0)):
    src = ("both" if (t in file_stems and t in counts) else
           "file" if t in file_stems else "token")
    o = owned(t)
    eng = 1 if ENGLISH.match(t) else 0
    prefix = "AQL" if t[:2] == "LA" else "AQ"
    prop = ""
    if o and not eng and bucket(t) in ("A_convention", "D_other", "C_underscore"):
        prop = prefix + t[2:]
    rows.append((src, t, int(o), prop, counts.get(t, 0), eng, kind(t)))

out = "rebrand/phase3_%s_symbols.csv" % proj
with open(out, "w", encoding="utf-8", newline="") as fh:
    fh.write("source,token,owned,proposed,refs,english,kind\n")
    for r in rows:
        fh.write("%s,%s,%d,%s,%d,%d,%s\n" % r)

ren = [r for r in rows if r[3]]
rev = [r for r in rows if r[2] and not r[3]]
notown = [r for r in rows if not r[2]]
print("project %s : %d files, %d LA*-file-stems, %d distinct tokens" %
      (proj, len(files), len(file_stems), len(counts)))
print("  auto-rename (owned)         : %d" % len(ren))
print("  owned + REVIEW (acronym/eng): %d" % len(rev))
print("  not owned (ref only - skip) : %d" % len(notown))
print("-> " + out)
print("\nREVIEW:")
for r in rev:
    print("  %-40s x%-5d %s  [%s]%s" % (r[1], r[4], r[0], r[6], "  ENGLISH?" if r[5] else ""))
print("\nfile-stems with NO content token (renamed blind - check they are real):")
for r in rows:
    if r[0] == "file" and r[4] == 0:
        print("  %-40s %s" % (r[1], r[6]))
