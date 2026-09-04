# Rebrand status — pause point 2026-09-05 ~04:00

**HEAD: `8e632c65`. Last user-confirmed green build: `5a29a51a` (step 6b).**
Unbuilt: `ab35bf0b` (7b prose sweep) and `8e632c65` (7c embedded LWO -> AQObj:
34 GTEST files renamed, 1108 fixture paths + their code strings moved in
lockstep, 200 project-file refs). Needs a full Rebuild + GTest.
Tree is clean. `baseline` tag is the pre-rebrand reference for GTest output diffs.

---

## Done this run (newest first)

| commit | what |
|---|---|
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

## Step 8 — public API naming scheme (done, 2026-09-04)

The handle API and the stateless API are two surfaces over the same products.
Excel cannot overload, so they need distinct names. The interim `aqObjects*`
scheme forced the object API into its own `Objects` category; step 8 replaces
that with an `aqObj` **prefix**, so both surfaces share one category list.

| Form | Meaning | Example |
|---|---|---|
| `aq<Category><Function>` | stateless — data in, value out | `aqSwapsParRate` |
| `aqObj<Category><Function>` | handle API — handle in | `aqObjSwapsParRate` |
| `aqObj<Lifecycle>` | lifecycle, no category word | `aqObjLoad`, `aqObjClearCache` |
| `AQObj*` / `AQOBJ_*` | internal C++ classes / macros | `AQObjCurve`, `AQOBJ_KEY` |

- **8A** `7fd00c7c` — 774 renames (`aqObjects*`→`aqObj<Category>*`,
  `tryAqObjects*`→`tryAqObj<Category>*`, `aqSwap*`→`aqSwaps*`). 108 src files and
  7,300 fixture files moved in the same commit as their path-strings.
- **8B** `30fc5505` — internal `AQO`→`AQObj` (140 tokens, 40 src files, 1,107
  resource paths, 64 VS filter labels).
- **8C** — docs + `docs/api_map.csv` + `rebrand/tools/api_pair_check.py`.

**Category list changed: `Objects` dropped, `FX` added — still 13.** `FX` is its
own category (Nicholas, this session), *not* folded into `Curves`.

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

Given ~20% of this code came from contractors (CLAUDE.md 7), other unattributed
third-party code may be present. A provenance scan of `math` / `models` /
`calibration` belongs in the Phase 6 licence work, before sale.

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
