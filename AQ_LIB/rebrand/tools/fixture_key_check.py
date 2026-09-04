"""Recorded-fixture keys are STRINGIFIED C++ PARAMETER NAMES.

RecordMacros.h expands WRITE_PARAMETER(P) to file.write("P", P), so the first
column of every *_inputs.csv is literally the parameter identifier as it was
spelled when the fixture was recorded. Renaming a parameter therefore breaks
the read path silently -- the build stays green and the test fails at run time
with:

    ReadDataFile::Load: unknown key: <newName>

This caught lwoCurveGeneratorName / lwoCurveMarketDataName, which survived two
renames (lwo -> aqo in step 7c, aqo -> aqObj in step 8B) because only code and
FILE NAMES were renamed, never fixture CONTENT.

Run after any rename that touches a validation function's parameters:
    python rebrand/tools/fixture_key_check.py

Exit status 1 if any fixture key carries a legacy prefix."""
import subprocess, re, os, sys, collections

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
os.chdir(ROOT)

LEGACY = re.compile(r"^(?:lwo|aqo|LWO|AQO)[A-Za-z0-9_]*$|^(?:try)?[Mm](?:e[A-Z]|ir[A-Z])")

keys = collections.Counter(); files = collections.defaultdict(set)
gen  = collections.Counter()
for p in subprocess.check_output(["git", "ls-files", "resources/test"], text=True).splitlines():
    if not p.endswith(".csv"):
        continue
    try:    t = open(p, encoding="utf-8", errors="replace").read()
    except Exception:
        continue
    for ln in t.split("\n"):
        parts = ln.split(",")
        if not parts:
            continue
        k = parts[0].strip()
        if k == "generatorFunction" and len(parts) > 1 and LEGACY.match(parts[1].strip()):
            gen[parts[1].strip()] += 1
        elif k and LEGACY.match(k):
            keys[k] += 1; files[k].add(p)

print("fixture KEYS carrying a legacy prefix: %d distinct" % len(keys))
for k, v in sorted(keys.items(), key=lambda x: -x[1]):
    print("   %5d occurrences  %-36s %d files" % (v, k, len(files[k])))
    for f in sorted(files[k])[:3]:
        print("         %s" % f)

print("\nADVISORY -- generatorFunction values naming a legacy function: %d distinct, %d rows"
      % (len(gen), sum(gen.values())))
for g, v in gen.most_common(8):
    print("   %5d  %s" % (v, g))
print("   (provenance metadata, not read by the loader -- Phase 6 resources sweep)")

sys.exit(1 if keys else 0)
