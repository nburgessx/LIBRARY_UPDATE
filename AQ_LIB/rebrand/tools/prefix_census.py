import glob, re, collections, sys

ROOT = r"C:/Users/nburg/Documents/MY_DOCUMENTS/REPO/LIBRARY_UPDATE/AQ_LIB"
proj = sys.argv[1] if len(sys.argv) > 1 else "math"
files = glob.glob(ROOT + "/src/%s/**/*.cpp" % proj, recursive=True) + \
        glob.glob(ROOT + "/src/%s/**/*.h" % proj, recursive=True)
files = [f for f in files if not f.endswith(("_wrap.cpp", "_wrap.cxx"))]

block  = re.compile(r"/\*.*?\*/", re.S)
line_c = re.compile(r"//[^\n]*")
dq     = re.compile(r'"(?:[^"\\]|\\.)*"')

def strip(t):
    return dq.sub(" ", line_c.sub(" ", block.sub(" ", t)))

TOK = re.compile(r"\b(?:LA|LB|MA|MB)[A-Za-z0-9_]+")
counts = collections.Counter()
parts = []
for f in files:
    r = open(f, encoding="utf-8", errors="replace").read()
    parts.append(r)
    for m in TOK.finditer(strip(r)):
        counts[m.group(0)] += 1
raw_all = "\n".join(parts)

def kind(tok):
    e = re.escape(tok)
    ks = []
    if re.search(r"\b(?:class|struct)\s+" + e + r"\b", raw_all): ks.append("class")
    if re.search(r"#\s*define\s+" + e + r"\b", raw_all): ks.append("macro")
    if re.search(r"\benum\s+(?:class\s+)?" + e + r"\b", raw_all): ks.append("enum")
    if re.search(e + r"\s*::", raw_all): ks.append("scope")
    if re.search(e + r"\s*\(", raw_all): ks.append("call")
    return ",".join(ks) or "?"

rows = sorted(counts.items(), key=lambda kv: (-kv[1], kv[0]))
A = [(t, c) for t, c in rows if re.match(r"^(?:LA|LB|MA|MB)[A-Z][a-z]", t)]
B = [(t, c) for t, c in rows if re.match(r"^(?:LA|LB|MA|MB)[A-Z][A-Z0-9]", t)]
C = [(t, c) for t, c in rows if re.match(r"^(?:LA|LB|MA|MB)_", t)]
D = [(t, c) for t, c in rows if re.match(r"^(?:LA|LB|MA|MB)[a-z0-9]", t)]

print("project %s: %d files | distinct (LA|LB|MA|MB)* tokens: %d" % (proj, len(files), len(rows)))
print("  A convention  LA[A-Z][a-z]       : %4d  (%d refs)" % (len(A), sum(c for _, c in A)))
print("  B acronym     LA[A-Z][A-Z0-9] REV: %4d" % len(B))
print("  C underscore  LA_...          REV: %4d" % len(C))
print("  D english     LA+lowercase   SKIP: %4d" % len(D))

out = ROOT + "/rebrand/phase3_%s_symbols.csv" % proj
with open(out, "w", encoding="utf-8") as fh:
    fh.write("bucket,token,proposed,refs,kind\n")
    for name, lst in [("A_convention", A), ("B_acronym_REVIEW", B),
                      ("C_underscore_REVIEW", C), ("D_english_SKIP", D)]:
        for t, c in lst:
            prop = "AQ" + t[2:] if name != "D_english_SKIP" else ""
            fh.write("%s,%s,%s,%d,%s\n" % (name, t, prop, c, kind(t)))
print("-> " + out)
print()
print("=== BUCKET B (acronym / ALLCAPS - REVIEW EACH) ===")
for t, c in B:
    print("  %-30s x%-6d %s" % (t, c, kind(t)))
print()
print("=== BUCKET C (LA_ / MA_ underscore - REVIEW) ===")
for t, c in C:
    print("  %-38s x%-6d %s" % (t, c, kind(t)))
print()
print("=== BUCKET D (LA + lowercase - English words, expect SKIP) ===")
for t, c in D:
    print("  %-22s x%d" % (t, c))
