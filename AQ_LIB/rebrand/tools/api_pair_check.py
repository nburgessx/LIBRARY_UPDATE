"""API surface check (CLAUDE.md 4.1 and 5.1a).

Hard gate  : every public aq* function must route through the validation layer,
             i.e. its body calls a validation::tryAq* (or, for a thin convenience
             overload, delegates to another overload of itself that does).
Advisory   : public aq*Foo whose wrapper is not named tryAq*Foo (name drift), and
             validation wrappers with no public function yet (port coverage --
             expected to be large until the Phase 4 XLL/API port is done).

  python rebrand/tools/api_pair_check.py            # report
  python rebrand/tools/api_pair_check.py --write    # also rewrite docs/api_map.csv

Exit status 1 if the hard gate fails."""
import subprocess, re, os, sys, csv, collections

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
os.chdir(ROOT)

# LOCKED (21), SINGULAR. Future added at step 11b. Product sub-types are their own category -- there is no
# Options umbrella. `Ois` is NOT a category (folded into Swap 2026-09-11 as a
# product variant -- aqSwapOis*, not its own top-level category; see CLAUDE.md
# Sec 5.1 / MIGRATION_PLAN.md Sec 2.7). Longest-first matching matters:
# BondFutureOption before BondOption, TRS/AssetSwap/
# CMS before Swap.
CATEGORIES = ["Date", "Curve", "FX", "Inflation", "Volatility", "IR", "Future",
              "Swap", "AssetSwap", "CMS", "TRS",
              "CapFloor", "Swaption", "BondOption", "BondFutureOption",
              "Bond", "Credit", "Math", "Model", "Generator", "Tool"]

def files(spec):
    return [p for p in subprocess.check_output(["git", "ls-files", spec], text=True).split()
            if "_wrap." not in p]

def read(p):
    return open(p, encoding="utf-8", errors="replace").read()

def body_of(text, start):
    """source text of one function, from its signature to the matching brace"""
    j = text.find("{", start)
    if j < 0:
        return ""
    depth, k = 0, j
    while k < len(text):
        if text[k] == "{":
            depth += 1
        elif text[k] == "}":
            depth -= 1
            if depth == 0:
                return text[start:k + 1]
        k += 1
    return text[start:]

# declared public names (headers) and their implementations (.cpp)
pub = {}
for p in files("src/AQ_API/source/*.h"):
    for m in re.finditer(r"\b(aq[A-Z][A-Za-z0-9_]*)\s*\(", read(p)):
        pub.setdefault(m.group(1), p)

impl_calls = {}
for p in files("src/AQ_API/source/*.cpp"):
    t = read(p)
    for m in re.finditer(r"\b(aq[A-Z][A-Za-z0-9_]*)\s*\(", t):
        n = m.group(1)
        b = body_of(t, m.start())
        if b.count("\n") > 1:                      # a definition, not a call
            impl_calls.setdefault(n, (p,
                sorted(set(re.findall(r"validation::(tryAq[A-Za-z0-9_]*)", b))),
                # a thin convenience overload may just delegate to a sibling overload
                b.count(n + "(") + b.count(n + " (") > 1))

wrap = {}
for p in files("src/validation/include/*.h"):
    for m in re.finditer(r"\b(tryAq[A-Z][A-Za-z0-9_]*)\s*\(", read(p)):
        wrap.setdefault(m.group(1), p)

def category(n):
    # aqDateFromTenor / aqBondObjectPrice / aqBondCurveYield / aqObjectLoad
    b = n[5:] if n.startswith("tryAq") else n[2:]
    if b.startswith("Object"):
        return "(lifecycle)"
    for c in sorted(CATEGORIES, key=len, reverse=True):
        if b.startswith(c):
            return c
    return "(lifecycle)"

unrouted, drift = [], []
for n in sorted(pub):
    p, calls, self_delegates = impl_calls.get(n, (None, [], False))
    if p is None:
        continue                                    # header-only declaration
    if not calls and not self_delegates:
        unrouted.append(n)
    elif calls and "tryAq" + n[2:] not in calls:
        drift.append((n, calls[0]))

orphan = sorted(n for n in wrap if "aq" + n[5:] not in pub)

print("public aq* declared : %d   (implemented: %d)" % (len(pub), len(impl_calls)))
print("validation tryAq*   : %d" % len(wrap))
print("\nHARD GATE -- public functions not routed through validation: %d" % len(unrouted))
for n in unrouted: print("   ", n)
print("\nADVISORY -- wrapper name drift: %d" % len(drift))
for n, c in drift: print("   %-50s -> %s" % (n, c))
print("\nADVISORY -- wrappers with no public function yet: %d "
      "(expected until the Phase 4 port lands)" % len(orphan))
print("\npublic surface by category:",
      dict(collections.Counter(category(n) for n in pub).most_common()))

if "--write" in sys.argv:
    rows = []
    for n in sorted(pub):
        p, calls, _ = impl_calls.get(n, (pub[n], [], False))
        w = "tryAq" + n[2:]
        rows.append([n, category(n),
                     "object" if "Object" in n[2:] else "stateless",
                     w if w in wrap else (calls[0] if calls else ""),
                     pub[n], wrap.get(w, "")])
    os.makedirs("docs", exist_ok=True)
    with open("docs/api_map.csv", "w", encoding="utf-8", newline="") as fh:
        cw = csv.writer(fh)
        cw.writerow(["public_name", "category", "surface", "validation_wrapper",
                     "public_header", "wrapper_header"])
        cw.writerows(rows)
    print("\nwrote docs/api_map.csv (%d rows)" % len(rows))

sys.exit(1 if unrouted else 0)
