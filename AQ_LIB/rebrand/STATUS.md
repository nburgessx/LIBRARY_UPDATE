# Rebrand status — 2026-09-11

## ⏸ PAUSED HERE — resume point (2026-09-11, session ran out of credit)

**Next action for the next session:** finish wiring `aqCurve.cpp` into the
build, then continue the Curve migration. In order:

1. **`aqCurve.cpp` is written (batch 1, 34 functions) but NOT YET added to
   `projects/AQ_XLL.vcxproj` / `.vcxproj.filters`.** Confirmed by grep — no
   `aqCurve.cpp` entry exists in the vcxproj yet. Add it the same way every
   other new category file was added this session (`<ClCompile
   Include="..\src\AQ_XLL\src\aqCurve.cpp" />` in both files, filter
   `Source Files`), then re-verify both files are well-formed XML
   (`python -c "import xml.etree.ElementTree as ET; ET.parse(path)"`) and that
   the `<ClCompile Include=` count matches the `</ClCompile>` count (the
   orphaned-tag bug from earlier this session).
2. **Batch-1 verification already done, before adding to the project:**
   whole-tree param-vs-`.arg()` audit run against every `src/AQ_XLL/src/*.cpp`
   — **290 functions total, 0 real mismatches** (`aqToolEcho` flagged by the
   quick regex is a false positive — it takes `const ExcelObj*`, not `&`, and
   is genuinely 1 param / 1 `.arg()`, pre-existing and fine). `aqCurve.cpp`
   itself: **34/34 clean**. No duplicate `XLO_FUNC` names anywhere in the tree.
   Signatures for all 34 were cross-checked line-by-line against the actual
   validation headers (`tryAqCurveObjectUtilities.h`, `...Display.h`,
   `...Calibrate.h`, `...CalibrateHedge.h`, `...DiscountFactor.h`,
   `...ForwardRate.h`, `tryAqCurveZeroRate.h`, `tryAqCurveGenerator.h`,
   `tryAqCurveMarketData.h`) — all match exactly. Caught and fixed two issues
   while cross-checking: (a) `toIntOr` is a **file-local** helper (lives in an
   anonymous namespace in `aqMath.cpp`, not shared via `aqXllTools.h`) — added
   a local copy to `aqCurve.cpp`'s anonymous namespace, same as the existing
   local `toStrOr`; (b) `etrading::trim_to_upper` (used in
   `aqCurveGeneratorCreate`/`aqCurveMarketDataCreate`) needs
   `#include <CoreEnumerations.h>`, which was missing — added. **Still not
   run through an actual compiler** — these are static/textual checks only;
   the real build is the next hard gate.
3. **`aqCurve.cpp` batch 1 covers:** object lifecycle
   (List/Delete/DeleteAll/Save/Load, 5), Display (1), Calibrate +
   CalibrateHedge (2 — the hedge one returns `etrading::HedgeCurveInfo`,
   marshalled as a 4-row key/value block), the DiscountFactor family (8,
   including the `void` out-param `DiscountFactorsTable` built as a
   date-column + one-column-per-curve-index matrix, and
   `DiscountFactorsWithSpread` which correctly uses `toDateVector`/
   `DateVector` not `toGregorianVector`), ForwardRate family (3), ZeroRate
   family (2), Generator (4, using the confirmed-safe
   `std::vector<validation::TableInfo>` pattern), MarketData (9). **34
   functions total.**
4. **After the vcxproj wiring:** run `rebrand/tools/api_pair_check.py` (HARD
   GATE must stay 0), rebuild all four projects
   (`validation → AQ_API → AQ_XLL → GTEST`), then continue with the rest of
   Curve — see the enumerated ~97-function list captured earlier this session
   (not reproduced here in full; re-derive from `src/validation/include/
   tryAqCurve*.h` if needed, there are 32 headers). **Not yet started:** the
   curve Results/Jacobian risk family (`tryAqCurveResults.h`,
   `tryAqCurveObjectJacobianDisplay.h`), `tryAqCurveObjectDualBootstrap.h`,
   `tryAqCurveHullWhite.h`/`tryAqCurveVasicek.h`, `tryAqCurveCalibrateCTD.h`,
   the misc utilities (`tryAqCurveCompoundRate.h`,
   `tryAqCurveEuroDollarConvexityAdjustment.h`, `tryAqCurveFrequency.h`,
   `tryAqCurveGetInterpolationJoinDate.h`, `tryAqCurveInterpolation.h`),
   `tryAqBondCurves.h` (check whether this belongs to `Bond` not `Curve` —
   name suggests bond-curve, likely already covered by `aqBond.cpp`, verify
   before touching), and `tryAqCurveObjectData.h`/`tryAqCurveObjectEngineCalibrate.h`
   (not yet read).
5. **Still an OPEN DECISION, not yet asked of Nicholas:** the four heavy
   one-shot creation functions `tryAqCurveObjectCreateBasis/FXForwards/OIS/Swap`
   (`tryAqCurveObjectCreateBasis.h`, `...CreateFXForwards.h`, `...CreateOIS.h`,
   `...CreateSwap.h` — 10-17 raw `AQLStringMatrix` params each). These look
   like a legacy one-shot path that may be superseded by the
   `CurveGenerator`+`CurveMarketData`+`tryAqCurveObjectCalibrate` two-step
   architecture already ported in batch 1 (which `CLAUDE.md` §4.4 explicitly
   endorses as the intended design). **Ask Nicholas before porting these** —
   don't guess.

**Also still open from earlier in this session (unrelated to Curve):**
- The legacy SABR calibration family decision
  (`tryAqVolatilitySABR{Calibrate,GetPrem,GetVol,...}` vs the already-ported
  object-based `tryAqVolatilityObjectSabr*`) — not yet asked.
- `Swap` category (~67 wrappers, including the `Ois`→`aqSwapOis*` rename work
  documented in `CLAUDE.md` §5.1) and `Credit` category (~34 wrappers) — not
  started, come after Curve.
- `src/validation/src/tryAqToolEchoDouble.cpp` needs adding to
  `projects/validation.vcxproj` (pre-existing gap, unrelated to this session).
- Confirm with Nicholas whether the improved `aqToolSEH()` message (see next
  section) actually resolves a `file(line)` in Excel once rebuilt — the
  DbgHelp path is untested end-to-end.
- Top-level `STATUS.md` (the short pointer file, not this one) has not been
  refreshed since the `InterestRate` rename — still says ~35%/Phase 4 without
  mentioning CMS/TRS/aqToolSEH/Curve-start. Worth a refresh next session.

Nothing in this session has been committed (still uncommitted, matching every
other entry in this file) and nothing has been built — every check above is
static (grep/regex/XML-parse), not a compiler run. **First thing next
session: do a real build** before trusting any of this further.

---

### `aqToolSEH()` feedback: structured-exception messages made plain-English + crash-located (2026-09-11, uncommitted, NOT BUILT)

Nicholas tried `aqToolSEH()` in Excel: **it works** — `AQ_XLL_GUARD` /
`etrading::StructuredExceptionHandler` catches the access violation cleanly,
no crash. Follow-up ask: make the returned error message concise plain English
(`#Structured Exception: <Category> - <plain english>`, not the old verbose
MSDN-quoted paragraph), and report the crash site (`__FILE__`/`__LINE__`) to
speed up debugging.

Changed `src/etrading/include/StructuredExceptionHandler.h` +
`src/etrading/src/StructuredExceptionHandler.cpp` (the shared SEH translator
behind every `AQ_XLL_GUARD`/`VALID_EXCEPTION_START`, not just `aqToolSEH` —
this improves every structured-exception message library-wide):

- `StructuredExceptionCodeMap`'s map value changed from a single blended
  `const char*` to a new `StructuredExceptionInfo{ category_, plainEnglish_ }`
  struct. All 20 Win32 exception codes re-worded: short category
  ("Access Violation", "Integer Divide By Zero", ...) + one concise
  plain-English sentence each (old text was a verbatim MSDN paragraph).
- `SEHandler` now formats `"#Structured Exception: <Category> - <plain
  english> at <file>(<line>)."` — e.g. `#Structured Exception: Access
  Violation - attempted to read from or write to a memory address it does not
  have access to. at aqTool.cpp(412).`
- **Crash-site location, not handler location:** added a file-local
  `resolveCrashLocation()` using DbgHelp (`SymInitialize` +
  `SymGetLineFromAddr64`) against `pExcept->ExceptionRecord->ExceptionAddress`
  — the actual faulting instruction — to resolve `file(line)` when a PDB is
  available (debug builds). Falls back to the raw hex address when no PDB /
  symbol is found (e.g. Release), and to nothing if resolution fails outright
  — never throws from inside the SE translator itself. `__FILE__`/`__LINE__`
  passed to the final `AQLCoreError` are still this handler's own location
  (unavoidable — that's what the macro expands to at the `throw` site); the
  crash site is carried in the message text instead, which is what actually
  answers "where did it crash".
- New includes: `<DbgHelp.h>` + `#pragma comment(lib, "Dbghelp.lib")` (Windows
  only, inside the existing `_WIN32`/`_WIN64` guard) — no `.vcxproj` change
  needed, the pragma embeds the linker directive in the `.obj` and every
  consumer (`AQ_XLL`, `AQ_API`, `GTEST`) picks it up at final link.

**Verification (no build yet):** grepped the tree for other callers of
`StructuredExceptionCodeMap`/`getExceptionCodes` — none found outside this
pair of files, so the map-value-type change is self-contained. Needs a real
build + a re-run of `aqToolSEH()` in Excel to confirm the DbgHelp path
resolves a file/line in the Debug config (PDB for `etrading`/`AQ_XLL` must be
present at runtime, same folder as the `.xll`/`.pdb`) — **not yet verified
end-to-end**, flagged for the next Excel round-trip with Nicholas.

---

### `TotalReturnSwap` category renamed to `TRS` (2026-09-11, uncommitted, NOT BUILT)

Same pattern as the `ConstantMaturitySwap` → `CMS` rename, one layer further:
**`TotalReturnSwap` already had a real `AQ_API` surface** (unlike CMS) — 4
functions in `aqCreditObject.cpp/.h` (filed under the Credit API source, not a
dedicated TRS file — same pre-existing filing quirk as the Ois/FixingTable
functions living in `aqCurveObject.h`), already generated into all 4 SWIG
`_wrap` files.

- `validation`: `tryAqSwapObjectPricing.{h,cpp}` — substring rename
  `TotalReturnSwapObject → TRSObject` (6 hits each file). Confirmed
  `etrading::TotalReturnSwap` (the internal pricer class) and
  `#include "TotalReturnSwap.h"` untouched — same anchor discipline as CMS.
- `AQ_API`: `aqCreditObject.h` (9 hits) / `.cpp` (16 hits) renamed
  (`aqTotalReturnSwapObject* → aqTRSObject*`, both the function defs and their
  `validation::tryAq...` call sites). The 4 generated `swig_*_wrap.*` files
  still say the old name — expected, untouched, will regenerate on next SWIG
  build.
- `GTEST`: `TestTotalReturnSwap.cpp` — 29 call-site substitutions. (Its own
  `TRS_*` constant names were already TRS-branded; only the wrapper-name calls
  needed changing.)
- `AQ_XLL`: `aqTotalReturnSwap.cpp` → `aqTRS.cpp`, 4 functions renamed,
  `AQ_XLL.vcxproj`/`.filters` updated (clean single-line edit this time — no
  repeat of the CMS batch's orphaned-tag mistake).
- 16 fixture files renamed (`tryAqTRSObject{PV,ParRate,ParSpread,Annuity}*_{inputs,outputs}.csv`,
  including variant suffixes like `_ALL`, `_Float`, `_Premium`, `_PayOnSurvival`).
- Docs: `CLAUDE.md` (both), `MIGRATION_PLAN.md`, `rebrand/tools/api_pair_check.py`
  `CATEGORIES` updated `TotalReturnSwap` → `TRS`; **`docs/api_map.csv`
  regenerated** this time (unlike CMS) since the AQ_API rows actually changed —
  `aqTRSObjectPV/ParRate/ParSpread/Annuity` now show category `TRS`.

**Verification:** HARD GATE = 0; `aqTRS.cpp` 4/4 param-vs-`.arg()`; zero
residual `TotalReturnSwapObject` outside the generated SWIG files; both
project files re-confirmed well-formed XML; fixture rename count matches (16
renamed, 0 old names left).

### Nicholas's follow-up on batch 1: CMS rename, BondOption/BondFutureOption merge, aqToolSEH diagnostic (2026-09-11, uncommitted, NOT BUILT)

Three requests after reviewing batch 1:

**1. `ConstantMaturitySwap` category renamed to `CMS`.** Golden-source rename,
validation first: `tryAqConstantMaturitySwapObject{PVUsingConvexityAdjustment,
ParRateUsingConvexityAdjustment}` → `tryAqCMSObject*` in the shared
`tryAqSwapObjectPricing.{h,cpp}` (no file rename there - CMS never had its own
validation file). **Anchoring note:** the first attempt used `\bConstantMaturitySwapObject\b`
and silently matched nothing - camelCase has no regex word boundary between
`tryAq` and `Constant...` (all letters, no separator), so `\b` never fires
there. Redone as a plain substring replace of the (distinctive, unambiguous)
compound `ConstantMaturitySwapObject`, verified against zero collisions first.
**Critically, `etrading::ConstantMaturitySwap` (the internal pricer class),
`ConstantMaturitySwap.h`/`.cpp`, and the `projects/etrading.vcxproj`/`GTEST.vcxproj`
entries referencing them were confirmed untouched** - the rename target is the
compound `...SwapObject...` (with `Object` immediately after), which never
matches the bare internal class name (per CLAUDE.md §5.1a: category names
don't propagate into `etrading` class/file names).
- `AQ_XLL`: `aqConstantMaturitySwap.cpp` → `aqCMS.cpp`, 2 functions renamed,
  `AQ_XLL.vcxproj`/`.filters` updated.
- `GTEST`: `TestConstantMaturitySwap.cpp` call sites updated (test class name,
  helper function name and `TEST_DIR` folder segment deliberately left as-is -
  internal test naming, not governed by the golden-source rule). 4 fixture
  files renamed (`tryAqCMSObject{PV,ParRate}UsingConvexityAdjustment_{inputs,outputs}.csv`).
- Docs: `CLAUDE.md` (both), `MIGRATION_PLAN.md`, `rebrand/tools/api_pair_check.py`
  `CATEGORIES` updated `ConstantMaturitySwap` → `CMS`.

**2. `aqBondOption.cpp` / `aqBondFutureOption.cpp` deleted; their 6 functions
moved into `aqBond.cpp` verbatim (signatures unchanged).** `AQ_XLL.vcxproj` +
`.filters` updated - **caught and fixed a self-inflicted XML corruption here**:
a scripted removal of the two `<ClCompile>` entries mishandled the 3-line
`.filters` block (`<ClCompile>` / `<Filter>` / `</ClCompile>`) and left two
orphaned `</ClCompile>` tags with no matching open tag. Caught by counting
`<ClCompile Include=` vs `</ClCompile>` (17 vs 19) before moving on, rather than
trusting the "no residual filename string" grep alone; fixed and reverified
both `.vcxproj` and `.filters` as well-formed XML via `xml.etree.ElementTree`.
Documented the resulting **file-per-category exception** in `CLAUDE.md` §5.1a
and `MIGRATION_PLAN.md` D18: consolidate a category's file into a sibling's
only when they share the same underlying cached object (see point 3), not
merely a name prefix.

**3. Nicholas asked where `aqBondFutureOptionObjectCreate` is - there isn't
one, and that's correct, not a gap.** Traced `tryAqBondOptionObjectCreate` →
`etrading::createOption()` → `OptionFactory.cpp`: the factory's
`OptionTradeTypeEnum` switch has exactly three cases - `CAP_FLOOR_TRADE`,
`EUROPEAN_SWAPTION_TRADE`, `BOND_OPTION_TRADE` (the `default` branch's own
error text confirms it: "must be 'CAPFLOOR', 'EUROPEAN_SWAPTION' or
'BONDOPTION'"). `tryAqBondFutureOptionObjectPV` confirms the design:
```
const auto& option = etrading::getOption(objectName);
const auto& bondOption = std::dynamic_pointer_cast<etrading::BondOption>(option);
...
auto result = pricer.forwardOptionPrice(bondFuturePrice);
```
`aqBondOptionObjectCreate` creates one `etrading::BondOption` object;
`aqBondOptionObjectPV` prices it against a bond spot price,
`aqBondFutureOptionObjectPV`/`Greeks` price the **same cached object** against
a bond-future price instead. One create function genuinely serves both
categories by design - nothing to build. Documented in the `aqBond.cpp` file
header and `CLAUDE.md` §5.1a so the next person doesn't go looking for it either.

**4. New diagnostic `aqToolSEH()` added to `aqTool.cpp`** (Nicholas): no args,
builds an empty `std::vector<int>` and calls `.front()` on it (undefined
behaviour - out-of-bounds, no bounds check, typically an access violation) to
let Nicholas observe firsthand whether `AQ_XLL_GUARD` /
`etrading::StructuredExceptionHandler` turns the resulting structured
exception into a clean Excel error, or whether it crashes the add-in. No
`tryAq*` wrapper - same class of AQ_XLL-only diagnostic as `aqToolEcho` /
`aqToolBuildTime` (deliberately not routed through validation; nothing to
validate). 0-arg, so trivially param/`.arg()`-clean.

**Verification (no build):** whole-tree param-vs-`.arg()` audit **256/256
across every `AQ_XLL/src/*.cpp`, 0 mismatches**; no duplicate `XLO_FUNC` names;
`vcxproj`/`.filters` file lists identical and both well-formed XML;
`api_pair_check` HARD GATE = 0.

---

### "Migrate all remaining categories" — batch 1 of N: 11 small/medium categories DONE (2026-09-11, uncommitted, NOT BUILT)

Nicholas asked to migrate everything still missing from `AQ_XLL`. Working in
size order (small/clean first), not yet touching the three big ones
(`Swap`+`Ois`, `Credit`, `Curve`) which need more care. **11 new category
files, 66 new `XLO_FUNC` functions**, all added to `AQ_XLL.vcxproj` +
`.filters`:

| File | Category | Fns |
|---|---|---:|
| `aqFuture.cpp` | Future | 4 |
| `aqBondOption.cpp` | BondOption | 4 |
| `aqBondFutureOption.cpp` | BondFutureOption | 2 |
| `aqConstantMaturitySwap.cpp` | ConstantMaturitySwap | 2 |
| `aqTotalReturnSwap.cpp` | TotalReturnSwap | 4 |
| `aqAssetSwap.cpp` | AssetSwap | 7 |
| `aqSwaption.cpp` | Swaption | 8 |
| `aqCapFloor.cpp` | CapFloor | 10 |
| `aqInflation.cpp` | Inflation | 7 |
| `aqFX.cpp` | FX | 11 |
| `aqVolatility.cpp` | Volatility | 7 |

**Verification:** whole-tree param-vs-`.arg()` audit — **255/255 functions
across every `AQ_XLL/src/*.cpp`, 0 mismatches**; no duplicate `XLO_FUNC` names;
every `validation::tryAq*` symbol called resolves in `src/validation/include`;
`vcxproj`/`.filters` file lists identical; `api_pair_check` HARD GATE = 0.

**Bug caught and fixed before it reached a build:** `aqInflationCurveCreate`
and the two `aqVolatilityObjectSabr{MarketDataCreate,ModelCalibrate}` functions
were first written using `std::vector<validation::TableInfo>` for the
multi-block-create pattern (copying the `aqBondCurveCreate` /
`aqBondGeneratorCreate` idiom) — but unlike `tryAqBondObject.h` /
`tryAqToolGrid.h`, neither `tryAqInflationObjectPricing.h` nor
`tryAqVolatilityObject.h` declares a `validation::TableInfo` typedef; both take
`etrading::JSONInfoBlockTuples` directly. `std::vector<validation::TableInfo>`
would have failed to compile ("no member named 'TableInfo' in namespace
'validation'"). Fixed by using `etrading::JSONInfoBlockTuples` directly (the
underlying tuple type is identical, so `toTableInfo(...)` still binds) —
caught by re-reading each header rather than assuming the pattern held.

**Decision needed — the legacy SABR calibration family, deferred:**
`tryAqVolatilitySABR{Calibrate,GetPrem,GetVol,OutputParameter,SetupConvention,
SetupParameter,SetupSwaptionVol}` (`tryAqVolatilitySABRCalibrate.h` and
siblings) is a **procedural, ID-string-driven** calibration workflow
(`alphaID`, `betaID`, `nuID`, `rhoID`, `convID`, `capConvID`, `curveMat`, …)
that looks superseded by the cleaner object-based `tryAqVolatilityObjectSabr*`
API already ported above. **Not ported — needs Nicholas's call:** port it
as-is (needs real domain understanding of what those ID strings reference to
marshal correctly — high risk of a plausible-but-wrong port), or drop it as
legacy since the object-based Sabr* API appears to be the intended replacement.

**Not attempted yet:** `Swap` (67 wrappers, incl. folding `Ois` in as
`aqSwapOis*` — see the section below), `Credit` (34), `Curve` (~100 — the
biggest, needs new enum-string marshalling helpers for `InterpolationEnum` /
`StateVariableEnum` / `DayCountEnum` / etc. and unblocks the deferred Math
`ForwardRate`/`DiscountFactor` functions too), `Model` (**empty — 0 validation
wrappers exist**, nothing to migrate), `Generator` (**0 wrappers — needs
`tryAqGeneratorList` written first**, unchanged from the earlier note).

**== PAUSE POINT (2026-09-10) ==**

**Committed HEAD: `18328864` ("Natvis Debug Vizulaization Update").** Since the
old `3809f148` baseline, Nicholas has committed:

- **`22212bc5`** ("AQ_XLL, validation and GTEST function name synchronization",
  7,905 files) — **task 2.6** (the full `validation` / `GTEST` / `AQ_API` /
  fixture rename to the 22-category singular scheme: `Vol`→`Volatility`,
  `Future` + `Ois` added, `tryAqObj<Cat>* → tryAq<Cat>Object*`), the fixture-
  rename recovery, the **AQ_XLL Tool + Object-lifecycle port** from
  `.APPLES\...\meUtilities.cpp`, and **Interpolation/PCA re-homed `Tool`→`Math`**
  (`tryAqMath{Interpolation,PCA}`). Built green, GTest passing.
- **`18328864`** — `Visualizer.natvis` refresh + `AQ_XLL.vcxproj`. ⚠ this commit
  also staged `src/AQ_API/source/swig_Python_wrap.cxx` — a **generated** file
  that the standing rule says never to commit (§"Known noise"). Flag to Nicholas:
  `git rm --cached` it and re-ignore, or accept it and note the exception.

**Uncommitted working-tree delta right now:**

| Path | State |
|---|---|
| `src/AQ_XLL/src/aqBond.cpp` `aqDate.cpp` `aqMath.cpp` `aqTool.cpp` | **BUILT GREEN, GTest passing** (Nicholas) — the "remaining Bond / Tool / Date / Math functions" fill-in |
| `src/AQ_XLL/src/aqInterestRate.cpp` (renamed from `aqRate.cpp`) + `AQ_XLL.vcxproj` / `.filters` | **NOT BUILT** — new category file, added since the last green build |
| `src/validation/{include,src}/tryAqInterestRate{FixingTable,FutureFra,ObjectFra}.{h,cpp}` (renamed from `tryAqRate*`) + `validation.vcxproj` / `.filters` | **NOT BUILT** — see the category rename below |
| `src/AQ_API/source/aqCurveObject.{h,cpp}` | `aqRateFixingTable* → aqInterestRateFixingTable*` (the Python/SWIG binding for this — it lives in the Curve API file, a pre-existing filing quirk, not touched) |
| `src/GTEST/src/Test{AQObjCurveDiscountFactorsWithSpread,AQObjHedgeCurveDelta,AQObjSwapDelta,AQObjSwapDeltaDualBootstrap,AQObjSwapDeltaWithCurveEngine,AQObjSwapDeltaXCCY,AQObjSwapDeltaXCCY_CSA,AQObjSwapDeltaXCCY_ZAR,AQObjSwapDelta_ARR,AQObjSwapDelta_JPY,AQObjSwapFloatRate}.cpp` | call-site rename, same reason |
| 3 fixtures (`FIXING@1`, `EUR6M_FIXINGS@191`, `EUR3M_FIXINGS@74` `_tryAqRateFixingTableCreate_inputs.csv`) | renamed to `tryAqInterestRateFixingTableCreate_inputs.csv` |
| `CLAUDE.md` (both), `MIGRATION_PLAN.md`, `rebrand/tools/api_pair_check.py`, `docs/api_map.csv` (regenerated) | category list `Rate → InterestRate` |
| `STATUS.md`, `rebrand/STATUS.md` | doc updates (this pause) |
| `rebrand/phase2_validation_rename_MAP.csv` | untracked — the task-2.6 rename map; sibling of the committed `phase3_*_MAP.csv` files, `git add` it with the next commit |

Also carried forward: `src/validation/src/tryAqToolEchoDouble.cpp` was committed
in `22212bc5` but is **not in `projects/validation.vcxproj`**, so it never
compiles — that is why `aqToolEchoDouble` was dropped from `aqTool.cpp` (link
error). Add the `.cpp` to `validation.vcxproj` + `.filters` on the next
`validation` rebuild, then re-add the XLL wrapper.

### `Rate` category renamed → `InterestRate` (2026-09-11, decided + applied, NOT BUILT)

Nicholas: `aqRate*` read oddly; chose **`InterestRate`** over `IR` (keeps the
spelled-out, IntelliSense-groupable style of the other categories) and over
leaving it as `Rate` (too vague — reads as FX/hazard rate). Applied as a full
golden-source category rename, anchored to the 3 known function families only
(`FixingTable`, `FutureFra`, `ObjectFra` — never a blind `Rate*` sweep, which
would have hit unrelated tokens like local variables `aqRatesFixingDates` and
the pre-existing `tryAqCurveCompoundRate` / `…EuroDollarConvexityAdjustment`
`@brief` comments):

- `validation`: 3 files renamed (`tryAqRateFixingTable.{h,cpp}` →
  `tryAqInterestRateFixingTable.{h,cpp}`, similarly `FutureFra` / `ObjectFra`);
  13 wrapper functions renamed; `validation.vcxproj` + `.filters` updated.
- `AQ_XLL`: `aqRate.cpp → aqInterestRate.cpp`, 13 functions renamed to match;
  `AQ_XLL.vcxproj` + `.filters` updated. `.arg()` count re-verified: 13/13.
- `AQ_API`: `aqCurveObject.cpp/.h` — the 4 `aqRateFixingTable*` Python/SWIG
  bindings renamed (this is the one place `Rate` had already reached a
  consumer; filed under the Curve API source, not moved).
- `GTEST`: 11 files' `tryAqRateFixingTable*` call sites updated.
- 3 fixture files renamed (`resources/test/inputs/...`).
- Docs: both `CLAUDE.md`, `MIGRATION_PLAN.md` §2.2/porting-order prose,
  `rebrand/tools/api_pair_check.py` `CATEGORIES`, `docs/api_map.csv`
  regenerated (118 rows, `InterestRate: 4` in the public-surface breakdown).
- `src/AQ_API/source/swig_Python_wrap.cxx` still has the old `aqRateFixingTable*`
  names — expected, it is generated and was reverted to `HEAD`, not hand-edited;
  it picks up the new names on the next SWIG regen.
- **Verification (no build):** HARD GATE = 0; tree-wide grep for
  `tryAqRate(FixingTable|FutureFra|ObjectFra)` / `aqRate(FixingTable|FutureFra|ObjectFra)`
  across `src/` + `projects/` = 0 (outside the generated SWIG file).
- **Caught and fixed mid-rename:** the first sweep scoped its file list with
  `git grep -l`, which only searches **tracked** files — it silently skipped
  the then-untracked `aqInterestRate.cpp` (a `mv`, not `git mv`, since it had
  never been committed) and missed the two `FutureFra` functions entirely,
  because their real names (`tryAqRateFuturePriceToFraRate[FromConvAdj]`) don't
  contain the anchor substring "FutureFra" — only their *file* does. Both fixed
  with a second, targeted pass once found; the residual-grep check above is now
  post-fix and clean. Lesson for future category renames: run the identifier
  scan against the full symbol list (or `git grep --untracked`), not just
  filename-shaped anchors.

**Needs a full rebuild** (`validation` → `AQ_API` → `AQ_XLL` → `GTEST`) since it
touches `validation` — larger than the AQ_XLL-only rebuilds this pause point
otherwise needed.

### `Ois` folded into `Swap` as a product variant (2026-09-11, DOCS ONLY — no code changed yet)

Nicholas: `Ois` should not be its own category — it is a vanilla swap whose
floating leg compounds an overnight index instead of a term rate, not a
structurally distinct product the way `AssetSwap`/`CapFloor` are. Decision:
category count drops **22 → 21**; the golden-source form is
**`aqSwapOis<Function>`** (stateless; `aqSwapOisObject<Function>` if a stateful
form is ever needed), living in `aqSwap.cpp` — **not** a separate `aqOis.cpp` —
and in `src\validation\Swap\` alongside the vanilla-swap wrappers.

**Docs updated now** (`CLAUDE.md` both copies §5.1/§5.1a, `MIGRATION_PLAN.md`
§2.2/§2.7/D14, `rebrand/tools/api_pair_check.py` `CATEGORIES` — `Ois` removed
from the list). Also added a standing **"category-migration sequence"** note
(`AQ_LIB\CLAUDE.md` new §5.1a): validation (golden source, rename legacy
wrappers here first) → `GTEST` → `AQ_API` → `AQ_XLL`, in that order, every time
— codifying what this session has been doing ad hoc, per Nicholas's request to
make sure all four surfaces stay synchronized to the `validation` name and any
drift gets cleaned up as part of the migration, not deferred.

**No code renamed yet — this is preparation for when `Swap` (and Ois within
it) is next migrated.** What that migration will need to do, found while
scoping this:

- `validation` — **pre-existing** `tryAqOisPV` / `tryAqOisParRate` +
  `*LVBKeys` companions (4 wrappers, `tryAqOisPV.{h,cpp}` /
  `tryAqOisParRate.{h,cpp}`) predate the category scheme and must rename to
  `tryAqSwapOisPV` / `tryAqSwapOisParRate` (+ files).
- `AQ_API` — **already shipped** `aqOisPV` / `aqOisParRate` (Python/C#/Java/R
  via SWIG, `aqOisPV.{h,cpp}` / `aqOisParRate.{h,cpp}`) rename to
  `aqSwapOisPV` / `aqSwapOisParRate` (+ files + the 4 SWIG `.i`).
- `GTEST` — `TryAqTestTradeEUROISParRate.cpp` call sites + likely the file name.
- `AQ_XLL` — new functions `aqSwapOisPV`, `aqSwapOisParRate` (+ `LVBKeys`) in
  `aqSwap.cpp`, alongside `aqSwapPV`/`aqSwapParRate` when that category is built.
- `docs/api_map.csv` — **not regenerated this session on purpose.** Its 4
  `aqOisPV*`/`aqOisParRate*` rows still say category `Ois`, which is accurate
  until the rename above lands; regenerating now (after removing `Ois` from
  `CATEGORIES`) would mislabel them via the script's no-match fallback
  (observed: they fall into the `(lifecycle)` bucket in the console report —
  cosmetic, `api_pair_check` HARD GATE is unaffected at 0). Regenerate
  `api_map.csv` only once the rename lands.

### Bond / Tool / Date / Math XLL fill-in (2026-09-10) — BUILT GREEN, tests passing (Nicholas)

Nicholas: "migrate the remaining Bond, Tool, Date and Math functions." **92 new
`XLO_FUNC` worksheet functions** added to the four existing category files (no
new files, no vcxproj change). Whole-file param-vs-`.arg()` audit: **165/165
across all AQ_XLL `.cpp`, 0 mismatch.** `api_pair_check` HARD GATE = 0. Every
function routes through its identically-named `validation::tryAq*` wrapper.
(One more, `aqToolEchoDouble`, was written and then dropped after a link
failure — see the `aqTool.cpp` note below.)

- **`aqDate.cpp` +26** (now 30) — `YearFraction`, `BusinessDays`,
  `ShiftedSpotDate`, `IsWorkingDay`, `IsHoliday`, `IsWeekend`, `IsWeekday`,
  `FuturesContract`, `CentralBank`, `ECB`, `ECBSwapStart/End`,
  `NthECBMeetingDate`, `NthECBSwapStart/EndDate`, `NextECBMeetingDate`,
  `NextECBSwapStart/EndDate`, `IMMFromMonth`, `NthIMM`, `NthIMMFromStartDate`,
  `IMMCurrent`, `IMMNext`, `IMMPrevious`, `IMMNth`, `IsRegularSwapSchedule`.
  Includes `<tryAqDateCentralBank.h>` `<tryAqDateIMM.h>`
  `<tryAqDateIsRegularSwapSchedule.h>`; file-local `toInt`.
- **`aqBond.cpp` +17** (now 59) — bond curve: `Create`, `Display`, `Yield`,
  `NelsonSiegelCalibrate`, `NelsonSiegelYield`, `PolynomialCalibrate`,
  `PolynomialYield`, `SvenssonCalibrate`, `SvenssonYield`; bond object:
  `PriceFromBondCurve`, `YieldFromBondCurve`, `ForwardReinvestedCoupon`,
  `Quote`, `YieldFromObject`, `YieldOptimized`; stateless: `BondSchedule`,
  `BondScheduleLVBKeys`. Includes `<tryAqBondCurves.h>` `<tryAqBondSchedule.h>`
  `<AQObjUtilities.h>` (`etrading::getBond` for the two `BondPtr`-arg wrappers)
  `<BondCurves.h>` `<NelsonSiegelFitting.h>` `<PolynomialFitting.h>`; file-local
  `nssParamsFromRange` / `nssResultBlock`. Calibrate results are rendered as a
  (label, value) block. `NelsonSiegel` reads a 4-cell InitialGuess, `Svensson`
  a 6-cell one — **verify the Excel layout of InitialGuess / bounds in use.**
  `aqBondCurveCreate` passes `std::vector<validation::TableInfo>` to the
  wrapper's `JSONInfoBlockTuples` param (the two tuple typedefs are identical).
- **`aqTool.cpp` +11** (now 35) — `LVBCreate` (1-2 value columns) and the
  object-grid family: `ObjectGridCreate/Save/Load/Display/ObjectNames/ClearOne/
  ClearAll`, `ObjectMultiGridCreate` (up to 3 named grids) `/Display/SubNames`.
  Includes `<tryAqToolGrid.h>` `<tryAqToolMultiGrid.h>`; file-local
  `flexibleDataToExcel` (prepends the column-name header row to the grid
  `Display` matrix).
  - **`aqToolEchoDouble` dropped** — link failed (`LNK2001` on
    `validation::tryAqToolEchoDouble`): `src/validation/src/tryAqToolEchoDouble.cpp`
    exists and defines it but is **not listed in `projects/validation.vcxproj`**,
    so it is never compiled into the `validation` lib. Pre-existing gap
    (`api_pair_check` counts it off the header). Fix when `validation` is next
    rebuilt: add the `.cpp` to `validation.vcxproj` + `.filters`, then re-add
    `aqToolEchoDouble`. It is only a round-trip diagnostic — `aqToolEcho`
    already echoes any value — so nothing is lost meanwhile.
- **`aqMath.cpp` +38** (now 41) — Black-Scholes `ImpliedVol` + 6 Greeks
  (`DeltaForward`, `DeltaSpot`, `Gamma`, `Vega`, `Theta`, `Rho`);
  `CapletFloorletPrice` / `ImpliedVol`; 6 volatility conversions
  (`VolatilityTo{Normal,Lognormal,ShiftedLognormal}From{...}`);
  `LiborConvexityAdjustmentInArrears` / `LiborRateInArrears` +
  `...ForArbitraryFixingDate` pair; European IR swaption `Price` / `ImpliedVol`
  / `Delta` / `Gamma` / `Vega` / `Theta` / `CashAnnuity`; normal distribution
  (`Standard`, `StandardPDF`, `StandardInverse`, and the mean/variance
  `NormalDistribution` / `PDF` / `Inverse`); polynomial
  `Interpolation` / `Interpolations` / `PoynomialIntegration` /
  `PoynomialIntegrations` (**wrapper misspells "Poynomial" — matched verbatim
  per golden source**); `IntegrateUsingTerms` and `Integrate` (date form).
  Includes `<tryAqMathCapletFloorlet.h>` `<tryAqMathConvexity.h>`
  `<tryAqMathEuropeanIRSwaption.h>` `<tryAqMathIntegrate.h>`
  `<tryAqMathPolynomial.h>`; file-local `toStrOr` / `toUInt`. `CallOrPut` and
  `VolatilityType` marshalled via `etrading::toCallOrPutEnum` /
  `toVolatilityTypeEnum`.

### Rate category → new `aqInterestRate.cpp` (2026-09-10, uncommitted, NOT BUILT)

**13 `XLO_FUNC` worksheet functions** in a **new file** `src/AQ_XLL/src/aqInterestRate.cpp`,
added to `AQ_XLL.vcxproj` + `.filters`. Param-vs-`.arg()`: 13/13, 0 mismatch.

- Fixing table (a cached table of historical index fixings):
  `aqInterestRateFixingTableCreate` (currency / curve-tenor form), `…Display`,
  `…Value` (one date), `…Values` (a column of dates).
- Rate future ↔ FRA (stateless): `aqInterestRateFuturePriceToFraRate`
  (Hull-White convexity), `aqInterestRateFuturePriceToFraRateFromConvAdj`
  (explicit adjustment).
- FRA object: `aqInterestRateObjectFraCreate`, `…PV`, `…Display`, `…DisplayCashflows`,
  `…Rate`, `…ToFuturePrice`, `…ToFuturePriceFromConvAdj`.

File-local `toGregorian` / `toGregorianVector` (the fixing-table wrappers take
`boost::gregorian::date`, not `AQLDate` — converted via
`etrading::toGregorianDateFromAQLDate`). `aqInterestRateFixingTableDisplay` uses the
`toExcelMatrix(const etrading::VariantMatrix&)` overload added this session.
`aqInterestRateObjectFraCreate` follows the generator-create pattern (decorate + call +
`appendInstanceCounter`, no create-vs-modify guard — matches
`aqBondObjectCreateFromGenerator`). The three multi-overload wrappers
(`FixingTableCreate`, `FixingTableValues`) are each exposed once, via the
currency/tenor + plain-dates forms; the `LabelValueBlock` overloads are not
wired (add later if wanted).

### Generator category — NOT created (no wrappers exist)

Asked for alongside Rate. **`aqGenerator.cpp` was not created:** there are **zero
`tryAqGenerator*` validation wrappers** in `src/validation/` — the `Generator`
category (CLAUDE.md §5.1: introspection only — `aqGeneratorList` / `Describe` /
`Validate`) has never been built. Generator *construction* lives in the asset
categories (`aqBondGeneratorCreate`, and the pending `aqCurveGeneratorCreate` /
`aqSwapGeneratorCreate`), not here. Creating an empty file would add noise. To
populate this category, first add an `etrading` directory scan of
`resources/config/{BOND,CURVE,SWAP}_GENERATOR/` + a `tryAqGeneratorList` wrapper
(then `Describe` / `Validate`); the XLL/API surface follows. Flagged for
Nicholas's go-ahead (same item as under "Next steps on resume").

### Still deferred from Bond / Tool / Date / Math

Need enum-vector or curve-context marshalling not yet in `aqXllTools` — do with
the Curve category:
- Math `ForwardRate` / `ForwardRates` / `DiscountFactor` / `DiscountFactors`
  (both overload sets, 6 wrappers) — `InterpolationEnum`, `StateVariableEnum`,
  `DayCountEnum`, `BusinessDayAdjustmentEnum`, `CompoundingFrequencyEnum` +
  `forwardAdjustmentTable`.
- Math vector overloads: `BlackScholesPrices` / `ImpliedVols`,
  `CapletFloorletPrices` / `ImpliedVols`, `EuropeanIRSwaptionPrices` /
  `ImpliedVols` (`std::vector<CallOrPutEnum>` etc.) — the scalar forms cover the
  common use.
- Bond `tryAqBondObjectZSpread` (singular) — `aqBondObjectZSpread` already
  routes to the plural `tryAqBondObjectZSpreads`.
- Tool `tryAqToolLVBAdd` (in-place `STDStringMatrix&` mutator, not
  worksheet-shaped); `tryAqToolSwapScheduleTemplate` (Tool wrapper living in
  `tryAqSwapObjectSchedule.h` — do with Swap).
- `meUtilityLWODecorateNames`, `meUtilityMLIBSetUp` / `MLIBTearDown` — still no
  golden-source wrapper.

### Next steps on resume (priority order)

0. **Rebuild `validation` → `AQ_API` → `AQ_XLL` → `GTEST`** — the `Rate` →
   `InterestRate` rename touches `validation`, so this is a full rebuild, not
   just `AQ_XLL`. Nothing else in the current delta needs more than that.
1. ~~Decide the `Rate` category name~~ — **done**: `InterestRate` (Nicholas,
   2026-09-11), applied.
2. **Commit the small remaining delta** (see the PAUSE POINT table): the
   Bond/Tool/Date/Math XLL fill-in + `aqInterestRate.cpp` (+ vcxproj/filters) +
   `rebrand/phase2_validation_rename_MAP.csv` + the doc updates. Task 2.6, the
   Tool/Object port and Interpolation/PCA→Math are **already committed** in
   `22212bc5`. Before staging, check
   `git status --porcelain -- 'src/AQ_API/source/swig_*_wrap.*'` is empty — and
   note `swig_Python_wrap.cxx` slipped into `18328864`; decide whether to
   `git rm --cached` it.
2. **Confirm the 4 pre-existing test failures** (`TestDatesCentralBank`,
   `TestAQObjCurve`, `TestExampleObject`,
   `TestStructuredExceptionHandler.UNIT_IntegerDivideByZero`) also fail on a
   clean `3809f148` checkout — i.e. not caused by any of this work. If Nicholas's
   green run already showed them passing, this is moot.
3. **New `AQ_XLL` file `aqFuture.cpp`** — `Future` is a validation-only category
   (`tryAqFutureTicker*`); no XLL surface yet. Small, same pattern as the Tool
   port. (`Ois` no longer gets its own file — see the "`Ois` folded into `Swap`"
   section above; its 4 `tryAqOis*` wrappers rename to `tryAqSwapOis*` and land
   in `aqSwap.cpp` when `Swap` is migrated.)
4. **`meUtilityLWODecorateNames`** (Tool) + **`meUtilityMLIBSetUp` /
   `meUtilityMLIBTearDown`** — deferred from the Tool port because no
   golden-source wrapper exists. Decide: write conforming
   `tryAqTool{ObjectDecorateNames,Setup,TearDown}` wrappers, or drop these
   functions from the surface.
5. **`aqGeneratorList`** — the generator-name introspection function
   (CLAUDE.md §5.1) that does not exist in either library. New `etrading`
   directory scan of `resources/config/{BOND,CURVE,SWAP}_GENERATOR/` +
   `tryAqGeneratorList` wrapper + XLL/API. Nicholas said Batch-1 `aqObjectList`
   is "ok for now"; this is the fuller answer when wanted.
6. **Resume the main XLL port backlog** (the bigger Phase 4 work) — Curves →
   Swaps → products → Models, driven by
   `rebrand/xll_function_inventory.csv`. See "⇒ RESUME HERE — AQ_XLL port".
7. Lower priority, still open: the validation **recording rollout** (plan 4.9,
   see "⇒ RESUME HERE — recording rollout"); `AQ_API` SEH guard (plan 4.12);
   the 5 + 1 wrapper-name-drift advisories; the `tryAqObjSwapsDV01*` /
   credit-fixture golden-source cleanup (Phase 6 resources audit).

- Docs: `Vol` → `Volatility`; **Future** + **Ois** added as categories (now 22);
  D19 = the `validation` wrapper is the GOLDEN SOURCE for names. `api_pair_check.py`
  CATEGORIES updated.
- Rename map: `rebrand/phase2_validation_rename_MAP.csv` — 467 wrappers, 416
  renames, 0 collisions. Decisions baked in: `tryAqFuturesTicker*` →
  `tryAqFutureTicker*` (new `Future` cat), `tryAqOIS*` → `tryAqOis*` (new `Ois`
  cat), `tryAqObj{QuickLoad,QuickSave,Type,TypeAsString,CreateRange}` →
  `tryAqObject*`, `tryAqObjBondsCompoundYields` → `tryAqBondObjectCompoundYield`
  (wrapper singularised), `tryAqBondsYield` → `tryAqBondCurveYield`.
- **DONE this session (uncommitted, NOT BUILT):**
  - **Date batch** — `tryAqDates*` → `tryAqDate*` (29 fns) across validation +
    `AQ_API` (`aqDates*` → `aqDate*` method names + files) + `GTEST` (suites +
    `TestAqDates*.cpp` → `TestAqDate*.cpp`) + `AQ_XLL` call sites + SWIG `.i` +
    all 3 vcxproj/.filters. **No fixtures exist for Date.**
  - **Object lifecycle batch** — `tryAqObj{Load,Save,Exists,ClearCache,
    CreateRange,Delete,DeleteAll,List,LoadFromString,LoadAndReturnTupleResults,
    QuickLoad,QuickSave,Type,TypeAsString}` → `tryAqObject*` (14 fns). Container
    files `tryAqObjects.{h,cpp}` → `tryAqObject.{h,cpp}`, AQ_API `aqObjects.*` →
    `aqObject.*`, ~40 GTEST includers, SWIG `.i`, vcxproj. `tryAqObjToolsGrid*`
    deliberately left (that is the Tool batch). **No fixtures.**
  - Reverted twice: `swig_Python_wrap.cxx` (generated — regenerated in Phase 5)
    and `rebrand/phase3_Bsimple_MAP.csv` (historical) were caught by an
    unscoped sweep. Sweeps are now scoped to `src/` + `projects/`, excluding
    `swig_*_wrap.*`.
- **FULL MAP APPLIED (uncommitted, NOT BUILT).** All 416 renames from
  `phase2_validation_rename_MAP.csv` applied in one pass (the user said "go"):
  - `validation` — 467 wrappers renamed; ~240 `.h/.cpp` files `git mv`'d
    (exact-stem + container headers like `tryAqObjBonds.h → tryAqBondObject.h`,
    `tryAqCurvesDiscountFactor.h → tryAqCurveDiscountFactor.h`, …); all `#include`
    updated.
  - `AQ_API` — `aq*` binding method names + source files + SWIG `.i` `%include`.
  - `GTEST` — `tryAq*` calls + `_inputs.csv` path-string literals + `TestAq*.cpp`
    file names where the stem was a function name.
  - `AQ_XLL` — `validation::tryAq…` call sites.
  - `resources/test` — **~3,560 fixture files** `git mv`'d
    (`<oldfn>_{inputs,outputs}*.csv → <newfn>_…`). Content untouched.
  - 3 × `.vcxproj`/`.filters`. `docs/api_map.csv` regenerated.
  - `swig_*_wrap.*` reverted twice (generated — Phase 5).
- **Verification (what I could check without a build):**
  - `api_pair_check.py` **HARD GATE = 0** (every public fn routes through validation).
  - Old function names in code: **0**. Old validation header files: **0**.
  - 5 wrapper-name-drift advisories — all pre-existing (the deferred set), not new.
  - **24 fixtures keep old names** — `tryAqObjSwapsDV01*`, `tryAqObjSwaps_{IRS,XCCY}_DV01*`,
    `tryAqObjCreditDefaultSwap{AnnuityFromHazardRate,PVIntegration,PVMonteCarlo,PVMonteCarloSobol}*`.
    Their GTEST path-strings are ALSO still old (hardcoded literals that aren't a
    function name), so file+string are self-consistent → tests still read them.
    These are drift/orphan fixtures for the Phase 6 resources audit.
  - A handful of GTEST `.csv` string literals reference fixtures that exist under
    **neither** old nor new name (`tryAqSwapObjectPV_inputs.csv`,
    `tryAqSwapObjectDeltaLadder_spot_4Y_inputs.csv`, the VariableNotional set) —
    **pre-existing** (0 files under the old name too), not caused by this rename.
- **GATE — build `validation` + `AQ_API` + `GTEST` and run GTest.** Output must
  be numerically identical to the pre-rename baseline. Any fixture-name failure
  will name the exact file; report it and I fix that one. This is the
  build-between-batches checkpoint (done once here, since the map went in as one
  batch).
- Still to do after green: the two new-category AQ_XLL files `aqFuture.cpp` /
  `aqOIS.cpp` (Future/Ois currently validation-only); the `meUtility*` → `Tool`
  port from `.APPLES`.

### AQ_XLL Tool / Object port (2026-09-09, in progress, uncommitted, NOT BUILT)

Third explicit task — expose the `Tool` + `Object`-lifecycle surface in the Excel
add-in. The `validation` wrappers (`tryAqTool*`, `tryAqObject*`) already exist
(ported in earlier phases); the missing piece is the `AQ_XLL`
`XLO_FUNC_START/END` code. Marshalling reference: `.APPLES\APPLE\src\
MLIBQ_ADDIN\src\meUtilities.cpp` (37 `IMPLEMENT_XLLFN4` exports).

Port maps to existing wrappers as:

| .APPLES `meUtilities.cpp` | validation wrapper (exists) | new AQ_XLL fn | file |
|---|---|---|---|
| `meLWOType` | `tryAqObjectType` | `aqObjectType` | aqObject.cpp |
| `meLWOList` | `tryAqObjectList` | `aqObjectList` | aqObject.cpp |
| `meLWODelete` | `tryAqObjectDelete` | `aqObjectDelete` | aqObject.cpp |
| `meLWODeleteAll` | `tryAqObjectDeleteAll` | `aqObjectDeleteAll` | aqObject.cpp |
| `meLWOExists` | `tryAqObjectExists` | `aqObjectExists` | ✅ already |
| `meLWOLoad` | `tryAqObjectLoad` | `aqObjectLoad` | ✅ already |
| `meLWOSave` | `tryAqObjectSave` | `aqObjectSave` | ✅ already |
| `meLWOLoadFromString` | `tryAqObjectLoadFromString` | `aqObjectLoadFromString` | aqObject.cpp |
| `meLWOQuickLoad` | `tryAqObjectQuickLoad` | `aqObjectQuickLoad` | aqObject.cpp |
| `meLWOQuickSave` | `tryAqObjectQuickSave` | `aqObjectQuickSave` | aqObject.cpp |
| `meUtilityClearLWOCache` | `tryAqObjectClearCache` | `aqObjectClearCache` | aqObject.cpp |
| `meUtilityResize` | (n/a) | `aqToolResize` | ✅ already |
| `meUtilityVersion` | `tryAqToolVersion` | `aqToolVersion` | aqTool.cpp |
| `meUtilityRecord` | `tryAqToolRecord` | `aqToolRecord` | aqTool.cpp |
| `meUtilityReplay` | `tryAqToolReplay` | `aqToolReplay` | aqTool.cpp |
| `meUtilityClearEntityPool` | `tryAqToolClearEntityPool` | `aqToolClearEntityPool` | aqTool.cpp |
| `meUtilityLoadStaticData` | `tryAqToolLoadStaticData` | `aqToolLoadStaticData` | aqTool.cpp |
| `meUtilityLoadCalendarFile` | `tryAqToolLoadCalendarFile` | `aqToolLoadCalendarFile` | aqTool.cpp |
| `meUtilityLoadConfigurationFiles` | `tryAqToolLoadConfigurationFiles` | `aqToolLoadConfigurationFiles` | aqTool.cpp |
| `meUtilityParallelModeEnable` | `tryAqToolParallelModeEnable` | `aqToolParallelModeEnable` | aqTool.cpp |
| `meUtilityParallelModeStatus` | `tryAqToolParallelModeStatus` | `aqToolParallelModeStatus` | aqTool.cpp |
| `meUtilityMLIBSetUp` | `tryAqToolSetup` | `aqToolSetup` | aqTool.cpp |
| `meUtilityInterpolation` | `tryAqToolInterpolation` | `aqToolInterpolation` | aqTool.cpp |
| `meUtilityPCA` | `tryAqToolPCA` | `aqToolPCA` | aqTool.cpp |
| `meUtilityLVBFromKeysValues` | `tryAqToolLVBFromKeysValues` | `aqToolLVBFromKeysValues` | aqTool.cpp |
| `meUtilityLVB` | `tryAqToolLVB` | `aqToolLVB` | aqTool.cpp |
| `meUtilityLVBGroup` | `tryAqToolLVBGroup` | `aqToolLVBGroup` | aqTool.cpp |
| `meUtilityLVBFromMultipleKeysValues` | `tryAqToolLVBFromMultipleKeysValues` | `aqToolLVBFromMultipleKeysValues` | aqTool.cpp |
| `meUtilityLVBFromKeysAndMultipleValues` | `tryAqToolLVBFromKeysAndMultipleValues` | `aqToolLVBFromKeysAndMultipleValues` | aqTool.cpp |
| `meUtilityClean` | `tryAqToolClean` | `aqToolClean` | aqTool.cpp |
| `meUtilityAppend` | `tryAqToolAppend` | `aqToolAppend` | aqTool.cpp |
| `meUtilityDataFilter` | `tryAqToolDataFilter` | `aqToolDataFilter` | aqTool.cpp |
| `meUtilityTermsToDates` | `tryAqToolTermsToDates` | `aqToolTermsToDates` | aqTool.cpp |
| `meUtilityDatesToTerms` | `tryAqToolDatesToTerms` | `aqToolDatesToTerms` | aqTool.cpp |
| `meUtilityValuationSettingsDisplay` | `tryAqToolValuationSettingsDisplay` | `aqToolValuationSettingsDisplay` | aqTool.cpp |

No validation wrapper in AQ_LIB (defer — needs a wrapper first, or drop):
`meUtilityLWODecorateNames`, `meUtilityThreadsEnableLocks`, `meUtilityMLIBTearDown`.
`meUtilityMLIBSetUp/TearDown` — `MLIB` is a legacy prefix; the setup half maps to
`tryAqToolSetup`, the teardown half has no wrapper. Also unmapped in AQ_XLL so far:
the `tryAqToolObjectGrid*` / `tryAqToolObjectMultiGrid*` / `tryAqToolSwapScheduleTemplate`
wrappers (these come from `.APPLES` files other than `meUtilities.cpp`).

Batches (build between each, on Nicholas's side):

1. **Object lifecycle → `aqObject.cpp`** — `aqObjectType`, `aqObjectList`,
   `aqObjectDelete`, `aqObjectDeleteAll`, `aqObjectLoadFromString`,
   `aqObjectQuickLoad`, `aqObjectQuickSave`, `aqObjectClearCache` (8 fns).
   **DONE (uncommitted, NOT BUILT).** Added `#include <boost/format.hpp>` +
   `<tryAqToolSetup.h>`. Every fn has `.arg()` count == parameter count.
   **BUILT GREEN in Excel (Nicholas, 2026-09-09).**
2. **Tool setup/version/parallel + record/replay → `aqTool.cpp`** —
   `aqToolVersion`, `aqToolClearEntityPool`, `aqToolLoadStaticData`,
   `aqToolLoadCalendarFile`, `aqToolLoadConfigurationFiles`,
   `aqToolParallelModeEnable`, `aqToolParallelModeStatus`, `aqToolRecord`,
   `aqToolReplay` (9 fns). All string / bool / int in, string out.
   **DONE — BUILT GREEN with tests passing (Nicholas, 2026-09-09).** Added
   `#include <cstdio>` `<ctime>` + `<tryAqToolSetup.h>` `<tryAqToolRecord.h>`
   `<tryAqToolReplay.h>`; file-local `toIntOr()` for the optional
   `StartIndex`/`MaxIndex` args.
   - `meUtilityMLIBSetUp` / `meUtilityMLIBTearDown` **dropped** — no
     golden-source wrapper: the only setup helpers in `tryAqToolSetup.h` are
     `trySetupAQL(irProps, calendar, cbCalendar)` / `tryTearDownAQL()`, which
     do not conform to the `tryAq<Category>` scheme. Needs a conforming
     `tryAqToolSetup` / `tryAqToolTearDown` wrapper defined first.
3. **Tool LVB family + data reshaping + maths → `aqTool.cpp`** (13 fns).
   **DONE — BUILT GREEN with tests passing (Nicholas, 2026-09-09).**
   - LVB: `aqToolLVBFromKeysValues`, `aqToolLVB`, `aqToolLVBGroup` (10 optional
     block args), `aqToolLVBFromMultipleKeysValues` (5 keys/values/prefix
     triples = 15 args), `aqToolLVBFromKeysAndMultipleValues` (6 args).
   - Reshaping: `aqToolClean` (5), `aqToolAppend` (11), `aqToolDataFilter` (2),
     `aqToolValuationSettingsDisplay` (1).
   - Maths: `aqToolInterpolation` (5), `aqToolPCA` (4), `aqToolTermsToDates` (2),
     `aqToolDatesToTerms` (2).
   - **New `aqXllTools` helpers** (added to `.h` + `.cpp`): `toAQLStringVector`,
     `toDoubleMatrix`, `toStandardStringMatrix`, `toVariantMatrix` (row-major,
     type-preserving, error cell → its text, blank → EMPTY Variant),
     `toVariantVector`, and a `toExcelMatrix( const etrading::VariantMatrix& )`
     overload (native Excel types out). File-local `toDoubleOr` / `addLvbBlock`
     / `addStringColumn` in `aqTool.cpp`.
   - **ValuationSettings orientation:** input read as-is (rows of key/value
     pairs) — matches the current `TestValuationSettings` GTEST contract
     (`{{ "CURVECOLLECTION", curveUSD3M }}`). The old XLL+ add-in force-
     transposed here but its own comment flagged that as a bug ("needs
     fixing!!!"); not carried.
   - `.arg()` count == parameter count verified for all 26 functions in
     `aqTool.cpp`.
4. `meUtilityLWODecorateNames` still deferred — no golden-source wrapper.

### Interpolation + PCA moved Tool → Math (2026-09-09) — BUILT GREEN, tests passing (Nicholas)

Nicholas: interpolation and PCA are `Math` building blocks, not `Tool`. Renamed
the golden-source wrappers and every surface that follows them:

- `validation`: `tryAqToolInterpolation` → **`tryAqMathInterpolation`**,
  `tryAqToolPCA` → **`tryAqMathPCA`**; files `tryAqTool{Interpolation,PCA}.{h,cpp}`
  → `tryAqMath{Interpolation,PCA}.{h,cpp}`; `projects/validation.vcxproj{,.filters}`.
- `AQ_API`: `aqToolInterpolation` / `aqToolPCA` → `aqMathInterpolation` /
  `aqMathPCA`; source files renamed; 4 SWIG `.i` (`#include` + `%include`);
  `projects/AQ_API.vcxproj{,.filters}`.
- `GTEST`: `TestUtilitiesInterpolation.cpp` call sites (12) + include. (Bespoke
  unit test — suite/case names left as-is; not a recording-generated test, and
  there are **no fixtures** for either function, so nothing else to move.)
- `AQ_XLL`: the two `XLO_FUNC` blocks moved out of `aqTool.cpp` into
  `aqMath.cpp` as `aqMathInterpolation` / `aqMathPCA`; `toDoubleOr` helper moved
  with them; `aqMath.cpp` gained a file-local `toIntOr` and `using namespace
  aq_xll;`.
- `docs/api_map.csv` regenerated — 118 rows, `aqMath{Interpolation,PCA}` now
  under `Math`. HARD GATE = 0. No `swig_*_wrap` touched.

### Open question raised — a generator-name listing function

Nicholas asked whether the old `.APPLES` add-in has a function to list the
generator names from the config folders (so users don't memorise them).
**Answer: no.** `.APPLES\...\MLIBQ_ADDIN` only has, per asset class,
`*GeneratorCreate` / `*GeneratorDisplay` / `*GeneratorModify` and
`*CreateFromGenerator` — `*GeneratorDisplay` dumps one *named* generator; nothing
enumerates the `resources/config/{BOND,CURVE,SWAP}_GENERATOR/*.JSON` names.
CLAUDE.md §5.1 already earmarks this as intended-but-unbuilt: the `Generator`
category = introspection only (`aqGeneratorList` / `Describe` / `Validate`).
Building it = a new `etrading` directory-scan + a `tryAqGeneratorList` wrapper +
XLL/API surface. Not started; flagged for Nicholas's go-ahead.

### Fixture-rename recovery + final cleanup (2026-09-09, uncommitted, NOT BUILT)

After Nicholas's build+GTest run flagged ~20 failures, root cause was a
fixture/​code-string boundary mismatch: the fixture sweep matched only names
**starting** with a function name (`b.startswith(old)`) while the code-string
rewrite used a **bounded-token** regex that also hit infixes like
`"..._tryAqObjSwaps..._inputs.csv"` — so prefixed fixture FILES kept old names
while their string LITERALS moved to new → "error opening file".

- **Reverted all `resources/test/` fixture moves to HEAD** (needed all three:
  `git checkout HEAD -- resources/test/`, `git reset HEAD -- resources/test/`,
  `git clean -fdq resources/test/inputs/` — `git add -A` had staged the renames
  as Add/Delete pairs).
- **Fixture rename take-3** — boundary `(?<![A-Za-z0-9])<old>(?![A-Za-z0-9])`
  (allows a leading `_`), driven by `pairs + apair` (both `tryAq*` and `aq*`
  names), skipping any result still containing a stale fragment:
  **7,551 renamed, 0 skipped, 0 `tryAqObj…` left**.
- **Relaxed code pass** — 69 GTEST/validation files: rewrote `_`-prefixed
  regex/scan/recording string fragments the first `(?<![A-Za-z0-9_])` pass had
  missed (e.g. `"_tryAqBondObjectPriceFromDiscountMargin_inputs.csv"` suffix
  constants).
- Verified `BOND1_tryAqBondObjectPrice_inputs.csv` and
  `EUR_FIXEDSCHEDULE_tryAqSwapObjectScheduleCreateBespoke_inputs.csv` now exist,
  matching the rewritten code strings.

**Residual function fixed (was the only real code-level miss):**
`tryAqObjCurvesDiscountFactorsForwardStartingFromTenor` (function *definition* at
`src/validation/src/tryAqCurveDiscountFactor.cpp:624`, missing from the map —
the map had the stateless `…Tenor` and the object-form plural `…Tenors` but not
the object-form singular) → **`tryAqCurveObjectDiscountFactorsForwardStartingFromTenor`**,
plus its `CreateDataFile`/`generatorFunction` labels. 0 fixtures / 0 GTEST readers
depended on the old name.

**Dead recording-label cleanup** (WRITE-side `CreateDataFile` / `decorateCurvename`
labels only — 0 committed fixtures, 0 GTEST readers, provenance not on the
test-read path; changed to golden names for internal consistency):
`tryAqObjCurvesCalibrate{Basis,OIS,Swap,FXForwards}` →
`tryAqCurveObjectCreate{Basis,OIS,Swap,FXForwards}` in the four
`tryAqCurveObjectCreate*.cpp`; `…DiscountFactorsForwardStartingFromTenor` labels
in `tryAqCurveObjectDiscountFactor.cpp` → `…Tenors` (match enclosing fn);
`tryAqObjSwapsDeltaLadderHorizontal` label in `tryAqSwapObjectDelta.cpp:395` →
`tryAqSwapObjectDeltaLadderHorizontal` (real fixtures already use the new name).

**Left as-is on purpose — `tryAqObjSwapsDV01*`** (18 fixtures on disk +
`TestAQObjSwapDelta{,XCCY_CSA,_JPY}.cpp` hardcoded path literals). File name and
GTEST read-string are mutually self-consistent, so the tests pass. Renaming would
mean an 18-fixture + 4-file lockstep move for zero functional gain — this is a
golden-source cleanup item for the Phase 6 resources audit, tracked, not a break.
Same disposition for the `tryAqObjCreditDefaultSwap{PVIntegration,PVMonteCarlo,
PVMonteCarloSobol}_outputs.csv` credit fixtures (stems that aren't function
names; self-consistent with unchanged test strings).

**Final no-build verification (2026-09-09):**
- `api_pair_check.py` **HARD GATE = 0**; 5 wrapper-name-drift advisories, all
  pre-existing (deferred set).
- Tree-wide code-level residual `tryAqObj{Curves,Bonds,Dates,Vols,OIS,Credits,
  Rates}*` (excluding `@brief`/comment text): **NONE**.
- ~9 `@brief` doc-comment mentions of `tryAqObjCurvesCalibration()` in GTEST
  headers — advisory comment cleanup only, no code effect.
- No `swig_*_wrap.*` staged. `docs/api_map.csv` regenerated (118 public / 467
  wrappers).

**4 failures assessed as pre-existing (NOT caused by the rename) — Nicholas to
confirm they also fail on clean HEAD:**
- `TestDatesCentralBank.UNIT_AreThereEnoughDates` — `totalDaysToLast: -1704`;
  stale ECB meeting-date data in `AQLMathCentralBank::meetingSchedule("ECB")`,
  no `tryAq*` involved.
- `TestAQObjCurve` + `TestExampleObject` — write to `.../resource/test/...`
  (singular); pre-existing FolderConfig path bug.
- `TestStructuredExceptionHandler.UNIT_IntegerDivideByZero` — SEH "throws
  nothing"; build/optimizer issue, no `tryAq*`.

**GATE — Nicholas: rebuild `validation` + `AQ_API` + `GTEST`, run GTest.**
Expect the ~15 fixture-mismatch failures gone; the 4 above expected to remain
(please confirm on clean HEAD). Then commit this batch.

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
| _(uncommitted)_ | **Step 11 — category naming scheme + AQ_XLL rebrand.** Categories now SINGULAR; handle marker moved from `aqObj<Category>` prefix to `aq<Category>Object<Fn>`; named sub-objects skip `Object` (`aqBondCurveYield`); lifecycle `aqObject<Lifecycle>`; `Vols`→`Volatility`. Docs: `CLAUDE.md` (both), `MIGRATION_PLAN.md` §2.2/§2.5/§2.6/D14/D15/D16/D18/Phase 3.2/3.5/4.7/4.11, `rebrand/STATUS.md`, `rebrand/tools/api_pair_check.py` (CATEGORIES + `category()`). **AQ_XLL code:** `git mv` `aqBonds/aqDates/aqObj/aqTools.cpp` → `aqBond/aqDate/aqObject/aqTool.cpp`; 53 functions renamed (42 bond + 4 date + 3 object + 4 tool); comments + `AQ_XLL.vcxproj`/`.filters` updated. Param/`.arg()` audit `aqBond.cpp` 42/42. `aqMath.cpp` / `aqMain.cpp` / `aqXllTools.{h,cpp}` unchanged names (Math already singular; XllTools is a utility, not a category). **`validation::tryAqObj…` call sites in AQ_XLL still use the OLD wrapper names** — temporary desync until MIGRATION_PLAN 2.6 renames `validation` / `AQ_API` / `GTEST` test names / fixtures. **NOT BUILT.** |
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
  `Vols`→`Volatility`. AQ_XLL done; rest pending.

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
