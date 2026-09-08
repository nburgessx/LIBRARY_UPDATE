# Rebrand status — pause point 2026-09-08

**HEAD: `e5c57046` ("aq_xll added function templates"). Uncommitted on top
of it — `src/AQ_XLL/src/aqObj.cpp` and `aqTools.cpp` (the `AsArray` override
+ `aqToolsCallerInfo` diagnostic below); plus the bond-port tranche (a)
below: `src/AQ_XLL/src/aqBonds.cpp`, `src/AQ_XLL/src/aqXllTools.cpp`,
`src/AQ_XLL/include/aqXllTools.h`. Not yet built or committed.** Last
GoogleTest-green tag remains `61795829`; the AQ_XLL work since has been
verified by Nicholas in Excel, not by a GTest run.

## ⇒ RESUME HERE — meLWOBond port (2026-09-08)

Porting `meLWOBond*` from `.APPLES\...\MLIBQ_ADDIN\src\meLWO.cpp` into
`src/AQ_XLL/src/aqBonds.cpp`, renamed `meLWOBond* → aqObjBonds*` (plural
`Bonds`, the locked category — NOT the literal `aqObjBond*` first asked for;
plural is what pairs with the `tryAqObjBonds*` wrappers and the
`api_pair_check` gate). Two-part plan agreed with Nicholas.

**Part (a) — DONE (uncommitted, NOT BUILT):** 11 functions added to
`aqBonds.cpp` — `aqObjBondsDisplaySchedule`, `aqObjBondsDisplayCashflows`,
`aqObjBondsDirtyPrice`, `aqObjBondsAccruedInterestDays`,
`aqObjBondsCleanPrice`, `aqObjBondsAccruedInterest`, `aqObjBondsYield`,
`aqObjBondsCompoundYield`, `aqObjBondsPrice`, `aqObjBondsDV01`,
`aqObjBondsDV01Numerical`. (`meLWOBondDisplay`/`meLWOBondCreate` were already
ported in `e5c57046` as `aqObjBondsDisplay`/`aqObjBondsCreate`.)
New marshalling helpers in `aqXllTools.{h,cpp}`: `toDoubleVector`,
`toExcelDoubleColumn`, `toExcelIntColumn`.
`aqObjBondsCompoundYield` calls the plural-spelled wrapper
`tryAqObjBondsCompoundYields` (pre-existing wrapper quirk).
No vcxproj change — `aqBonds.cpp` / `aqXllTools.cpp` already listed.
**Next: build all configs; then GoogleTest / Excel-verify.**

**Part (b) — batch b1 DONE (uncommitted, NOT BUILT).** 24 functions added to
`aqBonds.cpp`: `aqObjBondsModifiedDuration`, `aqObjBondsOisSpread`,
`aqObjBondsLastCouponDate`, `aqObjBondsPriceFromDirtyToClean`,
`aqObjBondsPriceFromCleanToDirty`, `aqObjBondsZSpreadFromRates`,
`aqObjBondsZSpread` (→ plural wrapper `tryAqObjBondsZSpreads`),
`aqObjBondsForwardPrice`, `aqObjBondsRepoRate`, `aqObjBondsRepoRateFromFuture`,
`aqObjBondsFuturePrice`, `aqObjBondsConversionFactor`, `aqObjBondsGrossBasis`,
`aqObjBondsNetBasis`, `aqObjBondsCheapestToDeliver`,
`aqObjBondsCheapestToDeliverByNetBasis`, `aqObjBondsFRNPriceFromDiscountMargin`,
`aqObjBondsFRNPriceFromYield`, `aqObjBondsFRNYieldFromPrice`,
`aqObjBondsFRNDiscountMarginFromPrice`, `aqObjBondsPriceFromCreditModel`,
`aqObjBondsBPVPerTick`, plus the two stateless `aqBondsAverageYield` /
`aqBondsYieldFromFuturePrice` (B4 decision: `aqBonds*` in `aqBonds.cpp`; they
call `validation::tryAqToolsBond*`, so `api_pair_check` will flag an
`aqBonds*`↔`tryAqToolsBond*` drift — deferred wrapper rename, same class as the
existing 5 advisories). New helper `toStringVector` in `aqXllTools.{h,cpp}`.
Whole-file param-vs-`.arg()` audit: 37/37 match (0 mismatches).
**Next: build all configs; verify in Excel.**

**Part (b) — batch b2a DONE (uncommitted, NOT BUILT).** 3 create functions added
to `aqBonds.cpp`, each with the `aqObjBondsCreate` create-vs-modify guard
(`decorateWithExcelLocation` + `allowAQObjUpdates` + `appendInstanceCounter`):
- `aqObjBondsCreateFromLVB` — keeps the legacy `VerticalLVBKeys` arg; new
  `toLabelValueBlock(obj, keysAreVertical)` overload in `aqXllTools` transposes
  the string matrix when FALSE.
- `aqObjBondsCreateFromGenerator` (legacy `meLWOBondCreateFromGeneratorLVB` — the
  `LVB` suffix is dropped to match `tryAqObjBondsCreateFromGenerator`).
- `aqObjBondsCreateAUDNotionalBond` — 10 Excel args, wrapper takes 9 (the 10th,
  `AllowUpdates`, drives the create guard only).
Whole-file audit: 40/40 param-vs-`.arg()` match. **Next: build; verify.**

**Part (b) — batch b2b DONE (uncommitted, NOT BUILT).** The 2 bond-generator
functions added to `aqBonds.cpp`:
- `aqObjBondsGeneratorCreate` — new `aqXllTools::toTableInfo(obj)` reads a range
  column-major into `tuple<COL_1.. names, Variant::getContainedTypeInfo types,
  column-major VariantMatrix of string Variants>` (the `validation::TableInfo`
  shape); `key2`/`value2` optional. No create-vs-modify guard (matches the
  source), just decorate + call + `appendInstanceCounter`.
- `aqObjBondsGeneratorDisplay` — `validation::tryAqObjBondsGeneratorDisplay`
  returns `etrading::VariantMatrix`; rendered via
  `etrading::toAQLStringMatrixFromVariantMatrix(result, false)` into the new
  `toExcelMatrix(const AQLStringMatrix&)` overload (numeric-aware, same cell
  path as `toExcelMatrix(AnyTypeMatrix)`). **Display orientation (transpose
  flag) is a guess — verify visually in Excel.**

**meLWO.cpp bond port — COMPLETE** (parts a + b1 + b2a + b2b): 42 XLO_FUNC in
`aqBonds.cpp` (2 pre-existing + 40 ported this session), whole-file
param-vs-`.arg()` audit 42/42. Remaining `meLWO.cpp`
items are the 5 `meLWOFixingTable*` (Rates category, a later `aqRates.cpp`) and
the 2 in the separate `meLWOBonds.cpp`.

**AQ_XLL comment / include cleanup (uncommitted, NOT BUILT).** Per Nicholas:
all 8 `src/AQ_XLL/{src,include}` files scrubbed of `.APPLES` paths, legacy
function/type names (`meLWO*`, `meUtility*`, `CXlOper`, `XllPlus*`,
`getDataByColumnAndDescription`, `populateExcelArrayWith*`, `MLIB_START*`,
`checkIfStaticDataLoaded`, `appendExcelLocation`, ...) and the word "legacy" in
comments; `@brief` markers removed; every `#include "x"` → `#include <x>` (all
resolve through the project `/I` dirs, so the form change is safe). No code
behaviour change. Grep for the legacy tokens over `src/AQ_XLL` now returns clean.

**Also fixed this session:** `aqObjSave` (`aqObj.cpp`) had 3 params / 4 `.arg()`
from commit `a8b50b47` — that was the "corrupt XLL / unhandled xloil::Exception"
at load, NOT the LTCG theory and NOT the bond port. Added the 4th param
`showArrayOutputs` + implemented the TRUE → 3-row column behaviour.

**Out of scope for `aqBonds.cpp`:** the 5 `meLWOFixingTable*` in `meLWO.cpp` are
the `Rates` category (`tryAqObjRatesFixingTable*` wrappers) — a later `aqRates.cpp`.
`meLWOBondQuote` / `meLWOBondForwardReinvestedCoupon` live in `meLWOBonds.cpp`,
not `meLWO.cpp`.

Two workstreams are open in parallel:
- **A. AQ_XLL xlOil port (active this session)** — see "⇒ RESUME HERE — AQ_XLL
  port" immediately below.
- **B. validation recording rollout (plan 4.9)** — see "⇒ RESUME HERE —
  recording rollout" further down. Untouched this session.

---

## ⇒ RESUME HERE — AQ_XLL port (2026-09-07)

**What works in Excel now (Nicholas-verified):**
- `aqToolsInitialize` — loads config, prints the resolved `Calendar.csv` path.
  Library auto-inits in the `AlgoQuantLib` add-in constructor (xlAutoOpen);
  `AQ_INITIALIZE` macro is the per-function lazy guard.
- `aqDatesFromTenor` / `aqDatesFromYearFraction` — calendars usable.
- `aqObjBondsCreate` / `aqObjBondsDisplay` (new `aqBonds.cpp`).
- `aqObjExists` / `aqObjLoad` / `aqObjSave` (new `aqObj.cpp`).
- `aqToolsResize` — positional clip/pad reshape (respects source dims;
  pad blanks / truncate). Row-major reflow bug fixed.
- Numeric-string cells in `Display`/matrix output now returned as real numbers
  (formattable), mirroring the legacy `canStringConvertToNumber` path.

**OPEN — blocking the multi-row `aqObjSave` output:**
`AQ_IS_ARRAY_OUTPUT` (macro → `aq_xll::isArrayOutput()` → `callerRangeSize()`
via raw `xlfCaller`, handles SRef + Ref) returns false even under
Ctrl+Shift+Enter on Nicholas's Excel, so `aqObjSave` returns the scalar and
Excel repeats it across the selection. Likely cause: modern Excel intercepts
CSE and never reports the array range to the add-in, so Enter-vs-CSE cannot be
distinguished at all.
- **Mitigation already coded (uncommitted):** `aqObjSave` 4th optional arg
  `AsArray` — TRUE forces the 3-row column, FALSE forces the message, omitted =
  auto. Plus `aqToolsCallerInfo()` diagnostic returning `"<r> x <c> (array=<0|1>)"`.
- **Next step:** build; run `=aqToolsCallerInfo()` plain vs CSE-over-3-cells.
  - both `1 x 1` → auto-detect is impossible on this Excel; keep `AsArray`,
    strip the auto-detect from other multi-output funcs (don't fake it).
  - CSE shows `3 x 1` → detection works, dig into why `aqObjSave` still
    scalars (array build path itself is proven — bond display uses the same
    `ExcelArrayBuilder` + `returnValue` and works).

**Then continue the port in the agreed order** — Dates → Tools → Curves →
Swaps → Products → Models. `rebrand/xll_function_inventory.csv` (653 rows) is
the driver; decision columns (`proposed_new_category`, `keep_drop_merge`,
new name) still need filling per file, starting with `meDates` (36 fns).
`~460` me*/meLWO* functions in the etrading filter to port; `LAXL.cpp` (161
`mir*`) deprecate-or-`aql*`; `msc*`/client (~29) delete.

**Carry forward (AQ_XLL specifics):**
- `AQ_API` has NO auto-init — bindings callers must call `setUpAQL()` /
  `setupAQL()` / `initAQL()` (all → `validation::trySetupAQL`). Only Python
  verified. `AQ_API_START` is still a bare `try{}` (plan 4.12).
- `aqObjSave` deliberately no longer upper-cases the file path (legacy did;
  breaks case-sensitive FS / Linux). Name + type still upper-cased.
- Naming calls made this session, matching the `tryAq*` wrappers + pairing
  check, NOT the literal ask: `Bonds` (plural, not `Bond`), `aqObj*` (not
  `aqObject*`). Nicholas has accepted these so far.
- New marshalling helpers in `aqXllTools.{h,cpp}`: `toBool`,
  `toAQLStringMatrix`, `toLabelValueBlock`, `toExcelMatrix` (numeric-aware),
  `toExcelColumn` (no pad — an N×1 must stay N×1), `reshapeToSize`,
  `decorateWithExcelLocation`, `callerRangeSize`, `isArrayOutput`.
- Dead XLL+ include/lib paths still in `projects/AQ_XLL.vcxproj` — remove when
  next touching it (plan carry-over).

---

## ⇒ RESUME HERE — recording rollout

**1. Continue the validation recording rollout (plan 4.9).** Pass (a) is
done: 32 conversions in 6 files. What remains, in the order I would do it:

- **Fix the misleading comment in `RecordMacros.h`** (cheap, do it first). Its
  worked example shows a `decorateFilename(...)` block being replaced by plain
  `AQ_RECORD_INPUTS`. That is WRONG — `AQ_RECORD_INPUTS` does not decorate, it
  always yields `<function>_inputs`, so following the comment silently renames
  the fixture file and breaks the test at run time with a green build.
- **99 decorated blocks** → `AQ_RECORD_DECORATED_INPUTS`. First confirm how that
  macro maps prefix/suffix onto `decorateFilename(name, prefix)`.
- **81 variable-filename blocks** — need reading individually.
- **18 key≠parameter-name recordings** (pass b) — listed in plan 4.9. Converting
  these mechanically WOULD break fixtures; rename the parameter to match the key
  instead, then re-run `fixture_key_check.py`.
- **Functions with no recording at all** (pass c) — largest, needs new fixtures.

**⚠ Two hard rules for the converter tool, learned the expensive way:**
1. Verify the replaced span is exactly the `if (recordEnabled()) { ... }` block
   and nothing more. One bad boundary ate a live statement
   (`double result = ...` became `double rAQ_RECORD_OUTPUTS( ... );`).
2. Verify the file includes `RecordMacros.h` before rewriting it. Three files
   had only hand-rolled recording and so never included it.

**2. Then the other open Phase 4 items:** 4.12 (SEH for `AQ_API` —
`AQ_API_START` is a bare `try{}`, so the bindings' own marshalling can still
kill the host process), 4.10 (`AQ_XLL_GUARD` on every new XLL function), and
the next tranche of XLL functions.

## Where the XLL port stands

As of 2026-09-07 (`e5c57046` + 2 uncommitted files) `src/AQ_XLL` is:

    include/  aqMain.h   aqXllTools.h
    src/      aqBonds.cpp  aqDates.cpp  aqMain.cpp  aqMath.cpp
              aqObj.cpp    aqTools.cpp  aqXllTools.cpp

Ported and working in Excel: `aqToolsInitialize`, `aqToolsResize`,
`aqToolsCallerInfo` (diag), `aqDatesFromTenor`, `aqDatesFromYearFraction`,
`aqObjBondsCreate`, `aqObjBondsDisplay`, `aqObjExists`, `aqObjLoad`,
`aqObjSave` (multi-row output still blocked — see RESUME HERE — AQ_XLL port).

Convention (plan 4.11): every file is `aq<Category>.{cpp,h}`; the single
exception is `aqXllTools.{h,cpp}`, the shared Excel-side helper library
(marshalling + AQObj handle decoration). `aqTools.cpp` is the Tools *category*;
`aqObj.cpp` holds the `aqObj<Lifecycle>` functions (no category word).

`XllPlusTips*.cpp` were NOT copied — 4,629 lines built on XLL+ types. Behaviour
was ported instead. The AQObj handle logic (instance counter, cell address,
create-vs-modify guard) is in `aqXllTools`; the core framework was already in
`src/etrading`.

SEH: `TestStructuredExceptionHandler` gives it automated coverage for the first
time. **If the translator ever stops working those tests do not fail, they
CRASH the runner** — a vanished test run is the signal.

## Phase 3 — complete

`src/` and `projects/` contain zero `Mizuho`, `MLIB`, `me*`, `mir*`,
`LA*`/`MA*`/`MB*`, `validation_api`, `XllPlus`, person names, or copyright
banners. `resources/` reduced to `config`, `test`, `setup` (3,522 files deleted);
legacy-named paths there fell from 1,310 to 126.

## ⚠ Carry these forward — they outrank routine cleanup

1. **Two undeclared third-party components in `src/math`**, both found by
   accident: `AQLSobol.cpp` (Peter Jaeckel — notice has a PRESERVATION CLAUSE)
   and `AQLNl2sol.cpp` (NL2SOL, 6,541 lines — Dennis/Welsch/Gay/Peters,
   NSF-funded, ACM TOMS). **Excluded by name from every sweep. Never strip their
   attribution.** Neither is in the CLAUDE.md §3.3 dependency table. The Phase 6
   provenance scan is REQUIRED BEFORE SALE.
2. **Recorded fixture keys are stringified C++ parameter names.** Renaming a
   validation parameter breaks tests silently, build still green.
   `fixture_key_check.py` guards it.
3. **Pre-existing bug:** `tryAqObjRatesFixingTable.cpp` writes
   `file.write("fixingValues", fixingDates)` and
   `file.write("fixingDates", fixingValues)` — the keys are swapped. Fix code
   and rebase those fixtures TOGETHER, never separately.
4. **`git add -A` stages the generated `swig_*_wrap.*` files.** Restore with
   `git checkout HEAD -- 'src/AQ_API/source/swig_*_wrap.*'`; never commit them.
5. **Exact-match auditing under-reports** — five times a "clean" result was
   defeated by a spelling variant. Re-scan loosely after any sweep reports zero.
6. **No test exercises `tryAqToolsReplay`** — untested public API, Phase 5 gap.

---

## Done this run (newest first)

| commit | what |
|---|---|
| _(uncommitted)_ | **Step 11 — category naming scheme + AQ_XLL rebrand.** Categories now SINGULAR; handle marker moved from `aqObj<Category>` prefix to `aq<Category>Object<Fn>`; named sub-objects skip `Object` (`aqBondCurveYield`); lifecycle `aqObject<Lifecycle>`; `Vols`→`Vol`. Docs: `CLAUDE.md` (both), `MIGRATION_PLAN.md` §2.2/§2.5/§2.6/D14/D15/D16/D18/Phase 3.2/3.5/4.7/4.11, `rebrand/STATUS.md`, `rebrand/tools/api_pair_check.py` (CATEGORIES + `category()`). **AQ_XLL code:** `git mv` `aqBonds/aqDates/aqObj/aqTools.cpp` → `aqBond/aqDate/aqObject/aqTool.cpp`; 53 functions renamed (42 bond + 4 date + 3 object + 4 tool); comments + `AQ_XLL.vcxproj`/`.filters` updated. Param/`.arg()` audit `aqBond.cpp` 42/42. `aqMath.cpp` / `aqMain.cpp` / `aqXllTools.{h,cpp}` unchanged names (Math already singular; XllTools is a utility, not a category). **`validation::tryAqObj…` call sites in AQ_XLL still use the OLD wrapper names** — temporary desync until MIGRATION_PLAN 2.6 renames `validation` / `AQ_API` / `GTEST` test names / fixtures. **NOT BUILT.** |
| _(uncommitted)_ | **AQ_XLL fails to load in Excel — ROOT CAUSE FOUND + FIXED.** `xlAutoOpen` threw `xloil::Exception<std::runtime_error>` ("unhandled C++ exception in Excel.EXE"). Cause: `src/AQ_XLL/src/aqObj.cpp` `aqObjSave` had **3 parameters but 4 `.arg()` descriptions** (the `ShowArrayOutputs` arg text was added in commit `a8b50b47` "AqObjSave update" without the matching 4th parameter). xlOil's `.arg(i>=nParams)` throws "Too many args for function" at registration. Introduced by `a8b50b47`, which post-dates the last Excel-verified commit `e5c57046`, so it was never caught. Fix: added the 4th param `showArrayOutputs` and implemented the documented TRUE → 3-row `{result, filePath, name}` column / else → name behaviour (uses existing `toBool` + `toExcelColumn`). **Not related to LTCG or to the tranche-(a) bond port** (all 11 new funcs audited: param/arg counts match). The earlier `AQ_XLL.vcxproj` LTCG edit was reverted — project settings unchanged from HEAD. `targets/64/{Release,Debug,ReleaseProfiler}/AQ_XLL` were deleted (build output only). **Next: rebuild AQ_XLL, verify it loads, then verify tranche (a).** |
| _(uncommitted)_ | **meLWOBond port tranche (a)** — 11 `meLWOBond*` functions ported from `.APPLES\...\meLWO.cpp` into `src/AQ_XLL/src/aqBonds.cpp` as `aqObjBonds*` (DisplaySchedule, DisplayCashflows, DirtyPrice, AccruedInterestDays, CleanPrice, AccruedInterest, Yield, CompoundYield, Price, DV01, DV01Numerical). New helpers `toDoubleVector` / `toExcelDoubleColumn` / `toExcelIntColumn` in `aqXllTools.{h,cpp}`. CLAUDE.md §9 gains an "always update STATUS.md" rule. **Not built yet.** |
| _(uncommitted)_ | `aqObjSave` gains optional `AsArray` arg (force column / force message / auto); new `aqToolsCallerInfo()` diagnostic. Mitigation for the CSE-detection problem. **Not built yet.** |
| `e5c57046` | **AQ_XLL port tranche 1** — `aqMain.cpp` auto-inits the library in the add-in ctor; `AQ_INITIALIZE` + `AQ_IS_ARRAY_OUTPUT` macros; new `aqBonds.cpp` (`aqObjBondsCreate/Display`), `aqObj.cpp` (`aqObjExists/Load/Save`); `aqToolsInitialize`, `aqToolsResize` (positional clip/pad), path slash-normalisation; marshalling helpers in `aqXllTools` (`toBool`, `toAQLStringMatrix`, `toLabelValueBlock`, `toExcelMatrix` numeric-aware, `toExcelColumn`, `reshapeToSize`, `callerRangeSize`, `isArrayOutput`). vcxproj/.filters updated. Verified in Excel by Nicholas, not by GTest. |
| `68ca1ceb` / `bf2cb765` | earlier AQ_XLL updates + step-2 build fixes (RecordMacros `AQLString`). |
| `2d7eb95c` | **Phase 3 step 5** — calendar holiday-centre delimiter `:` → `+`. New `CALENDAR_CENTRE_DELIMITER`/`splitCalendarCentres()` in `math/AQLPriceDataCalendar.{h,cpp}`; routed ~18 parse/build sites (convertFrom/ToString, 16 curve-helper `toToken(':')`, NYB-strip). Input still accepts `:`; output always `+`. New test `Calendars.UNIT_CentreDelimiter_PlusAndColonEquivalent`. |
| `530e764e` | **B-fixtures** — 7,960 `resources/test/inputs` fixture paths + 152 src files (TEST_DIR strings, `generatorFunction` labels, `decorateCurvename/Filename` prefixes, `UNIT_TestMe*` labels, comments) renamed `me/tryMe → aq/tryAq`. `me/tryMeUtilityClearLWOCache → aq/tryAqObjClearCache`. |
| `756d9bb8` | **B-LWO** — `meLWO*/tryMeLWO* → aqObjects*/tryAqObjects*` (496 ids, 85 files). Map A "by product" rejected (21 collisions with the stateless names); Map B `LWO→Objects` adopted. Code only. |
| `c9bb9f49` | **B-simple** — 442 `me*/tryMe*` (non-LWO) → `aq*/tryAq*` code identifiers (247 files). Family map in `rebrand/phase3_Bsimple_MAP.csv`. Code only (strings/resources were deferred to B-fixtures). |
| `649d3aa5` | **B-delete** — removed 6 orphaned securitisation validation files (`tryMe{CashflowClient,LoanCalculations,SupervisoryRules}`, incl. `tryMeSRT*`). Were not in any vcxproj. |
| `700f5722` | `LAString`/`LADate` embedded in 64 identifiers (`getCompulsoryValueAsLAString`, `getLADate`, `TestLAString`, …) → `AQLString`/`AQLDate`. 943 refs. |
| `a8905ca5` | `boost::swap` → `std::swap` in `StatisticsUtilities` (kills the deprecated-header warning). |
| `0bab362e` | `InitializeAQETrading → InitializeETrading`, `InitializeAQGoogleTest → InitializeGoogleTest`. |
| `91a0ad38` / `13ad0c2e` | **Phase 3.1c (Classic)** — `MA/MM/MV/MD/MF/ME_ → AQL` in math/models/calibration (431 renames). `MLIB_2D_MATRIX_CHECK → AQ_2D_MATRIX_CHECK`. Census tool widened. |
| earlier | Phase 3.1 math, 3.2 models, 3.2 etrading (`LA→AQL`, `LWO→AQObj`), Initialize renames, `validation_api→validation`, `MLIB_*→AQ_*`, project renames `AQ_BINDINGS→AQ_API` / `GOOGLE_TEST→GTEST`, solution `AlgoQuantLib-VS22.sln`, solution folders `AQ - Core` / `AQL - Classic`. |

**Group B (`me→aq`) is COMPLETE** — no `me*/tryMe*/meLWO*` names anywhere in `src/` or `resources/test/`.

---

## Pending — Phase 3

### Step 7a — legacy include guards  ← DONE (`4fb2fdb6`)
15 `__LAMATH*_H__` guards in `src/models/include/` → `__AQLMATH*_H__`. Guard-token
only, no build impact.

### Step 7b/7c — LWO + MLIB sweep  ← DONE (`ab35bf0b`, `8e632c65`)
7b: 299 bare LWO->AQObj (comments + 9 message strings), 117 plain-English
supplements, 28 MLIB test labels -> AQ, deleted stale AQ_API README.txt.
7c: 128 embedded LWO/lwo identifiers, 34 TestLWO*.cpp renamed, the
ETrading/LWObjects fixture tree (1108 files) renamed in lockstep with its code
path-strings, 200 vcxproj/.filters refs.
Only `lwoer` (a typo for 'lower' in a comment, math/src/AQLFunctionUtilities.cpp)
is left under src/ -- deliberately untouched. Remaining LWO lives in
resources/{documentation,examples} -> Phase 6 audit.

### OLD 7b note (superseded)
~70 `MLIB` GTest test-case labels (`UNIT_TestMacro_MLIB_THROW`) + ~296 `LWO`
prose/error-string mentions (`"LWO Curve … does not exist"` → `AQObj`/`Object`? — a
naming call). `validation_api` / `MLIB_CLIENT_API` / `GOOGLE_TEST` residue is
**Linux-Makefile only → Phase 7**. `XllPlus` → Phase 4.

### Step 6 — mir removal  ← DONE (6a `97c8461e`, 6b `5a29a51a`)
mir stack deleted. Residual `mir` only in strings/comments (Replay.cpp dispatch
keys `functionList["tryMirSetUpOISCurve"]`, a few error messages) -> fold into 7b.
`TestMirDateFunctions.cpp` kept (no mir) -> rename to a non-legacy name in 7b.

### Step 6 (old plan text below, superseded)
**6a DONE (`97c8461e`)**: 5 GTEST curve-fixture builders (`CurveOis/Std/TenorBasis/
XccyBasis/FwdFxConst.cpp`) now call `etrading::AQLUpdateStaticDataManager::setUp*
Curve` directly. Needs build + GTest.

**6b TODO** (bigger than first scoped):
  1. Port `src/GTEST/src/InterestRateSwap.cpp` — shared helper used by ~8 keeper
     Trade tests; its `parRate()/pv()/pv01()` call `tryMirGetParRate4` (→ schedule
     gen + `etrading::AQLCurveForwardRateHelpers::getParRate`), `tryMirSwapPV`,
     `tryMirSwapPV01`. Not trivial passthroughs — read the 3 wrappers, reproduce inline.
  2. Gut `TestCurveEngineCalibrate.cpp` — drop the DEAD `testEngineCurveForwardRates`
     method (all call sites already commented out) + `#include "tryMirGetForwardRate.h"`
     + `#include "CurveOis.h"`.
  3. `git rm`: `src/AQ_API/source/mir*.{h,cpp}` (58, nothing else references), 
     `src/validation/{include,src}/tryMir*.{h,cpp}` (65), and 36 mir-only GTEST .cpp
     (`InterestRateSwap.cpp`? NO — that's the ported helper; its `.h` in GTEST/include
     is shared, keep. The 36 are `TestCurve{Ois,Std,TenorBasis,XccyBasis,FwdFxConst}*`
     (test suites, not the builders), `TestCurveReplay`, `TestMirDateFunctions`,
     `TestDatesECB`, `TestTradeEUR{AssetSwapSpread,OISParRate,SwapStubRate}`,
     `CurveConsistencies_{OIS,STD,TenorBasis}`, `LinearSplineTests`, `Test_IsFwdInter`,
     `InterestRateSwap.cpp`→NO). 
     Recount: exclude the 5 kept builders and InterestRateSwap → ~35 test .cpp.
  4. Edit `swig_{CSharp,JAVA,Python,R}.i` — drop `%include`/`#include "mir*.h"` lines
     (keep `EntityPoolUtilities.h`).
  5. Edit `apiTryAqToolsLVB.h` — drop dead `#include "MBTemplateType.h"`.
  6. Edit `BindFileToClassConstructor.h:20` — stale `_tryMirSetUpOISCurve_inputs`
     comment example.
  7. vcxproj/.filters: strip mir + deleted-test entries from AQ_API / validation / GTEST.
  8. **KEEP all `resources/test/inputs/` fixture files** — keeper tests read the
     `*_tryMirSetUp*_inputs.csv` fixtures via the ported builders; orphan CSVs
     (dirs of deleted tests) → Phase 6 resources audit.
  9. `EntityPoolUtilities` + `tryAqToolsClearEntityPool` — CORE, keep.

### OLD step-7 note (broad attempt, reverted)
A quick script anchored on `#ifndef`/`#define` lines mangled **non-guard conditional-define macros**
(`IR_CALIBRATION_DATA_MAXLOOP → …AQXLOOP`, `MARKETTYPE → AQRKETTYPE`, `LEVENBERG_MARQUARDT → …AQRQUARDT`,
`MAXTERM → AQXTERM`) — the classic `MA`-in-English hazard. Fully reverted; nothing committed.

**Redo approach:** only rewrite guards that are genuinely legacy-prefixed *header guards*, i.e.
`__?(LA|LB)[A-Z]` or `__?LAMATH…_H__` style, verified as the file's `#ifndef X … #define X … #endif`
triple (X identical on all three), with the English-word filter (`MAX/MARKET/MARQUARDT/MANAGER/…`).
Real targets found: **~17 files in `src/models/include/`** with `__LAMATH…_H__` guards
(`AQLMathSABR.h`, `AQLMathJumpDiffusion.h`, …). `AQLMathSwaptionFormulaSZ.h` has a UTF-8 BOM
before `#ifndef` — a naive `^#` regex skips its `#ifndef` line (must use `utf-8-sig` / match after BOM).
Also `MARKETTYPE_SWAP/BASIS/ZERORATE`, `MANAGER` in `calibration/include/AQLDefinitions.h` /
`models/include/ConstantDeclarations.h` are English string-constant macros — **leave**.

Comment sweep (`LWO`/`mir` prose, stale fixture-format comments) — lower priority, do after guards.

### Step 6 — delete the `mir` stack  ← DEFERRED (needs helper port first)
Not cleanly deletable: `CurveOis.{h,cpp}` / `CurveStd` / `CurveTenorBasis` / `CurveXccyBasis` are
**shared GTest curve-fixture builders** used by ~15 non-mir pricing/risk/credit tests
(`TestConstantMaturitySwap`, `TestCreditBasketModel`, `TestTotalReturnSwap`, `TestRiskSwapDeltaLadder`,
`TestTrade{GBP,JPY,USD}SwapParRate`, …) and they build curves via `tryMirSetUp*Curve`.
60 GTest files touch the mir surface.

**To do it:** (1) port those 4 fixture builders to `tryAqObjectsCurveCreate*` / `tryAqCurvesCalibrate*`;
(2) delete `mir` impl (58 `AQ_API/mir*` + 65 `validation/tryMir*`), ~40 mir-only GTest files
(`TestCurve{Ois,Std,TenorBasis,XccyBasis,FwdFxConst}*`, `TestCurveReplay`, `TestMirDateFunctions`,
`TestDatesECB`, `TestTradeEUR*`, `InterestRateSwap`, `CurveConsistencies_*`, `LinearSplineTests`,
`Test_IsFwdInter`), 695 `resources` mir fixtures, the 58 `%include "mir*.h"` lines in the 4 SWIG `.i`
files, and all vcxproj/.filters entries.
User decision recorded: **losing the mir-based regression tests is acceptable — the aq/me suite is the replacement.**
`EntityPoolUtilities.{h,cpp}` is CORE (widely used) — **keep**; `tryAqToolsClearEntityPool()` is its wrapper.
`tryMirClearEntityPool` (mir duplicate) goes with the rest.

### Step 8 — category folder layout + `docs/api_map.csv`  ← DEFERRED
Folder the `validation` `tryAq*` wrappers by category (`src/validation/Curves/`, …), generate
`docs/api_map.csv` (`public | category | validation wrapper | etrading entry | test`), add the
`aq* ↔ tryAq*` CI check. Cleaner to do after the mir delete shrinks `validation`.

### Also outstanding in Phase 3
- `MBRoot.h` / `MBTemplateType.h` — **dead `#include`s that reference non-existent files** in
  `validation/include/apiTryAqToolsLVB.h:14` and `validation/include/tryMirSwapDuration.h:14-15`.
  `tryMirSwapDuration.h` goes with the mir delete. `apiTryAqToolsLVB.h` needs its dead include removed
  (it currently only survives because nothing compiled includes it).
- Guard-underscore normalisation (step 7 targets above).

---

## Public API naming scheme — CURRENT (step 11, 2026-09-08)

Categories are **SINGULAR**. The handle API is the word `Object` after the
category; a named sub-object (Curve, Generator, MarketData, Model, FixingTable)
already denotes an object and does not repeat it.

| Form | Meaning | Example |
|---|---|---|
| `aq<Category><Function>` | stateless — data in, value out | `aqBondScheduleKeys` |
| `aq<Category>Object<Function>` | handle API on the category's product | `aqBondObjectDirtyPrice`, `aqSwapObjectParRate` |
| `aq<Category><SubObject><Function>` | handle API on a named sub-object | `aqBondCurveYield`, `aqBondGeneratorCreate` |
| `aqObject<Lifecycle>` | generic handle lifecycle | `aqObjectLoad`, `aqObjectSave`, `aqObjectClearCache` |
| `AQObj*` / `AQOBJ_*` | internal C++ classes / macros (unchanged) | `AQObjCurve`, `AQOBJ_KEY` |

`validation` (`try` + identical name), `AQ_API` and `GTEST` **test names** all
use these forms. Do not use the earlier `aqObjects*` or `aqObj<Category>*`
prefixes or plural categories anywhere.

**Applied so far:** `AQ_XLL` only (step 11, this session — file renames
`aqBonds/aqDates/aqObj/aqTools.cpp` → `aqBond/aqDate/aqObject/aqTool.cpp`, 53
functions renamed, `.vcxproj`/`.filters` updated). Its `validation::tryAqObj…`
call sites still use the OLD wrapper names — `validation` / `AQ_API` / `GTEST` /
`resources\test` fixtures are the pending sync (MIGRATION_PLAN task 2.6, its own
branch + green-diff gate).

### History (commits — descriptions only, do not follow the old scheme)
- **step 8** `7fd00c7c` / `30fc5505` — introduced `aqObjects*` → `aqObj<Category>*`
  (774 renames, 7,300 fixtures) + internal `AQO`→`AQObj`. Superseded by step 11.
- **step 9** `db980842` — Options umbrella removed, sub-types promoted. Still holds.
- **step 11** (this session) — plural → singular, `aqObj<Cat>` → `aq<Cat>Object`,
  `Vols`→`Vol`. AQ_XLL done; rest pending.

### Two exclusions that must survive any future AQO/AQObj pass
`AQObjects` (fixture path segment), `IsAQObject`, `isAQObject` — these already
contain `AQObject`; renaming `AQO`→`AQObj` inside them yields `AQObjbject(s)`.

### Open, from the api_pair_check advisory
Five public functions whose wrapper name has drifted from `tryAq<same name>`.
All five *do* route through `validation` (the 4.1 contract holds) — this is
cosmetic naming only, deferred:
```
aqCurvesResultsDisplayDiscountFactors          -> tryAqCurvesResultsDiscountFactorsDisplayAll
aqObjCreditModelRiskyDiscountFactor            -> tryAqObjCreditModelRiskyDiscountFactors
aqObjCurvesMarketDataCreateUsingMultipleBlocks -> tryAqObjCurvesMarketDataCreate
aqObjSwapsUSDSpotDate                          -> tryAqCurvesUSDSpotDate   (also: a Swaps
                                                  function living in aqDates.cpp)
aqToolsLVBAppendAndCreate                      -> tryAqToolsLVBAdd
```
354 validation wrappers still have no public `aq*` function. That is the Phase 4
port backlog, not a defect.

### Repo-wide CLAUDE.md — synced
`REPO\CLAUDE.md` (one level above `LIBRARY_UPDATE`, outside the git repo) was
updated by hand once Nicholas granted write access: the category list (`Objects`
dropped, `FX` added), the `aq`/`aqObj` table, the `LWO -> AQObj` rows, the
`AQObjbject` exclusion rule, the architecture diagram, and a rewritten section
2.1 that now points here as the live running record instead of claiming the
code-level rebrand has not started. It is **not** version-controlled -- a
pre-edit copy is in the session scratchpad only, so treat further edits to it
with care.

---

## Step 9 — Options umbrella removed, sub-types promoted (done, 2026-09-04)

`db980842`. From Nicholas's reviewed map (column "use" of the returned
`phase3_step8_MAP.csv`). Rule: **a product sub-type is its own top-level
category**, not a member of an umbrella.

| Umbrella dropped | Promoted to categories |
|---|---|
| `Options` (removed entirely) | `CapFloor` 20, `Swaption` 17, `BondOption` 9, `BondFutureOption` 4 |
| `Swaps` (kept, narrowed) | `AssetSwap` 12, `TotalReturnSwap` 24, `ConstantMaturitySwap` 8 |
| `Curves` (kept, narrowed) | `Inflation` 15 |

113 renames, 0 collisions, 10 src files + 69 fixtures moved in lockstep.
**Category list is now LOCKED (20)** — updated in `CLAUDE.md`, `REPO\CLAUDE.md`,
`MIGRATION_PLAN.md` §2.2 and `rebrand/tools/api_pair_check.py`. `Swaps` now means
the vanilla swap plus legs and schedules. Stateless surface untouched (it never
had an `Options` category and already used `aqAssetSwap*`).

### Note on the map Nicholas returned
Its column C matched the committed map exactly on all 499 rows, but the file was
**truncated mid-alphabet** at `tryAqObjectsFixingTableValues_inputs` — 275 of 774
rows absent, including every `tryAqObj` family from `Fra` onward and all 53
stateless rows. The rule behind the 63 overrides was inferred, checked against
those overrides (61/63 reproduced; the 2 misses were a copy slip and a typo fix,
both confirmed by Nicholas), and then applied to the rows the file never reached.

### Typo fixes — exact tokens only
`DDeltaLadder->DeltaLadder`, `Implived->Implied`, `Mutiple->Multiple`,
`Asof->AsOf` (2). Applied as **exact whole tokens**: a substring sweep of `Asof`
would have hit `AsofDate` / `getAsofDate` / `mAsofDate` (which collide with
existing `AsOfDate` / `getAsOfDate` spellings — a separate, pre-existing internal
inconsistency), and a sweep of `Displaye` would have turned `msgDisplayed` into
`msgDisplayd`.

### Resolved: the Displaye_outputs recording name
`tryAqObjRatesFixingTable.cpp:165` now writes `tryAqObjRatesFixingTableDisplay_outputs`,
matching line 145. Nicholas confirmed this is safe: the two are the success and
error branches of one `if`, so only one of them runs per call and only one file
is written per invocation.

### Still open from step 8 (unchanged)
The 5 wrapper-name-drift advisories from `api_pair_check.py`. All route through
`validation` correctly; cosmetic only.

---

## Step 10 — MLIB removal (10a done, 2026-09-04)

`f9460bb8` (rename) + `23a83f3b` (the config-path fix it exposed). Build + GTest green.

  MLIBQ env var  -> AQ        setUpMLIB    -> setUpAQL      MLIBID  -> AQLID
  MLIB (prose)   -> AQ        trySetupMLIB -> trySetupAQL   -mlibid -> -aqlid
  MLIB_CLIENT_API -> AQ_API (Linux makefiles)
  deleted projects/BondUtilities.dll (prebuilt binary, unreferenced, wrong folder)

All three setUp aliases were kept and renamed, not collapsed -- Nicholas: they
are legacy methods marked for deprecation, not a new public surface.

### !! The library was loading its config from the READ-ONLY .APPLES tree !!
The single most important finding of this session. `MLIBQ` was still set on the
dev machine to `REPO\.APPLES\Apple`, and `FolderConfig` looked up
`toPath("MLIBQ", "/resource/config/...")` -- **`resource` singular, the .APPLES
layout**. So AQ_LIB, and the whole GoogleTest suite, silently read `Calendar.csv`,
`CBSchedule.csv`, `ir.properties` and `irsvr_excel.conf` out of the old client
tree. Every green test run before `23a83f3b` was reading config from `.APPLES`.

Switching the variable to `AQ` without correcting the suffix pointed the lookups
at `$(AQ)/resource/config`, which does not exist (AQ_LIB uses `resources`), so
every tier fell through, the calendar set loaded empty, and most tests failed
with "Unable to load calendar - nyb/tgt/lnb/tkb".

Fixed by correcting the suffix in both places:
  FolderConfig.cpp        /resource/config/  -> /resources/config/   (7 sites)
  AQLCurveProperties.cpp  \calendar.csv      -> 
esources\config\Calendar.csv (+2)

Behaviour-preserving: AQ_LIB
esources\config holds every file .APPLES had and
its Calendar.csv is byte-identical. **AQ_LIB no longer depends on .APPLES at
runtime, and MLIBQ can be unset.**

**Lesson for the remaining phases:** a passing test suite did not prove the tree
was self-contained. Before the clean-repo cut (Phase 7), build and run the tests
on a machine with no legacy environment variables set, from a clean checkout, to
flush out any other hidden dependency on `.APPLES` or `.ALGO_QUANT_LIB`.

### Step 10b -- pending sign-off
~133 mixed-case `Mlib`/`MLib`/`mlib` identifiers in 31 files that the upper-case
map could not reach. Map drafted, 0 collisions. The anchoring protects the 4,260
`Libor`-family tokens (`threeMLibor`, `sixMLibor`, `6mlibor`, `mLiborRateMap`) --
a case-insensitive `mlib` sweep would destroy them, the same trap as `me` in `name`.

---

## !! Recorded-fixture keys are stringified C++ parameter names !!

`RecordMacros.h` expands `WRITE_PARAMETER(P)` to `file.write("P", P)`. The first
column of every `*_inputs.csv` is therefore **literally the C++ parameter
identifier as spelled when the fixture was recorded**. Renaming a validation
function's parameter breaks the read path *silently*: the build stays green and
the test fails at run time with

    ReadDataFile::Load: unknown key: <newName>

This bit us with `lwoCurveGeneratorName` / `lwoCurveMarketDataName`, which
survived TWO renames (`lwo`->`aqo` in step 7c, `aqo`->`aqObj` in step 8B)
because every batch renamed code and fixture FILE NAMES but never fixture
CONTENT. 70 keys in 35 files, failing 16 Credit / CMS / DiscountFactorsWithSpread
tests. Fixed by rewriting the keys to the current parameter names.

**Guard added: `rebrand/tools/fixture_key_check.py`** -- run it after any rename
that touches validation parameters; exit 1 if a fixture key carries a legacy
prefix. Currently 0.

It also reports, as advisory, **5,070 `generatorFunction` rows naming 133 legacy
functions** (`tryMeLWOBondCreate`, `tryMirSetUpOISCurve`, ...). That field is
provenance, not read by the loader, so it breaks nothing -- but it is legacy
branding inside shipped test data and belongs to the Phase 6 resources sweep.
The phase-3 maps already hold the translations.

---

## !! THIRD-PARTY CODE FOUND IN src/math -- licence obligation !!

**TWO components so far, both undeclared. Both are EXCLUDED from every comment,
banner and name sweep. Do not strip their attribution.**

`src/math/src/AQLSobol.cpp` is **not ours**. It carries:

    2002 Peter Jkel "Monte Carlo Methods in Finance".
    All rights reserved.
    Permission to use, copy, modify, and distribute this software is freely
    granted, provided that this notice is preserved.

**"provided that this notice is preserved"** is a licence condition. The notice
must stay in the file, and it is deliberately EXCLUDED from every banner /
copyright sweep. Do not remove it.

Two follow-ups for Phase 6:
1. Add Peter Jaeckel / "Monte Carlo Methods in Finance" (2002) to
   `THIRD_PARTY_LICENSES.md` -- it is not in the CLAUDE.md 3.3 dependency table,
   so the library currently ships third-party code that the licensing audit does
   not know about. The file also contains the PrimitivePolynomialsModuloTwo
   tables, edited from `PrimitivePolynomialsModuloTwoUpToDegree27.c`.
2. The text is already mangled -- "Peter Jkel" should read "Peter Jaeckel"
   (the umlaut was lost to an encoding conversion) and the word "Copyright" is
   missing. **This predates the repo**: it is already absent at Base Commit
   36bffcc1, so no rebrand step caused it. Repairing an attribution we are
   contractually required to preserve is worth doing properly.

### 2. `src/math/src/AQLNl2sol.cpp` -- NL2SOL (6,541 lines)
Found while removing personal names from comments: the "coded by david m. gay"
comments are **not** a colleague's attribution, they are the provenance of
**NL2SOL**, the standard nonlinear least-squares solver:

    john dennis designed much of this routine ... roy welsch suggested the
    model switching strategy. david gay and stephen peters cast this
    subroutine into a more portable form (winter 1977) ...
    this subroutine was written in connection with research supported by the
    national science foundation under grants mcs-7600324, dcr75-10143, ...

Published via ACM TOMS; the file also cites Dennis/Gay/Welsch and Rabinowitz.
Attribution and the NSF funding acknowledgement must be preserved, and NL2SOL
needs a THIRD_PARTY_LICENSES.md entry alongside the Sobol/Jaeckel code.

### Provenance scan is now a MUST, not a nice-to-have
Two undeclared third-party components turned up incidentally, in one project,
while looking for something else. ~20% of this code came from contractors
(CLAUDE.md 7). A deliberate provenance scan of `math` / `models` /
`calibration` -- looking for academic citations, funding acknowledgements,
"coded by", "adapted from", ACM/Netlib/Numerical Recipes markers -- belongs in
Phase 6 **before sale**, and is the kind of question a solicitor will ask.

---

## Known noise / not-bugs
- **`src/AQ_API/source/swig_*_wrap.{cpp,cxx}`** — SWIG-generated. Still contain old `me*`/`mir*` names
  in HEAD (regenerated in Phase 5, not in the core build). They also keep re-appearing as phantom
  worktree modifications (autocrlf on a large generated file); `git checkout HEAD -- src/AQ_API/source/swig_*_wrap.*`
  clears them. Don't stage them.
- Ubiquitous `LF will be replaced by CRLF` git warnings — repo line-ending config, harmless.

## Later phases (unchanged)
- Phase 4: xlOil XLL port (blocked on Nicholas's worked examples), 4a editions manifest, 4b generators/config + calendar JSON `:`→`+` migration (103 files).
- Phase 5: verify C#/Java/R bindings, regenerate SWIG wrappers, expand tests (yield-curve/Jacobian, bond price/yield Bloomberg-parity).
- Phase 6: credit-cluster untangle (`CreditResults.h` etc.), legacy `calibration/math/models` extraction, `resources/{examples,api,documentation}` audit, per-file licence headers + `NOTICE`/`THIRD_PARTY_LICENSES.md`.
- Phase 7: Linux CMake rebrand + validate, clang-format bulk pass, clang-tidy CI gate, copy to clean repo.

## Tooling
- `rebrand/tools/prefix_census.py` — legacy-prefix census per project.
- `rebrand/tools/run_batch.py` — approved-list batch runner (git mv + \b-anchored replace + string-literal report + verify gate). `FILE_PREFIX` handles `LWO→AQObj`.
- Scratch scripts used this run (not committed) are in the session scratchpad:
  `map_bsimple.py`, `map_blwo.py`, `run_bsimple.py`, `run_blwo.py`, `bfixtures.py`,
  `step6b_delete.py`, `step6b_vcxproj.py`, `step7b_prose.py`, `step7c.py`,
  `step8_map.py`, `step8a.py`, `step8b.py`, `step9.py`.
- `rebrand/phase3_Bsimple_MAP.csv`, `phase3_BLWO_mapB.csv` — the approved `me→aq` maps (committed).
- `rebrand/phase3_step8_MAP.csv` — the approved step-8 map, 774 rows (committed).
- `rebrand/phase3_step9_MAP.csv` — the approved step-9 map, 113 rows (committed).
- `rebrand/tools/fixture_key_check.py` — fixture keys vs current parameter names.
- `rebrand/tools/api_pair_check.py` — `aq*` must route through `validation`; emits `docs/api_map.csv`.
  Run with `--write` to refresh the map; exit 1 makes it usable as a CI gate.
