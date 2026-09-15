# AlgoQuantLib — project status

**As at 2026-09-12.** Committed HEAD is `f0dd6217` — includes `aqCurve.cpp`/
`aqInterestRate.cpp` (now `aqIR.cpp`) promoted from `src\Optional` to
`src\Core` (the dependency fix from the previous entry below). Everything
below this point since is **uncommitted working-tree delta** — Nicholas
commits at his own pace. Build is green in all configurations and GoogleTest
passes (Nicholas-confirmed multiple times, most recently after the
2026-09-15 `aqCreditBasketModel*`/`aqCDSObject*`/`aqGridObject*` naming-
convention fix described below, including a `Release_XL_Manifest` build to
confirm `aqManifestList.h` regenerates correctly with the new names).

- The **phase-by-phase plan** is `MIGRATION_PLAN.md`.
- The **detailed running record** of the rebrand (per-batch, per-decision) is
  `rebrand\STATUS.md`. This file is the summary; that file is the evidence.

---

## 1. Headline

**Phase 4 (the xlOil XLL port) is essentially complete.** Every category with
an existing `validation` surface has been ported: **466 `AQ_XLL` worksheet
functions**, covering **458 of 467 `validation` wrappers** (98%). The 9
unported wrappers are all **deliberate, decided exclusions**, not gaps — see
§2. The only real category-level work left is `Model` and `Generator`, which
have **zero validation wrappers today** — that is new design-and-build work,
not a port.

| Phase | Scope | State |
|---|---|---|
| 0 | Baseline, inventories, safety net | **done** |
| 1 | Structural: projects, folders, macros, scripts | **done** |
| 2 | Category taxonomy (21 categories, locked) | **done** |
| 3 | Identifier rebrand + calendar delimiter | **done** |
| 3c | Retire `mir*` | **done** (deleted wholesale) |
| 4 | xlOil XLL port | **~98% of existing wrappers ported.** `Model`/`Generator` need net-new `validation` wrappers first (0 exist today) |
| 4a | Editions & manifest gating | **done (2026-09-15), `AQ_XLL`-only.** `Release`/`Release_XL_Bond`/`Release_XL_Swap`/`Release_XL_Credit`/`Release_XL_Curve` build configurations shipped and building green. The `AQ_API` runtime-manifest gate is **dropped, not deferred** — `AQ_API` has no edition concept. |
| 4b | Config folder & generators audit | not started |
| 5 | Bindings (C#/Java/R) & test coverage | not started |
| 6 | Legacy extraction, resources, docs, licence | not started |
| 7 | Linux build, final sweep, clean repo | not started |

---

## 2. Have we migrated all functions? Short answer: yes, bar 9 deliberate exclusions and 2 empty categories.

A script-driven audit (every `tryAq*` name declared in `src\validation\include\*.h`
vs. every `XLO_FUNC_START` name registered in `src\AQ_XLL\src\*.cpp`, matched
by golden name) is the source of truth here — not a manual category checklist.
Current result:

- **467 validation wrappers**, **466 `AQ_XLL` functions**, **458 wrappers with
  a matching `AQ_XLL` function**.
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
- **`Model` and `Generator`** — confirmed **0 validation wrappers exist** for
  either. Not a gap in the port; there is nothing to port yet. `Generator`
  needs `tryAqGeneratorList`/`Describe`/`Validate` (etc.) written from
  scratch against a directory scan of `resources\config\{SWAP,BOND,CURVE}
  _GENERATOR\`; `Model` may stay sparse per `CLAUDE.md` §5.1.

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

0. **(2026-09-15, latest) Naming-convention fix — three `AQ_XLL` function
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

### 4.1 `Model` and `Generator` — the only remaining category-level work

Both need `validation` wrappers **designed and written from scratch**
(`CLAUDE.md` §5.1a's four-surface order still applies: `validation` → `GTEST`
→ `AQ_API` → `AQ_XLL`). `Generator` is introspection-only per §5.1
(`aqGeneratorList`/`Describe`/`Validate`); construction stays in the asset
categories. Not scoped or started.

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
- Run `SetEnvironmentVariables.bat` once, then **restart Visual Studio**.
- **Confirmed green, all configurations, GoogleTest passing.**

Known landmines and fixes: `CLAUDE.md` §3.2 (MAX_PATH under Boost `b2`, the
xlOil static-build defines, the local `AppObjects.cpp` patch that must
survive any xlOil upgrade).
