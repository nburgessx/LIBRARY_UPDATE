"""Legacy-prefix census for a Phase 3 project batch of the AlgoQuantLib rebrand.

  python rebrand/tools/prefix_census.py <project>

Writes rebrand/phase3_<project>_symbols.csv. Two sources are unioned:
  * every  <PREFIX>*-named source file STEM under src/<project>/  (authoritative -
    the file WILL be git-mv'd, so its stem MUST be in the rename map)
  * every  <PREFIX>  token that appears in src/<project>/ content AND is DEFINED
    there (class/struct/enum/#define/typedef/namespace or its own .h)

<PREFIX> = one of the reviewed legacy families:
    LA LB MA MB   (Legacy Analytics / early type prefixes)
    MM MV MD MF   (models/math internal functor + data prefixes)
    ME            (ME_* legacy macros)

Per Nicholas (2026-09): in the three Classic projects (math / models /
calibration) EVERY legacy prefix maps to  AQL  ("AQ Legacy") so the whole tree
stays greppable and marked for deprecation. ME/me originating in AQ - Core
(validation / XLL / API) is a different prefix -> aq, handled by the me->aq step.

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

PRE = r"(?:LA|LB|MA|MB|MM|MV|MD|MF|ME)"

file_stems = {f.split("/")[-1].rsplit(".", 1)[0] for f in files
              if re.match("^" + PRE + r"[A-Z0-9_]", f.split("/")[-1])}

TOK = re.compile(r"\b" + PRE + r"[A-Za-z0-9_]+")
counts = collections.Counter()
blob_parts = []
for f in files:
    t = open(f, encoding="utf-8", errors="replace").read()
    blob_parts.append(t)
    for m in TOK.finditer(t):
        counts[m.group(0)] += 1
blob = "\n".join(blob_parts)

# one-pass extraction of everything DEFINED in this project's source
def_class = set(re.findall(r"\b(?:class|struct)\s+(" + PRE + r"[A-Za-z0-9_]+)", blob))
def_ns    = set(re.findall(r"\bnamespace\s+(" + PRE + r"[A-Za-z0-9_]+)", blob))
def_macro = set(re.findall(r"#\s*define\s+(" + PRE + r"[A-Za-z0-9_]+)", blob))
def_enum  = set(re.findall(r"\benum\s+(?:class\s+)?(" + PRE + r"[A-Za-z0-9_]+)", blob))
def_tdef  = set(re.findall(r"\btypedef\b[^;\n]*?\b(" + PRE + r"[A-Za-z0-9_]+)\s*;", blob))
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

# false positives that live inside the scanned families (LA / MA / ME ...)
ENGLISH = re.compile(
    r"^(?:"
    r"MA(?:X\b|X_|XIMUM|XIT|SK|TRIX|TCH|NAGER\b|NUAL\b|RGIN|RGINAL|RKET|RQUARDT|TURITY|SS\b)"
    r"|ME(?:AN\b|MORY|MBER|SSAGE|THOD|DIAN|TA\b|SH\b|TRE\b|TER\b)"
    r"|MB(?:ER\b)"
    r"|LA(?:BEL\b|ST\b|RGE\b|YER\b|TER\b|PACK\b|MBDA\b|UNCH|G\b|GGED)"
    r")"
)

def bucket(t):
    if re.match("^" + PRE + r"[A-Z][a-z]", t):  return "A_convention"
    if re.match("^" + PRE + r"_", t):           return "C_underscore"
    if re.match("^" + PRE + r"[A-Z][A-Z0-9]", t): return "B_acronym"
    return "D_other"

all_tokens = set(counts) | file_stems
rows = []
for t in sorted(all_tokens, key=lambda x: -counts.get(x, 0)):
    src = ("both" if (t in file_stems and t in counts) else
           "file" if t in file_stems else "token")
    o = owned(t)
    eng = 1 if ENGLISH.match(t) else 0
    prop = ""
    if o and not eng:                     # every legacy family -> AQL in Classic
        prop = "AQL" + t[2:]              # ME_BUILD.. -> AQL_BUILD..  LA_X -> AQL_X
    rows.append((src, t, int(o), prop, counts.get(t, 0), eng, bucket(t), kind(t)))

out = "rebrand/phase3_%s_symbols.csv" % proj
with open(out, "w", encoding="utf-8", newline="") as fh:
    fh.write("source,token,owned,proposed,refs,english,bucket,kind\n")
    for r in rows:
        fh.write("%s,%s,%d,%s,%d,%d,%s,%s\n" % r)

ren    = [r for r in rows if r[3]]
rev    = [r for r in rows if r[2] and not r[3]]
notown = [r for r in rows if not r[2]]
acr    = [r for r in ren if r[6] == "B_acronym"]
print("project %s : %d files, %d PREFIX*-file-stems, %d distinct tokens" %
      (proj, len(files), len(file_stems), len(counts)))
print("  auto-rename (owned, ->AQL)      : %d  (of which %d acronym-style)" % (len(ren), len(acr)))
print("  owned but NOT proposed (english): %d" % len(rev))
print("  not owned (ref only - skip)     : %d" % len(notown))
print("-> " + out)
print("\nNOT-PROPOSED (owned, english-looking - confirm skip):")
for r in rev:
    print("  %-42s x%-5d %s  [%s]" % (r[1], r[4], r[0], r[7]))
print("\nfile-stems with NO content token (renamed blind - check they are real):")
for r in rows:
    if r[0] == "file" and r[4] == 0:
        print("  %-42s %s" % (r[1], r[7]))
print("\nsample acronym-style auto-renames (eyeball a few):")
for r in acr[:25]:
    print("  %-42s -> %-42s x%d" % (r[1], r[3], r[4]))
