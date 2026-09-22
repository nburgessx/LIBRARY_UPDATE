# AlgoQuantLib — project status

**As at 2026-09-15.** Committed HEAD is `f0dd6217` — includes `aqCurve.cpp`/
`aqInterestRate.cpp` (now `aqIR.cpp`) promoted from `src\Optional` to
`src\Core`. Everything below this point since is **uncommitted working-tree
delta** — Nicholas commits at his own pace. Build is green in all
configurations: Nicholas-confirmed multiple times, most recently after the
2026-09-15 `aqCreditBasketModel*`/`aqCDSObject*`/`aqGridObject*`
naming-convention fix; and separately this session, the new `Generator`
category (below) was built and individually verified across `validation`
(Debug|x64), `GTEST` (Debug|x64, all 6 new cases), `AQ_API`
(`Debug_API_Python`, smoke-tested end-to-end), and `AQ_XLL` (`Debug`,
`Release`, all four `Release_XL_*` editions, and `Release_XL_Manifest`).

**A full-suite `GTEST` regression run this session was paused partway
through (Nicholas's call, session ending for the day), not completed.** What
ran so far passed; Nicholas separately reports roughly 10 failures overall,
almost all calendar-related, from his own run. Likely cause (not yet
confirmed against the actual failure list): `resources\config\Calendar.conf`
carries a **known stopgap** — its comment says the `LastCalendarUpdate` date
was bumped to silence `Calendars.UNIT_Expiry_Test` without actually
regenerating `Calendar.csv`, which is still dated 2022-07-22. Any test whose
result depends on holiday coverage for "today" (2026-09-15) or beyond would
be running against stale/incomplete holiday data. This is pre-existing,
flagged in `CLAUDE.md` as Phase 6.4 work (refresh from MarketWire/SwapsWire),
and unrelated to this session's `Generator`-category or rename work — **not
investigated further this session, picking back up next time.**

- The **phase-by-phase plan** is `MIGRATION_PLAN.md`.
- The **detailed running record** of the rebrand (per-batch, per-decision) is
  `rebrand\STATUS.md`. This file is the summary; that file is the evidence.

---

## 1. Headline

**Phase 4 (the xlOil XLL port) is complete.** Every category with an
existing `validation` surface has been ported, and the one category-level
gap flagged previously — `Generator` — was built from scratch this session
(§1a). `Model` remains genuinely unscoped: no legacy port source exists and
no design brief has been written for it, so it stays deliberately untouched.

| Phase | Scope | State |
|---|---|---|
| 0 | Baseline, inventories, safety net | **done** |
| 1 | Structural: projects, folders, macros, scripts | **done** |
| 2 | Category taxonomy (21 categories, locked) | **done** |
| 3 | Identifier rebrand + calendar delimiter | **done** |
| 3c | Retire `mir*` | **done** (deleted wholesale) |
| 4 | xlOil XLL port | **done.** All ported wrappers covered (§2) plus the new `Generator` category (§1a) built across all four surfaces. `Model` is out of scope until a design brief exists — see §1a. |
| 4a | Editions & manifest gating | **done (2026-09-15), `AQ_XLL`-only.** `Release`/`Release_XL_Bond`/`Release_XL_Swap`/`Release_XL_Credit`/`Release_XL_Curve` build configurations shipped and building green. The `AQ_API` runtime-manifest gate is **dropped, not deferred** — `AQ_API` has no edition concept. |
| 4b | Config folder & generators audit | not started |
| 5 | Bindings (C#/Java/R) & test coverage | not started |
| 6 | Legacy extraction, resources, docs, licence | not started |
| 7 | Linux build, final sweep, clean repo | not started |

---

## 1a. `Generator` category — built this session (2026-09-15)

Introspection-only per `CLAUDE.md` §5.1: `aqGeneratorList`/`Display`/
`Validate` (`Display` — renamed same-session from an initial `Describe`, to
match the pre-existing `aqSwapGeneratorDisplay`/`aqBondGeneratorDisplay`/
`aqCurveGeneratorDisplay` naming each asset category already used), scanning
`resources\config\{SWAP,BOND,CURVE}_GENERATOR` — not
construction, which stays in each asset category (`aqSwapGenerator*`,
`aqBondGenerator*`, `aqCurveGenerator*`, all pre-existing). Built end-to-end
across all four surfaces (`validation` → `GTEST` → `AQ_API` → `AQ_XLL`,
per §5.1a's order) and verified: `validation` builds Debug|x64; `GTEST`'s 6
new cases pass; `AQ_API` built and smoke-tested through the Python binding;
`AQ_XLL` builds in `Debug`, `Release`, all four `Release_XL_*` editions, and
`Release_XL_Manifest` (Generator is not edition-excluded — every edition
needs it for its own generator type). Full detail, including a
`ThreadGuard`-nesting bug GoogleTest caught before it shipped:
`rebrand\STATUS.md`'s latest entry.

**Also fixed same session:** `aqGeneratorDisplay`'s Excel output wasn't
transposed — the underlying block is column-major (one column per key/value
pair) and every sibling `*GeneratorDisplay` (`aqBondGeneratorDisplay` etc.)
already transposes before returning, via `toExcelMatrix(
etrading::toAQLStringMatrixFromVariantMatrix( result, true ) )` rather than
the plain `toExcelMatrix(VariantMatrix)` overload, which has no transpose
argument. `aqGeneratorDisplay` now matches that exact pattern. `AQ_XLL`
Debug rebuilt green after the fix.

`Model` was deliberately left untouched — it has no legacy port source
(`.APPLES` has nothing named `meModel*`) and no design brief (which model
types, what parameters), unlike `Generator` which only needed to expose
files that already exist on disk.

---

## 1b. Manifest files renamed (2026-09-15, same session)

`resources\manifest\active.txt` → `activeList.txt`, `demo.txt` →
`demoList.txt` (Nicholas's call — `List` signals these are the plain-text
function-list manifests `generateManifestList.bat` reads). Renamed via
`git mv`. The batch script needed no logic change (takes the manifest path
as a parameter); only its comment and every other hard-coded reference
(`AQ_XLL.vcxproj`'s `AQ_XLL_MANIFEST_FILE` default + comment + `<None>`
entries, the matching `.vcxproj.filters` entries, each manifest's own
cross-reference comment, `CLAUDE.md`, `MIGRATION_PLAN.md`) were updated.
Verified by rebuilding `Release_XL_Manifest|x64` green. Detail:
`rebrand\STATUS.md`.

---

## 2. Have we migrated all functions? Short answer: yes, bar 9 deliberate exclusions and 1 empty category (`Model`).

A script-driven audit (every `tryAq*` name declared in `src\validation\include\*.h`
vs. every `XLO_FUNC_START` name registered in `src\AQ_XLL\src\*.cpp`, matched
by golden name) is the source of truth here — not a manual category checklist.
Current result (including the 3 new `Generator` wrappers added this session):

- **470 validation wrappers**, **470 `AQ_XLL` functions**, **460 wrappers with
  a matching `AQ_XLL` function**.
- The audit now reports 10 unmatched rather than 9 — the 9 below, unchanged,
  plus `tryAqCurveObjectDataCreate` showing up as a **script artifact, not a
  real gap**: the regex extracts it with a trailing space (a pre-existing
  comment/formatting quirk in its header, unrelated to this session's
  changes), so the derived `aqCurveObjectDataCreate ` lookup never matches
  the real `aqCurveObjectDataCreate` in `AQ_XLL`. Noticed incidentally while
  re-running this audit after adding `Generator`; not investigated further.
- **9 wrappers with no `AQ_XLL` function — all intentional:**
  - `tryAqBondObjectZSpreads` — **false positive.** This plural-named wrapper's
    own doc comment says it's the public function behind the *singular*-named
    `aqBondObjectZSpread` (already ported, in `aqBond.cpp`). The genuinely
    singular `tryAqBondObjectZSpread` is marked "Helper interface" in its own
    doc comment — an internal building block, not meant to be public.
  - `tryAqToolLVBAdd` — an in-place mutator (`void f(STDStringMatrix&, ...)`),
    doesn't fit a worksheet-function shape (nothing to return). Left unported.
  - The 7 legacy procedural SABR functions
    (`tryAqVolatilitySABR{Calibrate,GetPrem,GetVol,OutputParameter,
    SetupConvention,SetupParameter,SetupSwaptionVol}`) — **decided with
    Nicholas, 2026-09-11: leave out**, superseded by the already-ported
    object-based `tryAqVolatilityObjectSabr*` API.
- **`Generator`** — **done (2026-09-15, this session)**: `tryAqGeneratorList`/
  `Display`/`Validate` written from scratch against a directory scan of
  `resources\config\{SWAP,BOND,CURVE}_GENERATOR\`. See §1a.
- **`Model`** — confirmed **0 validation wrappers exist**. Not a gap in the
  port; there is nothing to port yet, and unlike `Generator` there is no
  legacy source or on-disk schema to introspect — it needs a design brief
  first. May stay sparse per `CLAUDE.md` §5.1.

Re-run the audit yourself any time:

```bash
python -c "
import re, glob
declared = set()
for p in glob.glob('src/validation/include/*.h'):
    for m in re.finditer(r'\btryAq[A-Za-z0-9_]*\s*\(', open(p, encoding='utf-8', errors='replace').read()):
        declared.add(m.group().rstrip('('))
xll = set()
for p in glob.glob('src/AQ_XLL/src/*.cpp'):
    for m in re.finditer(r'XLO_FUNC_START\(\s*(\w+)\(', open(p, encoding='utf-8').read()):
        xll.add(m.group(1))
missing = sorted(w for w in declared if ('aq' + w[5:]) not in xll)
print(len(declared), 'wrappers,', len(missing), 'unmatched:'); [print(' -', m) for m in missing]
"
```

---

## 3. What was ported this session (chronological, newest first)

See `rebrand\STATUS.md` for full narrative detail on each of these; summary:

-1. **(2026-09-15, latest) `Generator` category built from scratch** — see
   §1a for the summary and `rebrand\STATUS.md`'s top entry for full detail,
   including the `ThreadGuard`-nesting bug GoogleTest caught and the decision
   to leave `Model` untouched pending a design brief.
0. **(2026-09-15) Naming-convention fix — three `AQ_XLL` function
   families corrected to fit `CLAUDE.md` §5.1:**
   `aqCreditObjectBasketModel*` → `aqCreditBasketModel*`,
   `aqCreditObjectDefaultSwap<X>` (11 functions) → `aqCDSObject<X>`,
   `aqToolObjectGrid<X>` → `aqGridObject<X>` (with `ObjectNames` collapsed to
   `aqGridObjectNames`). Renamed full-stack (`validation`/`GTEST`/`AQ_API`/
   `AQ_XLL`) plus recorded fixture CSVs and `docs\api_map.csv`; the generated
   `aqManifestList.h` and the SWIG `*_wrap.*` files were deliberately left
   alone (both regenerate on their own — confirmed for `aqManifestList.h` via
   a `Release_XL_Manifest` build). Build green, GoogleTest passing
   (Nicholas-confirmed). Full detail: `rebrand\STATUS.md`'s latest entry.
1. **(2026-09-12) `InterestRate`→`IR` category rename** (full-stack:
   `validation`/`AQ_XLL`/`AQ_API`/`GTEST`, 3 fixture files); **`aqObjectDeleteAll`**
   made category-optional (blank = every object of every category, via new
   `tryAqObjectList()`/`tryAqObjectDeleteAll()` no-arg validation overloads);
   **`aqObjectClearCache`** confirmed to already delete every category's
   objects (traced to `etrading::deleteAllObjects` looping every
   `CachedObjectEnum`) and fixed to also reset `AQ_XLL`'s own handle-name
   counter map, which it wasn't. Not yet build-confirmed.
2. **Gap-closing batch (23 functions, 5 existing files, no new files):**
   `aqObject.cpp` (+2, generic Object lifecycle), `aqCurve.cpp` (+1,
   `aqCurveUSDSpotDate`), `aqTool.cpp` (+4, incl. re-fixing the
   `aqToolEchoDouble` linker gap), `aqMath.cpp` (+10, vector overloads +
   low-level ForwardRate/DiscountFactor primitives), `aqSwap.cpp` (+4, the
   multi-trade risk-ladder family — needed a new `vector<LabelValueBlock>`
   marshalling pattern, `toLabelValueBlockVector`, built from scratch this
   session).
3. **Two build-fix rounds**, both resolved:
   - 4 compile errors in `aqCurve.cpp` batch 2 (type-marshalling mismatches
     between `AQL*` types and the `aq_xll::toExcel*` overload set).
   - 1 linker error (`tryAqCurveGetInterpolationJoinDate.cpp` existed on disk,
     correct, but was never added to `projects\validation.vcxproj`).
4. **Credit category, new `aqCredit.cpp` (34 functions).** Discovered these
   wrappers live inside `tryAqSwapObjectPricing.h` (shared with the
   already-ported CMS/TRS pricing) plus two more filed under Swap's leg/
   schedule headers — golden name decided the category, not the file.
5. **Swap category, new `aqSwap.cpp` (62 functions, later 66).** Required
   first renaming the pre-existing `tryAqOisPV`/`tryAqOisParRate` wrappers
   (predated the category scheme) to `tryAqSwapOisPV`/`tryAqSwapOisParRate`
   across validation/AQ_API/GTEST/100 untracked fixture files.
6. **Curve category batch 2 (59 functions, batch 1 was 34, now 94).** The
   legacy `curveCollection`+`curveIndex` stateless family alongside the
   AQObj-handle family (Nicholas: port both), Results/Jacobian risk, dual
   bootstrap, engine calibrate, curve groups, and the four heavy one-shot
   `Calibrate*`/`ObjectCreate*` functions.
7. Earlier in this session (see `rebrand\STATUS.md` for detail): the
   `src\Core`/`src\Optional` edition-filter split (Nicholas's own commit,
   `d5dc37d7`), the `StructuredExceptionHandler` plain-English error-message
   rework, the CMS/TRS/`InterestRate` category renames, `aqBond.cpp`
   consolidating `BondOption`+`BondFutureOption`, and the initial Date/Tool/
   Bond/Math XLL fill-in.

---

## 4. Outstanding work, in priority order

### 4.1 `Model` — the only remaining category-level work

`Generator` is **done (2026-09-15)** — see §1a. `Model` still needs a design
brief (which term-structure/stochastic model types, what parameters) before
any `validation` wrapper can be written — unlike `Generator`, there is no
legacy port source and nothing on disk to introspect. Not scoped or started;
blocked on Nicholas's input, not on engineering time.

### 4.2 Phase 4a — editions & manifest gating — **done (2026-09-15)**

- [x] `AQ_XLL` file organisation: `src\Core` (always-built) vs `src\Optional`
      (one file per category) — done, committed.
- [x] `AQ_XLL` per-edition **build configurations** — `Release_XL_Bond`,
      `Release_XL_Swap`, `Release_XL_Credit`, `Release_XL_Curve` (plus the
      existing `Release` as Full) — each excludes the `src\Optional` files
      its edition doesn't need. Confirmed building green.
- [x] `AQ_API` runtime edition manifest — **decided dropped, not built**
      (Nicholas, 2026-09-15). `AQ_API` ships one full binary per language
      with every category always registered; there is no edition concept on
      that surface. `config\editions.json`/`licence.json` will not be built.

### 4.3 Phase 5 — bindings & tests

- [ ] C#, R and Java bindings unverified (Python confirmed end-to-end).
- [ ] SWIG regeneration after all the renames this session (never commit
      `swig_*_wrap.*`).
- [ ] Coverage gaps per `CLAUDE.md` §11: yield-curve + Jacobian risk vs.
      bump-and-revalue, fixed-income price/yield Bloomberg-parity regression,
      date logic.

### 4.4 Phase 6 — legacy, licence, docs

- [ ] Third-party provenance scan — **required before sale** (two undeclared
      components already found by accident this rebrand: `AQLSobol.cpp` /
      Peter Jäckel, `AQLNl2sol.cpp` / NL2SOL — both correctly attributed and
      excluded from renames, but `THIRD_PARTY_LICENSES.md` still needs
      writing).
- [ ] Extract live pieces of `calibration`/`math`/`models`, delete the rest.
- [ ] Apply the frozen per-file copyright header (`CLAUDE.md` §7).
- [ ] `RELEASE_NOTES` — every renamed/removed public function name across
      this whole rebrand (a long list by now).
- [ ] Bulk `clang-format` pass (after all renames — now much closer).
- [ ] Resources audit — generator JSON / calendar config still carry
      client-specific conventions.

### 4.5 Phase 7 — clean repo

- [ ] Linux MAKE/CMake re-validated and rebranded.
- [ ] `clang-tidy` naming guard wired into CI.
- [ ] Copy to a fresh folder, `git init`, one clean base commit, delete the
      stash repo.

---

## 5. Hazards — still outrank routine cleanup

Unchanged from before, still live:

- **Third-party provenance** (§4.4) — do not strip `AQLSobol.cpp` /
  `AQLNl2sol.cpp` attribution; scan `math`/`models`/`calibration` for more
  before sale. ~20% of the codebase is contractor-written — solicitor
  sign-off on copyright needed before sale.
- **Recorded fixture keys are stringified parameter names** —
  `rebrand\tools\fixture_key_check.py` guards this; run after any parameter
  rename.
- **`git add -A` stages generated `swig_*_wrap.*` files** — restore with
  `git checkout HEAD -- 'src/AQ_API/source/swig_*_wrap.*'`.
- **The `me`/`MA` substring hazard** — Phase 3 is done, but this applies to
  any future sweep. Anchored, case-sensitive, map-driven replacement only.
- **Validation `.cpp` files can go silently unwired from their `.vcxproj`.**
  Hit twice this session (`tryAqCurveGetInterpolationJoinDate.cpp`,
  `tryAqToolEchoDouble.cpp`), both now fixed. A repo-wide check (every
  `validation/src/*.cpp` on disk vs. every `ClCompile` in
  `validation.vcxproj`) came back clean after the second fix — but re-check
  after adding any new validation source file.

---

## 6. Build

- VS2022, toolset v143, **x64 only**, `/std:c++17`, static runtime `/MT`,
  `/MP` on every project.
- Run `SetEnvVars.bat` once, then **restart Visual Studio**.
- **Confirmed green, all configurations, GoogleTest passing.**

Known landmines and fixes: `CLAUDE.md` §3.2 (MAX_PATH under Boost `b2`, the
xlOil static-build defines, the local `AppObjects.cpp` patch that must
survive any xlOil upgrade).
