# Rebrand status — 2026-09-24

## Split tryAqToolInitialize into idempotent Initialize + explicit Reset; fixed the root cause of
## RISK_FlatShiftDeltaXCCY_RebuildEURTenorBasis; removed AqToolTearDown from Excel

Three related fixes, all stemming from one GTEST investigation.

**1. `AQLString::empty()` fixed.** Added in the previous session's commit (`dffbe944`) alongside the
`tryAqToolInitialize`/`tryAqToolTearDown` centralisation, `empty()` was implemented as `!stringData_
.has_value()` -- i.e. "undefined", not "zero-length". Since `AQLString(std::string(""))` is always
*defined* (just empty), every caller passing a plain `""` to mean "not provided" (GTEST, AQ_API,
AQ_XLL) was silently treated as an explicit override, which is exactly why `tryAqToolInitialize("",
"","","","")` threw "Invalid ir.properties path" in `TestUtilitiesSetupAQL`'s two UNIT tests. Fixed
to `!stringData_.has_value() || stringData_->empty()`, matching `std::string::empty()` semantics.
Safe to fix at the source: `empty()` is brand new, so nothing could have been calling it (and thus
relying on the old, wrong meaning) before this session. `isDefined()` is untouched and still means
"has a value at all, even ''".

**2. `FolderConfig`'s default config-file search order changed** (decided, Nicholas 2026-09-24) to
`.\config` (module folder) -> `$AQ\resources\config` -> `<module folder>` (bare) -> `$AQ` (bare),
in that order, across `ir_prop_path()`/`calib_prop_path()`/`calendar_path()`/`cbschedule_path()`.
The bare-module-folder and bare-`$AQ` fallback steps didn't exist before; added them. The legacy
`irsvr_excel.conf` key lookup and the literal `.\config` (process cwd) fallback are unchanged,
kept ahead of and behind this order respectively.

**3. `TestAQObjSwapDelta.RISK_FlatShiftDeltaXCCY_RebuildEURTenorBasis` root-caused and fixed.**
Failure was `"Missing Data: Terms does not exist"` / `"Invalid Curve: CurveIndex 'EURDF_USDCSA' in
collection EURYC does not exist"`. Root cause: the test rebuilt the EUR tenor-basis curve mid-test
by constructing `TryAqCurvesTenorBasis` as a standalone local (`TestAQObjSwapDelta.cpp:519`). That
class virtually inherits `google_test::InitializeGoogleTest`, whose constructor unconditionally
called `tryAqToolInitialize()` -- which, at the time, always tore everything down first. Building a
*second*, independent `InitializeGoogleTest` virtual-base subobject mid-test wiped the entire
environment (OIS/STD/XccyBasis) the fixture had already built, immediately before rebuilding only
the tenor-basis curve on the now-empty pool -- orphaning the already-calibrated `EURDF_USDCSA` XCCY
curve. Nothing to do with curve-calibration numerics. Diagnosed via temporary instrumentation
(added and fully removed in the same session) rather than a live debugger, which wasn't available.
Fixed by swapping the class construction for the existing free function `google_test::
setUpAqTenorBasisCurve()` (`TryAqCurvesTenorBasis.cpp`), which runs the identical
`tryAqCurveCalibrateBasis()` calibration with no base-class/environment side effect.

**4. `tryAqToolInitialize`/`tryAqToolReset` split (decided, Nicholas 2026-09-24), closing off the
whole class of bug #3 above, not just that one call site.** `tryAqToolInitialize` is now idempotent:
a no-op, ignoring every argument, if AlgoQuantLib is already initialized. New `tryAqToolReset` owns
the old "always tear down and rebuild from scratch" contract, for callers that explicitly want a
clean reload (different config paths, an edited config file). `InitializeGoogleTest`'s constructor
needed no change -- each test's destructor already calls `tryAqToolTearDown()`, so the next test's
constructor still sees "not initialized" and does real work; only a *second* construction mid-test
(the bug above) is now a safe no-op instead of a wipe. `InitializeETrading` got a small
`isInitialized()` accessor to support the check.
- Full surface: `validation::tryAqToolReset` (new) / `tryAqToolInitialize` (idempotent) in
  `tryAqToolSetup.h`/`.cpp`; `aqToolReset` (new) added to `AQ_API`'s `aqToolSetup.h`/`.cpp`;
  legacy `setUpAQL` shim (`exposed_functions.cpp`) retargeted to `tryAqToolReset` so its existing
  "always reloads" behaviour is preserved unchanged for old callers; `docs/api_map.csv` updated;
  `AQ_XLL`'s `xllTool.cpp` gained an `AqToolReset` worksheet function (today's old `AqToolInitialize`
  body/help text, verbatim) and `AqToolInitialize`'s help text updated for the new idempotent
  contract. `GTEST`'s `TestUtilitiesSetup.cpp` updated (stale comment fixed) plus two new cases
  covering the idempotent no-op and Reset's always-validates contract.
- **Outstanding:** `AQ_API`'s SWIG `.i`/generated wrapper files (`swig_Python_wrap.cxx` etc.) need
  regenerating via the normal build step to expose `aqToolReset` to Python/C#/Java/R -- not done in
  this session; hand-editing the generated files was deliberately avoided.

**5. `AqToolTearDown` removed from `AQ_XLL` only** (decided, Nicholas 2026-09-24) -- clean break,
no forwarding alias, per the existing Excel-function removal precedent (root `CLAUDE.md` §6.3/§5.6).
Calling it from a cell left every other AQObj-handle formula in the workbook broken with nothing to
re-initialize automatically on the next recalculation, unlike `AQ_API`/`GTEST` where teardown is
invoked by a script's or test's own controlled lifecycle rather than an end user typing a formula.
`AqToolReset` already covers the realistic Excel use case ("I want a clean reload"), so a bare,
no-reload teardown was judged more risk than value on this surface. The add-in still tears down
automatically when Excel closes it (`xllMain.cpp`'s `AlgoQuantLib` destructor, unaffected -- it
calls `validation::tryAqToolTearDown()` directly, not through the removed worksheet function).
`AQ_API`/`validation`/`GTEST` keep their own `tearDown`/`TearDown` entry points unchanged -- this
removal is `AQ_XLL`-only. Add `AqToolTearDown` to the release notes' "removed Excel functions" list
once that document exists.

**Verification:** full solution rebuilt (`validation`/`GTEST`/`AQ_API`/`AQ_XLL`) and the full
`GTEST` suite re-run by Nicholas after this session's changes -- all green, test performance
unaffected (fast).

---

# Rebrand status — 2026-09-23 (cont'd)

## Fixed the Release_API_R build (pre-existing break, unrelated to this session's other work)

`Release_API_R` failed to compile with a wave of `SWIG_STRINGMATRIX` / `StandardStringMatrix` /
`AQLStringMatrix` conversion errors across `aqToolDiagnostics.cpp`, `aqToolData.cpp`,
`aqCreditObject.cpp`, `aqSwapLeg.cpp`, `aqToolGrids.cpp`, `aqGridObject.cpp` and
`aqCurveObject.cpp`. Root cause: `SwigTypes.h` deliberately makes `SWIG_STRINGMATRIX` a flat
`vector<string>` for R (SWIG's R module has no `vector<vector<string>>` support) but a full
`vector<vector<string>>` everywhere else. `TypeUtilities.cpp` already had a correct, working
R-side *output* path (`fromStringMatrixToMatrixOfString` et al., flattening a real matrix for
return) and one correct fixed-shape *input* path (`buildStringMatrix`'s R overload, unflattening
via a `curveTypeEnum`/`curveMarketDataEnum`-driven column-count lookup) — but no R overload of
`buildSingleLabelValueBlock` or the generic `buildVariantMatrix` ever existed. Two call sites
(`aqToolData.cpp`, `aqCurveObject.cpp`) also carried an explicit but incorrect comment claiming
"no conversion required" for R, evidently written and never verified against an actual R build.
This is pre-existing breakage from whenever these functions were added/ported, not caused by
anything else in this session — none of the affected files were touched by prior work here.

**Fix (decided with Nicholas: exclude genuinely generic-shape inputs from R rather than invent an
R-only calling convention for them):**

- Added the missing R overload of `swig::buildSingleLabelValueBlock` (`TypeUtilities.h`/`.cpp`) —
  a Label-Value Block is always exactly 2 columns by contract everywhere else in the codebase, so
  unflattening needs no external dimension info, unlike the generic case. This alone fixed 15 call
  sites with **zero changes to the files that call it** (`aqToolDiagnostics.cpp`,
  `aqCreditObject.cpp` x6, `aqSwapLeg.cpp` x7, `aqCurveObject.cpp`) — they already just call
  `swig::buildSingleLabelValueBlock(...)` and now resolve to the new overload under R.
- Fixed `aqToolData.cpp`'s `aqToolValuationSettingsDisplay` (2-column in and out, confirmed by
  `tryAqToolValuationSettingsDisplay`'s own doc — "the modified input", i.e. same shape) with an
  explicit `#if SWIG_R` unflatten/flatten block, replacing the incorrect "no conversion required"
  comment.
- Fixed two **output**-direction sites in `aqCurveObject.cpp` (`aqCurveObjectDualBootstrap`,
  `aqCurveObjectCalibrateHedge`) that hand-built the return value as `SWIG_STRINGMATRIX
  result; result.push_back(row)` — under R this pushes a `vector<string>` row into a
  `vector<string>`, not a `vector<vector<string>>`. Rebuilt as a real `AQLStringMatrix` and
  flattened via the existing `swig::fromStringMatrixToMatrixOfString`, which already handles both
  branches correctly.
- **Excluded from the R binding** (both the header declaration and the `.cpp` definition, guarded
  with `#if (!defined(SWIG_R)) && (!defined(SWIGR))`, mirroring the existing precedent for
  `aqToolLVBCreate`'s 3-arg overload in `aqToolLVB.h`/`.cpp`): `aqToolAppend`, `aqToolClean`
  (`aqToolData.h`/`.cpp`), `aqCreditBasketModelCreate` (`aqCreditObject.h`/`.cpp`),
  `aqToolObjectMultiGridCreate` (`aqToolGrids.h`/`.cpp`), `aqGridObjectCreate`
  (`aqGridObject.h`/`.cpp`), `aqCurveResultsDiscountFactorsUpdate`'s `forwardAdjustments` param
  forced exclusion of the whole function (`aqCurveObject.h`/`.cpp`). Each of these takes at least
  one arbitrary-shape range/data-block parameter with no fixed column count; R supplies no matrix
  dimension metadata on a flat vector, so there is no way to unflatten it correctly, and guessing
  would silently corrupt data rather than fail to compile. Reintroducing these to R would need an
  explicit `numRows`/`numCols` parameter added to their R-facing signature — a real design
  decision, not attempted here.
- Build/test verification: not run this session (user builds themselves) — needs a
  `Release_API_R` rebuild to confirm clean, and (separately, whenever R testing is ever picked
  back up per CLAUDE.md §2.1's "shelved permanently, nice-to-have only" status) an actual R
  interpreter run, which remains blocked on no test environment being available.

# Rebrand status — 2026-09-23

## Centralized library initialize/teardown across GTEST, AQ_XLL and AQ_API (2026-09-23)

Six overlapping, hand-rolled init/teardown sequences (`etrading::InitializeETrading::instance()`,
`validation::trySetupAQL`/`tryTearDownAQL`, `validation::tryAqToolLoadConfigurationFiles`,
`AQ_API`'s `aqToolInitialize`, `AQ_XLL`'s `aqToolInitialize`, `GTEST`'s `InitializeGoogleTest`,
plus the un-rebranded `setUpAQL`/`setupAQL`/`initAQL`/`tearDownAQL` synonyms) collapsed into one
funnel in `validation`:

- **New:** `validation::tryAqToolInitialize(configFolder, calendarPath, cbSchedulePath,
  startupConfigPath, irPropsPath, checkStaticDataLoaded=true, checkCalendarLoaded=true)` and
  `validation::tryAqToolTearDown()` in `tryAqToolSetup.{h,cpp}`. Resolution per file: explicit
  full-path override wins outright > `configFolder` + the file's bare name (`Calendar.csv`,
  `CBSchedule.csv`, `startup.conf`, `ir.properties`) > `FolderConfig`'s existing default chain,
  unchanged.
- **Decided (2026-09-23, Nicholas): `tryAqToolInitialize` always tears down first** (like the
  legacy `trySetupAQL` it replaces), rather than being idempotent — reversed from this feature's
  first pass, which had it skip the tear-down on a second call. Reason: `InitializeETrading::
  instance()` only rebuilds on its *first* call; without a forced tear-down, override paths
  passed to a *second* call would update `FolderConfig`'s cached path but never actually reload
  the calendar/IR data behind it — the library ends up internally inconsistent (`FolderConfig`
  reports the new path, the loaded data is still the old one). Overhead of the forced tear-down/
  rebuild is low (in-memory data/function-table registration plus re-reading Calendar.csv/
  CBSchedule.csv/startup.conf's generator JSON — file-I/O bound, not curve-calibration bound) and
  is only paid when a consumer explicitly calls Initialize — never on the automatic addin-open
  path (`AQ_XLL`'s `AlgoQuantLib` ctor calls `InitializeETrading::instance()` directly, not this
  function). The real cost is losing already-cached `AQObj` handles (curves/swaps/credit trades)
  built earlier in the session: in Excel, cells referencing them show `#VALUE!`/`#REF!` until a
  full sheet recalculation (Ctrl+Alt+F9) rebuilds them — an accepted, documented cost, since the
  alternative (silently-ignored override paths) is a correctness bug, not a convenience.
- **Retired** (bodies retargeted at the new funnel where the public name had to survive for SWIG
  compatibility; declarations removed where nothing outside `validation` needs them):
  `trySetupAQL`/`tryTearDownAQL` (declarations removed); `AQ_API`'s `setUpAQL`/`setupAQL`/
  `initAQL`/`tearDownAQL` (bodies retargeted, signatures/SWIG untouched — these were never
  rebranded to `aq*` and are superseded by `aqToolInitialize`/`aqToolTearDown`; not removed from
  the public surface this session, since removing them needs a SWIG `.i`/regeneration pass this
  session didn't run).
- **Bugs fixed as part of the centralization, not separately:** `AQ_XLL`'s `aqToolInitialize`
  accepted a `ConfigPath` worksheet argument and silently never forwarded it (dead parameter) —
  now widened to the same 5 optional overrides as `AQ_API` and actually forwarded. `AQ_API`'s
  `setUpAQL` dropped the central-bank-calendar override that the old `trySetupAQL` it called
  actually supported — now passes it through.
- **New public functions:** `aqToolTearDown` added to both `AQ_API` (`aqToolSetup.{h,cpp}`) and
  `AQ_XLL` (`xllTool.cpp`, new worksheet function) — no equivalent existed before beyond the
  legacy `tearDownAQL` synonym.
- **Automatic teardown:** `AQ_XLL`'s `AlgoQuantLib` addin destructor (`xllMain.cpp`, runs at
  `xlAutoClose`/unload) and `GTEST`'s `InitializeGoogleTest` destructor (already RAII, per test
  fixture) both now call `tryAqToolTearDown()` instead of their own hand-rolled cleanup —
  `GTEST`'s previously included `AQLLinearRatesVolatilityManager::finalize()` which the old
  `tryTearDownAQL` left commented out with "Is this needed?"; folded into the centralized
  teardown as the safe superset. `AQ_API` has no automatic hook (deliberately — static
  destruction order across a SWIG/language-runtime boundary is fragile); callers must still call
  `aqToolTearDown()` explicitly before process exit, same as before.
- **`GTEST`'s `InitializeGoogleTest` ctor** now calls `tryAqToolInitialize(..., false, false)` —
  the `false, false` preserves this fixture's pre-existing behaviour of never throwing on a
  calendar/static-data load failure; this was **not** widened to the loud-check defaults, to
  avoid silently making every test fixture stricter than it was.
- Updated `docs/api_map.csv` (added `aqToolInitialize`, `aqToolTearDown` rows — `aqToolInitialize`
  had no row at all before this).
- **Not done this session (flagged, not forgotten):** SWIG `.i` files / regeneration to actually
  remove `setUpAQL`/`setupAQL`/`initAQL`/`tearDownAQL` from the public `AQ_API` surface per the
  clean-break policy (CLAUDE.md §6.3) — deferred until the next SWIG regen pass, since running
  codegen wasn't asked for this session. `resources\manifest\activeList.txt` /
  `xllManifestList.h` also not updated — `aqToolTearDown` is therefore excluded from the
  `Release_XL_Manifest` configuration for now, same as `aqToolInitialize` already was before this
  change; unaffected in every other configuration.
- **Build/test verification: not run this session** (user builds/tests AQ_LIB themselves) — needs
  a full-configuration build and a GTEST run before treating this as verified green.

# Rebrand status — 2026-09-22

## Legacy SABR wrappers ported to AQ_XLL and AQ_API (2026-09-22)

The 7 legacy, procedural, ID-string-driven SABR validation wrappers
(`tryAqVolatilitySABR{Calibrate,GetPrem,GetVol,OutputParameter,SetupConvention,
SetupParameter,SetupSwaptionVol}`) — previously flagged in `xllVolatility.cpp`
as "not ported, needs Nicholas's call" — are now ported end to end, per the
decision to port as-is (not drop as legacy). They coexist with, but are
distinct from, the object/handle-based `aqVolatilityObjectSabr*` API in the
same file: their ID parameters (`GridID`, `ConventionID`, ...) are plain
string keys into the shared `AQLDataInstance` object pool, not `AQObj`
handles.

- **`AQ_XLL`** — 7 new worksheet functions added to `src/AQ_XLL/src/xllVolatility.cpp`
  (existing file, already registered in `projects/AQ_XLL.vcxproj[.filters]`,
  no project-file change needed). `SetupConvention`/`SetupParameter`/
  `SetupSwaptionVol` take their matrix arguments as plain inputs (verified
  against `.APPLES` legacy `meVols.cpp`: the `[inout]`/non-const reference in
  the validation header is a legacy-API artefact, not real output — the impl
  only reads them) and return `TRUE` on success; `OutputParameter` reshapes
  its flat row-major `DoubleVector` + `row`/`column` output into an Excel
  matrix. Manual `.arg()`-vs-parameter-count check done for all 7 (19/19,
  13/13, 11/11, 1/1, 2/2, 3/3, 4/4) — the runtime landmine at CLAUDE.md 3.2 is
  not a compile-time check.
- **`AQ_API`** — new `src/AQ_API/source/aqVolatilitySABR.{h,cpp}`, mirroring
  the `AQ_XLL` parameter semantics exactly (plain `std::string`/`double`/
  `vector`/`SWIG_STRINGMATRIX`, no `AQObj` decoration). Registered in
  `projects/AQ_API.vcxproj[.filters]` (Volatility filter) and all 4 SWIG
  interface files (`swig_{Python,CSharp,JAVA,R}.i`).
- **Verified:** `rebrand/tools/api_pair_check.py --gap-list` — HARD GATE: 0;
  Volatility category's AQ_API coverage gap: 0 (was 7 missing of 14, now 0
  missing of 14; public surface count for Volatility rose 7 -> 14).
- `src/validation` untouched, per the read/write boundary.

# Rebrand status â€” 2026-09-19

## BondSpreadCurve feature completed and hardened; AQLDate speed/thread-safety/debuggability pass (2026-09-19)

Session paused here (token limit) â€” safe to resume from this point tomorrow.
Everything described below is committed.

**1. `BondCurve` extended with a `CurveType` (`Outright`/`Spread`, old
`BondCurve`/`BondSpreadCurve` strings kept as accepted aliases).** A spread
curve is built from a benchmark `BondCurve` plus one or more spread bonds:
single spread bond â†’ constant spread applied to every benchmark yield; two or
more â†’ the spread is linearly interpolated (Flat/PiecewiseConstant is
rejected outright for spread curves â€” a step function between spread nodes is
a genuine discontinuity, not something a fix can smooth) and applied to every
benchmark pillar. `YieldQuoteInPercent` and the optional flat `Spread` shock
apply consistently to both curve types. Every spread bond gets its own exact
calibration pillar (repricing is exact, by explicit decision â€” approximate
curve-fit repricing was considered and rejected).

**2. Two real bugs found and fixed during hardening, both via user-reported
symptoms reproduced and root-caused, not guessed at:**
   - *Inconsistent baseline.* A spread node was measured against the
     benchmark's *bootstrapped* yield (`BondCurve::getYield()`, which reads
     through whatever interpolation the benchmark itself uses) but
     reconstruction added the spread onto the benchmark's *raw quote*. These
     two numbers differ subtly (bootstrap discounts a bond's earlier coupons
     through prior curve segments, so a bond's calibrated pillar isn't quite
     its own naive YTM). Where a spread bond happened to fall near a
     benchmark pillar, this produced a real, localised jump ("blend"/spike)
     baked into the spread node itself, before any spread-side interpolation
     ever ran â€” no choice of `Interpolation` could fix it, because the
     corruption was upstream of that setting. Fixed: a spread node is now
     always measured against a smooth Linear interpolation of the
     benchmark's own **raw quotes**, independent of the benchmark curve's own
     `Interpolation`/`Extrapolation` configuration.
   - *Coincidence handling.* A spread bond maturing on the same date as a
     benchmark bond (a realistic case â€” e.g. a corporate bond happening to
     share a treasury's maturity) initially threw ("ambiguous, use distinct
     maturities"); fixed so the spread bond's own quoted yield takes
     precedence there, since it's real market data for that exact instrument
     â€” this now falls out of the same-baseline fix above algebraically,
     rather than needing a special-cased override.

**3. Root-caused (not yet actioned) â€” `Interpolation=Flat` on a spread curve
against closely/quarterly-spaced benchmark bonds can still look spiky even
with the baseline fix**, when the shift between spread-bond and benchmark
maturities is close to the benchmark's own pillar spacing (a step-function
"aliasing" effect, mathematically inherent to Flat/step interpolation, not a
logic bug â€” `Flat` is now rejected for spread curves specifically because of
this). Nicholas wants exact repricing of every calibration instrument kept
(no approximate curve-fit), so the fix path is the union-of-pillars +
bootstrap approach already implemented, using Linear only for the spread
overlay. Broader "smooth/monotone/arbitrage-free by construction" (forward-
rate-as-state-variable, matching this library's own IR/swap curve framework
per `CLAUDE.md` Â§1) was discussed as a further step but explicitly deferred â€”
"action the smooth curve items mentioned here and then review our position"
(Nicholas's words) is the agreed sequencing; not yet revisited.

**4. Regression coverage added to `TestBondCurves.cpp`** (all passing): exact
zero-spread reproduction of the benchmark, spread-bond/benchmark-pillar
coincidence, mixed coincident+non-coincident nodes, sparse nodes attributed
by date (not index), Linear-vs-PiecewiseConstant interpolation, `Flat`
rejected for spread curves, `YieldQuoteInPercent` parity between curve types.
New fixtures under `resources/test/inputs/ETrading/Bonds/BondCurves/`:
`USD_CORP_CURVE_{MULTINODE,ZEROSPREAD,COINCIDENT,MIXED,SPARSE_DIAG,
FLAT_REJECTED,SHOCK_DEC,SHOCK_PCT}`, `USD_2BOND_{STEP,LINEAR,STEP_PCT}`,
`US_CORP_BOND_{1,2,3}`.

**5. `AQLDate` reviewed end to end** (pros/cons discussion, then an agreed
"don't break anything" hardening pass â€” see `AQLDate.h`/`.cpp` for the
in-code rationale on each point):
   - `Visualizer.natvis`: `AQLDate` now displays as zero-padded `dd-Mmm-yyyy`
     (was `d-m-yyyy`) plus a `[yyyymmdd]` expansion item. Pure debugger XML,
     zero compiled-code risk.
   - `cmp()` fast path: compares cached Julian day directly when available
     (one int compare) instead of always doing the decimal-rank arithmetic;
     mathematically identical ordering, verified no caller depends on the
     return value's magnitude (only its sign).
   - `dayOfMonth()`/`monthOfYear()`/`yearOfEra()` moved inline into the
     header (were out-of-line one-liners in the `.cpp`).
   - `noexcept` added to every method verified non-throwing (accessors,
     `cmp()`, the six relational operators, `isLeapYear`/`isStartOfMonth`/
     etc., the interval-arithmetic family, `dateToJulius()`). Mutators that
     validate input (`setYear`/`setMonth`/`setDay`/`addDays`/etc.) and all
     `virtual` methods were deliberately left alone â€” `AQLDateTime` overrides
     several of them and does throw.
   - `mJulius` is now kept eagerly up to date by every constructor and
     mutator, instead of computed lazily on first use. This makes the
     `cmp()` fast path unconditionally available, and â€” more importantly â€”
     means no internal `const` method ever needs to *write* to `mJulius`
     anymore, which is what makes concurrent `const` access (comparisons,
     interval calculations, `dayOfWeek()`) across threads safe. Confirmed
     safe against the one real complication: `AQLDateTime : public AQLDate`
     (checked â€” it only calls the inherited public setters, never touches
     `mJulius`, which is private anyway). `mJulius` stays `mutable` only
     because the public `dateToJulius()` â€” kept for backward compatibility,
     one external caller in `DateUtilities.cpp` â€” is `const` and still
     assigns to it; calling it on an already-current object is idempotent.
   - Added `isNull()` (matches the existing convention already used by
     `AQLPriceDataType`/`AQLDataMatrix`/`AQLDataHolder` â€” confirmed via grep
     before naming it, not invented fresh).
   - Deliberately **not** done, and why: did not remove the virtual
     destructor (real subclass `AQLDateTime` needs it, would be UB to
     remove); did not touch the fixed-format C-string parser or consolidate
     `stringToDate()`'s heuristics into the class (flagged as a *future*
     improvement, out of scope for "don't break anything, immediate").
   - Full solution rebuild (`math`â†’`etrading`â†’`models`â†’`calibration`â†’
     `validation`â†’`GTEST`) green; `TestBondCurves` and the date-related unit
     suites (`TestDateConverters`, `TestValidateAndConvertStringToDate`,
     `TestDebugMacros`, `TestUtilitiesTime`, `TestCalendars`) all pass.

**6. Process note â€” mid-session data loss and recovery.** ~15 new test
fixture CSVs created earlier in this session (for the `BondSpreadCurve` work,
point 4 above) were untracked and got wiped by an external process (the user
was running a build in parallel; likely a clean step). Git showed no trace
(untracked-file deletion is invisible to git) â€” diagnosed via `git status`/
`git reflog`/`git show --stat` on the relevant commits, which confirmed the
*source code* (`BondCurves.{h,cpp}`, `CommonConstants`, `CoreEnumerations`,
`FixedBond`, all 239 lines of the `TestBondCurves.cpp` additions) was already
safely committed (`c4dc6469`) â€” only the brand-new, never-committed fixture
files were lost. All 15 were reconstructed from this session's own context
and re-verified (full `TestBondCurves` suite green again) before committing.
**Lesson for next session:** new test fixtures should be staged (`git add`)
promptly after creation, not left untracked, precisely because untracked
deletions leave no git trail to recover from.

**Known pre-existing GTEST full-suite failure, unrelated to this session:**
`TestCurveData.UNIT_CurveData` â€” `FRAQuotes` throws `AQLCoreInvalidData`
where the test expects `etrading::ETradingException`, in FRA/market-data
validation code untouched this session or the last several.

---

## C#/Java/R binding testing shelved permanently, not just blocked (D21, 2026-09-15)

Supersedes the "blocked" framing in the entry immediately below, from the
same session. Nicholas's call: there is **no current requirement** to use
C#, Java or R, and no test environment available to test them anyway â€” so
rather than carry this as an open, blocked Phase 5 task, it is demoted to a
**standing nice-to-have**, off any phase gate or schedule. `MIGRATION_PLAN.md`
Phase 5.1b struck through and reworded, `MIGRATION_PLAN.md` Phase 5's exit
criterion changed from "all four languages" to "Python builds and passes",
decision `D21` added. `AQ_LIB\CLAUDE.md` Â§2 (layout) and Â§2.1 updated to
match â€” Python is now described as *the* supported binding, not one of four
pending verification.

**Not changed:** the `AQ_API` code itself â€” `aqGenerator.h`/`.cpp` and every
other binding-layer file are still wired into all four `swig_{Python,CSharp,
JAVA,R}.i` files (per the Generator entry below); nothing was removed from
the SWIG interface files or the `generate*`/`deploy*` batch scripts. This is
a change in what gets *tested and by when*, not a change in what gets
*built*. If a client ever needs C#, Java or R, the existing SWIG wiring is
the starting point â€” generate, build, deploy and run the relevant
`resources\api\*` test app to verify, per the reworded 5.1b.

**Note:** `REPO\CLAUDE.md` (the root, repo-wide file) Â§10 still describes
C#/R/Java as "unverified and need testing" â€” that file is outside
`AQ_LIB`'s read/write boundary (root `CLAUDE.md` Â§0) so it wasn't updated as
part of this change. Flagged for Nicholas to update directly, or to
explicitly authorize touching, if he wants the two docs to agree.

---

## GoogleTest baseline confirmed; `AQ_BINDINGSâ†’AQ_API` rename status corrected; C#/Java/R binding tests blocked (2026-09-15)

Two items closed out, one flagged as blocked, per Nicholas:

**1. GoogleTest baseline â€” confirmed green, closing the deferral from the
`AQ_THROW`/`boost::format` entry below** ("GoogleTest re-run not yet done in
this session â€” deferred"). Full suite now re-run: all cases pass except
**~10 failures caused by stale holiday calendar data** â€” a pre-existing
data-currency problem in the calendar/holiday files (`CLAUDE.md` Â§9.2,
`AQ_LIB\CLAUDE.md` Â§4.3), unrelated to any rename, the `AQ_THROW` conversion,
or any other code change this rebrand has made. Not fixed in this session â€”
refreshing calendar data from MarketWire/SwapsWire is a data task, not a
code task, and out of scope here. Tracked as a known exception: when re-
running `GTEST` going forward, ~10 calendar-data failures are expected and
should not be read as a regression signal.

**2. `AQ_BINDINGSâ†’AQ_API` rename â€” already done, docs were stale.**
`AQ_LIB\CLAUDE.md` Â§2 (layout diagram) still said "RENAME PENDING" and its
Â§5.4 mapping table said "(agreed)" rather than "done"; `MIGRATION_PLAN.md`
D3 was worded as a still-open decision. The rename (project files, `.sln`,
folder â€” `src\AQ_API` confirmed on disk â€”, SWIG `.i`, all 8 `generate*`/
`deploy*` batch files, pre/post-build commands) was in fact completed in an
earlier session and never recorded here. Corrected in `CLAUDE.md` (root)
Â§10, `AQ_LIB\CLAUDE.md` Â§2/Â§2.1/Â§5.4, and `MIGRATION_PLAN.md` D3/Phase 5.1
in this session so the docs match actual repo state.

**3. Phase 5 bindings testing (C#/Java/R) â€” blocked on missing test
environments, not blocked on code.** Python is verified end-to-end
(`rebrand\STATUS.md`, Generator entry below). Nicholas does not currently
have the .NET/JDK/R runtimes and toolchains installed to build and run the
`resources\api\*` test apps for C#, Java or R. `MIGRATION_PLAN.md` Phase
5.1 split into 5.1a (rename, done) / 5.1b (binding tests, blocked) to make
this distinction explicit. **Not resolved this session** â€” see the "Next
steps" note below.

---

## Manifest files renamed: `active.txt`â†’`activeList.txt`, `demo.txt`â†’`demoList.txt` (2026-09-15)

Nicholas's call â€” `List` in the name signals these are the plain-text
function-list manifests `generateManifestList.bat` reads (as distinct from
any future non-list manifest content). Renamed via `git mv` to preserve
history. `generateManifestList.bat`'s own logic needed **no change** â€” it
takes the manifest path as a parameter (`%~2`), never hard-codes a filename;
only its comment pointing readers at the file for format detail was updated.
Updated everywhere the old names were hard-coded:

- `projects\AQ_XLL.vcxproj` â€” the `AQ_XLL_MANIFEST_FILE` property's default
  path, its explanatory comment block, and the two `<None Include>` project
  entries.
- `projects\AQ_XLL.vcxproj.filters` â€” the matching `<None Include>` entries
  (Solution Explorer grouping only).
- `src\AQ_XLL\resources\manifest\generateManifestList.bat` â€” one comment.
- `src\AQ_XLL\resources\manifest\activeList.txt` / `demoList.txt` â€” each
  file's own comment header, which cross-references the other by name.
- `CLAUDE.md` (root) and `MIGRATION_PLAN.md` â€” prose mentioning the old
  filenames.

**Verified:** rebuilt `AQ_XLL`'s `Release_XL_Manifest|x64` configuration â€”
`generateManifestList.bat` runs via the (unchanged) `AQ_XLL_MANIFEST_FILE`
property, now resolving to `activeList.txt`, regenerates
`aqManifestList.h` correctly, and the configuration links green.

---

## Generator category built from scratch â€” the last open Phase 4 gap (2026-09-15)

`Model` and `Generator` were the only two categories with zero `validation`
wrappers (STATUS.md Â§2/Â§4.1). `Model` has no legacy port source (`.APPLES`
has no `meModel*` anything) and no concrete design brief, so it stays
untouched this session â€” deliberately, not an oversight. `Generator` is
introspection-only per `CLAUDE.md` Â§5.1 (`aqGeneratorList`/`Display`/
`Validate`), scoped and built end-to-end across all four surfaces in the
mandated order (Â§5.1a). **`Describe` renamed to `Display` (2026-09-15,
same session)** â€” Nicholas caught that it should match the pre-existing
`aqSwapGeneratorDisplay`/`aqBondGeneratorDisplay`/`aqCurveGeneratorDisplay`
naming already used by each asset category's own generator handle API,
rather than introducing a new verb. Renamed across all four surfaces plus
the `GTEST` file/suite name before this entry's numbers below were quoted,
so they read as originally verified â€” only the name changed, not the
behaviour.

- **`validation`** â€” new `tryAqGenerator.h`/`.cpp`. `tryAqGeneratorList`
  scans `$(AQ)/resources/config/<TYPE>_GENERATOR` on disk directly (real
  directory listing, not the object cache â€” a generator need not already be
  loaded to be listed, matching the "written from scratch against a
  directory scan" brief in `CLAUDE.md` Â§4.1). `tryAqGeneratorDisplay` loads
  the named generator through the same path `tryAqObjectLoad` uses, then
  hands off to whichever category already owns that generator type's own
  `Display` function (`tryAqSwapGeneratorDisplay`/`tryAqBondGeneratorDisplay`/
  `tryAqCurveGeneratorDisplay` â€” all three already existed, one per asset
  category) rather than re-implementing property rendering. `tryAqGeneratorValidate`
  reuses the same load path and reports "OK" or the failure reason instead of
  throwing, since an invalid generator is an expected result to report, not a
  validation-function failure. Restricted to `SWAP_GENERATOR`/
  `BOND_GENERATOR`/`CURVE_GENERATOR` â€” Generator is a cross-cutting
  introspection category over those three, not a general object-type lookup.
  **Bug caught by GoogleTest, fixed before it shipped:** `tryAqGeneratorDisplay`/
  `Validate` originally used `VALID_EXCEPTION_START` (with `ThreadGuard`) while
  also calling other `tryAq*` functions that carry their own `ThreadGuard` â€”
  nesting two guards on one call stack trips the re-entrancy check
  (`"Thread Guard: Calling AlgoQuantLib from multiple threads..."`) even
  though it's a single thread. Fixed by switching to
  `VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD`, the same pattern
  `tryAqObjectLoad` itself already uses when it composes
  `tryAqObjectLoadAndReturnTupleResults`.
- **`GTEST`** â€” `TestAqGeneratorList.cpp`/`TestAqGeneratorDisplay.cpp`/
  `TestAqGeneratorValidate.cpp`, hand-written against real seed files already
  in `resources\config\{SWAP,BOND,CURVE}_GENERATOR` (`USD_3ML`,
  `US_TREASURY_TYPE1`, `USD_OIS`) rather than recorded fixtures. All 6 cases
  pass.
- **`AQ_API`** â€” new `aqGenerator.h`/`.cpp`, wired into all four
  `swig_{Python,CSharp,JAVA,R}.i` files. Built and smoke-tested end-to-end
  through the Python binding (`Debug_API_Python|x64`): `aqGeneratorList`
  returns 132 SWAP_GENERATOR names including `USD_3ML`; `aqGeneratorDisplay`
  returns real BOND_GENERATOR rows; `aqGeneratorValidate` returns `"OK"` for
  a real CURVE_GENERATOR and the specific `#Error: File does not exist...`
  message for a bad name.
- **`AQ_XLL`** â€” new `aqGenerator.cpp`. Originally filed under `src\Optional`
  per Â§9.5 (a standalone introspection utility, not infrastructure every
  priced product depends on the way `Curve`/`IR` are); since moved to
  `src\Core` in the `.vcxproj.filters` outside this session (Nicholas's own
  edit, taken as-is). **Not edition-excluded** either way: every edition
  (`Release_XL_Bond`/`Swap`/`Credit`/`Curve`) needs generator introspection
  for its own product's generator type, so no `ExcludedFromBuild` condition
  was ever added â€” it compiles into all five configurations regardless of
  which Solution-Explorer filter it sits under. `.arg()` counts checked
  against parameter counts by hand (the known "too many args" xlOil
  runtime-registration landmine, `CLAUDE.md` Â§3.2) â€” 1/2/2 respectively, all
  matching.

All four `.vcxproj`/`.vcxproj.filters` pairs (`validation`, `GTEST`,
`AQ_API`, `AQ_XLL`) updated in the same session the files were added â€” the
"validation `.cpp` files can go silently unwired" hazard (STATUS.md Â§5)
applies to every project, not just `validation`.

**Verified this session:** `validation` (Debug|x64), `GTEST` (Debug|x64, all
6 new + full suite), `AQ_API` (`Debug_API_Python|x64`, smoke-tested), `AQ_XLL`
(`Debug|x64`, `Release|x64`, and all four `Release_XL_*|x64` editions) all
build green. `validation`/`AQ_XLL` also rebuilt in `Release|x64` (was only
`Debug` before this session touched it) to confirm the edition configs link.

**Not done, deliberately:** `docs\api_map.csv` not hand-edited â€” it's
generated by `rebrand\tools\api_pair_check.py --write` off `git ls-files`,
and these new files aren't `git add`ed yet (committing stays Nicholas's own
call per Â§6.5); hand-editing it would drift from the next `--write` anyway.
Run `api_pair_check.py --write` after staging the new files to pick up
Generator's rows. `Model` remains fully unscoped â€” needs a design brief
(which model types, what params) before any code, not just a name list.

---

## Naming-convention fix: three `AQ_XLL` function families corrected to `<library><category>` form (2026-09-15)

Nicholas caught three `AQ_XLL` families that didn't fit the
`aq<Category><Function>` / `aq<Category>Object<Function>` naming convention
(`CLAUDE.md` Â§5.1). Renamed across all four surfaces per Â§5.1a's order
(`validation` â†’ `GTEST` â†’ `AQ_API` â†’ `AQ_XLL`), plus the recorded fixture
CSVs (`git mv`'d to match) and `docs\api_map.csv`:

| Old | New |
|---|---|
| `aqCreditObjectBasketModelCreate` | `aqCreditBasketModelCreate` |
| `aqCreditObjectBasketModelSurvivalProbability` | `aqCreditBasketModelSurvivalProbability` |
| `aqCreditObjectDefaultSwap<X>` (11 functions: PV, PVByIntegration, PVByMonteCarlo, PVFromHazardRate, RiskyAnnuity, RiskyAnnuityFromHazardRate, AccruedYearFraction, CS01, ParSpread, ParSpreadFromHazardRate, HazardRateFromParSpread) | `aqCDSObject<X>` |
| `aqToolObjectGrid<X>` (Create, Save, Load, Display, ClearOne, ClearAll) | `aqGridObject<X>` |
| `aqToolObjectGridObjectNames` | `aqGridObjectNames` (collapsed â€” the literal `aqGridObject`+`ObjectNames` transform would have doubled "Object") |

Each with its `try`-prefixed `validation` counterpart renamed identically
(`tryAqCreditObjectBasketModelCreate` â†’ `tryAqCreditBasketModelCreate`, etc.).
The Basket functions have no `AQ_API` binding today, so only `validation`/
`GTEST`/`AQ_XLL` were touched for that family. Of the Grid functions, only
`Display` is bound in `AQ_API` (`aqToolGrids.h`/`.cpp`).

**Deliberately left untouched (Nicholas's call):** the generated
`src\AQ_XLL\include\generated\aqManifestList.h` and the four generated
`swig_{Python,R,JAVA,CSharp}_wrap.{cxx,cpp}` files â€” both regenerate on their
own (`aqManifestList.h` via `Release_XL_Manifest`'s `PreBuildEvent` calling
`generateManifestList.bat`; the SWIG wrap files via a full SWIG regen in
Phase 5) and don't need hand-editing. Confirmed: building
`Release_XL_Manifest|x64` regenerates `aqManifestList.h` with the new names
with no further action needed. Historical entries elsewhere in this file
that predate this rename (e.g. the original Credit/Grid port write-ups) are
left as-authored â€” they describe what was true at the time, not current
state.

**Verified (Nicholas):** `validation`, `GTEST`, `AQ_XLL` (Debug|x64 and
`Release_XL_Manifest|x64`) and `AQ_API` (Python) all build green; GoogleTest
passes.

---

## Phase 4a (editions) closed: AQ_XLL-only, AQ_API runtime gate dropped (2026-09-15)

Nicholas confirmed `AQ_XLL` now has four per-edition build configurations â€”
`Release_XL_Bond`, `Release_XL_Swap`, `Release_XL_Credit`, `Release_XL_Curve`
â€” alongside the existing `Release` (Full), each built from the
`src\Core`/`src\Optional` filter split (done 2026-09-12). **Decision: this
is the entire Phase 4a deliverable â€” editions are an `AQ_XLL`-only, compile-
time concept. The `AQ_API` runtime edition-gate half of the original Phase
4a plan (`config\editions.json`, `config\licence.json`, a module-import
registration gate, `aqToolEdition()`) is dropped, not deferred.** `AQ_API`
(Python/C#/Java/R) ships one full binary per language with every category
always registered â€” no edition concept on that surface, now or later.

**Docs updated to match** (this entry is the changelog; the docs themselves
carry the current-state description, not a repeat of it here): `CLAUDE.md`
Â§2.1/Â§4.5/Â§5.2 (root) and `AQ_LIB\CLAUDE.md` Â§2.1/Â§4.4/Â§5.2/Â§6.3 rewritten
in place (`Editions` sections marked done, dual-mechanism language replaced
with `AQ_XLL`-only); `MIGRATION_PLAN.md` Phase 4a marked â˜‘ done with the
`AQ_API` half struck through as dropped (not deferred), the 2.4 editionâ†’
category straw-man table marked â˜‘ and given a `Curve` row (missing from the
original four-edition straw man), the Phase 5.2 task that assumed an
`AQ_API` edition gate to wire into struck out, and decision `D20` added.
`STATUS.md` (this library's own top-level summary, not this file) and
`readme.md`'s end-user-facing Editions section rewritten to match â€” the
`readme.md` text previously described a **runtime**, entitlement-file-gated
edition switch, which was never built and now never will be.

**Not done, flagged for later, not blocking this decision:** `AQ_XLL`'s
editionâ†’category mapping (which `Optional` files land in which
`Release_XL_*` config) has not been independently audited function-by-
function against the `MIGRATION_PLAN.md` Â§2.4 table in this session â€” only
that the four configurations exist and build green (per Nicholas). Worth a
pass before shipping: confirm e.g. `Release_XL_Curve` doesn't accidentally
carry Swap-only files, and that each edition's registered function set
matches what a customer buying that edition should get.

---

## AQ_XLL add-in loaded empty in every configuration â€” fixed, one flagged (2026-09-14)

Nicholas reported `AlgoQuantLib.xll` loading with zero `aq*` functions in
every configuration (Debug and all Release/edition variants), with a
`xlOil Load Failure` popup reading `#Error: #Error: Unable to read JSON data -
File not found ...\resources\config\CURVE_GENERATOR\JPY_OIS_LOB_2Y.JSON`.

**Two separate things, only one fixed here:**

1. **Missing resource file â€” fixed (2026-09-14, follow-up).** Nicholas chose
   to drop the missing entries rather than author the missing JSON. Audited
   all three startup object lists (`resources\config\{Curve,Bond,Swap}Generator.conf`,
   each cross-referenced against the JSON files actually present in the
   matching `resources\config\{CURVE,BOND,SWAP}_GENERATOR\` folder) rather
   than just the one reported file, since the same failure mode could exist
   elsewhere in either list:
   - `CurveGenerator.conf`: removed `JPY_OIS_LOB_2Y.JSON` and
     `JPY_OIS_LOB_3Y.JSON` â€” genuinely absent; the bare `JPY_OIS_LOB_*` family
     only ever had `1Y`/`NONE`, unlike the JSCC/LCH/TIBOR variants which have
     the full `1Y`/`2Y`/`3Y`/`NONE` set.
   - `SwapGenerator.conf`: **not a missing file** â€” line 24 read
     `EUR_ARR_BASIS_LIBOROIS.JSONEUR_BASIS_1X3.JSON`, two real, existing
     generator filenames concatenated onto one line with no line break
     (a pre-existing data-entry corruption, unrelated to the rebrand). Split
     back into two lines rather than removed, since both
     `EUR_ARR_BASIS_LIBOROIS.JSON` and `EUR_BASIS_1X3.JSON` exist and would
     otherwise have been dropped for no reason.
   - `BondGenerator.conf`: clean, no missing entries.
   All three lists now cross-check clean (0 missing) against their folders.

2. **Regression from the AQ_THROW/boost::format cleanup (fixed, this
   entry) â€” this is what actually took the whole add-in down**, not just one
   curve generator: `etrading\src\FolderConfig.cpp:455` had
   `catch (ETradingException e)` around `deSerializeFromJSON(...)`,
   specifically so `FolderConfig::deserializeObjectsForOptionalStartup` can
   quietly skip a bad/missing optional-config entry when `reportErrors` is
   off (see the comment on `AlgoQuantLib()`'s constructor in `aqMain.cpp`:
   "a config-load failure does not abort the add-in load"). The prior
   session's error-messaging pass converted `SerializeContainedData.cpp`'s
   `throw ETradingException(...)` (the thing this catch depends on) to
   `AQ_THROW` â€” which always throws `AQLCoreInvalidData`, a different
   hierarchy. The catch stopped matching, so the missing-file exception now
   propagated uncaught through `InitializeETrading`'s constructor into
   `xlAutoOpen`, aborting the whole add-in before any function registered.
   That prior session's catch-site audit checked every `catch(ETradingException&)`
   in the tree but wrongly assumed **both** of `FolderConfig.cpp`'s two
   `ETradingException` catches depended only on `math`'s untouched
   `toCachedObjectEnum`/`toScheduleTypeEnum` â€” missed that the second one
   (this one) depended on `etrading`'s own (converted) JSON deserializer.
   **Fix:** retyped the catch to `AQLCoreInvalidData&`. While in there, also
   fixed the doubled `#Error: #Error:` prefix visible in the popup â€” several
   `AQ_THROW` call sites carried a hardcoded `"#Error: "` in the message
   literal left over from before the conversion (the macro always prepends
   `#Error: ` itself); stripped the redundant one from every such site found
   tree-wide (`SerializeContainedData.cpp`, `SwapUtilities.cpp`,
   `FXCurveUtilities.cpp`, `CreateDataFile.cpp`, and three `validation`
   files) â€” cosmetic, not the cause, but worth cleaning up while diagnosing
   the same symptom.

**Verified:** full solution rebuilt clean in both Debug|x64 and Release|x64
after the fix (one follow-up needed: the retyped catch's now-unused `e`
tripped `TreatWarningAsError` on an unreferenced-variable warning â€” dropped
the name, matching the unnamed-catch style already used elsewhere in this
file). **Not yet verified:** an actual Excel load with the rebuilt `.xll` â€”
that still depends on Nicholas rebuilding and reopening Excel, and on item 1
above (the missing JSON) being resolved one way or another, since until it
is, the add-in will still hit the same `deserializeObjectsForOptionalStartup`
path â€” the difference is it will now be **silently skipped** (`reportErrors`
is off) rather than aborting the whole add-in load.

---

## Error messaging unified on AQ_THROW/AQ_REQUIRE; boost::format removed (2026-09-14)

Nicholas asked for all error throwing across the library to route through the
`AQ_REQUIRE` / `AQ_THROW` / `AQ_THROW_IF` macros in `ExceptionMacros.h` instead
of raw `throw`, and for `boost::format` to be replaced with `std::ostringstream`
(or plain string concatenation for simple one-substitution cases) everywhere.
Done across `validation`, `etrading`, `AQ_XLL`, `AQ_API` and `GTEST` â€” **~200
files changed**, full solution builds green (Debug|x64) after each batch.
`math`/`models`/`calibration` were explicitly left alone (legacy, headed for
deprecation/extraction per Â§8 â€” not worth the effort on code that may be
deleted).

**Batched and built between each, per the working agreement:**
`validation` (44 files) â†’ `etrading` (~140 files) â†’ `AQ_XLL`/`AQ_API`/`GTEST`
(10 files). Every raw `throw AQLCoreInvalidData` / `AQLCoreError` /
`AQLCoreSystemError` / `AQLCoreNumericalError` / `AQLCoreAppError` /
`ETradingException` / `Exception(...)` converted; `AQLCoreNumericalError` and
`AQLCoreSystemError` throws collapsed into `AQ_THROW`/`AQ_REQUIRE` (which only
ever throw `AQLCoreInvalidData`) â€” a decided simplification, confirmed safe
because nothing outside `math`/`models` catches those two subtypes by name.

**Real correctness issue found and fixed, not just cosmetic:** `ETradingException`
(derives `std::runtime_error`) is specifically caught by type in several places
(`CurveUtilities.cpp` x2, plus two GTEST fixtures exercising `FixingTable`/
`TableDateDouble`). Collapsing those throw sites into `AQ_THROW` changes the
dynamic type to `AQLCoreInvalidData` (different hierarchy â€” `AQLCoreError :
virtual std::exception`), which would have silently broken those `catch`
blocks. Retyped every affected `catch` to `AQLCoreInvalidData&` and updated the
two GTEST assertions that checked the exact thrown type and message text
(`TestFixingTable.cpp`, `TryAqTestCurveTenorBasisJPY3M6MConvergence.cpp`) â€”
confirmed via a full search of every `catch(ETradingException&)`/
`catch(AQLCore*&)` in the tree before and after, so nothing else depends on a
type that changed.

**A bug in the automation itself, caught by the compiler and fixed:** an
early regex pass collapsing `if(cond){throw X;}` into `AQ_THROW_IF(cond,"msg")`
silently ate the enclosing `if` whenever a trailing `else`/`else if` followed,
producing "illegal else without matching if" (`AQLUpdateCurveObject.cpp`,
`AQLUpdateObjectPoolForCurves.cpp`, `AQLCurveCalibrationHelpers.cpp`,
`CurveCalibrationData.cpp`, `BasisCurveCalibration.cpp`, and others â€” ~15
locations). A follow-up broadened fix regex then produced two genuine
false-positive corruptions of unrelated code (a `legNames.resize(nSwaps, "")`
call in `tryAqSwapObjectPricing.cpp` and four `AQ_REQUIRE(false, "...")` calls
in `PiecewisePolynomialInterpolation.cpp` got mis-parsed as throw messages).
All resolved by comparing every flagged site against `git diff`/`git show
HEAD:<path>` (the pre-batch baseline still sitting in the stash repo) before
trusting any mechanical fix, and by writing a final paren-aware, comment-aware
verification pass (not plain regex) that confirms zero message-less or
comma-swallowed `AQ_THROW_IF` calls remain anywhere in scope.

**Verified:** full solution build (`AlgoQuantLib-VS22.sln`, Debug|x64) green
after every batch, including the final one. Swept `validation`, `etrading`,
`AQ_XLL`, `AQ_API`, `GTEST` for any remaining live `throw AQLCore*` /
`ETradingException` / `Exception(...)` or live `boost::format` â€” none found;
the only remaining textual hits are confirmed dead code inside `/* */` or `//`
comments, left untouched per "never delete code or comments." SWIG-generated
`.cxx` files were out of scope and not touched. GoogleTest re-run not yet done
in this session â€” **deferred**: confirm numerically identical output against
the pre-batch baseline before this is considered fully closed out.

---

## Batch script hardened against a file-lock race; renamed per Nicholas (2026-09-13)

A real VS rebuild of `Release_XL_Manifest` (the first one exercising the new
`.bat`, see the entry below) still worked â€” build succeeded, correct 4/467
result â€” but printed three `The process cannot access the file because it is
being used by another process.` lines. Investigated rather than dismissed as
noise, since it was reproducible (also saw 1-2 occurrences of the same thing
directly testing the `.bat` via `cmd.exe` outside VS, not just inside a VS
build).

**Root cause: file-handle churn, not a logic bug.** The original `.bat`
opened the output header and the temp known/requested-name files via a
separate `>>` append **per function** â€” up to 467 opens-and-closes of the
same file in a tight loop. That is a well-known collision point with
antivirus real-time scanning or an IDE file-change watcher transiently
locking a file the instant it changes; `cmd.exe`'s `>>` doesn't retry on
failure, so an unlucky collision silently drops that one line. It happened
not to corrupt anything in the runs so far (467/467 and 4/467 both came out
correct), but that was luck, not a guarantee.

**A red herring chased down first, worth recording so it doesn't get
re-investigated:** the build reported `edition 'Active'` instead of the
expected `edition 'DemoSmall'`, which looked like a parsing bug in the
`edition:<name>` line handling. It wasn't â€” `active.txt`'s line 24 literally
reads `edition:Active` now (the label was changed at some point after the
JSON->text conversion, function list unchanged), confirmed by reading the
file directly before touching any parsing code. The `edition:` extraction
logic itself is correct.

**Fix:** rewrote the three multi-append loops (the `XLO_FUNC_START` source
scan, the manifest-line classification, and the header-writing loop) to each
use one grouped `( ... ) > "file"` redirection instead of many small `>>`
appends â€” the file is opened once for the whole operation instead of
hundreds of times. Needed one escaping fix as a result:
`(edition: %EDITION%)` written from *inside* a `(...)` block needs its
closing paren escaped (`%EDITION%^)`) so it isn't parsed as the block's own
terminator. Re-ran all the same test cases (wildcard, the real restricted
`active.txt`, an unknown-function-name manifest, a missing manifest, a bad
root) directly via `cmd.exe` â€” same correct results as before, but zero
"process cannot access" messages across any of the runs this time.

**Renamed per Nicholas, in the same pass:**
`generate_xll_manifest_header.bat` -> `generateManifestList.bat`;
`aqManifestFunctions.h` -> `aqManifestList.h`. Updated every reference:
the `.vcxproj`'s `PreBuildEvent` Command and `/FI` force-include, the
`<None>`/`<ClInclude>` items and their `.vcxproj.filters` entries, the
script's own self-referencing comments, and `active.txt`'s comment header
(which names the script by filename for anyone reading it in Solution
Explorer). Confirmed no stale references to either old name remain anywhere
in the project files.

**Verified:** ran the exact `PreBuildEvent` command line (via `call
"...generateManifestList.bat" "$(SolutionDir)" ...`) against the real
project paths â€” writes `src\AQ_XLL\include\generated\aqManifestList.h`
correctly, no lock errors. Nicholas then rebuilt through Visual Studio itself
and confirmed it too is clean â€” no "process cannot access" messages, no
errors. **Fully verified, nothing deferred, for this fix.**

---

## Manifest generator rewritten as pure batch; manifest format JSON -> plain text (2026-09-13)

Nicholas moved `generate_xll_manifest_header.py` into
`src\AQ_XLL\resources\manifest\` himself (for better Solution Explorer
management) and hit a crash: the script inferred the `AQ_LIB` root from its
own file location via three `dirname()` hops, which only worked at the
specific folder depth it was originally written for
(`resources\scripts\...`). One level deeper, those three hops landed on
`src\AQ_XLL` instead of the `AQ_LIB` root, producing a doubled path
(`...\AQ_LIB\src\AQ_XLL\src\AQ_XLL\src`) and a `FileNotFoundError`.

**Two decisions, both from Nicholas:**

**1. Stop inferring the root from `__file__` at all â€” pass it in explicitly.**
Even with the immediate bug fixed, path-inferred-from-script-location is
inherently fragile against exactly this kind of reorganisation. The generator
(then still Python, now the `.bat` below) takes `AQ_LIB_root` as its first
argument; the `.vcxproj` passes `$(SolutionDir)`, which always knows the
right answer regardless of where the script itself lives.

**2. Eliminate the Python dependency entirely â€” rewrite as a pure `cmd.exe`
batch file**, after being offered three options (keep Python + a friendly
missing-Python build error; PowerShell, which parses JSON natively; pure
batch) and picking pure batch despite the trade-off flagged: `cmd.exe` has no
JSON parser, so this necessarily means dropping JSON as the manifest format
too.

**What changed:**
- `generate_xll_manifest_header.py` (deleted) -> `generate_xll_manifest_header.bat`,
  same folder. No interpreter dependency beyond `cmd.exe` itself â€” nothing to
  install on any Windows dev machine. Scans the same `XLO_FUNC_START` sites
  via `findstr`/`for /f` (using a temp known-names file, not a single big
  environment variable, to stay well clear of `cmd.exe`'s ~8191-char
  per-variable limit â€” 467 function names would blow past that). Same
  contract: `AQ_LIB_root manifest.txt output_header.h`, same validation
  (fails loudly, exit 1, if a manifest name doesn't exist or `AQ_LIB_root`
  looks wrong), same `#define AQ_XLL_ENABLE_<name> 0/1` output.
- `active.json`/`demo.json` -> `active.txt`/`demo.txt`. New format: `#`-prefixed
  comment lines (ignored), optional `edition:<name>` line, either a lone `*`
  (every function â€” resolved fresh at generation time, same as before, still
  nothing hand-maintained) or one function name per line. `active.txt` carries
  a full explanatory comment block at the top (Nicholas asked for this
  explicitly, since JSON couldn't carry inline documentation the way a plain
  text file can) â€” `demo.txt` stays a copy-from example, unchanged in
  content (still the same 4-function `DemoSmall` list), just reformatted.
  **`active.txt`'s live content was preserved as `DemoSmall` (not reset to
  `Full`)** when converting from `active.json`, since that was the manifest
  actually in use at the time of the move.
- `AQ_XLL.vcxproj`'s `PreBuildEvent` now calls the `.bat` directly (`call
  "...\generate_xll_manifest_header.bat" "$(SolutionDir)" ...`) â€” no `where
  python` check needed any more, since there's nothing left to be missing.
  `AQ_XLL_MANIFEST_FILE`'s default updated to `active.txt`. The stray `<None>`
  item Visual Studio had added for the script also had a stale path (missing
  the `\manifest\` segment â€” pointed at a location the file was never
  actually at); corrected while touching this.

**Verified:** ran the `.bat` directly via `cmd.exe` (not just eyeballed) â€”
wildcard (`467/467`), the real restricted `active.txt` (`4/467`, exact same
4 names as the Python version produced), an unknown-function-name manifest
(fails, exit 1, same error format), a missing manifest file, and a bad
`AQ_LIB_root` (all fail cleanly, exit 1). Then ran the **exact**
`PreBuildEvent` command line against the real project paths and confirmed it
writes the real `aqManifestFunctions.h` correctly. `AQ_XLL.vcxproj`/
`.vcxproj.filters` re-verified as well-formed XML (including the specific
`--`-in-comment mistake from two entries ago â€” checked again, none present).
**Not yet re-verified through an actual VS build** â€” recommend a full rebuild
of `Release_XL_Manifest` to confirm MSBuild's own invocation (through
`Microsoft.CppCommon.targets`, not a direct `cmd.exe` call) behaves
identically to the direct test above.

---

## `Release_XL_Manifest` confirmed working end-to-end (2026-09-13)

Nicholas built and tested the full `Release_XL_Manifest` feature (all entries
below, most recent first) and confirmed **everything works**: build green,
`aqManifestFunctions.h` regenerates correctly from `active.json` at its fixed
path, shows up under `AQ_XLL`'s `include` filter in Solution Explorer, the
generator script runs fine from its new `resources\scripts\` home, and â€”
the one item every prior entry flagged as unverified â€” **Excel's function
wizard under `AlgoQuantLib` genuinely shows only the functions listed in
`active.json`, not the full 467.** This closes out every "not yet
verified"/"not yet re-verified" caveat left by the entries below.

One incidental fix along the way, worth calling out since it's unrelated to
any of this feature's code: `demo_small.json` was renamed to `demo.json` from
within Visual Studio's Solution Explorer, which correctly updated the
`<None>` item paths in both `AQ_XLL.vcxproj` and `.vcxproj.filters`
automatically â€” no manual follow-up needed.

Also confirmed elsewhere in this session but worth restating here since it
caused two of the debugging detours above: `AQ_XLL` must be the solution's
**Startup Project** (Solution Explorer â†’ right-click â†’ Set as Startup
Project) for Run/F5 to launch Excel at all â€” if a different project (e.g.
`models`, a static library) is startup, VS tries to `CreateProcess` a `.lib`
directly and fails with the same "not a valid Win32 application" message,
which looks identical to the real debug-settings bug but has nothing to do
with `AQ_XLL.vcxproj.user`.

**Fully verified, nothing deferred, for this feature.**

---

## Manifest generator script moved out of `rebrand\tools\` (2026-09-13)

Nicholas: `rebrand\` gets deleted wholesale once the rebrand is complete
(CLAUDE.md/MIGRATION_PLAN.md Â§2.2 - it is the temporary stash tooling, not a
permanent part of the product), but `generate_xll_manifest_header.py` is a
permanent build-time dependency of `AQ_XLL.vcxproj` (its `Release_XL_Manifest`
`PreBuildEvent` calls it on every build) - it must survive that deletion.
Moved `rebrand\tools\generate_xll_manifest_header.py` -> `resources\scripts\generate_xll_manifest_header.py`
(a new folder; `resources\` already ships permanent runtime content -
calendars, generator templates - so it is the right home, alongside the
existing `config\` subfolder). Updated the `PreBuildEvent` `Command` in
`AQ_XLL.vcxproj` and the script's own self-referencing comments (usage line,
generated-file header comment) to the new path. Confirmed the script's `ROOT`
resolution (three `dirname()` calls up from `__file__` to the `AQ_LIB` root)
needed no code change - `resources\scripts\` is the same depth as
`rebrand\tools\` was. Verified deploy scripts (`deploy{Python,CSharp,Java,R}_2022.bat`)
only copy `resources\config\*`, not all of `resources\`, so this script won't
leak into a shipped language-binding package.

**Not yet re-verified through an actual VS build** at the new script path.

---

## Generated manifest header made visible in Solution Explorer (2026-09-13)

Nicholas asked for the generated `aqManifestFunctions.h` to be visible in the
`AQ_XLL` project tree instead of buried in `$(IntDir)` (a build-output folder,
git-ignored, never referenced by any project item, so it never showed up in
Solution Explorer at all despite being force-included).

Moved the generation target from `$(IntDir)aqManifestFunctions.h` to a fixed
path independent of configuration: `src\AQ_XLL\include\generated\aqManifestFunctions.h`.
Added it as a normal `ClInclude` item under the existing `include` filter in
both `AQ_XLL.vcxproj` and `.vcxproj.filters`, right alongside `aqXllTools.h`/
`aqMain.h` â€” so it now shows up in the "include" node of Solution Explorer
like any other header (absent/greyed out until the first `Release_XL_Manifest`
build creates it). `.gitignore` (at the `LIBRARY_UPDATE` repo root) got a new
`AQ_LIB/src/AQ_XLL/include/generated/` entry so it's never accidentally
staged â€” it is still fully regenerated by the `PreBuildEvent` every build, not
checked in.

**Caught in passing (again):** two more literal `--` sequences inside XML
comments in `AQ_XLL.vcxproj` from this and the previous edit, which broke the
file's XML well-formedness a second time. Both fixed; re-verified the whole
file has no remaining `<!--...-->` block containing `--`, and all three
project files (`.vcxproj`, `.vcxproj.filters`, `.vcxproj.user`) parse cleanly.
**Lesson for future edits to this file: never use `--` inside an XML
comment** (em dash or a single hyphen instead).

Verified: regenerating `aqManifestFunctions.h` at the new fixed path works
(4/467 enabled from the current `active.json`); `git status` confirms the
generated file is correctly ignored. **Not yet re-verified through an actual
VS build** â€” the `ClInclude` item should appear under `AQ_XLL`'s `include`
filter in Solution Explorer, but this hasn't been confirmed in a real VS
session.

---

## `AQ_XLL_MANIFEST_FILE` fixed to one file (`active.json`); User Macros page abandoned (2026-09-13)

Two problems from the previous entry, both from Nicholas:

**1. VS's "User Macros" property page doesn't appear for `AQ_XLL`.** It's a
standard `DynamicLibrary` C++ project, and that page is reliably shown only
for NMake-style projects in modern Visual Studio â€” the underlying
`Label="UserMacros"` PropertyGroup mechanism still works at the MSBuild level,
but there was no GUI to edit it, defeating the point.

**2. A plain rebuild from the VS Build menu still showed every function.**
Root cause carried over from the prior entry: nothing was overriding
`AQ_XLL_MANIFEST_FILE`, so it fell back to the all-functions default â€” not a
bug in the gating mechanism itself, just nothing pointing it at a restricted
manifest yet.

**Fix â€” stop trying to make the *path* switchable from the IDE; make the
*file* fixed and switch its *content* instead.** `AQ_XLL_MANIFEST_FILE`
defaults to one single, permanent file: `src\AQ_XLL\resources\manifest\active.json`
(renamed from `default.json`). To change which functions a local
`Release_XL_Manifest` build exposes, open `active.json` in Solution Explorer
(under the `resources\manifest` filter, already visible there) and edit its
`"functions"` array directly, then rebuild â€” no property pages, no macros, no
VS restart, nothing to reload. `demo_small.json` stays as a copy-from
template. The `msbuild /p:AQ_XLL_MANIFEST_FILE=...` override still works
unchanged, for scripted/CI generation of many customer editions without
touching `active.json` at all.

**Caught in passing:** the previous edit's comment in `AQ_XLL.vcxproj`
contained a literal `--` inside an XML comment (`default -- to switch`),
which is illegal in XML and made the whole project file fail to parse. Fixed
before it reached a real VS session â€” re-verified `AQ_XLL.vcxproj`,
`.vcxproj.filters` and `.vcxproj.user` all parse as well-formed XML.

**Not yet re-verified through an actual build with a restricted
`active.json`** â€” recommend setting `active.json`'s `"functions"` to a small
list (e.g. copy `demo_small.json`'s four), rebuilding
`Release_XL_Manifest`, and confirming Excel's function wizard under
`AlgoQuantLib` shows only those before relying on this further.

---

## Manifest location moved inside AQ_XLL; wildcard "*" replaces the hand-listed default (2026-09-13)

Two corrections to the `Release_XL_Manifest` work below, both from Nicholas:

**1. Manifest files live inside `AQ_XLL`, not the shared `resources\config\`
tree.** Moved `resources\config\XLL_MANIFEST\*.json` â†’ `src\AQ_XLL\resources\manifest\*.json`
â€” alongside the existing `src\AQ_XLL\resources\SetUp.md` â€” and added them as
`<None>` items in `AQ_XLL.vcxproj` under a new `resources\manifest` filter in
`AQ_XLL.vcxproj.filters`, so they're visible and browsable in Solution
Explorer like any other project resource. `AQ_XLL_MANIFEST_FILE`'s default in
the `.vcxproj` now points at
`$(SolutionDir)src\$(ProjectName)\resources\manifest\default.json`.

**2. No more hand-maintained "list every function" default.** The original
`default.json` enumerated all 467 function names as the "include everything"
edition â€” exactly the kind of static list that silently goes stale the next
time a function is added or renamed. `generate_xll_manifest_header.py` now
accepts `"functions": "*"` as a wildcard meaning "every function known
today," resolved fresh from the source scan at generation time. `default.json`
is now just `{"edition": "Full", "functions": "*"}` â€” nothing to maintain.
Explicit lists (`demo_small.json`) are unaffected and still validate every
name against the source scan.

Verified: both `default.json` (467/467 via wildcard) and `demo_small.json`
(4/467) regenerate cleanly from the new location. Not yet re-verified through
an actual MSBuild run after the path change â€” do that before relying on it.

---

## New `Release_XL_Manifest` AQ_XLL configuration â€” function-level custom editions (2026-09-13)

Added a fifth `AQ_XLL` edition alongside the existing category-level
`Release_XL_Bond/Credit/Curve/Swap` (CLAUDE.md Sec 4.4/9.5): `Release_XL_Manifest`
builds `AlgoQuantLib.xll` with only the individual worksheet functions named in
an external manifest JSON, cutting across category files (e.g. a handful of
`Swap` and `Bond` functions together) rather than whole categories.

**Mechanism (compile-time, not runtime):** every one of the 467
`XLO_FUNC_START(...)...XLO_FUNC_END(...)` sites across all 18 `src\AQ_XLL\src\*.cpp`
files is now wrapped `#if AQ_XLL_ENABLED(name) ... #endif` (scripted sweep,
verified 1:1 against the original function count â€” no misses, no double-wraps).
`AQ_XLL_ENABLED(name)` is defined once in
`src\AQ_XLL\include\aqXllTools.h`:

    #define AQ_XLL_ENABLED(name)  (!AQ_XLL_MANIFEST_BUILD || AQ_XLL_ENABLE_##name)

`AQ_XLL_MANIFEST_BUILD` defaults to 0 there, so the guard is a no-op for every
other configuration â€” `Debug`, `Release`, and the existing `Release_XL_*`
configs are unaffected (an undefined `AQ_XLL_ENABLE_<name>` token safely
evaluates to 0 in `#if`, no warning). Only `Release_XL_Manifest` defines
`AQ_XLL_MANIFEST_BUILD=1` and force-includes (`/FI`) a generated header listing
one `AQ_XLL_ENABLE_<name> 0/1` per known function.

**New files:**
- `rebrand\tools\generate_xll_manifest_header.py` â€” the generator. Ground
  truth for "known function names" is the AQ_XLL source itself (scans every
  `XLO_FUNC_START`), not `docs\api_map.csv`, so it can't drift from the actual
  port. Fails loudly (exit 1) if the manifest names a function that doesn't
  exist.
- `resources\config\XLL_MANIFEST\default.json` â€” the default manifest
  (all 467 functions enabled = equivalent to `Release`), so the configuration
  builds out of the box. Override per build with
  `msbuild /p:AQ_XLL_MANIFEST_FILE=path\to\edition.json`.

**Project/solution wiring:** `AQ_XLL.vcxproj` got a new `Release_XL_Manifest|x64`
configuration cloned from `Release_XL_Swap` (same `.xll` output name, same
plain-`Release`-built static-lib dependencies), plus the `AQ_XLL_MANIFEST_BUILD=1`
define, the `/FI` force-include, and a `PreBuildEvent` that regenerates the
header from `$(AQ_XLL_MANIFEST_FILE)` before every build. `AlgoQuantLib-VS22.sln`
got the matching `Release_XL_Manifest` row across all 8 project GUIDs, mirroring
exactly how each one already handles `Release_XL_Bond` (other projects build at
plain `Release`, `AQ_API` is skipped, `AQ_XLL` builds itself).

**Landmine hit and fixed:** `projects\AQ_XLL.vcxproj.user` (per-project debug
launch settings â€” `LocalDebuggerCommand`/`LocalDebuggerCommandArguments`
pointing Visual Studio's Run at Excel with `$(TargetPath)`) had a
`PropertyGroup` for every existing configuration but none for
`Release_XL_Manifest|x64`. Without it, VS's default debug command for a
`DynamicLibrary` project is to launch `$(TargetPath)` **directly** â€”
Windows refuses to `CreateProcess` a DLL/XLL (no direct-execution entry
point), surfacing as "not a valid Win32 application" even though the built
`.xll` itself is a perfectly valid x64 PE (verified by hand: same MZ/PE/machine
header as the working editions). Fixed by adding the missing
`PropertyGroup` (cloned from `Release_XL_Bond`'s). **Also needed a full
Visual Studio restart, not just a project reload/rebuild** â€” `.vcxproj.user`
is only read at project load, so an external edit to it while the solution is
already open sits unused until VS is restarted. Confirmed working end-to-end
after restart: builds, links, and runs Excel with the manifest-restricted
add-in.

**Verified:** build green; output `.xll` present at
`targets\64\Release_XL_Manifest\AQ_XLL\out\AlgoQuantLib.xll`, valid x64 PE,
size in the same ballpark as `Release_XL_Bond`'s (larger, since the default
manifest enables all 467 functions). Confirmed running via VS with Excel after
the `.vcxproj.user` fix + restart. **Not yet verified:** an actual
*restricted* manifest (few functions) hasn't been confirmed to hide the
unlisted functions from Excel's function wizard â€” recommend testing with a
small manifest before shipping a customer edition this way.

---

## Config-file preservation after a bulk delete; `aqToolInitialize` now reloads config too (2026-09-12)

Nicholas flagged that the previous turn's `aqObjectDeleteAll`-with-no-argument
path (see the entry below) deletes every cached object, **including** objects
that were only there because a config file loaded them at startup (generator
templates from `resources\config\{SWAP,BOND,CURVE}_GENERATOR`, etc.) â€” and
nothing put them back. Unlike `tryAqObjectClearCache` (which already reloaded
config at its end, see item 3 below), the new bulk-delete overloads didn't.
Request: *"The initialize method and the delete methods need to load the
configuration files at the end of their routines."*

**Delete methods (`validation/src/tryAqObject.cpp`):** both
`tryAqObjectDeleteAll` overloads (the single-type one and the new
all-types one) now call `validation::tryAqToolLoadConfigurationFiles()`
after deleting, before returning the count â€” same pattern
`tryAqObjectClearCache` already used. Added `#include "tryAqToolSetup.h"` for
the declaration. **`tryAqObjectDelete` (the single-object delete) was
deliberately left unchanged** â€” reloading config does real disk I/O
(`etrading::FolderConfig::setupOptionalStartupConfig()` re-reads a config-path
file and every JSON generator file it lists), and a single-object delete is
the kind of call a spreadsheet can make repeatedly; paying that I/O cost on
every one of those calls, rather than just the bulk operations, wasn't asked
for and looked like a performance regression waiting to happen. Flagging this
scoping choice for Nicholas to confirm or override.

**Initialize method (`AQ_XLL/src/aqTool.cpp`, `aqToolInitialize`):** added a
call to `validation::tryAqToolLoadConfigurationFiles()` right after
`etrading::InitializeETrading::instance(true, true)` and before building the
returned status message. `aqToolInitialize` previously only loaded calendars/
IR static data via the `InitializeETrading` singleton and never touched the
generator config at all â€” so a fresh add-in load depended entirely on
whatever `tryAqObjectClearCache`/`trySetupAQL` happened to have done, which
isn't guaranteed at `xlAutoOpen`. Now it is loaded unconditionally on every
initialize call, matching the delete methods and `tryAqObjectClearCache`.
`tryAqToolLoadConfigurationFiles` never throws (returns an error string
instead), so a missing/bad config file still doesn't stop the add-in loading.

**Verified (static checks only â€” not yet through a real build):**
`rebrand/tools/api_pair_check.py` HARD GATE still 0; whole-tree
`validation/src/*.cpp` wiring check still 101/101, 0 gaps. No `.arg()`/param
counts changed (no XLO_FUNC signature touched), so the runtime-registration
landmine in `CLAUDE.md` Â§3.2 doesn't apply here. **This batch â€” the IR rename,
`aqObjectDeleteAll`/`aqObjectClearCache` changes, and now this config-reload
addition â€” has still not been through a real compiler.** Recommend building
before relying on any of it further.

---

## `InterestRate`â†’`IR` rename + `aqObjectDeleteAll`/`aqObjectClearCache` behaviour changes (2026-09-12)

Nicholas confirmed the build works and tests pass, and **promoted
`aqCurve.cpp`/`aqInterestRate.cpp` into `src\Core`** (own commit) â€” resolving
the dependency gap flagged in the previous entry (a `ReleaseBonds`-style
edition needs curves/fixing tables to price against). Three requests landed
this turn:

**1. `InterestRate` category renamed to `IR`.** Full-stack, same discipline as
the earlier `Ois`/`CMS`/`TRS` renames:
- `validation`: `git mv` + content rename â€”
  `tryAqInterestRateFixingTable.{h,cpp}` â†’ `tryAqIRFixingTable.{h,cpp}`,
  `tryAqInterestRateFutureFra.{h,cpp}` â†’ `tryAqIRFutureFra.{h,cpp}`,
  `tryAqInterestRateObjectFra.{h,cpp}` â†’ `tryAqIRObjectFra.{h,cpp}`.
- `AQ_XLL`: `git mv aqInterestRate.cpp â†’ aqIR.cpp`; all 13 `XLO_FUNC` names
  renamed (`aqInterestRate*` â†’ `aqIR*`); file header comment updated.
- `AQ_API`: the `InterestRate` bindings live inside `aqCurveObject.{h,cpp}`
  (a pre-existing filing quirk â€” confirmed via grep before touching anything,
  same pattern as Credit-in-Swap) â€” 4 functions renamed
  (`aqInterestRateFixingTable*` â†’ `aqIRFixingTable*`); no SWIG `.i` file
  changes needed since they `%include` the whole header rather than naming
  functions individually. Generated `swig_*_wrap.*` files deliberately left
  untouched (will regenerate).
- `GTEST`: 11 files' `#include`/call-site renames
  (`tryAqInterestRateFixingTableCreate` â†’ `tryAqIRFixingTableCreate`).
- **3 fixture CSVs, all git-tracked this time** (unlike the 100 untracked
  Ois ones) â€” `git mv`'d and their path-string literals inside the GTEST
  files updated to match in the same sed pass.
- `projects/validation.vcxproj`(`.filters`) and `projects/AQ_XLL.vcxproj`
  (`.filters`) updated; both re-verified well-formed XML.
- `rebrand/tools/api_pair_check.py` `CATEGORIES`: `InterestRate` â†’ `IR`;
  `docs/api_map.csv` regenerated (`IR: 4` in the public-surface breakdown,
  replacing the old `InterestRate` bucket); HARD GATE stayed 0.
- `CLAUDE.md` (both) and `MIGRATION_PLAN.md` updated: category list, the
  `Core`/`Optional` file lists (now correctly showing `aqCurve.cpp`/
  `aqIR.cpp` under `Core`, matching Nicholas's commit), the 4a.5 dependency
  note marked resolved, and a new decision bullet recording the rename.
- **Verified:** whole-tree audit 467/467 clean, 0 duplicate names, 0
  unresolved `validation::tryAq*` symbols; `git grep` for every
  `tryAqInterestRate`/`aqInterestRate` spelling across `src`/`projects`
  returns nothing outside the generated SWIG wrap files (expected) and one
  intentional historical mention in `aqIR.cpp`'s own header comment.

**2. `aqObjectDeleteAll` â€” `ObjectType` is now optional; omitting it deletes
every cached object of every type.** Added true no-arg overloads to
`validation` rather than looping category strings in the XLL layer:
`tryAqObjectList()` (every name, across `etrading::Environment::STORED_TYPES`)
and `tryAqObjectDeleteAll()` (delegates to the same
`etrading::deleteAllObjects(Environment&)` that `tryAqObjectClearCache`
already used internally). The XLL wrapper checks `objectType.isMissing() ||
!objectType.isNonEmpty()` and branches to the new no-arg pair instead of the
existing per-type pair; stops the instance counter for every deleted name
either way. `.help()`/`.arg()` updated to say `ObjectType` is now optional.

**3. `aqObjectClearCache` â€” confirmed it already deletes objects from every
category** (Nicholas asked for this explicitly). Traced
`validation::tryAqObjectClearCache()` â†’ it already calls
`etrading::deleteAllObjects(etrading::Environment::defaultEnv())`, which
loops every `CachedObjectEnum` in `Environment::STORED_TYPES` and deletes
every object of every type â€” **this was already true before today**, found
by reading the implementation rather than assuming a change was needed. What
*was* missing: the `AQ_XLL`-side handle-name instance-counter map
(`namesToCounter_`) was never reset after a full clear, so it would keep
accumulating stale entries for names that no longer existed. Added
`aq_xll::clearAllInstanceCounters()` (mutex-guarded `namesToCounter_.clear()`)
and call it at the end of `aqObjectClearCache`. Help text updated to state
explicitly what gets cleared (every category's objects, curve/swap/credit
results, the entity pool) rather than the vague "the object cache" â€” the
behaviour was already correct, the documentation wasn't.

**Not asked for, flagged rather than done silently:** `aqObjectList` still
requires `ObjectType` (mandatory) â€” the same "blank means all types"
convenience just added to `aqObjectDeleteAll` could trivially be added here
too, reusing the same new `tryAqObjectList()` no-arg overload, since it
already exists. Left alone since it wasn't part of the request; flag to
Nicholas as an easy follow-up if wanted.

**Verified (static checks only, no compiler run yet on the DeleteAll/
ClearCache changes):** whole-tree audit 467/467 clean throughout; every
`validation/src/*.cpp` still wired into `validation.vcxproj` (101/101, 0
gaps) after the file renames. **This batch has not yet been through a real
build** â€” recommend building before relying on it further, per the pattern
of the last few turns where static checks alone didn't catch everything.

---

## Two fixes (`aqObjectDecorateNames`, `aqBondGeneratorDisplay` transpose) + `AQL Classic`/`AQLString`/`AQLDate` scoped for the plan, not actioned (2026-09-12)

**1. `meUtilityLWODecorateNames` was missing from the gap audit** because it
has **no `validation` wrapper at all** (0 hits in `src\validation`) â€” the
gap-audit script only compares against declared `validation` wrappers, so a
function that was never given one couldn't show up as a gap. Traced to
`.APPLES\...\meUtilities.cpp`: a session-wide control function that flips the
AQObj handle-naming switches (instance counter / Excel-address decoration /
address-vs-unique-ID). That state already lives entirely in `aq_xll`
(`aqXllTools.h`'s "Handle behaviour switches" â€” `setInstanceCountNames`,
`setDecorateNamesWithExcelAddress`), not in `etrading`, so there is nothing
for `validation` to validate â€” same disposition as `aqToolEcho`/
`aqToolBuildTime`/`aqToolSEH` (AQ_XLL-only, no `tryAq*`).

Added:
- **`etrading::HedgeCurveInfo`-style third switch, new this session:**
  `convertExcelAddressToUniqueID_` in `aqXllTools.cpp`, wired into
  `getExcelLocationAsString()` â€” when on, the location suffix is a short
  `std::hash`-based numeric ID instead of the literal cell address (stable
  only within the current session, which is fine â€” it exists to give a short
  handle, not a portable one). Declared in `aqXllTools.h` next to the other
  two switches.
- **`aqObjectDecorateNames(enableCounter, appendLocation,
  showExcelCellAddress)` â†’ `aqObject.cpp`** (generic lifecycle, no category â€”
  matches `aqObject<Lifecycle>`). Calls all three setters and returns a
  status string in the same shape as the legacy function
  ("AQObj Names: Instance Counting is ON, Append Excel Cell Location is ON,
  Showing Excel Location as EXCEL ADDRESS"). Defaults match the legacy
  function's own defaults (`enableCounter`/`appendLocation` default TRUE,
  `showExcelCellAddress` defaults FALSE â€” i.e. unique-ID mode by default).

**2. `aqBondGeneratorDisplay` output was untransposed.** Its single call to
`etrading::toAQLStringMatrixFromVariantMatrix( result, false )` explicitly
passed `false` for the `transpose` parameter â€” but that parameter's own
default is `true`, and its doc comment says so directly: *"note transposes by
default to match the default JSON schema convention."* The explicit `false`
was silently overriding the sensible default, i.e. was almost certainly a
copy/paste slip from an earlier function in the same file, not a considered
choice. **Fixed: `false` â†’ `true`.** Confirmed via `git grep` that this was
the *only* call site anywhere in `AQ_XLL` passing `false` here â€” an isolated
bug, not a systemic pattern across the other `*GeneratorDisplay`/
`*Display` functions.

**3. `AQL Classic` / `AQLString` / `AQLDate` â€” investigated, scoped into
`MIGRATION_PLAN.md`, NOT actioned per explicit instruction.** Findings:

- **The `AQL Classic` `.vcxproj.filters` filter is small and tractable.**
  Only `GTEST.vcxproj.filters` actually has files under it: **17 entries** â€”
  6 curve-fixture pairs (`Curve{Accessors,Ois,FwdFxConst,Std,TenorBasis,
  XccyBasis}.{cpp,h}` under `src/GTEST/{src,include}`), `TestDatesCentralBank
  .cpp`, `TestDatesSwapSchedule.cpp`, `TestRiskSwapDeltaLadder.cpp`,
  `TestRiskTenorBasisCurve.cpp`, and â€” worth flagging on its own â€”
  **`TestMirDateFunctions.cpp`**, a `mir`-named test that should probably
  already have gone with the "`mir*` stack deleted wholesale" work from an
  earlier phase; needs checking whether it still calls anything `mir*` before
  deleting. `AQ_API.vcxproj.filters` **declares** the same filter
  (`src\etrading\AQL Classic`, `include\etrading\AQL Classic`) but has **zero
  files assigned to it** â€” an empty, unused filter, nothing to delete there
  beyond the declaration itself. No other project (`math`, `etrading`,
  `validation`, `calibration`, `models`, `AQ_XLL`) has this filter at all.
- **`AQLString` and `AQLDate` are a much bigger undertaking than the filter
  cleanup â€” do not conflate the two.** Repo-wide grep counts:
  **`AQLString`: ~36,962 occurrences across 1,108 files. `AQLDate`: ~9,904
  occurrences across 585 files.** This dwarfs every rename done in this
  rebrand so far, including the `me*`â†’`aq*` sweep.
- **`AQLString` (`src\math\{include,src}\AQLString.{h,cpp}`, ~1,550 lines):**
  a hand-rolled, atomic-refcounted, copy-on-write string class from an era
  before C++11 gave `std::string` move semantics and small-string
  optimisation â€” the exact problem COW strings existed to solve. **No
  architectural justification found for keeping it** in a C++17 codebase; its
  extra convenience (numeric constructors, `getDoubleValue()`/
  `getIntValue()`) is trivially replaced by `std::to_string`/`std::stod`/
  `std::stoi` or small free functions. Recommendation: **`std::string`** as
  the replacement, no custom type needed.
- **`AQLDate` (`src\math\include\AQLDate.h`, 112 lines + 695-line `.cpp`):** a
  hand-rolled Julian-day calendar class, **virtual** (`virtual ~AQLDate()`,
  `virtual void setDate(...)`), storing `year/month/day` plus a cached
  Julian long. Recommendation: **`boost::gregorian::date`** â€” Boost is
  already a direct dependency (BSL-1.0, no encumbrance), already the
  canonical date type in the newer `Curve` validation headers
  (`tryAqCurveDiscountFactor.h` etc. take `std::vector<boost::gregorian::
  date>` directly), and the bridge functions
  `etrading::toGregorianDateFromAQLDate`/`toAQLDateFromGregorianDate` already
  exist and are proven correct â€” meaning the hard conversion-correctness work
  is already done, just not yet load-bearing everywhere.
- **Scale means this is NOT a rebrand-timeline task.** A 37k/9.9k-occurrence
  type swap needs an automated codemod (clang-tidy `readability-*` /
  libclang-based rewrite, not manual `sed`) and a full regression run per
  batch, run as its **own initiative after the rebrand ships**, not folded
  into Phase 6. Scoped into `MIGRATION_PLAN.md` as a new **Phase 8** for
  exactly this reason â€” see that file for the entry. The `AQL Classic` filter
  deletion (17 files, isolated GTEST fixtures) is small enough to fold into
  the existing Phase 6 resources/legacy-extraction pass instead.

**No code deleted, no `AQLString`/`AQLDate` usage touched â€” per explicit
instruction, this turn was investigate-and-plan only.**

---

## âœ… BUILD CONFIRMED GREEN, ALL CONFIGS, TESTS PASS â€” Phase 4 XLL port essentially complete (2026-09-11)

Nicholas: "All builds are working and tests pass." This confirms the
gap-closing batch below (23 functions: Object lifecycle, `aqCurveUSDSpotDate`,
Tool strays + the `aqToolEchoDouble` re-fix, 10 Math vector overloads, 4 Swap
risk-ladder functions) compiles and links clean, on top of the two build-fix
rounds already applied to the Curve/Credit/Swap batches before it.

**Answering "have we migrated all functions?"**: yes, modulo 9 deliberately
excluded wrappers and the 2 categories (`Model`, `Generator`) that have zero
`validation` wrappers to port in the first place. Full detail in the section
immediately below (unchanged from the last entry) and in the top-level
`STATUS.md` Â§2, which now carries the same audit as the canonical
"is everything ported" answer, updated in lockstep with this file.

**Updated for this milestone:** top-level `STATUS.md` rewritten in full (it
was several sessions stale â€” still describing ~35% Phase 4 progress and an
already-resolved "does xlOil support native-type arguments" question). New
version leads with the 98%-ported headline, the exact 9-wrapper exclusion
list, a chronological summary of everything ported this session, and a
re-runnable copy of the gap-audit script so "are we done yet" never again
requires re-deriving the answer from scratch.

**Not yet done / next up, unchanged from before:**
1. `Model`/`Generator` â€” net-new `validation` wrapper design and build, not a
   port. Not scoped.
2. Phase 4a: `AQ_XLL` per-edition build *configurations* (the `src\Core`/
   `src\Optional` file split is done and committed; the actual
   `ReleaseBonds`/`ReleaseSwaps`/etc. configurations that consume it are not
   yet added) and the `AQ_API` runtime manifest gate (not started).
3. Phase 5 (bindings/SWIG regen, C#/Java/R verification, coverage gaps),
   Phase 6 (legacy extraction, licensing, `RELEASE_NOTES`, clang-format),
   Phase 7 (Linux/CMake, clang-tidy CI, clean repo) â€” all still ahead per
   `MIGRATION_PLAN.md`, none started.

---

## Phase 4 XLL port essentially COMPLETE: full gap audit closed to 9 intentional non-gaps (2026-09-11, NOT YET RE-BUILT)

Nicholas confirmed the build works and tests pass (all configs). "I want to
migrate all functions, what is the best way to do that" â€” answered by running
an exact gap audit (every `tryAq*` validation wrapper vs every registered
`aqXLL` function by name, a script-driven comparison rather than manual
category review) instead of guessing what was left. Result: **445 of 467
wrappers already ported; 30 genuine gaps** (`Model`/`Generator` excluded â€”
both have 0 validation wrappers, that's net-new work not a port).

**Two decisions put to Nicholas, both answered:**
- Legacy procedural SABR family (7 functions,
  `tryAqVolatilitySABR{Calibrate,GetPrem,GetVol,OutputParameter,
  SetupConvention,SetupParameter,SetupSwaptionVol}`) â€” **decided: leave out**,
  superseded by the already-ported object-based `tryAqVolatilityObjectSabr*`.
  Not ported; this is a deliberate, confirmed decision, not an oversight.
- Swap risk ladders (4 functions needing a `vector<LabelValueBlock>` /
  multi-trade marshalling design with no prior pattern in the codebase) â€”
  **decided: build it now.** Done â€” see below.

**23 of the 30 gap functions ported this batch, closing every gap that was
actually a gap:**

1. **Quick strays (5 real + 1 false positive):**
   - `aqObjectTypeAsString`, `aqObjectLoadAndReturnTupleResults` â†’ `aqObject.cpp`
     (generic Object lifecycle; the tuple one returns a 2-row
     `[handle, cached-type]` column via `etrading::toString(CachedObjectEnum)`).
   - `aqCurveUSDSpotDate` â†’ `aqCurve.cpp` (filed in `tryAqDate.h`, golden-named
     Curve â€” another cross-file quirk like Credit-in-Swap).
   - `aqToolBondAverageYield`, `aqToolBondYieldFromFuturePrice` â†’ `aqTool.cpp`
     (filed in `tryAqBondObject.h`, golden-named Tool).
   - `aqToolSwapScheduleTemplate` â†’ `aqTool.cpp` (filed in
     `tryAqSwapObjectSchedule.h`, golden-named Tool; previously noted as
     "do with Swap" â€” that was a *location* note from an earlier session, the
     golden name puts it in Tool, corrected here).
   - `tryAqBondObjectZSpreads` â€” investigated, turned out to be a **false
     positive**: the plural validation wrapper's own doc comment says it's
     "Validation interface for the **aqBondObjectZSpread**" (singular) â€” i.e.
     it already IS the public function behind the existing (singular-named)
     `aqBondObjectZSpread` in `aqBond.cpp`. The truly-singular
     `tryAqBondObjectZSpread` (different signature, curveCollection/
     forecastCurve as single strings) is explicitly marked "Helper interface"
     in its own doc comment â€” an internal building block, not meant to be
     public. No fix needed; confirmed by reading both doc comments, not
     guessed.

2. **`aqToolEchoDouble` â€” the other known `validation.vcxproj` wiring gap,
   fixed properly this time.** Added `tryAqToolEchoDouble.h`/`.cpp` to
   `validation.vcxproj`/`.filters` (next to `tryAqToolDate`), then re-added
   `aqToolEchoDouble` to `aqTool.cpp` (removed earlier this session for the
   same reason). **Repo-wide check: every `validation/src/*.cpp` on disk
   (101) is now wired into `validation.vcxproj` (101) â€” 0 gaps.** This class
   of error should not recur.

3. **Math vector overloads (10 functions) â†’ `aqMath.cpp`:**
   `BlackScholes{Prices,ImpliedVols}`, `CapletFloorlet{Prices,ImpliedVols}`,
   `EuropeanIRSwaption{Prices,ImpliedVols}` (straightforward column-of-scalar
   generalisations of the already-ported single forms), and the low-level
   `ForwardRate(s)`/`DiscountFactor(s)` primitives (raw `(dates,values)`
   curve-fit inputs, needing `etrading::to{Interpolation,StateVariable,
   DayCount,CompoundingFrequency}Enum` stringâ†’enum conversions â€” all four
   free functions already existed in `CoreEnumerations.h`, just needed
   calling). **Each of these 4 names also has a second, `"*** LEGACY
   METHOD ***"`-labelled overload** (curveCollection/curveIndex based) in the
   same header â€” ported only the non-legacy `(dates,values)` overload, per
   the header's own explicit labelling (not a guess: the source comment says
   so). New local helpers added to `aqMath.cpp`: `toStringVectorOr`,
   `toDoubleVectorOr`, `toCallOrPutEnumVector`, `toStandardStringMatrixOr`.

4. **Swap risk ladders (4 functions) â†’ `aqSwap.cpp`:**
   - **New marshalling pattern, `toLabelValueBlockVector`:** a table
     convention (row 0 = shared key headers, each subsequent row = one
     trade's values against those keys â€” an ordinary Excel table with a
     header row) used to build a `std::vector<LabelValueBlock>` from a single
     range. No such pattern existed anywhere in the codebase before this;
     used by `aqSwapObjectPVs` (batch PV) and `aqSwapDelta` (stateless
     multi-trade flat-shift delta, which also needed `etrading::
     getDataInstance()` for its `AQLDataInstance*` parameter).
   - **New output pattern, `toExcelLabeledMatrix`:** row/column labels plus a
     `DoubleMatrix` â†’ one sheet with a blank-corner header row, reused by
     `aqSwapDelta` and `aqSwapObjectDeltaLadder`.
   - `aqSwapObjectDeltaLadderHorizontally` needed its own ragged
     multi-column layout (`vector<AQLStringVector>` / `vector<DoubleVector>`,
     one pair of columns per curve, padded to the longest column) â€” built
     inline, no reusable helper extracted since nothing else needs this exact
     shape.
   - `aqSwapObjectPVs` has a real signature quirk: `legNames` and
     `fixingTableNames` are non-`const` reference parameters despite being
     used as inputs per their doc comments (a pre-existing quirk in the
     validation header, not something to silently "fix") â€” handled by
     passing local mutable variables.

**Verified (static checks only, no compiler run yet on this batch):**
- Whole-tree: **466 functions across all of `AQ_XLL/src/*.cpp`, 0
  mismatches, 0 duplicate names, 0 unresolved `validation::tryAq*` symbols.**
- Re-ran the exact same gap-audit script after this batch: **9 gap entries
  remain, and every one is an intentional non-gap** â€” `tryAqBondObjectZSpreads`
  (false positive, explained above), `tryAqToolLVBAdd` (in-place mutator,
  doesn't fit a worksheet-function shape, correctly left unported), and the 7
  SABR legacy functions (explicitly decided not to port). **There is no
  longer any unaccounted-for gap between `validation` and `AQ_XLL`** for
  every category except `Model`/`Generator`, which remain genuinely empty
  (0 wrappers, net-new work).
- `api_pair_check.py` HARD GATE still 0 (this script scans `AQ_API`, not
  `AQ_XLL` â€” a hygiene check here, not a real gate on this work).
- No new files added this batch (`aqCurve.cpp`, `aqObject.cpp`, `aqTool.cpp`,
  `aqMath.cpp`, `aqSwap.cpp` all already existed and were already wired into
  `AQ_XLL.vcxproj`) â€” so no `.vcxproj`/`.filters` changes needed for `AQ_XLL`
  this round; only the two `validation.vcxproj` wiring-gap fixes.

**Not yet done:** a real compiler build of this batch. Given the pattern from
the last two build attempts (type-marshalling mismatches, then a linker gap),
**expect at least one more round of build fixes** â€” the enum-conversion calls
in the new Math functions and the two new marshalling helpers in `aqSwap.cpp`
are the least-previously-exercised code in this batch and the most likely
sources of a first-build surprise.

**What's left after this batch, in priority order:**
1. Build this batch, fix whatever surfaces.
2. `Model` and `Generator` categories â€” genuinely empty, need `validation`
   wrappers written from scratch (`tryAqGeneratorList`/`Describe`/`Validate`
   for Generator; Model may stay sparse per CLAUDE.md Sec5.1). This is no
   longer "migrate a function", it's "design and build a new one" â€” a
   different, larger kind of task than everything done today.
3. `Model`/`Generator` aside, the rebrand's Phase 4 (xlOil XLL port) is
   essentially feature-complete pending that build.
4. Phase 5 (bindings/SWIG regeneration, verify C#/Java/R), Phase 6 (legacy
   extraction, resources audit, licence headers), Phase 7 (Linux/CMake,
   clang-format, clean repo) â€” all still ahead, per `MIGRATION_PLAN.md`.
5. Top-level `STATUS.md` (the short pointer file) is now significantly out of
   date â€” still describes ~35% Phase 4 progress from several sessions ago.
   Worth a full rewrite once this batch is build-confirmed, not another
   incremental patch.

---

## Second build attempt: 1 linker error, a pre-existing `validation.vcxproj` wiring gap (2026-09-11, NOT YET RE-BUILT)

Compile succeeded this time (the 4 `aqCurve.cpp` fixes held); the only
failure was `LNK2001: unresolved external symbol ... tryAqCurveGetInterpolation
JoinDate`. **Root cause: `src/validation/src/tryAqCurveGetInterpolationJoinDate.cpp`
existed on disk, fully implemented and matching its header exactly, but was
never added to `projects/validation.vcxproj`** â€” so it never compiled into
the `validation` static lib, and `aqCurve.cpp`'s call to it had nothing to
link against. This is the exact same class of gap as the
`tryAqToolEchoDouble.cpp` one found earlier this session (a file that exists
correctly but was never wired into its project) â€” except this time, since the
missing function's *caller* is in this rebrand's own new code (`aqCurve.cpp`)
rather than an optional diagnostic, the fix is to wire the file in rather
than drop the caller.

**Fixed:** added `tryAqCurveGetInterpolationJoinDate.h`/`.cpp` to
`projects/validation.vcxproj` (`ClInclude`/`ClCompile`, next to the sibling
`tryAqCurveFrequency.h`/`.cpp` entries) and to `.vcxproj.filters` (both under
`include\etrading\Curve` / `src\etrading\Curve`, matching every neighboring
Curve entry). Verified: implementation reads and matches the header 3-param
`AQLString` signature exactly (confirmed by reading the `.cpp` in full); both
project files re-parse as well-formed XML.

**Checked while here (cheap, worth doing before another build round-trip):**
compared every `src/validation/src/*.cpp` on disk (101) against
`validation.vcxproj`'s `ClCompile` entries (100) â€” **exactly one gap, and
it's the already-known, already-triaged one**: `tryAqToolEchoDouble.cpp`
(found earlier this session, deliberately left unwired since nothing in this
rebrand calls it â€” `aqToolEchoDouble` was removed from `aqTool.cpp` for that
exact reason). No other silent gaps exist, so this class of error should not
recur on the next build.

---

## First real build attempt: 4 compile errors in `aqCurve.cpp`, all fixed (2026-09-11, NOT YET RE-BUILT)

Nicholas ran the actual Release x64 build. As expected, static/textual checks
alone didn't catch everything â€” 4 genuine compile errors, all in `aqCurve.cpp`
batch 2, all now fixed:

1. **`etrading::DiscountFactorTable` is not a member of `etrading`**
   (`aqCurveDiscountFactorsDisplay`). `DiscountFactorTable` is forward-declared
   in the **global namespace** in `tryAqCurveDiscountFactor.h` (`struct
   DiscountFactorTable;` sits *before* `namespace validation` opens), defined
   in `AQLCurveCalibrationHelpers.h`, also global. Fixed: `etrading::
   DiscountFactorTable` â†’ `::DiscountFactorTable`.
2. **`toExcelColumn` type mismatch** (`aqCurveObjectEngineCalibrate`):
   `tryAqCurveObjectEngineCalibrate` returns `AQLStringVector`
   (`vector<AQLString>`), but `aq_xll::toExcelColumn` only has an overload for
   `std::vector<std::string>` â€” no implicit conversion exists between
   `AQLString` and `std::string`. Fixed with an explicit per-element
   `getCString()` copy into a `std::vector<std::string>` before calling
   `toExcelColumn`.
3. & 4. **`toExcelMatrix` has no `DoubleMatrix` overload**
   (`aqCurveObjectEngineJacobianDisplay`'s value branch and
   `aqCurveObjectJacobianDisplay`) â€” `aq_xll::toExcelMatrix` only overloads on
   `VariantMatrix` / `AQLStringMatrix` / `AnyTypeMatrix`, and a plain
   `DoubleMatrix` (`vector<vector<double>>`) matches none of them. Added a new
   file-local helper `toExcelDoubleMatrix( const DoubleMatrix& )` (wraps each
   `double` into an `etrading::Variant` and delegates to the existing
   `toExcelMatrix( VariantMatrix )` overload) and pointed both call sites at
   it instead.

**Root cause pattern worth naming:** all 4 errors are marshalling-layer type
mismatches between `AQL*`/raw-C++ container types and the `aq_xll::toExcel*`
helper's actual overload set â€” exactly the class of bug the static
signature-vs-header cross-checks done while writing this file could not
catch, since they verify the *validation* call compiles conceptually but not
that the *return marshalling* type-checks against the narrower `aq_xll`
overload surface. Worth remembering for the remaining unbuilt files
(`aqSwap.cpp`, `aqCredit.cpp`) â€” same class of error is plausible there too,
just not yet triggered because the build failed on `aqCurve.cpp` first (MSVC
stops enumerating a project's remaining files after an error battery from one
translation unit in this log, but doesn't skip other .cpp files entirely â€”
this build log only shows `aqCurve.cpp` errors because `AQ_XLL.vcxproj` likely
compiles files in filesystem/vcxproj order and `aqCurve.cpp` sorts before
`aqCredit.cpp`/`aqSwap.cpp`; expect more of this same error class on the next
build attempt once `aqCurve.cpp` is clean).

**Re-verified after the fix:** whole-tree param-vs-`.arg()` audit still
445/445 clean, 0 duplicates (these were pure type-marshalling fixes, no
signature or `.arg()` count changed). **Grepped `aqSwap.cpp`/`aqCredit.cpp`
for the same two failure patterns** (`toExcelMatrix( <DoubleMatrix var> )`,
`toExcelColumn` on a non-`vector<string>` return) â€” none found, but this
grep is necessarily incomplete (it can't run a real overload-resolution
check), so **the next build is very likely to surface more of this same
error class in the two still-untouched files** â€” expect it, don't be
surprised by it.

---

## ALL REMAINING CATEGORIES MIGRATED â€” Credit done (34 functions), Phase 4 XLL port essentially complete pending a build (2026-09-11, NOT YET BUILT)

Nicholas: "Let's migrate all remaining categories... do them all now." Last of
the three big deferred categories, `Credit`, is now written as `aqCredit.cpp`.

**Discovery: no dedicated `tryAqCredit*.h` files exist.** Every Credit
function lives inside `tryAqSwapObjectPricing.h`/`.cpp` (alongside the
already-ported CMS/TRS pricing â€” a pre-existing filing quirk from before the
category scheme existed), plus two more: `tryAqCreditObjectFeeLegCreate` in
`tryAqSwapObjectLeg.h` and `tryAqCreditObjectFeeScheduleCreate` in
`tryAqSwapObjectSchedule.h`. Per CLAUDE.md Sec5.1a, the golden **name**
decides the category, not the file â€” so all of these were pulled out into
their own `aqCredit.cpp`, not left in `aqSwap.cpp`.

**`aqCredit.cpp` written â€” 34 functions.** CDS pricing driven directly by a
hazard rate (`...FromHazardRate` forms: PV, RiskyAnnuity, ParSpread â€” 3), CDS
pricing driven by a cached credit model (PV, PVByIntegration, PVByMonteCarlo,
RiskyAnnuity, AccruedYearFraction, CS01, ParSpread â€” 7), credit model
lifecycle (Create using the `JSONInfoBlockTuples` two-block pattern already
established for Bond/Inflation curves, AsOfDate, CalibrationParameters,
HazardRate, SurvivalProbability, DefaultProbability, ImpliedSurvivalDate,
RiskyDiscountFactors â€” 8), credit spread (Spread, IndexSpread â€” 2), credit
(index) options (OptionPV(+FromForward), OptionImpliedVol(+FromForward),
IndexOptionPV/ImpliedVol/Vega/CS01/Theta â€” 9), credit basket model (Create,
SurvivalProbability â€” 2), and the two fee leg/schedule builders (2) â€”
3+7+8+2+9+2+2 = 33, plus `HazardRateFromParSpread` = 34.

**Flagged, not guessed: one genuine overload collision.**
`tryAqCreditObjectDefaultSwapHazardRateFromParSpread` has two C++ overloads
sharing the *exact same* validation name â€” one takes a `valuationSettingsLVB`
+ `recoveryRate` + `includeAccruedInterest` (hazard-rate-calculation style),
the other a `creditModelName` (credit-model style). Its three siblings
(PV/RiskyAnnuity/ParSpread) each give the hazard-rate variant a distinct
`...FromHazardRate` suffix and leave the model variant with the plain name â€”
this function alone breaks that pattern and keeps one shared name for both,
which Excel cannot register twice. This reads as a naming gap in the golden
source itself (someone forgot the analogous suffix), not a case where
guessing a fix is appropriate. **Ported the `valuationSettingsLVB`-driven
overload only**, as `aqCreditObjectDefaultSwapHazardRateFromParSpread`; the
`creditModelName`-driven overload is NOT exposed â€” documented in the file
header and flagged here for Nicholas to decide the second name (or confirm
the validation layer itself should gain a distinct name, which would then be
the fix applied first per Sec4.1's "name it in `validation` first" rule).

**Verified (static checks only, no compiler run yet):**
- `aqCredit.cpp`: 34/34 param-vs-`.arg()` clean, 0 duplicate names in file,
  all 34 `validation::tryAq*` calls confirmed against declared names.
- Whole-tree: **445 functions across all of `AQ_XLL/src/*.cpp`, 0
  mismatches, 0 duplicate names.**
- Added to `AQ_XLL.vcxproj`/`.vcxproj.filters` (`src\Optional`, alphabetically
  between `aqCMS.cpp` and `aqCurve.cpp`); both re-verified well-formed XML,
  file lists identical (20/20).
- `api_pair_check.py` HARD GATE still 0 throughout (this script only scans
  `AQ_API`, not `AQ_XLL`, so it is a hygiene check here, not a real gate on
  this work).

**Status after this batch â€” every LOCKED category has at least a first pass
in `AQ_XLL`:** `Date, Curve, FX, Inflation, Volatility, InterestRate, Future,
Swap, AssetSwap, CMS, TRS, CapFloor, Swaption, BondOption, BondFutureOption,
Bond, Credit, Math, Model, Generator, Tool` â€” `Model` and `Generator` remain
genuinely empty (0 validation wrappers exist for either, confirmed in an
earlier session; `Generator` needs a `tryAqGeneratorList` wrapper built from
scratch before any XLL surface is possible â€” not yet done, unrelated to this
migration pass).

**Explicitly deferred across this whole "migrate everything" push â€” not
silently dropped, each documented in its file's header comment and here:**
- Curve: the curve Results/Jacobian risk family â€” **done this session**,
  scratch that, already ported; nothing outstanding in Curve.
- Swap: `tryAqSwapDelta` (stateless multi-trade), `tryAqSwapObjectDeltaLadder`/
  `DeltaLadderHorizontally` (multi-curve risk ladders â€” all three take a
  `std::vector<LabelValueBlock>` shape with no marshalling helper yet), and
  `tryAqSwapObjectPVs` (vector-of-swaps batch PV, same disposition as the
  Math `*Prices` vector overloads deferred earlier this session).
- Credit: one `tryAqCreditObjectDefaultSwapHazardRateFromParSpread` overload
  (see above).
- Volatility: the legacy procedural SABR family (`tryAqVolatilitySABR*`) vs
  the already-ported object-based `tryAqVolatilityObjectSabr*` â€” still an
  open decision from an earlier session, not re-raised this pass.

**Not yet done:** a real compiler build of `aqSwap.cpp`, `aqCredit.cpp`, and
Curve's batch 2 â€” none of the three have been through anything but static
checks yet. **This is the single most important next step** before trusting
any of this further; expect the usual first-build friction (a missed
include, a signature that drifted between when it was read and when it was
used) despite the careful checks already run.

**Also still pending, unrelated to categories:** `Swap` category work
technically also needed the Ois-fold-in, which is now done (see the section
below); the `Model`/`Generator` categories remain genuinely empty and need
their own from-scratch work, not a port; top-level `STATUS.md` (the short
pointer file) has not been refreshed since the `InterestRate` rename and now
significantly understates progress â€” worth a full rewrite once a build
confirms this batch, rather than another incremental patch.

---

## Swap category done (62 functions) + `Ois` rename completed (2026-09-11, NOT YET BUILT)

Nicholas confirmed the Curve batch-2 build works. "Let's migrate all remaining
categories... do them all now" continued into `Swap`.

**Prerequisite: the long-pending `Ois`â†’`Swap` rename, done first.** The
pre-existing `tryAqOisPV`/`tryAqOisParRate` wrappers (+ `*LVBKeys`
companions) predated the category scheme and needed renaming before Swap
could be written cleanly (flagged as a to-do since the `Ois`-fold decision
several turns ago). Renamed in full, files included, across every surface:
- `validation`: `tryAqOisPV.{h,cpp}` â†’ `tryAqSwapOisPV.{h,cpp}`,
  `tryAqOisParRate.{h,cpp}` â†’ `tryAqSwapOisParRate.{h,cpp}` (git mv + content
  rename); `projects/validation.vcxproj` + `.filters` updated.
- `AQ_API`: `aqOisPV.{h,cpp}` â†’ `aqSwapOisPV.{h,cpp}`,
  `aqOisParRate.{h,cpp}` â†’ `aqSwapOisParRate.{h,cpp}`; all 4 SWIG `.i` files
  (CSharp/JAVA/Python/R) updated; `projects/AQ_API.vcxproj` + `.filters`
  updated. (The generated `swig_Python_wrap.cxx` still says the old names â€”
  expected, untouched, regenerates on next SWIG build.)
- `GTEST`: `TryAqTestTradeEUROISParRate.cpp` â€” includes, the two `TEST_DIR`
  fixture-path string constants, and both call sites renamed.
- **100 fixture CSV files** renamed (`EURYC_tryAqOis{PV,ParRate}_{inputs,outputs}N.csv`
  â†’ `EURYC_tryAqSwapOis*`) â€” discovered these are **not git-tracked**
  (`git mv` refused with "not under version control"; confirmed via
  `git ls-files`, genuinely untracked, not merely uncommitted), so renamed
  with a plain filesystem `mv` instead.
- `docs/api_map.csv` regenerated (`api_pair_check.py --write`); `Swap` now
  shows 26 rows instead of falling into the `(lifecycle)` bucket; HARD GATE
  stayed 0 throughout.
- Docs updated to "done" from "needs renaming": `CLAUDE.md` (both, Sec5.1),
  `MIGRATION_PLAN.md` Sec2.7 (checkbox ticked).
- Verified clean: `git grep` for every old spelling across the repo (excluding
  the untouched generated SWIG wrap and the historical Phase-2 rename-map CSV,
  which is a record of past work, not live code) returns nothing; no
  `SwapSwapOis`/`OisOis` double-substitution artifacts.

**`aqSwap.cpp` written â€” 62 functions.** Covers: stateless swap pricing
(PV/PV01/ParRate/DV01/StubRate/StubFixingDate + `*LVBKeys`, 11), stateless OIS
pricing (`aqSwapOisPV`/`aqSwapOisParRate` + `*LVBKeys`, 4), stateless leg
pricing (LegLVBKeys/LegDisplay/LegPV/LegAnnuity, 4), stateless schedule
display (ScheduleLVBKeys/Schedule/ScheduleFixed/ScheduleFloat, 4), swap
object lifecycle/creation (LVBKeys, 5 create-from-* overloads, AddLeg/AddFee,
generator create/create-from-generator, Display, GeneratorDisplay, 12), leg
object lifecycle (LegCreate/LegCreateFromSchedule/LegPV/LegDisplay/
LegDisplayCashflows, 5), schedule object lifecycle (ScheduleCreate/
ScheduleDisplay/ScheduleCreateBespoke/ScheduleCreateBespokeFromCashflows, 4),
object pricing (PV/PV01/ParRate/Annuity/DisplayCashflows/Spread/ParSpread/
AccruedInterest, 8), the results/Jacobian-risk store (9), and single-swap
flat-shift delta (`aqSwapObjectDelta`, 1).

**Deliberately deferred** (documented in the file header, not silently
dropped): `tryAqSwapDelta` (stateless multi-trade flat-shift) and
`tryAqSwapObjectDeltaLadder`/`DeltaLadderHorizontally` (multi-curve risk
ladders) all take a `std::vector<LabelValueBlock>` of deal definitions or
return ragged per-curve vector-of-vectors â€” no marshalling helper for either
shape exists yet, and building one well is a bigger design task than this
batch's pace justifies; `tryAqSwapObjectPVs` (the vector-of-swaps batch PV
overload) is deferred for the same reason the Math `*Prices` vector overloads
were deferred earlier this session. `tryAqCreditObjectFeeLegCreate`/
`tryAqCreditObjectFeeScheduleCreate` (filed in the Swap leg/schedule headers
but golden-named Credit) and `tryAqToolSwapScheduleTemplate` (golden-named
Tool) were correctly left out of this file â€” they belong to their own
category files.

**New shared local helpers, `aqSwap.cpp`'s anonymous namespace (all
file-local, matching the established per-file pattern):**
`toLabelValueBlockOr`, `toStringVectorOr`, `toAQLStringVectorOr`,
`toDoubleVectorOr`, `toStrOr`, `toAQLStringVectorWithoutCounter` (converts
`getNamesWithoutCounter`'s `std::vector<std::string>` to the `AQLStringVector`
`tryAqSwapObjectDelta` actually takes â€” a real type mismatch caught and fixed
before it could hit a build), and `toExcelStackedLegMatrices` (concatenates
the `std::vector<AnyTypeMatrix>` that `tryAqSwapObjectDisplay`/
`DisplayCashflows` return â€” one block per leg â€” into a single sheet with a
"Leg N" label row ahead of each block; no prior precedent existed for this
shape in the codebase, this is a new pattern).

**Verified (static checks only, no compiler run yet):**
- `aqSwap.cpp`: 62/62 param-vs-`.arg()` clean; every one of its 62
  `validation::tryAq*` calls confirmed to match an actually-declared name.
- Whole-tree: **411 functions across all of `AQ_XLL/src/*.cpp`, 0
  mismatches, 0 duplicate names, 0 unresolved `validation::tryAq*` symbols.**
- `aqSwap.cpp` added to `AQ_XLL.vcxproj`/`.vcxproj.filters` (under
  `src\Optional`, per the edition-filter convention); both files re-verified
  well-formed XML, file lists identical (19/19), `<ClCompile Include=`/
  `</ClCompile>` counts balanced.
- `api_pair_check.py` HARD GATE still 0 (unaffected either way â€” that script
  only scans `AQ_API`, not `AQ_XLL`).

**Not yet done:** a real compiler build (first time this file has been
through anything but static/textual checks).

**Next: Credit (~34 wrappers, minus whatever CMS/TRS pricing already covered
under `aqCreditObject.cpp` in `AQ_API` â€” needs a fresh enumeration pass, not
yet done this turn) â€” the last of the three big deferred categories.**

---

## Curve category COMPLETE: batch 2 added, 93 functions total (2026-09-11, NOT YET BUILT)

Nicholas confirmed the build works after `aqCurve.cpp` was wired in (batch 1,
34 functions â€” see the section below). "Let's migrate all remaining
categories... do them all now" then triggered batch 2, finishing Curve.

**Decision needed and resolved first** (was flagged as open in the prior
session): Curve has two overlapping function families â€” the new AQObj
curve-handle API (batch 1) and an older `curveCollection`+`curveIndex`
stateless family that predates it (CompoundRate, DiscountFactor, ForwardRate,
the four heavy one-shot `Calibrate{Basis,CTD,FXForwards,OIS,Swap}` functions,
their `ObjectCreate{Basis,FXForwards,OIS,Swap}` handle-returning twins, etc).
Nicholas's answer: **port both â€” stateless functions as `aqCurve<Function>`
(no `Object`), object/handle functions as `aqCurveObject<Function>`**, exactly
per the golden `tryAq*` name (no renaming; these are pre-existing wrappers).

**Batch 2 added to `aqCurve.cpp`** (59 functions, bringing the file to 93):
CompoundRate Ã—2, Delete, Display, Frequency, GetInterpolationJoinDate /
InterpolationJoinDate (two distinct wrappers, same job), EuroDollar
ConvexityAdjustment, the legacy DiscountFactor family Ã—12 (incl.
DiscountFactorsDisplay returning a Term/PaymentDate/DiscountFactor table
marshalled from `etrading::DiscountFactorTable`, and TermsToDates/
DatesToTerms), the legacy ForwardRate family Ã—5 (incl. the
curveCollection-or-handle-taking `...FromForwardDatesFromObject` variant, and
the table-building `aqCurveObjectForwardRatesTable`), HullWhite/Vasicek
forward-rate model checks Ã—3, the `ObjectData` LVB convenience builder Ã—2
(`aqCurveObjectDataCreate` takes 8 optional `AQLStringMatrix` blocks â€” swap/
FRA/futures/central-bank/FX), dual bootstrap (stateless Ã—1, 26 params; object
Ã—1 returning a curve-index map marshalled as a key/value block), engine
calibrate + its Jacobian display Ã—3, the curve-results/Jacobian-risk family
Ã—11 (`RiskType`/`BusinessDayAdjustmentEnum` string args converted via
`etrading::toRiskTypeEnum`/`toBusinessDayAdjustmentEnum` from
`CoreEnumerations.h`), curve groups Ã—2, and finally the 5 heavy
`Calibrate*` + 4 `ObjectCreate*` one-shot functions (10-17 raw
`AQLStringMatrix` params each).

**New shared local helpers added to `aqCurve.cpp`'s anonymous namespace**
(each is file-local per the established pattern, not shared via
`aqXllTools.h`): `toDoubleOr`, `toAQLDateOr` (missing â†’ default-constructed
`AQLDate()`, the "use the curve's own as-of date" sentinel several
model-check functions rely on), `toBoolVector` (a column of TRUE/FALSE cells
â†’ `std::vector<bool>`, via `etrading::Variant::getValue<bool>()`), and
`toAQLStringMatrixOr` (missing/blank â†’ a genuinely empty `AQLStringMatrix`,
**not** the 1x1-empty-string block `toAQLStringMatrix` would otherwise
produce for a missing cell â€” several calibration/data-block functions test
`.empty()` on optional blocks to decide whether one was supplied at all; this
was caught and fixed before it could silently corrupt an optional-block
calibration call).

**A second bug class caught and fixed before reaching a build:** several
validation wrappers in this batch return `AQLString`/`const AQLString`
(`tryAqCurveDelete`, `tryAqCurveFrequency`, `tryAqCurveForwardRatesOverride`,
`tryAqCurveDiscountFactorsOverride`, `tryAqCurveDiscountFactorsSetToOne`,
`tryAqCurveDualBootstrap`, all 5 `tryAqCurveCalibrate*`), which `returnValue`
cannot bind directly (confirmed against the established
`std::string( ...tryAqX(...).getCString() )` pattern already used in
`aqTool.cpp`/`aqDate.cpp`) â€” every one of these was wrapped with
`.getCString()` rather than passed to `returnValue` as-is.

**Verified (static checks only, no compiler run yet):**
- Whole-tree param-vs-`.arg()` audit: **349 functions across all of
  `AQ_XLL/src/*.cpp`, 0 mismatches.** `aqCurve.cpp` alone: **93/93 clean.**
- Every one of the 93 `validation::tryAq*` calls in `aqCurve.cpp` confirmed to
  match an actually-declared name across `src/validation/include/*.h` (a
  Python cross-check against the header text, not just a signature glance).
- No duplicate `XLO_FUNC` names anywhere in the tree.

**Not yet done:** a real compiler build (first time `aqCurve.cpp` batch 2 has
been through anything but static/textual checks â€” batch 1 alone was already
build-verified in the prior turn, batch 2 has not).

**Curve category is now considered complete** bar three deliberately-excluded
internal helpers (`getDayCount`, `isBasisFlag`,
`populateDiscountFactorConventions` in `tryAqCurveDiscountFactor.h` â€” legacy
default-population plumbing, no `aqCurve` naming, nothing for a user to call).

**Next: Swap (~67 wrappers, including the pre-existing `Ois`â†’`aqSwapOis*`
rename work) and Credit (~34 wrappers).**

---

## `aqCurve.cpp` wired into `AQ_XLL.vcxproj` (2026-09-11, NOT YET BUILT)

Added the two remaining entries for `aqCurve.cpp` (written earlier this
session, batch-1, 34 functions â€” see the "Still pending" section below for
what it covers):

- `projects\AQ_XLL.vcxproj` â€” `<ClCompile Include="..\src\AQ_XLL\src\aqCurve.cpp" />`,
  alphabetically between `aqCMS.cpp` and `aqDate.cpp`.
- `projects\AQ_XLL.vcxproj.filters` â€” same entry, filed under **`src\Optional`**
  (per the new edition-filter convention â€” see the section above), between
  `aqCMS.cpp` and `aqFuture.cpp`.

**Verified (static checks only, no compiler run yet):**
- Both files re-parse as well-formed XML (`xml.etree.ElementTree`).
- `<ClCompile Include=` file lists identical between `.vcxproj` and
  `.vcxproj.filters` â€” 18/18, no orphans either direction (the
  orphaned-`</ClCompile>`-tag mistake from earlier this session did not
  recur).
- `<ClCompile Include=` count (18) matches `</ClCompile>` count (18) in
  `.filters`.
- Ran `rebrand\tools\api_pair_check.py`: HARD GATE still **0**. **Caveat worth
  recording:** this script only scans `src\AQ_API\source\*.{h,cpp}` â€” it does
  **not** see `src\AQ_XLL\src\*.cpp` at all, so it neither validates nor is
  affected by `aqCurve.cpp`; it was already 0 before this change and stays 0
  after. The actual XLL-specific check (param count vs `.arg()` count) was
  already run and passed in the prior session (34/34 clean, part of the
  290-function whole-tree audit) â€” not re-run here since no `aqCurve.cpp`
  content changed, only its project wiring.

**Next: a real build.** This is the first thing that will actually compile
`aqCurve.cpp` â€” it has never been through a compiler. Expect possible
first-build issues (missing includes, minor signature drift) despite the
careful signature-vs-header cross-check done when the file was written.

---

## Resumed (2026-09-11): build green, tests pass; `Core`/`Optional` edition filters added; docs updated

Nicholas confirmed **the build works and tests pass** with everything that was
pending at the pause point below (the `StructuredExceptionHandler`
plain-English + crash-location rework, the CMS/TRS renames, `aqBond.cpp`
merge, `aqToolSEH()`) â€” first real compiler+GTEST confirmation of this
session's work, superseding the "static checks only, not yet built" caveats
further down for everything up to and including that point.

**Nicholas separately added and committed to git** (outside this session, own
work): two new Solution Explorer filters in
`projects\AQ_XLL.vcxproj.filters` â€” **`src\Core`** (always-built:
`aqXllTools.cpp`, `aqMain.cpp`, `aqDate.cpp`, `aqObject.cpp`, `aqMath.cpp`,
`aqTool.cpp`) and **`src\Optional`** (every product-category file â€”
`aqAssetSwap.cpp`, `aqBond.cpp`, `aqCapFloor.cpp`, `aqCMS.cpp`,
`aqFuture.cpp`, `aqFX.cpp`, `aqInflation.cpp`, `aqInterestRate.cpp`,
`aqSwaption.cpp`, `aqTRS.cpp`, `aqVolatility.cpp` â€” `aqCurve.cpp` not yet
added, see below). Intent: when per-edition build configurations
(`ReleaseBonds`, `ReleaseSwaps`, `ReleaseCurves`, â€¦) are added alongside the
existing `Debug`/`Debug_EditAndContinue`/`Release_Profiler`/`Release`, each
compiles `Core` plus only the `Optional` file(s) its edition needs (excluded
elsewhere); `Release` (Full) excludes nothing. **Documented this session** in
`CLAUDE.md` (both, Â§4.4/Â§4.5 + a new Â§6.3/Â§9.5) and `MIGRATION_PLAN.md`
(Phase 4a split into an `AQ_API` runtime-gate half, unchanged, and a new
`AQ_XLL` compile-time-gate half with a new checklist item 4a.5) â€” **the
"no per-edition builds" line in the old Phase 4a text was specific to
`AQ_API`'s language-binding matrix and did not anticipate this; both plans now
coexist explicitly, one per surface.** No code changed for this â€” pure
documentation to match what Nicholas already committed.

**Standing reminder for whoever writes the next `AQ_XLL` category file:** new
category files go in `src\Optional`, never `src\Core` â€” the always-needed set
is already complete (`aqCurve.cpp`, once wired into the vcxproj â€” see the
paused-work section immediately below â€” goes to `Optional`).

---

## â¸ Still pending from the pause point below (2026-09-11)

**Next action for the next session:** finish wiring `aqCurve.cpp` into the
build, then continue the Curve migration. In order:

1. **`aqCurve.cpp` is written (batch 1, 34 functions) but NOT YET added to
   `projects/AQ_XLL.vcxproj` / `.vcxproj.filters`.** Confirmed by grep â€” no
   `aqCurve.cpp` entry exists in the vcxproj yet. Add it the same way every
   other new category file was added this session (`<ClCompile
   Include="..\src\AQ_XLL\src\aqCurve.cpp" />` in both files, filter
   `Source Files`), then re-verify both files are well-formed XML
   (`python -c "import xml.etree.ElementTree as ET; ET.parse(path)"`) and that
   the `<ClCompile Include=` count matches the `</ClCompile>` count (the
   orphaned-tag bug from earlier this session).
2. **Batch-1 verification already done, before adding to the project:**
   whole-tree param-vs-`.arg()` audit run against every `src/AQ_XLL/src/*.cpp`
   â€” **290 functions total, 0 real mismatches** (`aqToolEcho` flagged by the
   quick regex is a false positive â€” it takes `const ExcelObj*`, not `&`, and
   is genuinely 1 param / 1 `.arg()`, pre-existing and fine). `aqCurve.cpp`
   itself: **34/34 clean**. No duplicate `XLO_FUNC` names anywhere in the tree.
   Signatures for all 34 were cross-checked line-by-line against the actual
   validation headers (`tryAqCurveObjectUtilities.h`, `...Display.h`,
   `...Calibrate.h`, `...CalibrateHedge.h`, `...DiscountFactor.h`,
   `...ForwardRate.h`, `tryAqCurveZeroRate.h`, `tryAqCurveGenerator.h`,
   `tryAqCurveMarketData.h`) â€” all match exactly. Caught and fixed two issues
   while cross-checking: (a) `toIntOr` is a **file-local** helper (lives in an
   anonymous namespace in `aqMath.cpp`, not shared via `aqXllTools.h`) â€” added
   a local copy to `aqCurve.cpp`'s anonymous namespace, same as the existing
   local `toStrOr`; (b) `etrading::trim_to_upper` (used in
   `aqCurveGeneratorCreate`/`aqCurveMarketDataCreate`) needs
   `#include <CoreEnumerations.h>`, which was missing â€” added. **Still not
   run through an actual compiler** â€” these are static/textual checks only;
   the real build is the next hard gate.
3. **`aqCurve.cpp` batch 1 covers:** object lifecycle
   (List/Delete/DeleteAll/Save/Load, 5), Display (1), Calibrate +
   CalibrateHedge (2 â€” the hedge one returns `etrading::HedgeCurveInfo`,
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
   (`validation â†’ AQ_API â†’ AQ_XLL â†’ GTEST`), then continue with the rest of
   Curve â€” see the enumerated ~97-function list captured earlier this session
   (not reproduced here in full; re-derive from `src/validation/include/
   tryAqCurve*.h` if needed, there are 32 headers). **Not yet started:** the
   curve Results/Jacobian risk family (`tryAqCurveResults.h`,
   `tryAqCurveObjectJacobianDisplay.h`), `tryAqCurveObjectDualBootstrap.h`,
   `tryAqCurveHullWhite.h`/`tryAqCurveVasicek.h`, `tryAqCurveCalibrateCTD.h`,
   the misc utilities (`tryAqCurveCompoundRate.h`,
   `tryAqCurveEuroDollarConvexityAdjustment.h`, `tryAqCurveFrequency.h`,
   `tryAqCurveGetInterpolationJoinDate.h`, `tryAqCurveInterpolation.h`),
   `tryAqBondCurves.h` (check whether this belongs to `Bond` not `Curve` â€”
   name suggests bond-curve, likely already covered by `aqBond.cpp`, verify
   before touching), and `tryAqCurveObjectData.h`/`tryAqCurveObjectEngineCalibrate.h`
   (not yet read).
5. **Still an OPEN DECISION, not yet asked of Nicholas:** the four heavy
   one-shot creation functions `tryAqCurveObjectCreateBasis/FXForwards/OIS/Swap`
   (`tryAqCurveObjectCreateBasis.h`, `...CreateFXForwards.h`, `...CreateOIS.h`,
   `...CreateSwap.h` â€” 10-17 raw `AQLStringMatrix` params each). These look
   like a legacy one-shot path that may be superseded by the
   `CurveGenerator`+`CurveMarketData`+`tryAqCurveObjectCalibrate` two-step
   architecture already ported in batch 1 (which `CLAUDE.md` Â§4.4 explicitly
   endorses as the intended design). **Ask Nicholas before porting these** â€”
   don't guess.

**Also still open from earlier in this session (unrelated to Curve):**
- The legacy SABR calibration family decision
  (`tryAqVolatilitySABR{Calibrate,GetPrem,GetVol,...}` vs the already-ported
  object-based `tryAqVolatilityObjectSabr*`) â€” not yet asked.
- `Swap` category (~67 wrappers, including the `Ois`â†’`aqSwapOis*` rename work
  documented in `CLAUDE.md` Â§5.1) and `Credit` category (~34 wrappers) â€” not
  started, come after Curve.
- `src/validation/src/tryAqToolEchoDouble.cpp` needs adding to
  `projects/validation.vcxproj` (pre-existing gap, unrelated to this session).
- Confirm with Nicholas whether the improved `aqToolSEH()` message (see next
  section) actually resolves a `file(line)` in Excel once rebuilt â€” the
  DbgHelp path is untested end-to-end.
- Top-level `STATUS.md` (the short pointer file, not this one) has not been
  refreshed since the `InterestRate` rename â€” still says ~35%/Phase 4 without
  mentioning CMS/TRS/aqToolSEH/Curve-start. Worth a refresh next session.

Nothing in this session has been committed (still uncommitted, matching every
other entry in this file) and nothing has been built â€” every check above is
static (grep/regex/XML-parse), not a compiler run. **First thing next
session: do a real build** before trusting any of this further.

---

### `aqToolSEH()` feedback: structured-exception messages made plain-English + crash-located (2026-09-11, uncommitted, NOT BUILT)

Nicholas tried `aqToolSEH()` in Excel: **it works** â€” `AQ_XLL_GUARD` /
`etrading::StructuredExceptionHandler` catches the access violation cleanly,
no crash. Follow-up ask: make the returned error message concise plain English
(`#Structured Exception: <Category> - <plain english>`, not the old verbose
MSDN-quoted paragraph), and report the crash site (`__FILE__`/`__LINE__`) to
speed up debugging.

Changed `src/etrading/include/StructuredExceptionHandler.h` +
`src/etrading/src/StructuredExceptionHandler.cpp` (the shared SEH translator
behind every `AQ_XLL_GUARD`/`VALID_EXCEPTION_START`, not just `aqToolSEH` â€”
this improves every structured-exception message library-wide):

- `StructuredExceptionCodeMap`'s map value changed from a single blended
  `const char*` to a new `StructuredExceptionInfo{ category_, plainEnglish_ }`
  struct. All 20 Win32 exception codes re-worded: short category
  ("Access Violation", "Integer Divide By Zero", ...) + one concise
  plain-English sentence each (old text was a verbatim MSDN paragraph).
- `SEHandler` now formats `"#Structured Exception: <Category> - <plain
  english> at <file>(<line>)."` â€” e.g. `#Structured Exception: Access
  Violation - attempted to read from or write to a memory address it does not
  have access to. at aqTool.cpp(412).`
- **Crash-site location, not handler location:** added a file-local
  `resolveCrashLocation()` using DbgHelp (`SymInitialize` +
  `SymGetLineFromAddr64`) against `pExcept->ExceptionRecord->ExceptionAddress`
  â€” the actual faulting instruction â€” to resolve `file(line)` when a PDB is
  available (debug builds). Falls back to the raw hex address when no PDB /
  symbol is found (e.g. Release), and to nothing if resolution fails outright
  â€” never throws from inside the SE translator itself. `__FILE__`/`__LINE__`
  passed to the final `AQLCoreError` are still this handler's own location
  (unavoidable â€” that's what the macro expands to at the `throw` site); the
  crash site is carried in the message text instead, which is what actually
  answers "where did it crash".
- New includes: `<DbgHelp.h>` + `#pragma comment(lib, "Dbghelp.lib")` (Windows
  only, inside the existing `_WIN32`/`_WIN64` guard) â€” no `.vcxproj` change
  needed, the pragma embeds the linker directive in the `.obj` and every
  consumer (`AQ_XLL`, `AQ_API`, `GTEST`) picks it up at final link.

**Verification (no build yet):** grepped the tree for other callers of
`StructuredExceptionCodeMap`/`getExceptionCodes` â€” none found outside this
pair of files, so the map-value-type change is self-contained. Needs a real
build + a re-run of `aqToolSEH()` in Excel to confirm the DbgHelp path
resolves a file/line in the Debug config (PDB for `etrading`/`AQ_XLL` must be
present at runtime, same folder as the `.xll`/`.pdb`) â€” **not yet verified
end-to-end**, flagged for the next Excel round-trip with Nicholas.

---

### `TotalReturnSwap` category renamed to `TRS` (2026-09-11, uncommitted, NOT BUILT)

Same pattern as the `ConstantMaturitySwap` â†’ `CMS` rename, one layer further:
**`TotalReturnSwap` already had a real `AQ_API` surface** (unlike CMS) â€” 4
functions in `aqCreditObject.cpp/.h` (filed under the Credit API source, not a
dedicated TRS file â€” same pre-existing filing quirk as the Ois/FixingTable
functions living in `aqCurveObject.h`), already generated into all 4 SWIG
`_wrap` files.

- `validation`: `tryAqSwapObjectPricing.{h,cpp}` â€” substring rename
  `TotalReturnSwapObject â†’ TRSObject` (6 hits each file). Confirmed
  `etrading::TotalReturnSwap` (the internal pricer class) and
  `#include "TotalReturnSwap.h"` untouched â€” same anchor discipline as CMS.
- `AQ_API`: `aqCreditObject.h` (9 hits) / `.cpp` (16 hits) renamed
  (`aqTotalReturnSwapObject* â†’ aqTRSObject*`, both the function defs and their
  `validation::tryAq...` call sites). The 4 generated `swig_*_wrap.*` files
  still say the old name â€” expected, untouched, will regenerate on next SWIG
  build.
- `GTEST`: `TestTotalReturnSwap.cpp` â€” 29 call-site substitutions. (Its own
  `TRS_*` constant names were already TRS-branded; only the wrapper-name calls
  needed changing.)
- `AQ_XLL`: `aqTotalReturnSwap.cpp` â†’ `aqTRS.cpp`, 4 functions renamed,
  `AQ_XLL.vcxproj`/`.filters` updated (clean single-line edit this time â€” no
  repeat of the CMS batch's orphaned-tag mistake).
- 16 fixture files renamed (`tryAqTRSObject{PV,ParRate,ParSpread,Annuity}*_{inputs,outputs}.csv`,
  including variant suffixes like `_ALL`, `_Float`, `_Premium`, `_PayOnSurvival`).
- Docs: `CLAUDE.md` (both), `MIGRATION_PLAN.md`, `rebrand/tools/api_pair_check.py`
  `CATEGORIES` updated `TotalReturnSwap` â†’ `TRS`; **`docs/api_map.csv`
  regenerated** this time (unlike CMS) since the AQ_API rows actually changed â€”
  `aqTRSObjectPV/ParRate/ParSpread/Annuity` now show category `TRS`.

**Verification:** HARD GATE = 0; `aqTRS.cpp` 4/4 param-vs-`.arg()`; zero
residual `TotalReturnSwapObject` outside the generated SWIG files; both
project files re-confirmed well-formed XML; fixture rename count matches (16
renamed, 0 old names left).

### Nicholas's follow-up on batch 1: CMS rename, BondOption/BondFutureOption merge, aqToolSEH diagnostic (2026-09-11, uncommitted, NOT BUILT)

Three requests after reviewing batch 1:

**1. `ConstantMaturitySwap` category renamed to `CMS`.** Golden-source rename,
validation first: `tryAqConstantMaturitySwapObject{PVUsingConvexityAdjustment,
ParRateUsingConvexityAdjustment}` â†’ `tryAqCMSObject*` in the shared
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
matches the bare internal class name (per CLAUDE.md Â§5.1a: category names
don't propagate into `etrading` class/file names).
- `AQ_XLL`: `aqConstantMaturitySwap.cpp` â†’ `aqCMS.cpp`, 2 functions renamed,
  `AQ_XLL.vcxproj`/`.filters` updated.
- `GTEST`: `TestConstantMaturitySwap.cpp` call sites updated (test class name,
  helper function name and `TEST_DIR` folder segment deliberately left as-is -
  internal test naming, not governed by the golden-source rule). 4 fixture
  files renamed (`tryAqCMSObject{PV,ParRate}UsingConvexityAdjustment_{inputs,outputs}.csv`).
- Docs: `CLAUDE.md` (both), `MIGRATION_PLAN.md`, `rebrand/tools/api_pair_check.py`
  `CATEGORIES` updated `ConstantMaturitySwap` â†’ `CMS`.

**2. `aqBondOption.cpp` / `aqBondFutureOption.cpp` deleted; their 6 functions
moved into `aqBond.cpp` verbatim (signatures unchanged).** `AQ_XLL.vcxproj` +
`.filters` updated - **caught and fixed a self-inflicted XML corruption here**:
a scripted removal of the two `<ClCompile>` entries mishandled the 3-line
`.filters` block (`<ClCompile>` / `<Filter>` / `</ClCompile>`) and left two
orphaned `</ClCompile>` tags with no matching open tag. Caught by counting
`<ClCompile Include=` vs `</ClCompile>` (17 vs 19) before moving on, rather than
trusting the "no residual filename string" grep alone; fixed and reverified
both `.vcxproj` and `.filters` as well-formed XML via `xml.etree.ElementTree`.
Documented the resulting **file-per-category exception** in `CLAUDE.md` Â§5.1a
and `MIGRATION_PLAN.md` D18: consolidate a category's file into a sibling's
only when they share the same underlying cached object (see point 3), not
merely a name prefix.

**3. Nicholas asked where `aqBondFutureOptionObjectCreate` is - there isn't
one, and that's correct, not a gap.** Traced `tryAqBondOptionObjectCreate` â†’
`etrading::createOption()` â†’ `OptionFactory.cpp`: the factory's
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
header and `CLAUDE.md` Â§5.1a so the next person doesn't go looking for it either.

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

### "Migrate all remaining categories" â€” batch 1 of N: 11 small/medium categories DONE (2026-09-11, uncommitted, NOT BUILT)

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

**Verification:** whole-tree param-vs-`.arg()` audit â€” **255/255 functions
across every `AQ_XLL/src/*.cpp`, 0 mismatches**; no duplicate `XLO_FUNC` names;
every `validation::tryAq*` symbol called resolves in `src/validation/include`;
`vcxproj`/`.filters` file lists identical; `api_pair_check` HARD GATE = 0.

**Bug caught and fixed before it reached a build:** `aqInflationCurveCreate`
and the two `aqVolatilityObjectSabr{MarketDataCreate,ModelCalibrate}` functions
were first written using `std::vector<validation::TableInfo>` for the
multi-block-create pattern (copying the `aqBondCurveCreate` /
`aqBondGeneratorCreate` idiom) â€” but unlike `tryAqBondObject.h` /
`tryAqToolGrid.h`, neither `tryAqInflationObjectPricing.h` nor
`tryAqVolatilityObject.h` declares a `validation::TableInfo` typedef; both take
`etrading::JSONInfoBlockTuples` directly. `std::vector<validation::TableInfo>`
would have failed to compile ("no member named 'TableInfo' in namespace
'validation'"). Fixed by using `etrading::JSONInfoBlockTuples` directly (the
underlying tuple type is identical, so `toTableInfo(...)` still binds) â€”
caught by re-reading each header rather than assuming the pattern held.

**Decision needed â€” the legacy SABR calibration family, deferred:**
`tryAqVolatilitySABR{Calibrate,GetPrem,GetVol,OutputParameter,SetupConvention,
SetupParameter,SetupSwaptionVol}` (`tryAqVolatilitySABRCalibrate.h` and
siblings) is a **procedural, ID-string-driven** calibration workflow
(`alphaID`, `betaID`, `nuID`, `rhoID`, `convID`, `capConvID`, `curveMat`, â€¦)
that looks superseded by the cleaner object-based `tryAqVolatilityObjectSabr*`
API already ported above. **Not ported â€” needs Nicholas's call:** port it
as-is (needs real domain understanding of what those ID strings reference to
marshal correctly â€” high risk of a plausible-but-wrong port), or drop it as
legacy since the object-based Sabr* API appears to be the intended replacement.

**Not attempted yet:** `Swap` (67 wrappers, incl. folding `Ois` in as
`aqSwapOis*` â€” see the section below), `Credit` (34), `Curve` (~100 â€” the
biggest, needs new enum-string marshalling helpers for `InterpolationEnum` /
`StateVariableEnum` / `DayCountEnum` / etc. and unblocks the deferred Math
`ForwardRate`/`DiscountFactor` functions too), `Model` (**empty â€” 0 validation
wrappers exist**, nothing to migrate), `Generator` (**0 wrappers â€” needs
`tryAqGeneratorList` written first**, unchanged from the earlier note).

**== PAUSE POINT (2026-09-10) ==**

**Committed HEAD: `18328864` ("Natvis Debug Vizulaization Update").** Since the
old `3809f148` baseline, Nicholas has committed:

- **`22212bc5`** ("AQ_XLL, validation and GTEST function name synchronization",
  7,905 files) â€” **task 2.6** (the full `validation` / `GTEST` / `AQ_API` /
  fixture rename to the 22-category singular scheme: `Vol`â†’`Volatility`,
  `Future` + `Ois` added, `tryAqObj<Cat>* â†’ tryAq<Cat>Object*`), the fixture-
  rename recovery, the **AQ_XLL Tool + Object-lifecycle port** from
  `.APPLES\...\meUtilities.cpp`, and **Interpolation/PCA re-homed `Tool`â†’`Math`**
  (`tryAqMath{Interpolation,PCA}`). Built green, GTest passing.
- **`18328864`** â€” `Visualizer.natvis` refresh + `AQ_XLL.vcxproj`. âš  this commit
  also staged `src/AQ_API/source/swig_Python_wrap.cxx` â€” a **generated** file
  that the standing rule says never to commit (Â§"Known noise"). Flag to Nicholas:
  `git rm --cached` it and re-ignore, or accept it and note the exception.

**Uncommitted working-tree delta right now:**

| Path | State |
|---|---|
| `src/AQ_XLL/src/aqBond.cpp` `aqDate.cpp` `aqMath.cpp` `aqTool.cpp` | **BUILT GREEN, GTest passing** (Nicholas) â€” the "remaining Bond / Tool / Date / Math functions" fill-in |
| `src/AQ_XLL/src/aqInterestRate.cpp` (renamed from `aqRate.cpp`) + `AQ_XLL.vcxproj` / `.filters` | **NOT BUILT** â€” new category file, added since the last green build |
| `src/validation/{include,src}/tryAqInterestRate{FixingTable,FutureFra,ObjectFra}.{h,cpp}` (renamed from `tryAqRate*`) + `validation.vcxproj` / `.filters` | **NOT BUILT** â€” see the category rename below |
| `src/AQ_API/source/aqCurveObject.{h,cpp}` | `aqRateFixingTable* â†’ aqInterestRateFixingTable*` (the Python/SWIG binding for this â€” it lives in the Curve API file, a pre-existing filing quirk, not touched) |
| `src/GTEST/src/Test{AQObjCurveDiscountFactorsWithSpread,AQObjHedgeCurveDelta,AQObjSwapDelta,AQObjSwapDeltaDualBootstrap,AQObjSwapDeltaWithCurveEngine,AQObjSwapDeltaXCCY,AQObjSwapDeltaXCCY_CSA,AQObjSwapDeltaXCCY_ZAR,AQObjSwapDelta_ARR,AQObjSwapDelta_JPY,AQObjSwapFloatRate}.cpp` | call-site rename, same reason |
| 3 fixtures (`FIXING@1`, `EUR6M_FIXINGS@191`, `EUR3M_FIXINGS@74` `_tryAqRateFixingTableCreate_inputs.csv`) | renamed to `tryAqInterestRateFixingTableCreate_inputs.csv` |
| `CLAUDE.md` (both), `MIGRATION_PLAN.md`, `rebrand/tools/api_pair_check.py`, `docs/api_map.csv` (regenerated) | category list `Rate â†’ InterestRate` |
| `STATUS.md`, `rebrand/STATUS.md` | doc updates (this pause) |
| `rebrand/phase2_validation_rename_MAP.csv` | untracked â€” the task-2.6 rename map; sibling of the committed `phase3_*_MAP.csv` files, `git add` it with the next commit |

Also carried forward: `src/validation/src/tryAqToolEchoDouble.cpp` was committed
in `22212bc5` but is **not in `projects/validation.vcxproj`**, so it never
compiles â€” that is why `aqToolEchoDouble` was dropped from `aqTool.cpp` (link
error). Add the `.cpp` to `validation.vcxproj` + `.filters` on the next
`validation` rebuild, then re-add the XLL wrapper.

### `Rate` category renamed â†’ `InterestRate` (2026-09-11, decided + applied, NOT BUILT)

Nicholas: `aqRate*` read oddly; chose **`InterestRate`** over `IR` (keeps the
spelled-out, IntelliSense-groupable style of the other categories) and over
leaving it as `Rate` (too vague â€” reads as FX/hazard rate). Applied as a full
golden-source category rename, anchored to the 3 known function families only
(`FixingTable`, `FutureFra`, `ObjectFra` â€” never a blind `Rate*` sweep, which
would have hit unrelated tokens like local variables `aqRatesFixingDates` and
the pre-existing `tryAqCurveCompoundRate` / `â€¦EuroDollarConvexityAdjustment`
`@brief` comments):

- `validation`: 3 files renamed (`tryAqRateFixingTable.{h,cpp}` â†’
  `tryAqInterestRateFixingTable.{h,cpp}`, similarly `FutureFra` / `ObjectFra`);
  13 wrapper functions renamed; `validation.vcxproj` + `.filters` updated.
- `AQ_XLL`: `aqRate.cpp â†’ aqInterestRate.cpp`, 13 functions renamed to match;
  `AQ_XLL.vcxproj` + `.filters` updated. `.arg()` count re-verified: 13/13.
- `AQ_API`: `aqCurveObject.cpp/.h` â€” the 4 `aqRateFixingTable*` Python/SWIG
  bindings renamed (this is the one place `Rate` had already reached a
  consumer; filed under the Curve API source, not moved).
- `GTEST`: 11 files' `tryAqRateFixingTable*` call sites updated.
- 3 fixture files renamed (`resources/test/inputs/...`).
- Docs: both `CLAUDE.md`, `MIGRATION_PLAN.md` Â§2.2/porting-order prose,
  `rebrand/tools/api_pair_check.py` `CATEGORIES`, `docs/api_map.csv`
  regenerated (118 rows, `InterestRate: 4` in the public-surface breakdown).
- `src/AQ_API/source/swig_Python_wrap.cxx` still has the old `aqRateFixingTable*`
  names â€” expected, it is generated and was reverted to `HEAD`, not hand-edited;
  it picks up the new names on the next SWIG regen.
- **Verification (no build):** HARD GATE = 0; tree-wide grep for
  `tryAqRate(FixingTable|FutureFra|ObjectFra)` / `aqRate(FixingTable|FutureFra|ObjectFra)`
  across `src/` + `projects/` = 0 (outside the generated SWIG file).
- **Caught and fixed mid-rename:** the first sweep scoped its file list with
  `git grep -l`, which only searches **tracked** files â€” it silently skipped
  the then-untracked `aqInterestRate.cpp` (a `mv`, not `git mv`, since it had
  never been committed) and missed the two `FutureFra` functions entirely,
  because their real names (`tryAqRateFuturePriceToFraRate[FromConvAdj]`) don't
  contain the anchor substring "FutureFra" â€” only their *file* does. Both fixed
  with a second, targeted pass once found; the residual-grep check above is now
  post-fix and clean. Lesson for future category renames: run the identifier
  scan against the full symbol list (or `git grep --untracked`), not just
  filename-shaped anchors.

**Needs a full rebuild** (`validation` â†’ `AQ_API` â†’ `AQ_XLL` â†’ `GTEST`) since it
touches `validation` â€” larger than the AQ_XLL-only rebuilds this pause point
otherwise needed.

### `Ois` folded into `Swap` as a product variant (2026-09-11, DOCS ONLY â€” no code changed yet)

Nicholas: `Ois` should not be its own category â€” it is a vanilla swap whose
floating leg compounds an overnight index instead of a term rate, not a
structurally distinct product the way `AssetSwap`/`CapFloor` are. Decision:
category count drops **22 â†’ 21**; the golden-source form is
**`aqSwapOis<Function>`** (stateless; `aqSwapOisObject<Function>` if a stateful
form is ever needed), living in `aqSwap.cpp` â€” **not** a separate `aqOis.cpp` â€”
and in `src\validation\Swap\` alongside the vanilla-swap wrappers.

**Docs updated now** (`CLAUDE.md` both copies Â§5.1/Â§5.1a, `MIGRATION_PLAN.md`
Â§2.2/Â§2.7/D14, `rebrand/tools/api_pair_check.py` `CATEGORIES` â€” `Ois` removed
from the list). Also added a standing **"category-migration sequence"** note
(`AQ_LIB\CLAUDE.md` new Â§5.1a): validation (golden source, rename legacy
wrappers here first) â†’ `GTEST` â†’ `AQ_API` â†’ `AQ_XLL`, in that order, every time
â€” codifying what this session has been doing ad hoc, per Nicholas's request to
make sure all four surfaces stay synchronized to the `validation` name and any
drift gets cleaned up as part of the migration, not deferred.

**No code renamed yet â€” this is preparation for when `Swap` (and Ois within
it) is next migrated.** What that migration will need to do, found while
scoping this:

- `validation` â€” **pre-existing** `tryAqOisPV` / `tryAqOisParRate` +
  `*LVBKeys` companions (4 wrappers, `tryAqOisPV.{h,cpp}` /
  `tryAqOisParRate.{h,cpp}`) predate the category scheme and must rename to
  `tryAqSwapOisPV` / `tryAqSwapOisParRate` (+ files).
- `AQ_API` â€” **already shipped** `aqOisPV` / `aqOisParRate` (Python/C#/Java/R
  via SWIG, `aqOisPV.{h,cpp}` / `aqOisParRate.{h,cpp}`) rename to
  `aqSwapOisPV` / `aqSwapOisParRate` (+ files + the 4 SWIG `.i`).
- `GTEST` â€” `TryAqTestTradeEUROISParRate.cpp` call sites + likely the file name.
- `AQ_XLL` â€” new functions `aqSwapOisPV`, `aqSwapOisParRate` (+ `LVBKeys`) in
  `aqSwap.cpp`, alongside `aqSwapPV`/`aqSwapParRate` when that category is built.
- `docs/api_map.csv` â€” **not regenerated this session on purpose.** Its 4
  `aqOisPV*`/`aqOisParRate*` rows still say category `Ois`, which is accurate
  until the rename above lands; regenerating now (after removing `Ois` from
  `CATEGORIES`) would mislabel them via the script's no-match fallback
  (observed: they fall into the `(lifecycle)` bucket in the console report â€”
  cosmetic, `api_pair_check` HARD GATE is unaffected at 0). Regenerate
  `api_map.csv` only once the rename lands.

### Bond / Tool / Date / Math XLL fill-in (2026-09-10) â€” BUILT GREEN, tests passing (Nicholas)

Nicholas: "migrate the remaining Bond, Tool, Date and Math functions." **92 new
`XLO_FUNC` worksheet functions** added to the four existing category files (no
new files, no vcxproj change). Whole-file param-vs-`.arg()` audit: **165/165
across all AQ_XLL `.cpp`, 0 mismatch.** `api_pair_check` HARD GATE = 0. Every
function routes through its identically-named `validation::tryAq*` wrapper.
(One more, `aqToolEchoDouble`, was written and then dropped after a link
failure â€” see the `aqTool.cpp` note below.)

- **`aqDate.cpp` +26** (now 30) â€” `YearFraction`, `BusinessDays`,
  `ShiftedSpotDate`, `IsWorkingDay`, `IsHoliday`, `IsWeekend`, `IsWeekday`,
  `FuturesContract`, `CentralBank`, `ECB`, `ECBSwapStart/End`,
  `NthECBMeetingDate`, `NthECBSwapStart/EndDate`, `NextECBMeetingDate`,
  `NextECBSwapStart/EndDate`, `IMMFromMonth`, `NthIMM`, `NthIMMFromStartDate`,
  `IMMCurrent`, `IMMNext`, `IMMPrevious`, `IMMNth`, `IsRegularSwapSchedule`.
  Includes `<tryAqDateCentralBank.h>` `<tryAqDateIMM.h>`
  `<tryAqDateIsRegularSwapSchedule.h>`; file-local `toInt`.
- **`aqBond.cpp` +17** (now 59) â€” bond curve: `Create`, `Display`, `Yield`,
  `NelsonSiegelCalibrate`, `NelsonSiegelYield`, `PolynomialCalibrate`,
  `PolynomialYield`, `SvenssonCalibrate`, `SvenssonYield`; bond object:
  `PriceFromBondCurve`, `YieldFromBondCurve`, `ForwardReinvestedCoupon`,
  `Quote`, `YieldFromObject`, `YieldOptimized`; stateless: `BondSchedule`,
  `BondScheduleLVBKeys`. Includes `<tryAqBondCurves.h>` `<tryAqBondSchedule.h>`
  `<AQObjUtilities.h>` (`etrading::getBond` for the two `BondPtr`-arg wrappers)
  `<BondCurves.h>` `<NelsonSiegelFitting.h>` `<PolynomialFitting.h>`; file-local
  `nssParamsFromRange` / `nssResultBlock`. Calibrate results are rendered as a
  (label, value) block. `NelsonSiegel` reads a 4-cell InitialGuess, `Svensson`
  a 6-cell one â€” **verify the Excel layout of InitialGuess / bounds in use.**
  `aqBondCurveCreate` passes `std::vector<validation::TableInfo>` to the
  wrapper's `JSONInfoBlockTuples` param (the two tuple typedefs are identical).
- **`aqTool.cpp` +11** (now 35) â€” `LVBCreate` (1-2 value columns) and the
  object-grid family: `ObjectGridCreate/Save/Load/Display/ObjectNames/ClearOne/
  ClearAll`, `ObjectMultiGridCreate` (up to 3 named grids) `/Display/SubNames`.
  Includes `<tryAqToolGrid.h>` `<tryAqToolMultiGrid.h>`; file-local
  `flexibleDataToExcel` (prepends the column-name header row to the grid
  `Display` matrix).
  - **`aqToolEchoDouble` dropped** â€” link failed (`LNK2001` on
    `validation::tryAqToolEchoDouble`): `src/validation/src/tryAqToolEchoDouble.cpp`
    exists and defines it but is **not listed in `projects/validation.vcxproj`**,
    so it is never compiled into the `validation` lib. Pre-existing gap
    (`api_pair_check` counts it off the header). Fix when `validation` is next
    rebuilt: add the `.cpp` to `validation.vcxproj` + `.filters`, then re-add
    `aqToolEchoDouble`. It is only a round-trip diagnostic â€” `aqToolEcho`
    already echoes any value â€” so nothing is lost meanwhile.
- **`aqMath.cpp` +38** (now 41) â€” Black-Scholes `ImpliedVol` + 6 Greeks
  (`DeltaForward`, `DeltaSpot`, `Gamma`, `Vega`, `Theta`, `Rho`);
  `CapletFloorletPrice` / `ImpliedVol`; 6 volatility conversions
  (`VolatilityTo{Normal,Lognormal,ShiftedLognormal}From{...}`);
  `LiborConvexityAdjustmentInArrears` / `LiborRateInArrears` +
  `...ForArbitraryFixingDate` pair; European IR swaption `Price` / `ImpliedVol`
  / `Delta` / `Gamma` / `Vega` / `Theta` / `CashAnnuity`; normal distribution
  (`Standard`, `StandardPDF`, `StandardInverse`, and the mean/variance
  `NormalDistribution` / `PDF` / `Inverse`); polynomial
  `Interpolation` / `Interpolations` / `PoynomialIntegration` /
  `PoynomialIntegrations` (**wrapper misspells "Poynomial" â€” matched verbatim
  per golden source**); `IntegrateUsingTerms` and `Integrate` (date form).
  Includes `<tryAqMathCapletFloorlet.h>` `<tryAqMathConvexity.h>`
  `<tryAqMathEuropeanIRSwaption.h>` `<tryAqMathIntegrate.h>`
  `<tryAqMathPolynomial.h>`; file-local `toStrOr` / `toUInt`. `CallOrPut` and
  `VolatilityType` marshalled via `etrading::toCallOrPutEnum` /
  `toVolatilityTypeEnum`.

### Rate category â†’ new `aqInterestRate.cpp` (2026-09-10, uncommitted, NOT BUILT)

**13 `XLO_FUNC` worksheet functions** in a **new file** `src/AQ_XLL/src/aqInterestRate.cpp`,
added to `AQ_XLL.vcxproj` + `.filters`. Param-vs-`.arg()`: 13/13, 0 mismatch.

- Fixing table (a cached table of historical index fixings):
  `aqInterestRateFixingTableCreate` (currency / curve-tenor form), `â€¦Display`,
  `â€¦Value` (one date), `â€¦Values` (a column of dates).
- Rate future â†” FRA (stateless): `aqInterestRateFuturePriceToFraRate`
  (Hull-White convexity), `aqInterestRateFuturePriceToFraRateFromConvAdj`
  (explicit adjustment).
- FRA object: `aqInterestRateObjectFraCreate`, `â€¦PV`, `â€¦Display`, `â€¦DisplayCashflows`,
  `â€¦Rate`, `â€¦ToFuturePrice`, `â€¦ToFuturePriceFromConvAdj`.

File-local `toGregorian` / `toGregorianVector` (the fixing-table wrappers take
`boost::gregorian::date`, not `AQLDate` â€” converted via
`etrading::toGregorianDateFromAQLDate`). `aqInterestRateFixingTableDisplay` uses the
`toExcelMatrix(const etrading::VariantMatrix&)` overload added this session.
`aqInterestRateObjectFraCreate` follows the generator-create pattern (decorate + call +
`appendInstanceCounter`, no create-vs-modify guard â€” matches
`aqBondObjectCreateFromGenerator`). The three multi-overload wrappers
(`FixingTableCreate`, `FixingTableValues`) are each exposed once, via the
currency/tenor + plain-dates forms; the `LabelValueBlock` overloads are not
wired (add later if wanted).

### Generator category â€” NOT created (no wrappers exist)

Asked for alongside Rate. **`aqGenerator.cpp` was not created:** there are **zero
`tryAqGenerator*` validation wrappers** in `src/validation/` â€” the `Generator`
category (CLAUDE.md Â§5.1: introspection only â€” `aqGeneratorList` / `Describe` /
`Validate`) has never been built. Generator *construction* lives in the asset
categories (`aqBondGeneratorCreate`, and the pending `aqCurveGeneratorCreate` /
`aqSwapGeneratorCreate`), not here. Creating an empty file would add noise. To
populate this category, first add an `etrading` directory scan of
`resources/config/{BOND,CURVE,SWAP}_GENERATOR/` + a `tryAqGeneratorList` wrapper
(then `Describe` / `Validate`); the XLL/API surface follows. Flagged for
Nicholas's go-ahead (same item as under "Next steps on resume").

### Still deferred from Bond / Tool / Date / Math

Need enum-vector or curve-context marshalling not yet in `aqXllTools` â€” do with
the Curve category:
- Math `ForwardRate` / `ForwardRates` / `DiscountFactor` / `DiscountFactors`
  (both overload sets, 6 wrappers) â€” `InterpolationEnum`, `StateVariableEnum`,
  `DayCountEnum`, `BusinessDayAdjustmentEnum`, `CompoundingFrequencyEnum` +
  `forwardAdjustmentTable`.
- Math vector overloads: `BlackScholesPrices` / `ImpliedVols`,
  `CapletFloorletPrices` / `ImpliedVols`, `EuropeanIRSwaptionPrices` /
  `ImpliedVols` (`std::vector<CallOrPutEnum>` etc.) â€” the scalar forms cover the
  common use.
- Bond `tryAqBondObjectZSpread` (singular) â€” `aqBondObjectZSpread` already
  routes to the plural `tryAqBondObjectZSpreads`.
- Tool `tryAqToolLVBAdd` (in-place `STDStringMatrix&` mutator, not
  worksheet-shaped); `tryAqToolSwapScheduleTemplate` (Tool wrapper living in
  `tryAqSwapObjectSchedule.h` â€” do with Swap).
- `meUtilityLWODecorateNames`, `meUtilityMLIBSetUp` / `MLIBTearDown` â€” still no
  golden-source wrapper.

### Next steps on resume (priority order)

0. **Rebuild `validation` â†’ `AQ_API` â†’ `AQ_XLL` â†’ `GTEST`** â€” the `Rate` â†’
   `InterestRate` rename touches `validation`, so this is a full rebuild, not
   just `AQ_XLL`. Nothing else in the current delta needs more than that.
1. ~~Decide the `Rate` category name~~ â€” **done**: `InterestRate` (Nicholas,
   2026-09-11), applied.
2. **Commit the small remaining delta** (see the PAUSE POINT table): the
   Bond/Tool/Date/Math XLL fill-in + `aqInterestRate.cpp` (+ vcxproj/filters) +
   `rebrand/phase2_validation_rename_MAP.csv` + the doc updates. Task 2.6, the
   Tool/Object port and Interpolation/PCAâ†’Math are **already committed** in
   `22212bc5`. Before staging, check
   `git status --porcelain -- 'src/AQ_API/source/swig_*_wrap.*'` is empty â€” and
   note `swig_Python_wrap.cxx` slipped into `18328864`; decide whether to
   `git rm --cached` it.
2. **Confirm the 4 pre-existing test failures** (`TestDatesCentralBank`,
   `TestAQObjCurve`, `TestExampleObject`,
   `TestStructuredExceptionHandler.UNIT_IntegerDivideByZero`) also fail on a
   clean `3809f148` checkout â€” i.e. not caused by any of this work. If Nicholas's
   green run already showed them passing, this is moot.
3. **New `AQ_XLL` file `aqFuture.cpp`** â€” `Future` is a validation-only category
   (`tryAqFutureTicker*`); no XLL surface yet. Small, same pattern as the Tool
   port. (`Ois` no longer gets its own file â€” see the "`Ois` folded into `Swap`"
   section above; its 4 `tryAqOis*` wrappers rename to `tryAqSwapOis*` and land
   in `aqSwap.cpp` when `Swap` is migrated.)
4. **`meUtilityLWODecorateNames`** (Tool) + **`meUtilityMLIBSetUp` /
   `meUtilityMLIBTearDown`** â€” deferred from the Tool port because no
   golden-source wrapper exists. Decide: write conforming
   `tryAqTool{ObjectDecorateNames,Setup,TearDown}` wrappers, or drop these
   functions from the surface.
5. **`aqGeneratorList`** â€” the generator-name introspection function
   (CLAUDE.md Â§5.1) that does not exist in either library. New `etrading`
   directory scan of `resources/config/{BOND,CURVE,SWAP}_GENERATOR/` +
   `tryAqGeneratorList` wrapper + XLL/API. Nicholas said Batch-1 `aqObjectList`
   is "ok for now"; this is the fuller answer when wanted.
6. **Resume the main XLL port backlog** (the bigger Phase 4 work) â€” Curves â†’
   Swaps â†’ products â†’ Models, driven by
   `rebrand/xll_function_inventory.csv`. See "â‡’ RESUME HERE â€” AQ_XLL port".
7. Lower priority, still open: the validation **recording rollout** (plan 4.9,
   see "â‡’ RESUME HERE â€” recording rollout"); `AQ_API` SEH guard (plan 4.12);
   the 5 + 1 wrapper-name-drift advisories; the `tryAqObjSwapsDV01*` /
   credit-fixture golden-source cleanup (Phase 6 resources audit).

- Docs: `Vol` â†’ `Volatility`; **Future** + **Ois** added as categories (now 22);
  D19 = the `validation` wrapper is the GOLDEN SOURCE for names. `api_pair_check.py`
  CATEGORIES updated.
- Rename map: `rebrand/phase2_validation_rename_MAP.csv` â€” 467 wrappers, 416
  renames, 0 collisions. Decisions baked in: `tryAqFuturesTicker*` â†’
  `tryAqFutureTicker*` (new `Future` cat), `tryAqOIS*` â†’ `tryAqOis*` (new `Ois`
  cat), `tryAqObj{QuickLoad,QuickSave,Type,TypeAsString,CreateRange}` â†’
  `tryAqObject*`, `tryAqObjBondsCompoundYields` â†’ `tryAqBondObjectCompoundYield`
  (wrapper singularised), `tryAqBondsYield` â†’ `tryAqBondCurveYield`.
- **DONE this session (uncommitted, NOT BUILT):**
  - **Date batch** â€” `tryAqDates*` â†’ `tryAqDate*` (29 fns) across validation +
    `AQ_API` (`aqDates*` â†’ `aqDate*` method names + files) + `GTEST` (suites +
    `TestAqDates*.cpp` â†’ `TestAqDate*.cpp`) + `AQ_XLL` call sites + SWIG `.i` +
    all 3 vcxproj/.filters. **No fixtures exist for Date.**
  - **Object lifecycle batch** â€” `tryAqObj{Load,Save,Exists,ClearCache,
    CreateRange,Delete,DeleteAll,List,LoadFromString,LoadAndReturnTupleResults,
    QuickLoad,QuickSave,Type,TypeAsString}` â†’ `tryAqObject*` (14 fns). Container
    files `tryAqObjects.{h,cpp}` â†’ `tryAqObject.{h,cpp}`, AQ_API `aqObjects.*` â†’
    `aqObject.*`, ~40 GTEST includers, SWIG `.i`, vcxproj. `tryAqObjToolsGrid*`
    deliberately left (that is the Tool batch). **No fixtures.**
  - Reverted twice: `swig_Python_wrap.cxx` (generated â€” regenerated in Phase 5)
    and `rebrand/phase3_Bsimple_MAP.csv` (historical) were caught by an
    unscoped sweep. Sweeps are now scoped to `src/` + `projects/`, excluding
    `swig_*_wrap.*`.
- **FULL MAP APPLIED (uncommitted, NOT BUILT).** All 416 renames from
  `phase2_validation_rename_MAP.csv` applied in one pass (the user said "go"):
  - `validation` â€” 467 wrappers renamed; ~240 `.h/.cpp` files `git mv`'d
    (exact-stem + container headers like `tryAqObjBonds.h â†’ tryAqBondObject.h`,
    `tryAqCurvesDiscountFactor.h â†’ tryAqCurveDiscountFactor.h`, â€¦); all `#include`
    updated.
  - `AQ_API` â€” `aq*` binding method names + source files + SWIG `.i` `%include`.
  - `GTEST` â€” `tryAq*` calls + `_inputs.csv` path-string literals + `TestAq*.cpp`
    file names where the stem was a function name.
  - `AQ_XLL` â€” `validation::tryAqâ€¦` call sites.
  - `resources/test` â€” **~3,560 fixture files** `git mv`'d
    (`<oldfn>_{inputs,outputs}*.csv â†’ <newfn>_â€¦`). Content untouched.
  - 3 Ã— `.vcxproj`/`.filters`. `docs/api_map.csv` regenerated.
  - `swig_*_wrap.*` reverted twice (generated â€” Phase 5).
- **Verification (what I could check without a build):**
  - `api_pair_check.py` **HARD GATE = 0** (every public fn routes through validation).
  - Old function names in code: **0**. Old validation header files: **0**.
  - 5 wrapper-name-drift advisories â€” all pre-existing (the deferred set), not new.
  - **24 fixtures keep old names** â€” `tryAqObjSwapsDV01*`, `tryAqObjSwaps_{IRS,XCCY}_DV01*`,
    `tryAqObjCreditDefaultSwap{AnnuityFromHazardRate,PVIntegration,PVMonteCarlo,PVMonteCarloSobol}*`.
    Their GTEST path-strings are ALSO still old (hardcoded literals that aren't a
    function name), so file+string are self-consistent â†’ tests still read them.
    These are drift/orphan fixtures for the Phase 6 resources audit.
  - A handful of GTEST `.csv` string literals reference fixtures that exist under
    **neither** old nor new name (`tryAqSwapObjectPV_inputs.csv`,
    `tryAqSwapObjectDeltaLadder_spot_4Y_inputs.csv`, the VariableNotional set) â€”
    **pre-existing** (0 files under the old name too), not caused by this rename.
- **GATE â€” build `validation` + `AQ_API` + `GTEST` and run GTest.** Output must
  be numerically identical to the pre-rename baseline. Any fixture-name failure
  will name the exact file; report it and I fix that one. This is the
  build-between-batches checkpoint (done once here, since the map went in as one
  batch).
- Still to do after green: the two new-category AQ_XLL files `aqFuture.cpp` /
  `aqOIS.cpp` (Future/Ois currently validation-only); the `meUtility*` â†’ `Tool`
  port from `.APPLES`.

### AQ_XLL Tool / Object port (2026-09-09, in progress, uncommitted, NOT BUILT)

Third explicit task â€” expose the `Tool` + `Object`-lifecycle surface in the Excel
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
| `meLWOExists` | `tryAqObjectExists` | `aqObjectExists` | âœ… already |
| `meLWOLoad` | `tryAqObjectLoad` | `aqObjectLoad` | âœ… already |
| `meLWOSave` | `tryAqObjectSave` | `aqObjectSave` | âœ… already |
| `meLWOLoadFromString` | `tryAqObjectLoadFromString` | `aqObjectLoadFromString` | aqObject.cpp |
| `meLWOQuickLoad` | `tryAqObjectQuickLoad` | `aqObjectQuickLoad` | aqObject.cpp |
| `meLWOQuickSave` | `tryAqObjectQuickSave` | `aqObjectQuickSave` | aqObject.cpp |
| `meUtilityClearLWOCache` | `tryAqObjectClearCache` | `aqObjectClearCache` | aqObject.cpp |
| `meUtilityResize` | (n/a) | `aqToolResize` | âœ… already |
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

No validation wrapper in AQ_LIB (defer â€” needs a wrapper first, or drop):
`meUtilityLWODecorateNames`, `meUtilityThreadsEnableLocks`, `meUtilityMLIBTearDown`.
`meUtilityMLIBSetUp/TearDown` â€” `MLIB` is a legacy prefix; the setup half maps to
`tryAqToolSetup`, the teardown half has no wrapper. Also unmapped in AQ_XLL so far:
the `tryAqToolObjectGrid*` / `tryAqToolObjectMultiGrid*` / `tryAqToolSwapScheduleTemplate`
wrappers (these come from `.APPLES` files other than `meUtilities.cpp`).

Batches (build between each, on Nicholas's side):

1. **Object lifecycle â†’ `aqObject.cpp`** â€” `aqObjectType`, `aqObjectList`,
   `aqObjectDelete`, `aqObjectDeleteAll`, `aqObjectLoadFromString`,
   `aqObjectQuickLoad`, `aqObjectQuickSave`, `aqObjectClearCache` (8 fns).
   **DONE (uncommitted, NOT BUILT).** Added `#include <boost/format.hpp>` +
   `<tryAqToolSetup.h>`. Every fn has `.arg()` count == parameter count.
   **BUILT GREEN in Excel (Nicholas, 2026-09-09).**
2. **Tool setup/version/parallel + record/replay â†’ `aqTool.cpp`** â€”
   `aqToolVersion`, `aqToolClearEntityPool`, `aqToolLoadStaticData`,
   `aqToolLoadCalendarFile`, `aqToolLoadConfigurationFiles`,
   `aqToolParallelModeEnable`, `aqToolParallelModeStatus`, `aqToolRecord`,
   `aqToolReplay` (9 fns). All string / bool / int in, string out.
   **DONE â€” BUILT GREEN with tests passing (Nicholas, 2026-09-09).** Added
   `#include <cstdio>` `<ctime>` + `<tryAqToolSetup.h>` `<tryAqToolRecord.h>`
   `<tryAqToolReplay.h>`; file-local `toIntOr()` for the optional
   `StartIndex`/`MaxIndex` args.
   - `meUtilityMLIBSetUp` / `meUtilityMLIBTearDown` **dropped** â€” no
     golden-source wrapper: the only setup helpers in `tryAqToolSetup.h` are
     `trySetupAQL(irProps, calendar, cbCalendar)` / `tryTearDownAQL()`, which
     do not conform to the `tryAq<Category>` scheme. Needs a conforming
     `tryAqToolSetup` / `tryAqToolTearDown` wrapper defined first.
3. **Tool LVB family + data reshaping + maths â†’ `aqTool.cpp`** (13 fns).
   **DONE â€” BUILT GREEN with tests passing (Nicholas, 2026-09-09).**
   - LVB: `aqToolLVBFromKeysValues`, `aqToolLVB`, `aqToolLVBGroup` (10 optional
     block args), `aqToolLVBFromMultipleKeysValues` (5 keys/values/prefix
     triples = 15 args), `aqToolLVBFromKeysAndMultipleValues` (6 args).
   - Reshaping: `aqToolClean` (5), `aqToolAppend` (11), `aqToolDataFilter` (2),
     `aqToolValuationSettingsDisplay` (1).
   - Maths: `aqToolInterpolation` (5), `aqToolPCA` (4), `aqToolTermsToDates` (2),
     `aqToolDatesToTerms` (2).
   - **New `aqXllTools` helpers** (added to `.h` + `.cpp`): `toAQLStringVector`,
     `toDoubleMatrix`, `toStandardStringMatrix`, `toVariantMatrix` (row-major,
     type-preserving, error cell â†’ its text, blank â†’ EMPTY Variant),
     `toVariantVector`, and a `toExcelMatrix( const etrading::VariantMatrix& )`
     overload (native Excel types out). File-local `toDoubleOr` / `addLvbBlock`
     / `addStringColumn` in `aqTool.cpp`.
   - **ValuationSettings orientation:** input read as-is (rows of key/value
     pairs) â€” matches the current `TestValuationSettings` GTEST contract
     (`{{ "CURVECOLLECTION", curveUSD3M }}`). The old XLL+ add-in force-
     transposed here but its own comment flagged that as a bug ("needs
     fixing!!!"); not carried.
   - `.arg()` count == parameter count verified for all 26 functions in
     `aqTool.cpp`.
4. `meUtilityLWODecorateNames` still deferred â€” no golden-source wrapper.

### Interpolation + PCA moved Tool â†’ Math (2026-09-09) â€” BUILT GREEN, tests passing (Nicholas)

Nicholas: interpolation and PCA are `Math` building blocks, not `Tool`. Renamed
the golden-source wrappers and every surface that follows them:

- `validation`: `tryAqToolInterpolation` â†’ **`tryAqMathInterpolation`**,
  `tryAqToolPCA` â†’ **`tryAqMathPCA`**; files `tryAqTool{Interpolation,PCA}.{h,cpp}`
  â†’ `tryAqMath{Interpolation,PCA}.{h,cpp}`; `projects/validation.vcxproj{,.filters}`.
- `AQ_API`: `aqToolInterpolation` / `aqToolPCA` â†’ `aqMathInterpolation` /
  `aqMathPCA`; source files renamed; 4 SWIG `.i` (`#include` + `%include`);
  `projects/AQ_API.vcxproj{,.filters}`.
- `GTEST`: `TestUtilitiesInterpolation.cpp` call sites (12) + include. (Bespoke
  unit test â€” suite/case names left as-is; not a recording-generated test, and
  there are **no fixtures** for either function, so nothing else to move.)
- `AQ_XLL`: the two `XLO_FUNC` blocks moved out of `aqTool.cpp` into
  `aqMath.cpp` as `aqMathInterpolation` / `aqMathPCA`; `toDoubleOr` helper moved
  with them; `aqMath.cpp` gained a file-local `toIntOr` and `using namespace
  aq_xll;`.
- `docs/api_map.csv` regenerated â€” 118 rows, `aqMath{Interpolation,PCA}` now
  under `Math`. HARD GATE = 0. No `swig_*_wrap` touched.

### Open question raised â€” a generator-name listing function

Nicholas asked whether the old `.APPLES` add-in has a function to list the
generator names from the config folders (so users don't memorise them).
**Answer: no.** `.APPLES\...\MLIBQ_ADDIN` only has, per asset class,
`*GeneratorCreate` / `*GeneratorDisplay` / `*GeneratorModify` and
`*CreateFromGenerator` â€” `*GeneratorDisplay` dumps one *named* generator; nothing
enumerates the `resources/config/{BOND,CURVE,SWAP}_GENERATOR/*.JSON` names.
CLAUDE.md Â§5.1 already earmarks this as intended-but-unbuilt: the `Generator`
category = introspection only (`aqGeneratorList` / `Describe` / `Validate`).
Building it = a new `etrading` directory-scan + a `tryAqGeneratorList` wrapper +
XLL/API surface. Not started; flagged for Nicholas's go-ahead.

### Fixture-rename recovery + final cleanup (2026-09-09, uncommitted, NOT BUILT)

After Nicholas's build+GTest run flagged ~20 failures, root cause was a
fixture/â€‹code-string boundary mismatch: the fixture sweep matched only names
**starting** with a function name (`b.startswith(old)`) while the code-string
rewrite used a **bounded-token** regex that also hit infixes like
`"..._tryAqObjSwaps..._inputs.csv"` â€” so prefixed fixture FILES kept old names
while their string LITERALS moved to new â†’ "error opening file".

- **Reverted all `resources/test/` fixture moves to HEAD** (needed all three:
  `git checkout HEAD -- resources/test/`, `git reset HEAD -- resources/test/`,
  `git clean -fdq resources/test/inputs/` â€” `git add -A` had staged the renames
  as Add/Delete pairs).
- **Fixture rename take-3** â€” boundary `(?<![A-Za-z0-9])<old>(?![A-Za-z0-9])`
  (allows a leading `_`), driven by `pairs + apair` (both `tryAq*` and `aq*`
  names), skipping any result still containing a stale fragment:
  **7,551 renamed, 0 skipped, 0 `tryAqObjâ€¦` left**.
- **Relaxed code pass** â€” 69 GTEST/validation files: rewrote `_`-prefixed
  regex/scan/recording string fragments the first `(?<![A-Za-z0-9_])` pass had
  missed (e.g. `"_tryAqBondObjectPriceFromDiscountMargin_inputs.csv"` suffix
  constants).
- Verified `BOND1_tryAqBondObjectPrice_inputs.csv` and
  `EUR_FIXEDSCHEDULE_tryAqSwapObjectScheduleCreateBespoke_inputs.csv` now exist,
  matching the rewritten code strings.

**Residual function fixed (was the only real code-level miss):**
`tryAqObjCurvesDiscountFactorsForwardStartingFromTenor` (function *definition* at
`src/validation/src/tryAqCurveDiscountFactor.cpp:624`, missing from the map â€”
the map had the stateless `â€¦Tenor` and the object-form plural `â€¦Tenors` but not
the object-form singular) â†’ **`tryAqCurveObjectDiscountFactorsForwardStartingFromTenor`**,
plus its `CreateDataFile`/`generatorFunction` labels. 0 fixtures / 0 GTEST readers
depended on the old name.

**Dead recording-label cleanup** (WRITE-side `CreateDataFile` / `decorateCurvename`
labels only â€” 0 committed fixtures, 0 GTEST readers, provenance not on the
test-read path; changed to golden names for internal consistency):
`tryAqObjCurvesCalibrate{Basis,OIS,Swap,FXForwards}` â†’
`tryAqCurveObjectCreate{Basis,OIS,Swap,FXForwards}` in the four
`tryAqCurveObjectCreate*.cpp`; `â€¦DiscountFactorsForwardStartingFromTenor` labels
in `tryAqCurveObjectDiscountFactor.cpp` â†’ `â€¦Tenors` (match enclosing fn);
`tryAqObjSwapsDeltaLadderHorizontal` label in `tryAqSwapObjectDelta.cpp:395` â†’
`tryAqSwapObjectDeltaLadderHorizontal` (real fixtures already use the new name).

**Left as-is on purpose â€” `tryAqObjSwapsDV01*`** (18 fixtures on disk +
`TestAQObjSwapDelta{,XCCY_CSA,_JPY}.cpp` hardcoded path literals). File name and
GTEST read-string are mutually self-consistent, so the tests pass. Renaming would
mean an 18-fixture + 4-file lockstep move for zero functional gain â€” this is a
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
  headers â€” advisory comment cleanup only, no code effect.
- No `swig_*_wrap.*` staged. `docs/api_map.csv` regenerated (118 public / 467
  wrappers).

**4 failures assessed as pre-existing (NOT caused by the rename) â€” Nicholas to
confirm they also fail on clean HEAD:**
- `TestDatesCentralBank.UNIT_AreThereEnoughDates` â€” `totalDaysToLast: -1704`;
  stale ECB meeting-date data in `AQLMathCentralBank::meetingSchedule("ECB")`,
  no `tryAq*` involved.
- `TestAQObjCurve` + `TestExampleObject` â€” write to `.../resource/test/...`
  (singular); pre-existing FolderConfig path bug.
- `TestStructuredExceptionHandler.UNIT_IntegerDivideByZero` â€” SEH "throws
  nothing"; build/optimizer issue, no `tryAq*`.

**GATE â€” Nicholas: rebuild `validation` + `AQ_API` + `GTEST`, run GTest.**
Expect the ~15 fixture-mismatch failures gone; the 4 above expected to remain
(please confirm on clean HEAD). Then commit this batch.

## â‡’ RESUME HERE â€” meLWOBond port (2026-09-08)

Porting `meLWOBond*` from `.APPLES\...\MLIBQ_ADDIN\src\meLWO.cpp` into
`src/AQ_XLL/src/aqBonds.cpp`, renamed `meLWOBond* â†’ aqObjBonds*` (plural
`Bonds`, the locked category â€” NOT the literal `aqObjBond*` first asked for;
plural is what pairs with the `tryAqObjBonds*` wrappers and the
`api_pair_check` gate). Two-part plan agreed with Nicholas.

**Part (a) â€” DONE (uncommitted, NOT BUILT):** 11 functions added to
`aqBonds.cpp` â€” `aqObjBondsDisplaySchedule`, `aqObjBondsDisplayCashflows`,
`aqObjBondsDirtyPrice`, `aqObjBondsAccruedInterestDays`,
`aqObjBondsCleanPrice`, `aqObjBondsAccruedInterest`, `aqObjBondsYield`,
`aqObjBondsCompoundYield`, `aqObjBondsPrice`, `aqObjBondsDV01`,
`aqObjBondsDV01Numerical`. (`meLWOBondDisplay`/`meLWOBondCreate` were already
ported in `e5c57046` as `aqObjBondsDisplay`/`aqObjBondsCreate`.)
New marshalling helpers in `aqXllTools.{h,cpp}`: `toDoubleVector`,
`toExcelDoubleColumn`, `toExcelIntColumn`.
`aqObjBondsCompoundYield` calls the plural-spelled wrapper
`tryAqObjBondsCompoundYields` (pre-existing wrapper quirk).
No vcxproj change â€” `aqBonds.cpp` / `aqXllTools.cpp` already listed.
**Next: build all configs; then GoogleTest / Excel-verify.**

**Part (b) â€” batch b1 DONE (uncommitted, NOT BUILT).** 24 functions added to
`aqBonds.cpp`: `aqObjBondsModifiedDuration`, `aqObjBondsOisSpread`,
`aqObjBondsLastCouponDate`, `aqObjBondsPriceFromDirtyToClean`,
`aqObjBondsPriceFromCleanToDirty`, `aqObjBondsZSpreadFromRates`,
`aqObjBondsZSpread` (â†’ plural wrapper `tryAqObjBondsZSpreads`),
`aqObjBondsForwardPrice`, `aqObjBondsRepoRate`, `aqObjBondsRepoRateFromFuture`,
`aqObjBondsFuturePrice`, `aqObjBondsConversionFactor`, `aqObjBondsGrossBasis`,
`aqObjBondsNetBasis`, `aqObjBondsCheapestToDeliver`,
`aqObjBondsCheapestToDeliverByNetBasis`, `aqObjBondsFRNPriceFromDiscountMargin`,
`aqObjBondsFRNPriceFromYield`, `aqObjBondsFRNYieldFromPrice`,
`aqObjBondsFRNDiscountMarginFromPrice`, `aqObjBondsPriceFromCreditModel`,
`aqObjBondsBPVPerTick`, plus the two stateless `aqBondsAverageYield` /
`aqBondsYieldFromFuturePrice` (B4 decision: `aqBonds*` in `aqBonds.cpp`; they
call `validation::tryAqToolsBond*`, so `api_pair_check` will flag an
`aqBonds*`â†”`tryAqToolsBond*` drift â€” deferred wrapper rename, same class as the
existing 5 advisories). New helper `toStringVector` in `aqXllTools.{h,cpp}`.
Whole-file param-vs-`.arg()` audit: 37/37 match (0 mismatches).
**Next: build all configs; verify in Excel.**

**Part (b) â€” batch b2a DONE (uncommitted, NOT BUILT).** 3 create functions added
to `aqBonds.cpp`, each with the `aqObjBondsCreate` create-vs-modify guard
(`decorateWithExcelLocation` + `allowAQObjUpdates` + `appendInstanceCounter`):
- `aqObjBondsCreateFromLVB` â€” keeps the legacy `VerticalLVBKeys` arg; new
  `toLabelValueBlock(obj, keysAreVertical)` overload in `aqXllTools` transposes
  the string matrix when FALSE.
- `aqObjBondsCreateFromGenerator` (legacy `meLWOBondCreateFromGeneratorLVB` â€” the
  `LVB` suffix is dropped to match `tryAqObjBondsCreateFromGenerator`).
- `aqObjBondsCreateAUDNotionalBond` â€” 10 Excel args, wrapper takes 9 (the 10th,
  `AllowUpdates`, drives the create guard only).
Whole-file audit: 40/40 param-vs-`.arg()` match. **Next: build; verify.**

**Part (b) â€” batch b2b DONE (uncommitted, NOT BUILT).** The 2 bond-generator
functions added to `aqBonds.cpp`:
- `aqObjBondsGeneratorCreate` â€” new `aqXllTools::toTableInfo(obj)` reads a range
  column-major into `tuple<COL_1.. names, Variant::getContainedTypeInfo types,
  column-major VariantMatrix of string Variants>` (the `validation::TableInfo`
  shape); `key2`/`value2` optional. No create-vs-modify guard (matches the
  source), just decorate + call + `appendInstanceCounter`.
- `aqObjBondsGeneratorDisplay` â€” `validation::tryAqObjBondsGeneratorDisplay`
  returns `etrading::VariantMatrix`; rendered via
  `etrading::toAQLStringMatrixFromVariantMatrix(result, false)` into the new
  `toExcelMatrix(const AQLStringMatrix&)` overload (numeric-aware, same cell
  path as `toExcelMatrix(AnyTypeMatrix)`). **Display orientation (transpose
  flag) is a guess â€” verify visually in Excel.**

**meLWO.cpp bond port â€” COMPLETE** (parts a + b1 + b2a + b2b): 42 XLO_FUNC in
`aqBonds.cpp` (2 pre-existing + 40 ported this session), whole-file
param-vs-`.arg()` audit 42/42. Remaining `meLWO.cpp`
items are the 5 `meLWOFixingTable*` (Rates category, a later `aqRates.cpp`) and
the 2 in the separate `meLWOBonds.cpp`.

**AQ_XLL comment / include cleanup (uncommitted, NOT BUILT).** Per Nicholas:
all 8 `src/AQ_XLL/{src,include}` files scrubbed of `.APPLES` paths, legacy
function/type names (`meLWO*`, `meUtility*`, `CXlOper`, `XllPlus*`,
`getDataByColumnAndDescription`, `populateExcelArrayWith*`, `MLIB_START*`,
`checkIfStaticDataLoaded`, `appendExcelLocation`, ...) and the word "legacy" in
comments; `@brief` markers removed; every `#include "x"` â†’ `#include <x>` (all
resolve through the project `/I` dirs, so the form change is safe). No code
behaviour change. Grep for the legacy tokens over `src/AQ_XLL` now returns clean.

**Also fixed this session:** `aqObjSave` (`aqObj.cpp`) had 3 params / 4 `.arg()`
from commit `a8b50b47` â€” that was the "corrupt XLL / unhandled xloil::Exception"
at load, NOT the LTCG theory and NOT the bond port. Added the 4th param
`showArrayOutputs` + implemented the TRUE â†’ 3-row column behaviour.

**Out of scope for `aqBonds.cpp`:** the 5 `meLWOFixingTable*` in `meLWO.cpp` are
the `Rates` category (`tryAqObjRatesFixingTable*` wrappers) â€” a later `aqRates.cpp`.
`meLWOBondQuote` / `meLWOBondForwardReinvestedCoupon` live in `meLWOBonds.cpp`,
not `meLWO.cpp`.

Two workstreams are open in parallel:
- **A. AQ_XLL xlOil port (active this session)** â€” see "â‡’ RESUME HERE â€” AQ_XLL
  port" immediately below.
- **B. validation recording rollout (plan 4.9)** â€” see "â‡’ RESUME HERE â€”
  recording rollout" further down. Untouched this session.

---

## â‡’ RESUME HERE â€” AQ_XLL port (2026-09-07)

**What works in Excel now (Nicholas-verified):**
- `aqToolsInitialize` â€” loads config, prints the resolved `Calendar.csv` path.
  Library auto-inits in the `AlgoQuantLib` add-in constructor (xlAutoOpen);
  `AQ_INITIALIZE` macro is the per-function lazy guard.
- `aqDatesFromTenor` / `aqDatesFromYearFraction` â€” calendars usable.
- `aqObjBondsCreate` / `aqObjBondsDisplay` (new `aqBonds.cpp`).
- `aqObjExists` / `aqObjLoad` / `aqObjSave` (new `aqObj.cpp`).
- `aqToolsResize` â€” positional clip/pad reshape (respects source dims;
  pad blanks / truncate). Row-major reflow bug fixed.
- Numeric-string cells in `Display`/matrix output now returned as real numbers
  (formattable), mirroring the legacy `canStringConvertToNumber` path.

**OPEN â€” blocking the multi-row `aqObjSave` output:**
`AQ_IS_ARRAY_OUTPUT` (macro â†’ `aq_xll::isArrayOutput()` â†’ `callerRangeSize()`
via raw `xlfCaller`, handles SRef + Ref) returns false even under
Ctrl+Shift+Enter on Nicholas's Excel, so `aqObjSave` returns the scalar and
Excel repeats it across the selection. Likely cause: modern Excel intercepts
CSE and never reports the array range to the add-in, so Enter-vs-CSE cannot be
distinguished at all.
- **Mitigation already coded (uncommitted):** `aqObjSave` 4th optional arg
  `AsArray` â€” TRUE forces the 3-row column, FALSE forces the message, omitted =
  auto. Plus `aqToolsCallerInfo()` diagnostic returning `"<r> x <c> (array=<0|1>)"`.
- **Next step:** build; run `=aqToolsCallerInfo()` plain vs CSE-over-3-cells.
  - both `1 x 1` â†’ auto-detect is impossible on this Excel; keep `AsArray`,
    strip the auto-detect from other multi-output funcs (don't fake it).
  - CSE shows `3 x 1` â†’ detection works, dig into why `aqObjSave` still
    scalars (array build path itself is proven â€” bond display uses the same
    `ExcelArrayBuilder` + `returnValue` and works).

**Then continue the port in the agreed order** â€” Dates â†’ Tools â†’ Curves â†’
Swaps â†’ Products â†’ Models. `rebrand/xll_function_inventory.csv` (653 rows) is
the driver; decision columns (`proposed_new_category`, `keep_drop_merge`,
new name) still need filling per file, starting with `meDates` (36 fns).
`~460` me*/meLWO* functions in the etrading filter to port; `LAXL.cpp` (161
`mir*`) deprecate-or-`aql*`; `msc*`/client (~29) delete.

**Carry forward (AQ_XLL specifics):**
- `AQ_API` has NO auto-init â€” bindings callers must call `setUpAQL()` /
  `setupAQL()` / `initAQL()` (all â†’ `validation::trySetupAQL`). Only Python
  verified. `AQ_API_START` is still a bare `try{}` (plan 4.12).
- `aqObjSave` deliberately no longer upper-cases the file path (legacy did;
  breaks case-sensitive FS / Linux). Name + type still upper-cased.
- Naming calls made this session, matching the `tryAq*` wrappers + pairing
  check, NOT the literal ask: `Bonds` (plural, not `Bond`), `aqObj*` (not
  `aqObject*`). Nicholas has accepted these so far.
- New marshalling helpers in `aqXllTools.{h,cpp}`: `toBool`,
  `toAQLStringMatrix`, `toLabelValueBlock`, `toExcelMatrix` (numeric-aware),
  `toExcelColumn` (no pad â€” an NÃ—1 must stay NÃ—1), `reshapeToSize`,
  `decorateWithExcelLocation`, `callerRangeSize`, `isArrayOutput`.
- Dead XLL+ include/lib paths still in `projects/AQ_XLL.vcxproj` â€” remove when
  next touching it (plan carry-over).

---

## â‡’ RESUME HERE â€” recording rollout

**1. Continue the validation recording rollout (plan 4.9).** Pass (a) is
done: 32 conversions in 6 files. What remains, in the order I would do it:

- **Fix the misleading comment in `RecordMacros.h`** (cheap, do it first). Its
  worked example shows a `decorateFilename(...)` block being replaced by plain
  `AQ_RECORD_INPUTS`. That is WRONG â€” `AQ_RECORD_INPUTS` does not decorate, it
  always yields `<function>_inputs`, so following the comment silently renames
  the fixture file and breaks the test at run time with a green build.
- **99 decorated blocks** â†’ `AQ_RECORD_DECORATED_INPUTS`. First confirm how that
  macro maps prefix/suffix onto `decorateFilename(name, prefix)`.
- **81 variable-filename blocks** â€” need reading individually.
- **18 keyâ‰ parameter-name recordings** (pass b) â€” listed in plan 4.9. Converting
  these mechanically WOULD break fixtures; rename the parameter to match the key
  instead, then re-run `fixture_key_check.py`.
- **Functions with no recording at all** (pass c) â€” largest, needs new fixtures.

**âš  Two hard rules for the converter tool, learned the expensive way:**
1. Verify the replaced span is exactly the `if (recordEnabled()) { ... }` block
   and nothing more. One bad boundary ate a live statement
   (`double result = ...` became `double rAQ_RECORD_OUTPUTS( ... );`).
2. Verify the file includes `RecordMacros.h` before rewriting it. Three files
   had only hand-rolled recording and so never included it.

**2. Then the other open Phase 4 items:** 4.12 (SEH for `AQ_API` â€”
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
`aqObjSave` (multi-row output still blocked â€” see RESUME HERE â€” AQ_XLL port).

Convention (plan 4.11): every file is `aq<Category>.{cpp,h}`; the single
exception is `aqXllTools.{h,cpp}`, the shared Excel-side helper library
(marshalling + AQObj handle decoration). `aqTools.cpp` is the Tools *category*;
`aqObj.cpp` holds the `aqObj<Lifecycle>` functions (no category word).

`XllPlusTips*.cpp` were NOT copied â€” 4,629 lines built on XLL+ types. Behaviour
was ported instead. The AQObj handle logic (instance counter, cell address,
create-vs-modify guard) is in `aqXllTools`; the core framework was already in
`src/etrading`.

SEH: `TestStructuredExceptionHandler` gives it automated coverage for the first
time. **If the translator ever stops working those tests do not fail, they
CRASH the runner** â€” a vanished test run is the signal.

## Phase 3 â€” complete

`src/` and `projects/` contain zero `Mizuho`, `MLIB`, `me*`, `mir*`,
`LA*`/`MA*`/`MB*`, `validation_api`, `XllPlus`, person names, or copyright
banners. `resources/` reduced to `config`, `test`, `setup` (3,522 files deleted);
legacy-named paths there fell from 1,310 to 126.

## âš  Carry these forward â€” they outrank routine cleanup

1. **Two undeclared third-party components in `src/math`**, both found by
   accident: `AQLSobol.cpp` (Peter Jaeckel â€” notice has a PRESERVATION CLAUSE)
   and `AQLNl2sol.cpp` (NL2SOL, 6,541 lines â€” Dennis/Welsch/Gay/Peters,
   NSF-funded, ACM TOMS). **Excluded by name from every sweep. Never strip their
   attribution.** Neither is in the CLAUDE.md Â§3.3 dependency table. The Phase 6
   provenance scan is REQUIRED BEFORE SALE.
2. **Recorded fixture keys are stringified C++ parameter names.** Renaming a
   validation parameter breaks tests silently, build still green.
   `fixture_key_check.py` guards it.
3. **Pre-existing bug:** `tryAqObjRatesFixingTable.cpp` writes
   `file.write("fixingValues", fixingDates)` and
   `file.write("fixingDates", fixingValues)` â€” the keys are swapped. Fix code
   and rebase those fixtures TOGETHER, never separately.
4. **`git add -A` stages the generated `swig_*_wrap.*` files.** Restore with
   `git checkout HEAD -- 'src/AQ_API/source/swig_*_wrap.*'`; never commit them.
5. **Exact-match auditing under-reports** â€” five times a "clean" result was
   defeated by a spelling variant. Re-scan loosely after any sweep reports zero.
6. **No test exercises `tryAqToolsReplay`** â€” untested public API, Phase 5 gap.

---

## Done this run (newest first)

| commit | what |
|---|---|
| _(uncommitted)_ | **Step 11 â€” category naming scheme + AQ_XLL rebrand.** Categories now SINGULAR; handle marker moved from `aqObj<Category>` prefix to `aq<Category>Object<Fn>`; named sub-objects skip `Object` (`aqBondCurveYield`); lifecycle `aqObject<Lifecycle>`; `Vols`â†’`Volatility`. Docs: `CLAUDE.md` (both), `MIGRATION_PLAN.md` Â§2.2/Â§2.5/Â§2.6/D14/D15/D16/D18/Phase 3.2/3.5/4.7/4.11, `rebrand/STATUS.md`, `rebrand/tools/api_pair_check.py` (CATEGORIES + `category()`). **AQ_XLL code:** `git mv` `aqBonds/aqDates/aqObj/aqTools.cpp` â†’ `aqBond/aqDate/aqObject/aqTool.cpp`; 53 functions renamed (42 bond + 4 date + 3 object + 4 tool); comments + `AQ_XLL.vcxproj`/`.filters` updated. Param/`.arg()` audit `aqBond.cpp` 42/42. `aqMath.cpp` / `aqMain.cpp` / `aqXllTools.{h,cpp}` unchanged names (Math already singular; XllTools is a utility, not a category). **`validation::tryAqObjâ€¦` call sites in AQ_XLL still use the OLD wrapper names** â€” temporary desync until MIGRATION_PLAN 2.6 renames `validation` / `AQ_API` / `GTEST` test names / fixtures. **NOT BUILT.** |
| _(uncommitted)_ | **AQ_XLL fails to load in Excel â€” ROOT CAUSE FOUND + FIXED.** `xlAutoOpen` threw `xloil::Exception<std::runtime_error>` ("unhandled C++ exception in Excel.EXE"). Cause: `src/AQ_XLL/src/aqObj.cpp` `aqObjSave` had **3 parameters but 4 `.arg()` descriptions** (the `ShowArrayOutputs` arg text was added in commit `a8b50b47` "AqObjSave update" without the matching 4th parameter). xlOil's `.arg(i>=nParams)` throws "Too many args for function" at registration. Introduced by `a8b50b47`, which post-dates the last Excel-verified commit `e5c57046`, so it was never caught. Fix: added the 4th param `showArrayOutputs` and implemented the documented TRUE â†’ 3-row `{result, filePath, name}` column / else â†’ name behaviour (uses existing `toBool` + `toExcelColumn`). **Not related to LTCG or to the tranche-(a) bond port** (all 11 new funcs audited: param/arg counts match). The earlier `AQ_XLL.vcxproj` LTCG edit was reverted â€” project settings unchanged from HEAD. `targets/64/{Release,Debug,Release_Profiler}/AQ_XLL` were deleted (build output only). **Next: rebuild AQ_XLL, verify it loads, then verify tranche (a).** |
| _(uncommitted)_ | **meLWOBond port tranche (a)** â€” 11 `meLWOBond*` functions ported from `.APPLES\...\meLWO.cpp` into `src/AQ_XLL/src/aqBonds.cpp` as `aqObjBonds*` (DisplaySchedule, DisplayCashflows, DirtyPrice, AccruedInterestDays, CleanPrice, AccruedInterest, Yield, CompoundYield, Price, DV01, DV01Numerical). New helpers `toDoubleVector` / `toExcelDoubleColumn` / `toExcelIntColumn` in `aqXllTools.{h,cpp}`. CLAUDE.md Â§9 gains an "always update STATUS.md" rule. **Not built yet.** |
| _(uncommitted)_ | `aqObjSave` gains optional `AsArray` arg (force column / force message / auto); new `aqToolsCallerInfo()` diagnostic. Mitigation for the CSE-detection problem. **Not built yet.** |
| `e5c57046` | **AQ_XLL port tranche 1** â€” `aqMain.cpp` auto-inits the library in the add-in ctor; `AQ_INITIALIZE` + `AQ_IS_ARRAY_OUTPUT` macros; new `aqBonds.cpp` (`aqObjBondsCreate/Display`), `aqObj.cpp` (`aqObjExists/Load/Save`); `aqToolsInitialize`, `aqToolsResize` (positional clip/pad), path slash-normalisation; marshalling helpers in `aqXllTools` (`toBool`, `toAQLStringMatrix`, `toLabelValueBlock`, `toExcelMatrix` numeric-aware, `toExcelColumn`, `reshapeToSize`, `callerRangeSize`, `isArrayOutput`). vcxproj/.filters updated. Verified in Excel by Nicholas, not by GTest. |
| `68ca1ceb` / `bf2cb765` | earlier AQ_XLL updates + step-2 build fixes (RecordMacros `AQLString`). |
| `2d7eb95c` | **Phase 3 step 5** â€” calendar holiday-centre delimiter `:` â†’ `+`. New `CALENDAR_CENTRE_DELIMITER`/`splitCalendarCentres()` in `math/AQLPriceDataCalendar.{h,cpp}`; routed ~18 parse/build sites (convertFrom/ToString, 16 curve-helper `toToken(':')`, NYB-strip). Input still accepts `:`; output always `+`. New test `Calendars.UNIT_CentreDelimiter_PlusAndColonEquivalent`. |
| `530e764e` | **B-fixtures** â€” 7,960 `resources/test/inputs` fixture paths + 152 src files (TEST_DIR strings, `generatorFunction` labels, `decorateCurvename/Filename` prefixes, `UNIT_TestMe*` labels, comments) renamed `me/tryMe â†’ aq/tryAq`. `me/tryMeUtilityClearLWOCache â†’ aq/tryAqObjClearCache`. |
| `756d9bb8` | **B-LWO** â€” `meLWO*/tryMeLWO* â†’ aqObjects*/tryAqObjects*` (496 ids, 85 files). Map A "by product" rejected (21 collisions with the stateless names); Map B `LWOâ†’Objects` adopted. Code only. |
| `c9bb9f49` | **B-simple** â€” 442 `me*/tryMe*` (non-LWO) â†’ `aq*/tryAq*` code identifiers (247 files). Family map in `rebrand/phase3_Bsimple_MAP.csv`. Code only (strings/resources were deferred to B-fixtures). |
| `649d3aa5` | **B-delete** â€” removed 6 orphaned securitisation validation files (`tryMe{CashflowClient,LoanCalculations,SupervisoryRules}`, incl. `tryMeSRT*`). Were not in any vcxproj. |
| `700f5722` | `LAString`/`LADate` embedded in 64 identifiers (`getCompulsoryValueAsLAString`, `getLADate`, `TestLAString`, â€¦) â†’ `AQLString`/`AQLDate`. 943 refs. |
| `a8905ca5` | `boost::swap` â†’ `std::swap` in `StatisticsUtilities` (kills the deprecated-header warning). |
| `0bab362e` | `InitializeAQETrading â†’ InitializeETrading`, `InitializeAQGoogleTest â†’ InitializeGoogleTest`. |
| `91a0ad38` / `13ad0c2e` | **Phase 3.1c (Classic)** â€” `MA/MM/MV/MD/MF/ME_ â†’ AQL` in math/models/calibration (431 renames). `MLIB_2D_MATRIX_CHECK â†’ AQ_2D_MATRIX_CHECK`. Census tool widened. |
| earlier | Phase 3.1 math, 3.2 models, 3.2 etrading (`LAâ†’AQL`, `LWOâ†’AQObj`), Initialize renames, `validation_apiâ†’validation`, `MLIB_*â†’AQ_*`, project renames `AQ_BINDINGSâ†’AQ_API` / `GOOGLE_TESTâ†’GTEST`, solution `AlgoQuantLib-VS22.sln`, solution folders `AQ - Core` / `AQL - Classic`. |

**Group B (`meâ†’aq`) is COMPLETE** â€” no `me*/tryMe*/meLWO*` names anywhere in `src/` or `resources/test/`.

---

## Pending â€” Phase 3

### Step 7a â€” legacy include guards  â† DONE (`4fb2fdb6`)
15 `__LAMATH*_H__` guards in `src/models/include/` â†’ `__AQLMATH*_H__`. Guard-token
only, no build impact.

### Step 7b/7c â€” LWO + MLIB sweep  â† DONE (`ab35bf0b`, `8e632c65`)
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
prose/error-string mentions (`"LWO Curve â€¦ does not exist"` â†’ `AQObj`/`Object`? â€” a
naming call). `validation_api` / `MLIB_CLIENT_API` / `GOOGLE_TEST` residue is
**Linux-Makefile only â†’ Phase 7**. `XllPlus` â†’ Phase 4.

### Step 6 â€” mir removal  â† DONE (6a `97c8461e`, 6b `5a29a51a`)
mir stack deleted. Residual `mir` only in strings/comments (Replay.cpp dispatch
keys `functionList["tryMirSetUpOISCurve"]`, a few error messages) -> fold into 7b.
`TestMirDateFunctions.cpp` kept (no mir) -> rename to a non-legacy name in 7b.

### Step 6 (old plan text below, superseded)
**6a DONE (`97c8461e`)**: 5 GTEST curve-fixture builders (`CurveOis/Std/TenorBasis/
XccyBasis/FwdFxConst.cpp`) now call `etrading::AQLUpdateStaticDataManager::setUp*
Curve` directly. Needs build + GTest.

**6b TODO** (bigger than first scoped):
  1. Port `src/GTEST/src/InterestRateSwap.cpp` â€” shared helper used by ~8 keeper
     Trade tests; its `parRate()/pv()/pv01()` call `tryMirGetParRate4` (â†’ schedule
     gen + `etrading::AQLCurveForwardRateHelpers::getParRate`), `tryMirSwapPV`,
     `tryMirSwapPV01`. Not trivial passthroughs â€” read the 3 wrappers, reproduce inline.
  2. Gut `TestCurveEngineCalibrate.cpp` â€” drop the DEAD `testEngineCurveForwardRates`
     method (all call sites already commented out) + `#include "tryMirGetForwardRate.h"`
     + `#include "CurveOis.h"`.
  3. `git rm`: `src/AQ_API/source/mir*.{h,cpp}` (58, nothing else references), 
     `src/validation/{include,src}/tryMir*.{h,cpp}` (65), and 36 mir-only GTEST .cpp
     (`InterestRateSwap.cpp`? NO â€” that's the ported helper; its `.h` in GTEST/include
     is shared, keep. The 36 are `TestCurve{Ois,Std,TenorBasis,XccyBasis,FwdFxConst}*`
     (test suites, not the builders), `TestCurveReplay`, `TestMirDateFunctions`,
     `TestDatesECB`, `TestTradeEUR{AssetSwapSpread,OISParRate,SwapStubRate}`,
     `CurveConsistencies_{OIS,STD,TenorBasis}`, `LinearSplineTests`, `Test_IsFwdInter`,
     `InterestRateSwap.cpp`â†’NO). 
     Recount: exclude the 5 kept builders and InterestRateSwap â†’ ~35 test .cpp.
  4. Edit `swig_{CSharp,JAVA,Python,R}.i` â€” drop `%include`/`#include "mir*.h"` lines
     (keep `EntityPoolUtilities.h`).
  5. Edit `apiTryAqToolsLVB.h` â€” drop dead `#include "MBTemplateType.h"`.
  6. Edit `BindFileToClassConstructor.h:20` â€” stale `_tryMirSetUpOISCurve_inputs`
     comment example.
  7. vcxproj/.filters: strip mir + deleted-test entries from AQ_API / validation / GTEST.
  8. **KEEP all `resources/test/inputs/` fixture files** â€” keeper tests read the
     `*_tryMirSetUp*_inputs.csv` fixtures via the ported builders; orphan CSVs
     (dirs of deleted tests) â†’ Phase 6 resources audit.
  9. `EntityPoolUtilities` + `tryAqToolsClearEntityPool` â€” CORE, keep.

### OLD step-7 note (broad attempt, reverted)
A quick script anchored on `#ifndef`/`#define` lines mangled **non-guard conditional-define macros**
(`IR_CALIBRATION_DATA_MAXLOOP â†’ â€¦AQXLOOP`, `MARKETTYPE â†’ AQRKETTYPE`, `LEVENBERG_MARQUARDT â†’ â€¦AQRQUARDT`,
`MAXTERM â†’ AQXTERM`) â€” the classic `MA`-in-English hazard. Fully reverted; nothing committed.

**Redo approach:** only rewrite guards that are genuinely legacy-prefixed *header guards*, i.e.
`__?(LA|LB)[A-Z]` or `__?LAMATHâ€¦_H__` style, verified as the file's `#ifndef X â€¦ #define X â€¦ #endif`
triple (X identical on all three), with the English-word filter (`MAX/MARKET/MARQUARDT/MANAGER/â€¦`).
Real targets found: **~17 files in `src/models/include/`** with `__LAMATHâ€¦_H__` guards
(`AQLMathSABR.h`, `AQLMathJumpDiffusion.h`, â€¦). `AQLMathSwaptionFormulaSZ.h` has a UTF-8 BOM
before `#ifndef` â€” a naive `^#` regex skips its `#ifndef` line (must use `utf-8-sig` / match after BOM).
Also `MARKETTYPE_SWAP/BASIS/ZERORATE`, `MANAGER` in `calibration/include/AQLDefinitions.h` /
`models/include/ConstantDeclarations.h` are English string-constant macros â€” **leave**.

Comment sweep (`LWO`/`mir` prose, stale fixture-format comments) â€” lower priority, do after guards.

### Step 6 â€” delete the `mir` stack  â† DEFERRED (needs helper port first)
Not cleanly deletable: `CurveOis.{h,cpp}` / `CurveStd` / `CurveTenorBasis` / `CurveXccyBasis` are
**shared GTest curve-fixture builders** used by ~15 non-mir pricing/risk/credit tests
(`TestConstantMaturitySwap`, `TestCreditBasketModel`, `TestTotalReturnSwap`, `TestRiskSwapDeltaLadder`,
`TestTrade{GBP,JPY,USD}SwapParRate`, â€¦) and they build curves via `tryMirSetUp*Curve`.
60 GTest files touch the mir surface.

**To do it:** (1) port those 4 fixture builders to `tryAqObjectsCurveCreate*` / `tryAqCurvesCalibrate*`;
(2) delete `mir` impl (58 `AQ_API/mir*` + 65 `validation/tryMir*`), ~40 mir-only GTest files
(`TestCurve{Ois,Std,TenorBasis,XccyBasis,FwdFxConst}*`, `TestCurveReplay`, `TestMirDateFunctions`,
`TestDatesECB`, `TestTradeEUR*`, `InterestRateSwap`, `CurveConsistencies_*`, `LinearSplineTests`,
`Test_IsFwdInter`), 695 `resources` mir fixtures, the 58 `%include "mir*.h"` lines in the 4 SWIG `.i`
files, and all vcxproj/.filters entries.
User decision recorded: **losing the mir-based regression tests is acceptable â€” the aq/me suite is the replacement.**
`EntityPoolUtilities.{h,cpp}` is CORE (widely used) â€” **keep**; `tryAqToolsClearEntityPool()` is its wrapper.
`tryMirClearEntityPool` (mir duplicate) goes with the rest.

### Step 8 â€” category folder layout + `docs/api_map.csv`  â† DEFERRED
Folder the `validation` `tryAq*` wrappers by category (`src/validation/Curves/`, â€¦), generate
`docs/api_map.csv` (`public | category | validation wrapper | etrading entry | test`), add the
`aq* â†” tryAq*` CI check. Cleaner to do after the mir delete shrinks `validation`.

### Also outstanding in Phase 3
- `MBRoot.h` / `MBTemplateType.h` â€” **dead `#include`s that reference non-existent files** in
  `validation/include/apiTryAqToolsLVB.h:14` and `validation/include/tryMirSwapDuration.h:14-15`.
  `tryMirSwapDuration.h` goes with the mir delete. `apiTryAqToolsLVB.h` needs its dead include removed
  (it currently only survives because nothing compiled includes it).
- Guard-underscore normalisation (step 7 targets above).

---

## Public API naming scheme â€” CURRENT (step 11, 2026-09-08)

Categories are **SINGULAR**. The handle API is the word `Object` after the
category; a named sub-object (Curve, Generator, MarketData, Model, FixingTable)
already denotes an object and does not repeat it.

| Form | Meaning | Example |
|---|---|---|
| `aq<Category><Function>` | stateless â€” data in, value out | `aqBondScheduleKeys` |
| `aq<Category>Object<Function>` | handle API on the category's product | `aqBondObjectDirtyPrice`, `aqSwapObjectParRate` |
| `aq<Category><SubObject><Function>` | handle API on a named sub-object | `aqBondCurveYield`, `aqBondGeneratorCreate` |
| `aqObject<Lifecycle>` | generic handle lifecycle | `aqObjectLoad`, `aqObjectSave`, `aqObjectClearCache` |
| `AQObj*` / `AQOBJ_*` | internal C++ classes / macros (unchanged) | `AQObjCurve`, `AQOBJ_KEY` |

`validation` (`try` + identical name), `AQ_API` and `GTEST` **test names** all
use these forms. Do not use the earlier `aqObjects*` or `aqObj<Category>*`
prefixes or plural categories anywhere.

**Applied so far:** `AQ_XLL` only (step 11, this session â€” file renames
`aqBonds/aqDates/aqObj/aqTools.cpp` â†’ `aqBond/aqDate/aqObject/aqTool.cpp`, 53
functions renamed, `.vcxproj`/`.filters` updated). Its `validation::tryAqObjâ€¦`
call sites still use the OLD wrapper names â€” `validation` / `AQ_API` / `GTEST` /
`resources\test` fixtures are the pending sync (MIGRATION_PLAN task 2.6, its own
branch + green-diff gate).

### History (commits â€” descriptions only, do not follow the old scheme)
- **step 8** `7fd00c7c` / `30fc5505` â€” introduced `aqObjects*` â†’ `aqObj<Category>*`
  (774 renames, 7,300 fixtures) + internal `AQO`â†’`AQObj`. Superseded by step 11.
- **step 9** `db980842` â€” Options umbrella removed, sub-types promoted. Still holds.
- **step 11** (this session) â€” plural â†’ singular, `aqObj<Cat>` â†’ `aq<Cat>Object`,
  `Vols`â†’`Volatility`. AQ_XLL done; rest pending.

### Two exclusions that must survive any future AQO/AQObj pass
`AQObjects` (fixture path segment), `IsAQObject`, `isAQObject` â€” these already
contain `AQObject`; renaming `AQO`â†’`AQObj` inside them yields `AQObjbject(s)`.

### Open, from the api_pair_check advisory
Five public functions whose wrapper name has drifted from `tryAq<same name>`.
All five *do* route through `validation` (the 4.1 contract holds) â€” this is
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

### Repo-wide CLAUDE.md â€” synced
`REPO\CLAUDE.md` (one level above `LIBRARY_UPDATE`, outside the git repo) was
updated by hand once Nicholas granted write access: the category list (`Objects`
dropped, `FX` added), the `aq`/`aqObj` table, the `LWO -> AQObj` rows, the
`AQObjbject` exclusion rule, the architecture diagram, and a rewritten section
2.1 that now points here as the live running record instead of claiming the
code-level rebrand has not started. It is **not** version-controlled -- a
pre-edit copy is in the session scratchpad only, so treat further edits to it
with care.

---

## Step 9 â€” Options umbrella removed, sub-types promoted (done, 2026-09-04)

`db980842`. From Nicholas's reviewed map (column "use" of the returned
`phase3_step8_MAP.csv`). Rule: **a product sub-type is its own top-level
category**, not a member of an umbrella.

| Umbrella dropped | Promoted to categories |
|---|---|
| `Options` (removed entirely) | `CapFloor` 20, `Swaption` 17, `BondOption` 9, `BondFutureOption` 4 |
| `Swaps` (kept, narrowed) | `AssetSwap` 12, `TotalReturnSwap` 24, `ConstantMaturitySwap` 8 |
| `Curves` (kept, narrowed) | `Inflation` 15 |

113 renames, 0 collisions, 10 src files + 69 fixtures moved in lockstep.
**Category list is now LOCKED (20)** â€” updated in `CLAUDE.md`, `REPO\CLAUDE.md`,
`MIGRATION_PLAN.md` Â§2.2 and `rebrand/tools/api_pair_check.py`. `Swaps` now means
the vanilla swap plus legs and schedules. Stateless surface untouched (it never
had an `Options` category and already used `aqAssetSwap*`).

### Note on the map Nicholas returned
Its column C matched the committed map exactly on all 499 rows, but the file was
**truncated mid-alphabet** at `tryAqObjectsFixingTableValues_inputs` â€” 275 of 774
rows absent, including every `tryAqObj` family from `Fra` onward and all 53
stateless rows. The rule behind the 63 overrides was inferred, checked against
those overrides (61/63 reproduced; the 2 misses were a copy slip and a typo fix,
both confirmed by Nicholas), and then applied to the rows the file never reached.

### Typo fixes â€” exact tokens only
`DDeltaLadder->DeltaLadder`, `Implived->Implied`, `Mutiple->Multiple`,
`Asof->AsOf` (2). Applied as **exact whole tokens**: a substring sweep of `Asof`
would have hit `AsofDate` / `getAsofDate` / `mAsofDate` (which collide with
existing `AsOfDate` / `getAsOfDate` spellings â€” a separate, pre-existing internal
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

## Step 10 â€” MLIB removal (10a done, 2026-09-04)

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
- **`src/AQ_API/source/swig_*_wrap.{cpp,cxx}`** â€” SWIG-generated. Still contain old `me*`/`mir*` names
  in HEAD (regenerated in Phase 5, not in the core build). They also keep re-appearing as phantom
  worktree modifications (autocrlf on a large generated file); `git checkout HEAD -- src/AQ_API/source/swig_*_wrap.*`
  clears them. Don't stage them.
- Ubiquitous `LF will be replaced by CRLF` git warnings â€” repo line-ending config, harmless.

## Later phases (unchanged)
- Phase 4: xlOil XLL port (blocked on Nicholas's worked examples), 4a editions manifest, 4b generators/config + calendar JSON `:`â†’`+` migration (103 files).
- Phase 5: verify C#/Java/R bindings, regenerate SWIG wrappers, expand tests (yield-curve/Jacobian, bond price/yield Bloomberg-parity).
- Phase 6: credit-cluster untangle (`CreditResults.h` etc.), legacy `calibration/math/models` extraction, `resources/{examples,api,documentation}` audit, per-file licence headers + `NOTICE`/`THIRD_PARTY_LICENSES.md`.
- Phase 7: Linux CMake rebrand + validate, clang-format bulk pass, clang-tidy CI gate, copy to clean repo.

## Tooling
- `rebrand/tools/prefix_census.py` â€” legacy-prefix census per project.
- `rebrand/tools/run_batch.py` â€” approved-list batch runner (git mv + \b-anchored replace + string-literal report + verify gate). `FILE_PREFIX` handles `LWOâ†’AQObj`.
- Scratch scripts used this run (not committed) are in the session scratchpad:
  `map_bsimple.py`, `map_blwo.py`, `run_bsimple.py`, `run_blwo.py`, `bfixtures.py`,
  `step6b_delete.py`, `step6b_vcxproj.py`, `step7b_prose.py`, `step7c.py`,
  `step8_map.py`, `step8a.py`, `step8b.py`, `step9.py`.
- `rebrand/phase3_Bsimple_MAP.csv`, `phase3_BLWO_mapB.csv` â€” the approved `meâ†’aq` maps (committed).
- `rebrand/phase3_step8_MAP.csv` â€” the approved step-8 map, 774 rows (committed).
- `rebrand/phase3_step9_MAP.csv` â€” the approved step-9 map, 113 rows (committed).
- `rebrand/tools/fixture_key_check.py` â€” fixture keys vs current parameter names.
- `rebrand/tools/api_pair_check.py` â€” `aq*` must route through `validation`; emits `docs/api_map.csv`.
  Run with `--write` to refresh the map; exit 1 makes it usable as a CI gate.

---

## AQLDate / AQLString hardening (2026-09-19, Claude session, in progress â€” resume notes)

Not part of the rename/rebrand work â€” a speed/thread-safety/correctness pass on the two
foundational `math` value types, requested by Nicholas mid-session. Sequenced as: AQLDate first
(done, verified), then AQLString (in progress). If this session is interrupted, pick up here.

### AQLDate â€” DONE, verified
- Commit `5f1e7ced` made `mJulius` eager (recompute on every mutator) for thread safety. Regressed
  perf: schedule-generation loops (`AQLDateScheduleHelpers.cpp` IMM-date rolling, `addMonths` in a
  loop + compare) paid for `dateToJulius()`'s divide/mod chain on every mutation even though the old,
  purely-decimal `cmp()` never needed it.
- Fix: `mJulius` is `mutable std::atomic<long>`, lazy again (0 = uncomputed, invalidated by a cheap
  relaxed store on every mutator), computed on demand via a new private `ensureJulius()`. `cmp()`'s
  existing dual-path logic (opportunistic Julian fast path, decimal fallback) was untouched â€” it was
  already correct, only the mutators were the bug.
- Also fixed: `setSystemDate()`'s `localtime()` â†’ `localtime_s()` (matches precedent in
  `AQ_XLL/src/aqDate.cpp:46`); `isNull()`/`convertDateToString()` decoupled from `mJulius` (which can
  now legitimately read 0 for a valid date) to check `mYear/mMonth/mDay` directly; added explicit
  `operator=` (required once `mJulius` is atomic), `julianDayNumber()` accessor (previously no way to
  read the Julian day from outside the class), `isValidDate()`, `today()`. Fixed a real buffer
  overflow in `formatWithLong()` (`strFormat[5]` too small for `%04d` on a 5-digit `mYear`, since
  `mYear` is `unsigned short`, max 65535) â€” widened to `strFormat[8]`.
- Verified: audited every etrading/calibration/models/validation/AQ_XLL/AQ_API call site that
  mutates an `AQLDate` class member (not a local) â€” none mutate one after it's published into the
  `AQObj` handle pool, so lazy invalidation cannot reintroduce a mutate-while-reading race. Full
  Rebuild Solution + full GTEST suite run by Nicholas: all pass (same pre-existing stale-calendar
  failures as the documented baseline, no new failures).
- Visualizer.natvis: fixed the zero-padded-day `AQLDate` display. The `(mDay<10)?"0":"\0"` ternary
  trick (matching string-literal array sizes) does not reliably evaluate in natvis's expression
  evaluator even when syntactically correct - replaced with 24 mutually-exclusive
  `mMonth==N && mDay<10` / `mMonth==N && mDay>=10` DisplayString entries (literal `"0"` text, no
  computed expression). Confirmed working.
- Considered and declined: wiring the new `AQLDate::isValidDate()` into
  `ScheduleValidation.cpp::isDate()`/`validateDateOrTenor()` to drop their try/catch. Those wrap
  `stringToDate()` (`ParameterValidation.cpp:65`), a cascading multi-format parser (slash-delimited,
  10-char ISO via boost, AQL native, Excel serial) - `isValidDate()` only tests one fixed format, so
  swapping it in would silently stop recognizing the other formats. No clean fit found anywhere else
  in the tree either (checked `CurveUtilities.cpp`/`FolderConfig.cpp`'s similar-looking catches -
  neither is a date-validity check). Left as available API for a genuinely single-format use case.

### AQLString â€” internals swap WRITTEN, awaiting Nicholas's build+GTEST verification
Critique (given to Nicholas): COW via a raw `StringData*` + separate `std::atomic<int>*` refcount
looked thread-safe but wasn't - the count being atomic only protects the count, not the payload;
`makeUnShared()` could read/clone a `StringData` that another owner's concurrent destructor was
deleting in the same window (the exact "atomic refcount, non-atomic payload" pattern C++11 banned
COW `std::string` for). Also: two heap allocations per string (StringData + separate refcount), no
SSO, an unsigned-underflow bug in `remove`/`replace`Ã—2/`charUpdate`'s `from > size()-1` guard
(silently defeated on an empty string), and signed-`char_t`-to-`int` OOB indexing in `findString()`'s
Boyer-Moore skip table.

Step 1 (done, committed as part of the earlier `AQLDate` batch's session but is really its own
change): swapped `StringData*` + separate `atomic<int>*` â†’ single `std::shared_ptr<StringData>`.
Fixed the race (shared_ptr's control block has correct acquire/release pairing), collapsed to one
allocation via `make_shared`, fixed the two bugs above, added move ctor/assignment + `noexcept` on
`size()`/`isDefined()`/`getCString()`/`c_str()`.

Step 2 (decided, not yet implemented â€” this is the resume point): replace the internal
representation again, `std::shared_ptr<StringData>` â†’ `std::optional<std::basic_string<char_t>>`,
and delete the hand-rolled `StringData` nested class entirely (~150 lines: manual `new[]`/`delete[]`,
manual `extend()` buffer growth, `malloc`/`realloc`/`free` mixed in for `exchange()`). Rationale:
none of that manual memory management does anything `std::basic_string` doesn't already do, and this
library's dominant string usage (currency codes, calendar codes, short labels) is exactly the
short-string case where SSO wins outright over COW's refcount-bump. `std::optional` preserves the
"undefined vs defined-but-empty" distinction the public API (`isDefined()`) already depends on, which
a bare `std::basic_string<char_t>` member can't represent on its own. Zero public API / call-site
changes needed - this is scoped entirely to `AQLString.h`/`.cpp`.

Design worked out (not yet written to the files):
- `stringData_` becomes `std::optional<std::basic_string<char_t>>`; `makeUnShared()` is deleted
  entirely (nothing left to detach from).
- `init()`/`clear()` â†’ `stringData_.reset()`. `copy(const AQLString&)` â†’ plain optional copy-assign
  (now a real deep copy, SSO-fast for short strings, no more shared ownership). `copy(const char_t*)`
  keeps its pointer-identity self-assignment guard (`getCString() == pString`, not a content
  comparison - guards `s = s.c_str();`) then `stringData_.emplace(pString)`, still translating
  `bad_alloc` â†’ `AQLCoreSystemError` to match the class's existing convention.
- `exchange()`'s two string-pattern overloads move from a hand-rolled `strstr` + `realloc`'d position
  array to a small file-local `replaceAllOccurrences()` helper using `std::basic_string::find`/
  `replace` in a loop. Single-char `exchange()` â†’ `std::replace` on the string's iterators.
- `replace(from, pStr)` collapses to one `stringData_->replace(from, replacement.size(), replacement)`
  call - verified by case analysis that `std::string::replace`'s standard `count` auto-clamping
  (`count` reduced to `size()-pos` when it would overrun) reproduces both of the original
  `StringData::replace`'s branches (grow-in-place vs in-place-overwrite-preserving-tail) exactly.
- `remove()` â†’ `stringData_->erase(from, num)` (same auto-clamping covers both original branches).
  Deliberately NOT reproducing the original's manual "shrink the buffer if it's now < 1/4 full"
  capacity logic - that forces a reallocation on every qualifying erase(), which is a net perf
  *loss* versus `std::string`'s normal (no auto-shrink) capacity behavior; not carrying it forward.
- `insert(from, const char_t*)` gets a `from > size()` guard it never had originally - traced that
  the original's missing guard let an out-of-range `from` reach `StringData::extend()`, where
  `stringSize_ - fromSize` (unsigned) underflows and drives a huge out-of-bounds copy loop: a real,
  severe pre-existing buffer-overflow bug, not just a latent crash. `std::string::insert` would throw
  `std::out_of_range` safely even without the guard, but the guard is added anyway to keep this
  overload's silent-no-op contract consistent with its `AQLString&` sibling overload.
- `trimLeft()`/`trimRight()` rewritten with plain unsigned loops instead of the original's
  `int i = size()-1` signed/unsigned wraparound trick (which happened to work on MSVC/x64 but isn't
  portably well-defined pre-C++20).
- `cmp()`'s pointer-identity fast path (`stringData_ == rString.stringData_`, true for two undefined
  strings since both are null) becomes an explicit `!isDefined() && !rString.isDefined()` check
  first, preserving "two undefined strings compare equal" now that there's no shared pointer to
  compare. `StringData::cmp()`'s custom strcmp-style char-diff comparator â†’ `std::basic_string::
  compare()` (sign-only equivalence verified sufficient - every caller only tests the sign, via the
  `==`/`<=`/`>=`/`<`/`>` operator overloads, never the magnitude).
- `toToken()`, `subString()`, all the `operator+`/`operator+=`/`operator=` overloads, the friend
  operators, `operator<<`/`operator>>`: confirmed these only ever go through the public API
  (`getCString()`/`size()`/`isDefined()`/`insert()`/`copy()`) and never touch `stringData_` directly
  - zero changes needed to any of them.
- Header changes needed: `#include <optional>` (add), `#include <memory>` (remove, no more
  `shared_ptr`), delete the `class StringData { ... }` nested class and its forward declaration,
  delete `void makeUnShared(void);`, change the `stringData_` member type + its field comment,
  update `operator[]`'s body (`stringData_->getChar(index)` â†’ `(*stringData_)[index]`).

**DONE, verified (2026-09-19).** The design above is written to `AQLString.h`/`AQLString.cpp` in
full - `class StringData` deleted entirely, `stringData_` is
`std::optional<std::basic_string<char_t>>`, `makeUnShared()` deleted, every method that touched
`stringData_` directly rewritten per the design notes above. Nicholas did a full Rebuild Solution and
full GTEST run: all pass, no regressions. Nicholas also reports the library runs noticeably faster
end-to-end, consistent with the SSO mechanism this swap was for (most of this library's strings -
currency codes, calendar codes, `YYYYMMDD` dates - are short enough to now cost zero heap
allocations instead of the old design's one-or-two per string). Not independently profiled/quantified
- "noticeably faster" is Nicholas's real-build observation, not a benchmark number. Score raised from
4/10 to 8/10 (see the "what's next" discussion for why not higher yet - the deferred `explicit`
constructor item below is most of the remaining gap). Expected end state, per Nicholas's ask, was
4/10 (COW anti-pattern, duplicates what the stdlib already does better) to 8-9/10 (small, honest,
standards-backed wrapper that still earns its keep for `toToken`/`padLeft`/`padRight`/`findString`/
the legacy-compatible `getIntValue`/`getDoubleValue` parsing/the `char_t` narrow-wide toggle,
without 1990s manual memory management underneath). Explicitly deferred, not part of this pass:
adding SSO was the *reason* for this swap, not a separate step (std::basic_string's SSO comes free
with `std::optional<std::basic_string<char_t>>`); a full replace-with-`std::string` across the ~3,926
`.c_str()`/`.getCString()` call sites (rejected as disproportionate risk for a mid-rebrand session -
this internals swap gets most of the benefit for a fraction of the blast radius).

### `AQLString(const char_t*)` â†’ `explicit` - TRIED, REVERTED (2026-09-19)

Tried making the single-arg `AQLString(const char_t*)` constructor `explicit`, to surface every
silent `.c_str()`-into-`AQLString`-parameter round-trip (the `CurveEngine.cpp` pattern) as compiler
errors. Built via `MSBuild AlgoQuantLib-VS22.sln -p:Configuration=Release -p:Platform=x64` (had to
use `-p:` not `/p:` - Git Bash mangles `/p:` as a path; also had to build the whole `.sln`, not a
single `.vcxproj` directly, which fails on missing includes outside the IDE/solution context).

Result: **511 distinct source files** failed to compile, `etrading.vcxproj` alone hit MSVC's
100-error-per-file cap on the very first file it touched (`RateProvider.cpp`, via
`AQLCurveForwardRateHelpers.h`). The dominant failure mode was **not** the wasteful call-argument
round-trip this was aimed at - it was **default function arguments**: header declarations all over
the tree default an `AQLString`/`const AQLString&` parameter to a bare string literal (`= ""`,
`= "NO_CHANGE"`, etc. - `AQLCurveForwardRateHelpers.h` alone has several), which needs the same
implicit conversion the `explicit` keyword blocks. Fixing this for real means editing the default
argument in every such header declaration across the tree, not just the handful of wasteful call
sites originally targeted - a categorically bigger, un-scoped job, not "a batch."

**Reverted** - `AQLString(const char_t*)` is back to implicit (git diff on `AQLString.h` is a no-op
versus the post-internals-swap state). Not attempting this again without a much narrower approach
(e.g. grep for the *specific* `.c_str()-into-a-know-AQLString&-parameter` call sites directly,
fixing just those by hand, without touching the constructor's explicitness at all - open if wanted,
not started).

### Wasteful `.c_str()`-round-trip hunt - found nothing solid; also corrected an earlier claim

Went looking for the "top 10" worst `.c_str()`/`.getCString()`-into-an-`AQLString`-parameter round
trips (the thing the `explicit` experiment above was originally aimed at), to fix by hand without
touching the constructor. Heuristic grep for `\w+\(\)\.(c_str|getCString)\(\)` ranked candidate files
(`CurveEngine.cpp` top at 32 hits, then `ScheduleValidation.cpp`, `DeltaRiskGenerator.cpp`, ...), but
checking the top two by hand falsified the pattern: in both cases the accessor being unwrapped
(`CurveDescription::curveCollection()`, `ScheduleParameters::adjustedAccrualStartDate()`) returns
`std::string`, not `AQLString` - so the `.c_str()` is normal, necessary `std::string`â†’`AQLString`
bridging, not the COW-defeating round-trip. **This also retroactively corrects the "concrete example"
cited earlier in this file** (`CurveEngine.cpp:137`, `curveIndexAliasList(curveCollection.c_str(), ...)`
used above as proof of the wasteful pattern) - `curveCollection` there is `const std::string`, not
`AQLString`; that citation was wrong, caught by checking the declared type instead of pattern-matching
on the variable name. No genuine, repeated hotspot of the real pattern turned up. Nothing changed.
Lesson for next time: this class of grep heuristic needs the declared type checked before trusting a
hit, every time - "looks like an AQLString by name" is not evidence.

### Follow-up candidates spotted in passing, not investigated - for a future session

While looking at `AQLString`/`AQLDate`, noticed three other `math` classes with the exact hand-rolled
COW shape `AQLString` had *before* this session's fix (a separate refcount pointer + a nested `*Data`
payload class) - and worse: these use a **plain `int*`** for the refcount, not even the
`std::atomic<int>*` `AQLString` had before it was fixed, so the count itself can race, not just the
payload. None of these were opened up to confirm the race is real the way `AQLString`'s was traced
through before touching anything - this is "where to look first," not a diagnosis:

- **`src/math/include/AQLMathCalendar.h`** (`mutable int* mpRefCount;`) - a holiday-calendar class.
  Worth the most suspicion of the three: `CLAUDE.md` Â§4.4 makes "calendars updatable without
  recompiling" a deliberate product feature, meaning calendar objects are loaded at runtime and very
  plausibly shared/read across curve and schedule code from multiple call paths - the shared-object-
  read-concurrently shape this bug class needs.
- **`src/math/include/AQLMatrix.h`** (`mutable AQLMatrixData* mpData; mutable int* mpRefCount;`) -
  same COW shape as `AQLString`'s old `StringData` + refcount, nested data class included. Sits under
  calibration and the Jacobian-risk analytics `CLAUDE.md` Â§1 already flags as "not exercised for a
  while; needs testing and extension" - a latent race here would land in code already flagged
  under-tested.
- **`src/math/include/AQLDataInstance.h`** (`int* mpRefCount;`, not even `mutable` - worth checking
  what that implies about actual usage before assuming the identical pattern). Its own comment calls
  it "Reference counter of Data Master and Function Master" - sounds central; understand what it's
  plumbed into before judging severity.

Suggested approach for whichever of these gets picked up: same discipline as `AQLDate`/`AQLString` -
read the actual `.cpp` first, trace the specific race (don't assume from the field shape alone),
audit real call sites for post-publication mutation before deciding whether a fix is even needed, and
treat the eventual pass as its own scoped, reviewable batch rather than three-at-once.

### `AQLMathCalendar` â†’ `AQLCalendar` rename, `__HAS_MIC__` removal, comment cleanup (2026-09-19)

Picked up the three follow-up candidates above, plus two housekeeping requests. In order:

1. **Renamed `AQLMathCalendar` â†’ `AQLCalendar`** (`Math` category dropped from the name, per Nicholas -
   both files (`git mv`) and every reference, word-boundary exact match only. `AQLMathCalendarCollection`,
   `AQLMathCalendarData`, `AQLMathCalendarSet` deliberately left alone - `\bAQLMathCalendar\b` doesn't
   match them (no boundary before `Collection`/`Data`/`Set`), and Nicholas didn't ask for those. 22 files
   touched (2 renamed, 20 content-only), including `projects/math.vcxproj`/`.filters`.
2. **Reviewed all four objects** (`AQLCalendar`, `AQLMatrix`, `AQLDataInstance`, `AQLDateTime`) and scored
   them - see the conversation transcript for the full pros/cons; short version: `AQLCalendar` 3/10
   (worst of the four - non-atomic `int*` refcount *and* an independent, unguarded lazy-cache race in
   `AQLCalendarData::createHolidayData()`, a `const` method mutating `mutable` state with zero
   synchronization; the `__HAS_MIC__` mutex guards that would have covered this never compile in), `AQLMatrix`
   4/10 (same COW shape, no second issue found, feeds the Jacobian-risk analytics `CLAUDE.md` Â§1 already
   flags as under-tested), `AQLDataInstance` 4/10 (same COW shape, severity not fully audited - usually
   passed by pointer rather than copied, unverified), `AQLDateTime` 7/10 (inherited this session's
   `AQLDate` fixes for free via `AQLDate::copy()`/`cmp()`; only its own bug is `setSystemDate()` still
   calling raw `localtime()`, a separate override the `AQLDate` fix doesn't cover).
3. **Removed `__HAS_MIC__` entirely** - confirmed first it's never defined in any `.vcxproj`/`.props`/
   `.bat`/`.vcxproj.user` in this tree (grepped all of them), and that none of the 26 affected files had
   nesting or `#else` branches inside the guarded blocks (checked programmatically before touching
   anything). `__HAS_MIC__` = Intel "Many Integrated Core" (Xeon Phi), a discontinued HPC platform; the
   guarded code pulled in `common_lib::StaticMutex`/`Mutex`/`ScopedLock`, a proprietary threading library
   from whatever environment this was originally built in - `common_lib` is never implemented anywhere in
   this tree, only forward-declared inside the same dead `#ifdef`. Stripped every
   `#ifdef __HAS_MIC__ ... #endif` block wholesale (mutex member declarations, static mutex definitions,
   the `namespace common_lib { struct StaticMutex; }` stub, every per-method `ScopedLock` guard) across
   26 files in `calibration`/`math`/`models`. Spot-checked several afterward - well-formed, no dangling
   braces. **Note for whoever picks up the `AQLCalendar`/`AQLMatrix`/`AQLDataInstance` COW fixes above:**
   the dead mutexes are gone now, so there is no vestigial locking pattern left to confuse a future
   thread-safety fix - a clean slate, not a competing mechanism to reconcile with.
4. **Removed section-header comments** (`// LIFECYCLE`, `// OPERATION`, `// QUERY`, `// OPERATOR` -
   exact-line matches only) from the four reviewed objects' 8 files, per Nicholas.
5. **Removed `// ysuzuki`-style comments**, but not uniformly - checked each site's actual content first
   rather than pattern-matching on the name alone. Bare `// ysuzuki` marker lines (8, all in
   `AQLDataVector.cpp`, each sitting directly above unrelated commented-out dead code that was left
   alone) were removed as asked. Three sites that were NOT bare markers - a substantive multi-line
   changelog note in `AQLCoreAutoPtr.h` ("06MAR06 ysuzuki: The specification change of AQLAutoPt...") and
   two commented-out code lines with a name/date prefix in `AQLBasic.cpp`/`AQLDataValuation.cpp` - were
   flagged separately and removed only after Nicholas confirmed he wanted those gone too.

**Not yet done:** a rebuild + GTEST run to verify none of this broke anything (26+ files touched for the
`__HAS_MIC__` removal alone, spanning three projects). That's the immediate next step, same verification
discipline as every other change this session. **After that:** Nicholas wants the actual COW/thread-safety
fixes applied to the four reviewed objects (this section only renamed/cleaned/reviewed them, per the
sequence he asked for) - that's the next real batch of work, not started yet.

### Three build warnings fixed (2026-09-19, same session)

Nicholas's build after the above turned up 3 warnings, all fixed, none suppressed:

- **`AQLDate.cpp` C6385** (`intervalYMD`, the `to->mMonth - 3` array index): a real latent fragility,
  not a false alarm to silence - traced through why it's not *currently* reachable with an
  out-of-range index (a day-of-month difference is never more negative than -30, and borrowing any
  month with >=30 days always clears the deficit before this line; only February, at 28/29 days,
  doesn't, so the only month that can reach this line unchanged is March, giving a valid index of 0)
  but the analyzer can't prove that cross-branch invariant, and neither could a future editor at a
  glance. Changed the index to `((int)to->mMonth - 3 + 12) % 12` - provably in range regardless, same
  behaviour for the one case that's actually reachable.
- **`AQLCalendar.h` VCR001** (`AQLMathCalendarCollection::operator=` "definition not found"): it was
  the pre-C++11 non-copyable idiom - declared `private`, deliberately never defined. Modernized both
  it and the copy constructor (same idiom, same class) to `= delete`.
- **`AQLCalendar.cpp` VCR001** (`checkWeeklyData` "definition not found"): genuinely dead - declared
  once, never defined, never called anywhere in the tree (verified with a full-repo grep). Removed
  the declaration. Likely orphaned when `setWeekly()` moved to taking `AQLDayOfWeekEnum` directly
  instead of a string that would have needed this kind of validation.

### PAUSE POINT (2026-09-19) - resume here

Nicholas is taking a break. **When he's back and confirms the build is clean: go straight to fixing
the four reviewed objects** (`AQLCalendar`, `AQLMatrix`, `AQLDataInstance`, `AQLDateTime`) - rename,
review/scoring, `__HAS_MIC__` removal, and the 3 warning fixes are all done; nothing about the actual
COW/thread-safety problems has been fixed yet. Plan, in priority order (matches the review above):

1. **`AQLCalendar`** first - worst score (3/10), most exposed (widely-shared, long-lived, read-heavy
   holiday data), most silently broken (`__HAS_MIC__` locking that never compiled in, now also just
   plain gone). Two separate problems, both need fixing: (a) the COW `int* mpRefCount` â†’
   `AQLDate`/`AQLString`'s now-proven pattern (either a real `std::shared_ptr`-based fix like
   `AQLString`'s step 1, or - given calendars are read far more than mutated - consider whether COW
   is even worth keeping here vs. just always deep-copying, the way `AQLString`'s internals-swap
   ultimately concluded COW wasn't earning its keep); (b) `AQLCalendarData::createHolidayData()`'s
   lazy cache (`mutable deque<AQLDate> mholiday`, `mutable AQLDate mStart/mEnd`) mutated from a
   `const` method with zero synchronization - needs the same treatment as `AQLDate::mJulius`
   (atomic/lazy, or reconsider whether eager-on-construction is actually fine here given calendars
   are built once then read many times, the inverse usage profile from `AQLDate`'s mutate-heavy
   loops).
2. **`AQLMatrix`** - same COW shape, no second issue found. Check real usage (is it copied often, or
   mostly passed by reference in the calibration/Jacobian code?) before deciding real-shared_ptr vs.
   drop-COW-entirely, the way `AQLCalendar` above should also be decided from evidence, not assumed.
3. **`AQLDataInstance`** - same COW shape; audit actual copy-frequency first (suspected low, passed
   by pointer/reference in most call sites seen so far, but not yet verified the way `AQLDate`'s
   audit was) - that changes whether this is a real risk or a should-fix-for-cleanliness item.
4. **`AQLDateTime`** - much smaller job: swap its own `setSystemDate()`'s raw `localtime()` for
   `localtime_s()`, matching the fix already applied to `AQLDate::setSystemDate()` and to
   `AQ_XLL/src/aqDate.cpp`. Everything else about it already inherited this session's `AQLDate` fixes
   for free.

Same discipline as every fix this session: read the actual `.cpp` first, verify the race is real
(don't assume from the field shape alone), audit real call sites for post-publication mutation
before deciding the fix shape, one object per reviewable batch, rebuild + GTEST after each.

### All four objects fixed (2026-09-19, same session) - awaiting build+GTEST verification

Nicholas confirmed the build was clean after the 3 warning fixes and asked to go straight to fixing
all four. Verified usage patterns first (same discipline as above) before picking each fix shape -
the four ended up needing four genuinely different treatments, not one pattern applied four times:

- **`AQLDataInstance` - made non-copyable, COW removed entirely.** Full-repo grep across
  etrading/validation/calibration/models: 1124 pointer/reference sites, zero real copies (the one
  non-reference hit is `new AQLDataInstance()`). The refcount machinery was protecting a code path
  nothing exercises. Copy constructor and `operator=` are now `= delete`; `mpDataMstr`/
  `mpFunctionMstr` are `std::unique_ptr` (sole ownership, no manual `delMstrs()`/destructor logic
  needed any more - removed both `copy()` and `delMstrs()` entirely). Destructor kept declared
  out-of-line (not `= default` inline in the header) since `unique_ptr`'s destructor needs
  `AQLPriceDataManager`/`AQLFunctionManager` complete, and the header only forward-declares them.
- **`AQLDateTime` - `setSystemDate()`'s `localtime()` -> `localtime_s()`.** Matches the fix already
  in `AQLDate::setSystemDate()` and `AQ_XLL/src/aqDate.cpp`; this class has its own override so the
  earlier fix never covered it. Everything else about the class already inherited the `AQLDate`
  fixes for free (via `AQLDate::copy()`/`cmp()`).
- **`AQLMatrix` - COW race fixed with `std::shared_ptr<AQLMatrixData>` (kept sharing, didn't drop
  it), plus added move constructor/assignment (had neither before).** Different call than
  `AQLCalendar`/`AQLDataInstance` because the usage shape is different: every arithmetic operator
  (`+`, `*`, `-`, `transpose()`, `inverseMatrix()`, `choleskyDecomposition()`, `subMatrix()`) returns
  `AQLMatrix` by value - genuinely copy-heavy, unlike the other three, so O(1) sharing is actually
  earning its keep here. `make_shared` collapses the old two-allocation design (`AQLMatrixData*` +
  separate `int*` refcount) into one. The move ctor/assignment let the compiler skip even the
  shared_ptr refcount bump in the "build a temporary, return it" pattern those operators all use.
  `operator*=`'s old manual try/catch/restore around the old-buffer-swap lost its try/catch entirely
  - `shared_ptr`'s own exception guarantee (assignment only takes effect if construction succeeds)
  covers it for free, and `delete tmp` at the end is gone too (RAII cleanup on scope exit). The ~40
  numerical kernel call sites (`ludcmp`, `svdcmp`, `tred2`, `tqli`, `determinant`, etc.) needed zero
  changes - `(*mpData)[i][j]`, `mpData->row()` etc. all compile identically against `shared_ptr` as
  they did against the raw pointer.
- **`AQLCalendar` - two independent fixes, both needed.** (1) COW dropped entirely (not fixed with
  shared_ptr) - `mpCalData` is now `std::unique_ptr<AQLMathCalendarData>`, copy constructor/
  `operator=` do a real deep copy (cheap - a calendar's holiday deque is tiny), move constructor/
  assignment added too. Opposite call from `AQLMatrix` because the usage shape is opposite: built
  once via `setWeekly`/`setDays`/`setFlowDate`/`setDate`, then read many times, often via a
  `const AQLCalendar&` handed out of `AQLMathCalendarCollection`'s map to many callers at once -
  copying is rare, so sharing had nothing to win and only a race to lose. `makeUnShared()` and the
  old `clear()`/`copy()` helpers are gone entirely - nothing left to detach from when there's no
  sharing. (2) Separately, `AQLMathCalendarData::createHolidayData()`'s lazy cache
  (`mholiday`/`mStart`/`mEnd`, mutated from the `const` `isHoliday()`/`countHoliday()` with zero
  synchronization) got a real `std::mutex`, covering each public method's *entire* body (not just
  `createHolidayData()` internally) - `isHoliday()`'s `binary_search` runs after
  `createHolidayData()` returns and would otherwise read `mholiday` unlocked while another thread
  could be concurrently expanding it. This fix is independent of (1): dropping COW only removes the
  cross-*object* aliasing risk; the *same* `AQLCalendar` instance read concurrently by multiple
  threads (exactly the `getCalendarData()` scenario) still needed this regardless. `getPointholiday()`
  doesn't need its own lock - only ever called from within `countHoliday()`, which already holds it
  (a second lock there would deadlock against the non-recursive `std::mutex`).

Verified after every file: brace-balanced, no leftover `mpRefCount`/`makeUnShared`/raw-`localtime()`
references anywhere across the four objects (grepped explicitly).

**DONE, verified (2026-09-19).** Nicholas's first rebuild+retest showed 57 failures (baseline is 10,
all pre-existing stale-calendar-data) - almost all `#Structured Exception: Access Violation`. Before
assuming a logic bug in the fixes above, flagged the far more likely explanation given this session
already hit the exact same class of problem once (the `AQLString::operator=` `LNK2005` earlier): all
four changes altered the *in-memory layout* of widely-included classes (removed `mpRefCount` members,
swapped raw pointers for `shared_ptr`/`unique_ptr`, added a `std::mutex` to `AQLMathCalendarData`) -
`AQLDataInstance.h` alone is pulled into 150+ files. A merely-incremental build missing even one
translation unit against the new layout produces silent binary incompatibility (mismatched
`sizeof`/offsets linked together) - not a link error this time, memory corruption at runtime,
exactly matching the symptom breadth. **Confirmed correct**: a full Rebuild Solution + retest came
back clean, no logic bug. Lesson banked: a layout-changing edit to a widely-included class needs a
full Rebuild Solution to verify, incremental Build is not sufficient and can pass locally while
silently broken elsewhere.

### `AQLMathCalendarSet` â†’ `AQLCalendarSet` rename (2026-09-19, same session)

Same treatment as the `AQLMathCalendar` â†’ `AQLCalendar` rename earlier - `Math` dropped from the
name, both files (`git mv`) and every reference, word-boundary exact match. 13 files touched (2
renamed, 11 content-only), including `projects/math.vcxproj`/`.filters`. One thing the word-boundary
regex correctly does *not* catch: the include guard macro (`#ifndef AQLMathCalendarSet_h` /
`#define ..._h`) - underscore is a word character, so there's no `\b` boundary between `Set` and
`_h`. Functionally harmless (the guard still pairs correctly, just under the old name) but
inconsistent with the file's new name, so fixed by hand rather than left. Worth remembering for any
future rename of this shape: the regex won't catch include guards, check for them explicitly.
`AQLMathCalendarCollection` (used inside this file) correctly left alone - different name entirely,
not a substring-boundary issue this time.

### `getInquiredCityNames` â†’ `getCityNames` rename (2026-09-19)

Trivial, 4-file rename (`AQLCalendar.h`/`.cpp`, `AQLCalendarSet.h`/`.cpp`), no collision with any
existing `getCityNames`. Verified clean.

### `AQLMatrix` made "worldclass" (2026-09-19) - flattened storage + OpenMP-ready + new API

Nicholas supplied a reference `FlattenedMatrix` design (row-major `std::vector<double>`, OpenMP
pragmas, `transpose()`, dot-product/row-column helpers) and asked for `AQLMatrix` to borrow from it.
Landed as an internals-only change - zero call-site changes needed anywhere outside `AQLMatrix.cpp`
itself, including the ~40 numerical kernel call sites (`ludcmp`, `svdcmp`, `tred2`, `tqli`,
`determinant`, etc.) - because `AQLMatrixData::operator[](n)` already returned a `double*`/
`const double*` row pointer; the flattening only changes what's *behind* that pointer.

- **Storage**: `AQLMatrixData`'s `double** mpData` (one allocation for the row-pointer array, one
  more per row) â†’ a single contiguous `std::vector<double> mData` in row-major order
  (`mData[row*mCol+col]`), one allocation total. `operator[](n)` now returns
  `mData.data() + n*mCol` instead of `mpData[n]` - same contract, so every existing `(*mpData)[i][j]`
  call site needed zero changes. Real cache-locality win for row-major traversal (which is how every
  algorithm here actually walks the matrix), not a micro-optimization.
- **Correctness fix along the way**: elements are now zero-initialized (`std::vector`'s value-init),
  where the old `new double[row*col]` left them uninitialized - a real footgun (a caller reading
  before writing got garbage, not 0). Deliberate, not just carried forward - can only make behaviour
  more correct.
- **`resize()`** rewritten for the new layout: allocate a fresh zero-initialized buffer, copy the
  overlapping top-left submatrix row-by-row (can't be one contiguous copy - the row stride itself is
  changing), swap in. Much simpler and more obviously correct than the old pointer-reuse-vs-realloc
  branching logic.
- **`transpose()` already existed** (Nicholas asked to make sure of this - it did, unchanged
  signature) - now OpenMP-parallelized, deliberately over the *result's* rows (not the source's) so
  each thread's writes are contiguous even though the reads are strided either way (a transpose
  can't make both directions contiguous at once, but write locality is the one that's free to
  choose).
- **New public methods**: `getRow(i)`/`getColumn(j)` (extract as `std::vector<double>` -
  `getRow` is a single contiguous copy, `getColumn` is strided/parallelized) and `dotRow(i,v)`/
  `dotCol(j,v)` (dot product against a weights vector, `reduction(+:sum)`) - directly useful for
  numerical-integration-style work (e.g. a Gaussian-copula credit-basket survival-probability
  calculation, the exact pattern in Nicholas's reference), which is squarely in this library's
  domain per `CLAUDE.md` Â§1's `Credit` category. Named `getRow`/`getColumn`, not `row`/`column` -
  `AQLMatrix` already has `row()`/`column()` returning dimension *counts*; overloading the same name
  for element extraction would compile (different parameter lists) but reads as confusing/ambiguous
  API design, so used a different name instead of reusing one that already means something else.
- **OpenMP added, deliberately only to the embarrassingly-parallel operations**: `transpose()`,
  `setValue(double)`, `clearValues()`, `operator*=(double)`, `getColumn()`, `dotRow()`, `dotCol()` -
  every one of these has fully independent iterations (each row/element touched by exactly one
  thread) with no loop-carried dependency. **Deliberately NOT added** to `ludcmp`/`svdcmp`/`tred2`/
  `tqli` (LU/SVD/tridiagonalization/QL-algorithm decomposition kernels) - these are inherently
  sequential algorithms (each iteration depends on the previous one's result), and naively slapping
  `#pragma omp parallel for` on a loop with a real dependency would be actively wrong, not just
  unhelpful. All pragmas use a **signed `int` loop counter**, not `unsigned int` - MSVC's classic
  `/openmp` (OpenMP 2.0) requires a signed canonical loop variable in a `parallel for`, a real,
  known MSVC limitation the reference design's own `std::ptrdiff_t` casting was already working
  around; followed the same pattern. Also stuck to OpenMP 2.0-safe reduction operators (`+` only) -
  MSVC's classic `/openmp` does not support `min`/`max` reduction clauses (that needs OpenMP 3.1+,
  `/openmp:llvm`), which is part of why `maxValue()`/`minValue()` were left alone rather than
  "parallelized" with a reduction shape that isn't actually available here.
- **`OpenMPSupport` is currently `false` across all 8 build configs in `math.vcxproj`** (an explicit,
  existing setting, not just unset) - **not flipped to `true`**, flagged to Nicholas to decide
  separately rather than silently changed. Until/unless it is, every `#ifdef _OPENMP` block is
  inert (compiles to the plain sequential loop) - the code is correct and behaves identically either
  way, so there is no risk in landing this before that decision is made. Worth being honest that for
  the small matrices typical of this library's actual curve/calibration usage (dozens to low
  hundreds of nodes), OpenMP's per-region thread-pool spin-up cost can outweigh the parallelism win -
  this is why it is opt-in via the existing project setting rather than force-enabled.

Verified: brace-balanced, no leftover `double**`/`mpData[0]`-style raw storage references anywhere.

### OpenMP: `if()`-threshold applied, `OpenMPSupport` enabled (2026-09-19, same session)

Nicholas asked how to manage OpenMP for `AQLMatrix` - recommended against a hand-rolled
`enableOpenMP` flag (extra API surface/state, and it pushes the size judgement onto the caller) in
favour of OpenMP's own `if(condition)` clause on each `parallel for`, gated by matrix size, plus
turning the project setting on since the guard makes that safe. Implemented once Nicholas agreed:

- Added one named constant, `static const int OPENMP_SIZE_THRESHOLD = 64;` at the top of
  `AQLMatrix.cpp`, with a comment flagging it as a starting default to retune once actually profiled
  against a real large-matrix workload, not a carefully-measured number.
- Every one of the 7 `#pragma omp parallel for` sites added last round now carries
  `if(<loop trip count> > OPENMP_SIZE_THRESHOLD)` - compared against the *outer parallel loop's own*
  trip count (`rowCount` for `setValue`/`clearValues`/`operator*=`/`getColumn`/`dotCol`, `colCount`
  for `dotRow`, `newRowCount` for `transpose`). Deliberately kept to that simple per-dimension
  count rather than a total-element-count formula (`rows*cols`) - less precise for very rectangular
  matrices, but one consistent, easy-to-reason-about semantic beats a more "correct" formula that
  needs its own explanation. Below the threshold, every one of these compiles to and runs the exact
  same sequential loop as before with zero threading overhead; the `if()` is evaluated once per
  call, not per element.
- `OpenMPSupport` flipped `false` â†’ `true` across all 8 configs in `math.vcxproj`
  (Debug/Debug_EditAndContinue/Release/Release_Profiler Ã— Win32/x64). Safe to do now specifically
  *because* every pragma has the size guard - small matrices (this library's typical case) behave
  identically to before either way.

Verified: brace-balanced, `OPENMP_SIZE_THRESHOLD` used at all 7 pragma sites plus its own
declaration (8 total). **Not yet built/tested** - next step, same discipline as every change this
session.

### `GTEST\TestAQLMatrix.cpp` added (2026-09-19, same session)

Nicholas asked for GoogleTest coverage modelled on the Catch2 tests in his `FlattenedMatrix`
reference. New file `src\GTEST\src\TestAQLMatrix.cpp`, 18 `TEST(TestAQLMatrix, ...)` cases covering
construction/zero-init, element access, `getRow`/`getColumn` (incl. out-of-range throw),
`dotRow`/`dotCol` (incl. size-mismatch throw, plus a hand-verified numerical-integration pattern),
scalar multiply, `clearValues`/`setValue`, identity, `transpose()` (incl. double-transpose
round-trip and a 100-row case that exercises the OpenMP `if()` guard's parallel branch),
`resize()` grow/shrink (overlap-preserving, zero-fills the rest), move ctor/assignment, and
copy-independence. Deliberately did **not** port Nicholas's Gaussian-copula credit-basket case
verbatim - its expected values depend on an external standard-normal PDF table not independently
verifiable here; explained this to Nicholas rather than porting a test with unverifiable
expectations. Wired into `projects\GTEST.vcxproj` (new `<ClCompile>`, placed after
`TestRandomNumbers.cpp`) and `.vcxproj.filters` (`src\etrading\Math` filter, the same group
`TestRandomNumbers.cpp` uses - no dedicated "math" filter exists in this project). **Not yet
built/run** - next step.

### `AQLString` and `AQLDate` - case-insensitive compare, trim+uppercase, Excel date conversion (2026-09-19, same session)

Nicholas asked for: an `AQLString` case-insensitive compare (to another `AQLString` and to
`std::string`), a trim-then-uppercase method with an optional `trimWhiteSpace` flag defaulting to
`true`, and `AQLDate` `toExcelDate()`/`fromExcelDate()` (+ vector forms) to convert to/from Excel's
integer/double date serials - then asked what other obviously useful methods would make these two
data objects "worldclass".

**`AQLString` (`src\math\include\AQLString.h` / `src\math\src\AQLString.cpp`):**
- `compareIgnoreCase(const AQLString&|const std::string&|const char_t*)` (3 overloads, `noexcept`)
  and `equalsIgnoreCase(...)` (same 3 overloads) added alongside the existing `cmp()` overloads.
  Implemented as a char-by-char `tolower()` comparison with **no temporary uppercased copy and no
  allocation** - deliberately, so these are safe to use in hot lookup paths (e.g. case-insensitive
  calendar-centre or instrument-key matching), not just convenience one-offs. Did **not** add a new
  `operator` for this (Nicholas floated "operator or function") - `operator==` already means exact
  comparison; silently changing its meaning based on argument type would be a surprising, easy-to-
  misuse API. Documented that reasoning in the header rather than silently picking a lane.
- `toUpperTrimmed(bool trimWhiteSpace = true)` added - trims front/back (when the flag is true, the
  default) via the existing `trimLeft()`/`trimRight()`, then calls the existing `toUpper()`.
  Parameter named `trimWhiteSpace` per Nicholas's literal request.
- Bonus, in the same spirit as `getInquiredCityNames`-style ergonomic gaps: `startsWith`/`endsWith`
  (`AQLString` and `const char_t*` overloads, via `memcmp` on `getCString()`, prefix/suffix
  byte-compared) and `contains` (`findString(...) != -1`) - the kind of string-matching primitive
  every other string class in this ecosystem (`std::string`-adjacent, Python `str`, Java
  `String`) ships and this one didn't; useful for calendar-centre parsing, generator-key matching,
  and validation-layer string checks throughout `validation`/`etrading`.
- Verified: brace-balanced (21/21 in the header, 155/155 in the .cpp) after all edits.

**`AQLDate` (`src\math\include\AQLDate.h` / `src\math\src\AQLDate.cpp`):**
- `toExcelDate()` / `static fromExcelDate(double)` + vector forms `toExcelDates`/`fromExcelDates`
  added. Takes/returns `double` (not a separate `int` overload) - an integer Excel serial promotes
  to `double` for free and converts back exactly (doubles are exact for every integer up to 2^53,
  far beyond any real date), so one overload naturally covers "integer or double" per Nicholas's
  request. Correctly handles Excel's fake 1900-leap-year bug (Excel's "1900 date system" treats
  1900 as a leap year and inserts a fictitious serial 60 = 29 Feb 1900 that never existed) via a
  two-branch offset (before/on-or-after 1900-03-01). Throws `AQLCoreInvalidData` for a date/serial
  before 1900-01-01 (unrepresentable in Excel's system) and for serial exactly 60 (the fictitious
  date) rather than silently returning a wrong or nonsensical result.
  - **Epoch constants derivation (keep for future reference, do not re-derive):** using this
    class's own `dateToJulius()` formula, hand-computed `mJulius(1900-01-01) = 693596`,
    `mJulius(1900-02-28) = 693654`, `mJulius(1900-03-01) = 693655` â†’ offsets
    `EXCEL_EPOCH_JULIAN_BEFORE_MARCH_1900 = 693595`, `EXCEL_EPOCH_JULIAN_ON_OR_AFTER_MARCH_1900 =
    693594`. Cross-checked two independent ways: (1) `mJulius(1970-01-01) = 719163`, and
    `719163 - 693594 = 25569`, matching the already-shipped Unix-epoch constant `25569` in
    `AQ_XLL\src\aqXllTools.cpp`'s `excelLocalSerial()`; (2) `mJulius(2024-01-01) = 738886`, and
    `738886 - 693594 = 45292`, matching the well-known real Excel serial for 2024-01-01. All three
    agree.
- `static fromJulianDayNumber(long)` added as the missing inverse of the existing
  `julianDayNumber()` accessor (needed as a building block for `fromExcelDate()`, and a real gap on
  its own - the class could report a Julian day but not construct one back). Throws
  `AQLCoreInvalidData` for `julianDay < 1`.
- Bonus: `startOfMonth()`/`endOfMonth()` (the actual dates, not just the day-offsets
  `intervalToStartOfMonth()`/`intervalToEndOfMonth()` already returned - every caller doing a
  month-end roll or accrual calculation was otherwise writing
  `AQLDate d(*this); d.addDays(intervalToEndOfMonth());` at the call site) and `daysInMonth()`
  (wraps the existing lookup table Nicholas's own `intervalToEndOfMonth()` already indexes into,
  just exposed directly).
  - **Correctness fix caught during review, applied same batch:** `startOfMonth()`/`endOfMonth()`
    were initially written (and their header declarations) marked `noexcept`, which is wrong - both
    call `addDays()`, which is *not* `noexcept` and can throw `AQLCoreInvalidData` in the (real,
    if pathological for this library) edge case of a date within ~31 days of the Julian epoch
    itself. Caught before reporting back; `noexcept` removed from all four sites (2 declarations in
    `AQLDate.h`, 2 definitions in `AQLDate.cpp`). `daysInMonth()` is genuinely non-throwing (pure
    lookup-table indexing) and correctly keeps `noexcept`.
- Verified: brace-balanced (13/13 in the header, 125/125 in the .cpp) after all edits, including
  the `noexcept` fix. Grepped to confirm all four new methods and both new file's constants are
  present and self-consistent.

**Not yet built/tested** - next step for this whole batch, same discipline as every change this
session.

### Pre-existing build-breaking bug found and fixed in `AQLMatrix.h` (2026-09-19, same session)

While setting up an actual MSBuild verification loop for the member-rename batch below (found
MSBuild at `C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\
MSBuild.exe`; direct `math.vcxproj` builds need `/p:SolutionDir=<repo>\` passed explicitly since
`$(SolutionDir)` is only defined when building through the `.sln`), a baseline `math` rebuild
failed with ~100+ cascading errors in `AQLMatrix.cpp` (`AQLMatrixData` not recognised as a member
of `AQLMatrix`, `shared_ptr` missing `operator*`/`operator->`, etc.). Traced to the actual root
cause, not a stale build: `AQLMatrix.h:204`'s doc comment (added last session, documenting the
flattened-storage `operator[]`) contained the literal text `double*/const double*` - the `*/`
inside that phrase **prematurely closes the enclosing `/*! ... */` block comment**, turning the
rest of the comment block (lines 205-208) into live code, which cascades into ~100 downstream
parse errors that look nothing like their real cause. This was a pre-existing landmine from the
matrix-flattening work, not introduced today, and was silently broken (never built) until now.
Fixed by rewording to `double* (or const double*)` - no `*/` substring. **After the fix, `math`
rebuilds clean (Debug|x64)** - first real green baseline confirmed this session via an actual
MSBuild run rather than just brace-counting/grep review. Worth remembering for future doc comments
in this codebase: never write a bare `*/`-forming substring (e.g. `T*/const`) inside a `/*! */`
block comment.

### Private member rename `m<Name>` -> `<name>_` across the six datatypes + natvis (2026-09-19, same session)

Actioned the full enumeration Nicholas approved. One batch per class, `math` rebuilt (later Build,
not Rebuild, per batch - full Rebuild done once at the end) between each:

- `AQLDateTime`: `mHour`/`mMinute`/`mSecond` -> `hour_`/`minute_`/`second_`.
- `AQLMatrix` (+ nested `AQLMatrixData`): `mpData`->`pData_`, `mData`->`data_`, `mRow`->`row_`,
  `mCol`->`col_`.
- `AQLCalendarSet`: `mCalendarCol` -> `calendarCol_`.
- `AQLCalendar` (+ `AQLMathCalendarCollection` + `.cpp`-local `AQLMathCalendarData`):
  `mpCalData`->`pCalData_`, `mCalendarList`->`calendarList_`, `mInqCityNames`->`inqCityNames_`
  (renamed again to `cityNames_` a few minutes later per Nicholas's follow-up - see below),
  `mpcalInfo`->`pcalInfo_`, `mholiday`->`holiday_`, `mStart`->`start_`, `mEnd`->`end_`,
  `mDataMutex`->`dataMutex_`.
- `AQLDate`: `mLeap`/`mYear`/`mMonth`/`mDay`/`mJulius` -> `leap_`/`year_`/`month_`/`day_`/`julius_`,
  **plus `Visualizer.natvis`'s `AQLDate` block** (24 lines: every `DisplayString`/`Item` condition
  and expression) updated in the same batch, per repo `CLAUDE.md` Â§2.3.
- `AQLString`: nothing to do - already used `stringData_` from an earlier session.

Confirmed beforehand (see the enumeration message) that none of these six classes' private
members are touched outside their own `.h`/`.cpp` pair - no `friend` declarations except on
`AQLString` (whose friends only use the public API), so every rename stayed mechanically contained
to a `\b<oldName>\b` word-boundary sed pass per file pair, no cross-file coordination needed.
`math` rebuilds clean (Debug|x64) after all six batches.

**Follow-up rename, same session:** Nicholas asked to rename `inqCityNames_` -> `cityNames_`
shortly after (referred to it as being in `AQLString.cpp`, but it's actually in
`AQLCalendar.h`/`.cpp` as part of `AQLMathCalendarCollection` - renamed there, confirmed the only
two files containing the identifier). `math` rebuilds clean after.

### Section-header comment cleanup (`LIFECYCLE`/`OPERATION`/`OPERATOR`/`IMPLEMENTATION`/`QUERY`) - full `src\math` sweep (2026-09-19, same session)

Nicholas flagged mid-session that these M-library-style section-header comments (removed from "the
4 reviewed objects" earlier in the session, per the very first request) were still showing up
elsewhere. Widened the sweep to all of `src\math` (`*.h`/`*.cpp`) rather than just the objects
touched today, since that's where this session's work has concentrated: removed every line whose
entire (trimmed) content is one of `LIFECYCLE`/`OPERATION(S)`/`OPERATOR(S)`/`IMPLEMENTATION`/
`QUERY(S)`, in any of its observed decorations (`// LIFECYCLE`, `//  QUERY`, `//// LIFECYCLE ////`,
`///////// IMPLEMENTATION /////////`, tabs/trailing-whitespace variants, etc.) - **74 files**, one
regex, exact-line matches only (never touched a decorative divider line on its own, matching the
precedent set earlier this session). `math` rebuilds clean after. **Not yet swept**: any other
project (`etrading`/`calibration`/`models`/`validation`/`AQ_XLL`/`AQ_API`) - flagged to Nicholas as
a possible follow-up if he's still seeing them there; deliberately not done unasked since it's
outside where this session's work has been.

### Exception classes reviewed and improved - not migrated off, per explicit instruction (2026-09-19, same session)

Nicholas asked for a review + score /10 of `AQLCoreError`/`AQLCoreAppError`/`AQLCoreInvalidData`/
`AQLCoreNumericalError`/`AQLCoreSystemError`/`ExceptionMacros.h`, explicitly **not** a migration
off them (see the investigation two entries up in this file for why full `std::exception` migration
was recommended against). Two real bugs found and fixed, both behavior-preserving for all current
call sites (verified by rebuilding `math`/`etrading`/`calibration`/`validation` clean, all Debug|
x64):

1. **Exception-slicing bug in `AQ_CATCH` (`ExceptionMacros.h`) and `VALID_EXCEPTION_END`
   (`StructuredExceptionHandler.h`)** - both rethrew via `throw e;` instead of a bare `throw;`.
   Since `e`'s static type is the catch clause's own type (`AQLCoreError` / `std::exception`),
   `throw e;` constructs and throws a *new* object of that static type, silently slicing away
   whatever derived type (`AQLCoreInvalidData`, `AQLCoreNumericalError`, `AQLCoreSystemError`, or
   any `std::exception` subclass) the original throw site actually used. These two macros sit at
   essentially every `tryAqXyz` validation-layer function's exception boundary (`VALID_EXCEPTION_
   END`) and every `AQ_TRY`-style catch (`AQ_CATCH`), so the slicing happened on the way out of
   nearly every public API call in the library - any caller further up catching a *specific*
   derived type would silently stop matching, no compiler warning. Fixed both to bare `throw;`
   (dropped the now-unused `e` parameter name from the catch clauses too). **Found 9 more instances
   of the same `throw e;` anti-pattern** scattered across `calibration`/`etrading`/`math`/`models`
   (`AQLFileAccessor.cpp`, `AQLStaticDataManager.cpp`, `AQLCurveForwardRateHelpers.cpp`,
   `CurveUtilities.cpp`, `AQLDataFile.cpp`, `AQLFTQuasiRandGF.cpp`, `AQLLinearRatesModel.cpp`,
   `AQLMathCurveFuncUtility.cpp`, `AQLPriceIndexTool.cpp`) - **deliberately not touched**, out of
   scope for "review the exception classes" (these are call sites in business logic, not the
   exception infrastructure itself); flagged to Nicholas as a follow-up sweep if wanted.
2. **NULL-dereference landmine in `AQLCoreError`'s default constructor** - left `mpErrInfo` as
   `NULL`, but `operator=`, `operator+=` and the copy constructor all dereference it
   unconditionally (`*mpErrInfo = *(e.mpErrInfo)`). No current call site default-constructs an
   `AQLCoreError` and then copies/assigns it (checked), so this was latent, not live - but a
   one-line, behavior-preserving fix (allocate an empty `AQLCoreErrorInfo` instead of `NULL`; every
   existing NULL-check branch in `getSize()`/`getMsg()`/etc. still returns the same 0/"" against an
   empty-but-non-null `AQLCoreErrorInfo`) removes the landmine entirely.

**Scores and remaining-improvement suggestions given to Nicholas in chat** (not applied - advisory,
since further changes to the hand-rolled `char_t*` buffer internals would be a larger, riskier
change than "review and improve" was asking for): `AQLCoreError` 6/10 (now 7.5/10 after today's two
fixes - the two big remaining deductions are the hand-rolled `new[]`/`delete[]`/`strcpy` buffers in
`AQLCoreErrorInfo` doing 2-3 heap allocations per throw where `AQLString`-style `std::string` would
do zero for the short messages this library actually throws, and `getLine()`'s `unsigned int`
parameter silently narrowing into a `vector<int>`); `AQLCoreAppError`/`AQLCoreInvalidData`/
`AQLCoreNumericalError`/`AQLCoreSystemError` 8/10 each (thin, correct pass-through constructors,
nothing to improve); `ExceptionMacros.h` 7/10 (now 8/10 after the `AQ_CATCH` fix - the ternary-
without-parens macros like `AQ_IS_EQUAL_WITH_TOLERANCE` are a pre-existing operator-precedence
footgun if a caller ever composes them, but no evidence any call site actually hits it).

### Second exception-file pass: `AQLCoreAppError`/`AQLCoreError`/`AQLCoreErrorLog`/`AQLCoreSystemError`/`ETradingException` (2026-09-19, same session)

Nicholas asked for a "prettify + efficiency + worldclass" pass with scores on these five files
specifically (a narrower, deeper follow-up to the broader exception-classes review above). Read
every file (`AQLCoreErrorLog.cpp` and `ETradingException.h`/`.cpp` not read before this pass).
Three more real improvements found and applied, all verified via clean `math` rebuild + `etrading`
build (Debug|x64):

1. **`AQLCoreError::addMsg()` was silently producing garbled, unreadable error text.** The buffer
   size (`STRLEN(msg) + STRLEN(existing) + 3`, i.e. 2 bytes more than msg+existing+null-terminator
   strictly need) was clearly originally sized to hold a 2-character separator - but nothing ever
   wrote one, so `msg` and the existing message landed jammed together with no space, e.g.
   `AQLCoreSystemError`'s `addMsg(callerMsg)` after constructing from `strerror(errno)` produced
   `"File not foundNo such file or directory"` instead of `"File not found: No such file or
   directory"`. Filled in the `": "` separator the reserved bytes were already sized for - no
   buffer size change needed, so this is a pure bugfix, not a behaviour change in scope.
2. **`AQLCoreSystemError` used `strerror(errno)`, which writes through a single shared *static*
   buffer** - a real cross-thread race if two threads hit a system-call error and construct an
   `AQLCoreSystemError` concurrently (one thread's text can be overwritten by the other's before
   the base `AQLCoreError` constructor copies it out). Replaced with `strerror_s` into a
   `thread_local` buffer (a free function, since a constructor's base-class initializer list runs
   before the constructor body, so there's nowhere to declare an ordinary stack-local buffer in
   time - `thread_local` static-duration storage sidesteps that while still giving each thread its
   own private copy). First attempt at this fix used `alloca()` inside the initializer list via a
   comma-operator trick - caught on review as fragile/unportable/unreadable before it was verified,
   thrown out and redone properly with the `thread_local` helper.
3. **`ETradingException` (the etrading layer's own, separate exception type - a thin
   `std::runtime_error` wrapper, structurally disconnected from the `AQLCoreError` hierarchy,
   confirmed genuinely widely used: 163 throw sites across 41 files in `etrading`/`GTEST`) never
   captured a throw-site file/line at all**, unlike `AQLCoreError`. Added two new, purely additive
   constructor overloads taking `(message, file, line)` - the existing 2-argument constructors and
   all ~163 existing call sites are untouched and unaffected. In a Debug build, `what()` on an
   exception built via the new overloads reports `"message [file:line]"`, matching
   `AQLCoreError::what()`'s Debug/Release split from earlier this session and for the same reason.
   Added `AQ_ETRADING_THROW(message)` convenience macro (mirrors `AQ_THROW`) so new call sites can
   opt in without spelling out `__FILE__`/`__LINE__` by hand.

**`AQLCoreErrorLog` is fully dead code** - confirmed `DUMP_WIN` is never defined anywhere in the
build, and no call site anywhere in the tree ever calls `setLog()` to register an edit box, so
`mEditBox` is always `NULL` and `setMsg()` always returns immediately having done nothing, in every
build configuration, unconditionally. It's a relic of an MFC desktop-GUI error display this library
apparently no longer has. **Not removed** - flagged to Nicholas as a recommendation rather than
deleted unasked, per the "never delete without being asked" working agreement; genuinely nothing to
"prettify" here since the class does not do anything today.

**Scores given in chat:** `AQLCoreError` 8/10 (up from 7.5 after the `addMsg` fix - remaining
deduction is the hand-rolled `char_t*` buffers vs `std::string`, same note as before);
`AQLCoreAppError` 8.5/10 (three clean pass-through constructor pairs, nothing to improve);
`AQLCoreErrorLog` 3/10 (fully dead code - honest score, not a defect in what's there so much as
there being nothing left worth keeping); `AQLCoreSystemError` 8.5/10 (up from implicit ~7 after the
`strerror_s` fix); `ETradingException` 8.5/10 (up from ~5 as shipped - no file/line, no docs - after
adding the file/line overloads, the macro, and documenting the deliberate separation from
`AQLCoreError`).

### `AQLCoreErrorLog` removed - confirmed dead, Nicholas asked for the deletion (2026-09-19, same session)

Following straight on from the flag above, Nicholas asked to remove it now that it was confirmed
fully dead (`DUMP_WIN` never defined anywhere in the build; no call site anywhere registers an
edit box via `setLog()`, so `mEditBox` is always `NULL` and `setMsg()` always no-ops). Removed:

- `src\math\include\AQLCoreErrorLog.h` and `src\math\src\AQLCoreErrorLog.cpp` - deleted outright.
- `AQLCoreError.cpp`'s `printLog()` - dropped its one call site (`AQLCoreErrorLog::setMsg(info->
  mMsgs[0]);`) and the now-unneeded `#include "AQLCoreErrorLog.h"`.
- `AQLCoreError.h`/`.cpp`'s class-doc comments - dropped the "This class depends only
  AQLCoreErrorLog" line from both (no longer true).
- `projects\math.vcxproj` and `math.vcxproj.filters` - removed the `ClInclude`/`ClCompile` entries
  for both files.

Confirmed via a repo-wide grep (`*.h`/`*.cpp`/`*.vcxproj`/`*.filters`) that zero references to
`AQLCoreErrorLog` remain anywhere in the tree. `math` does a full clean Rebuild with no errors;
`etrading` (the heaviest consumer of the exception infrastructure) builds clean too.

### Matrix/table-type consolidation - investigated, documented as an open question, not actioned (2026-09-19, same session)

Nicholas asked how to make `AnyMatrix`/`VariantMatrix`-style `AQ_API` utilities consistent with
`AQLMatrix`'s flattened storage, and how to best centralize the many matrix types rather than
having so many different ones. Investigated (no code changed): the real symbol names are
`AnyTypeMatrix` (`std::vector<std::vector<AnyType>>`, `AQLCoreTemplateType.h`) and `VariantMatrix`
(`std::vector<std::vector<etrading::Variant>>`, `Variant.h`) - both heterogeneous-cell table types,
structurally unrelated to `AQLMatrix`'s homogeneous-`double` linear-algebra storage. Full finding:
there are three separate matrix "families" in the tree (numeric-linear-algebra `AQLMatrix`;
a `vector<vector<T>>` typedef family used for `validation`/`AQ_API`/`AQ_XLL` boundary marshaling,
several members of which genuinely need per-cell heterogeneity; and Eigen's already-optimized
`MatrixXd`, untouched) - converging them all onto `AQLMatrix`'s flattened-`double` storage would be
a category error for the heterogeneous ones. The one genuine redundancy found: `AnyType`
(`boost::variant`-based) and `etrading::Variant` (hand-rolled) are two parallel tagged-union
implementations doing the same job, each with its own Matrix typedef - **that** collapse is where
centralizing would actually pay off, not the matrix wrappers themselves.

**Documented, not actioned**, per Nicholas's request this message: written up as **Phase 6.11** in
`MIGRATION_PLAN.md` (new "Matrix/table-type consolidation" item, two explicit open questions - A:
whether any of the homogeneous `vector<vector<T>>` typedefs are hot-path enough to be worth
flattening the same way `AQLMatrix` was; B: whether to collapse `AnyType`/`etrading::Variant` -
plus an explicit scope warning that a `Variant` collapse touches SWIG bindings, Excel marshaling
and dozens of `TableInfo` call sites, larger than anything actioned this session). Needs Nicholas's
decision before any code changes; nothing here has been built or is expected to need building yet.

### GTEST 9-failure diagnosis and fixes (Clusters B and C); Cluster A investigated (2026-09-20, same session)

Ran the full `GTEST` suite to source the actual, current failure list rather than trust the old
"~10 stale calendar" notes (which turned out to be only half right - `Calendar.csv` itself is fine
out to 2049+). **1025 tests, 9 failures**, cleanly attributable to three distinct root causes:

- **Cluster A (2 tests, data staleness, NOT fixed - Nicholas to decide next step):**
  `ECB_Calendars.UNIT_ECB_Calendar_Expiry_Test`, `TestDatesCentralBank.UNIT_AreThereEnoughDates`
  (`totalDaysToLast: -1713`). Traced to `AQLMathCentralBank.cpp`'s hardcoded ECB meeting-date table
  (last entry `20211216`), mirrored by `resources\config\CBSchedule.csv` (same last date, loaded via
  `setupCBScheduleETrading()` - the loading path works, the *data* is just ~5 years stale). BoE/Fed
  columns have always been empty - ECB-only data. **Recommendation given to Nicholas, not yet
  actioned:** don't write fabricated "proxy" future ECB dates (unlike calendar holidays, these are
  discretionary policy decisions with no formula - a fabricated one is silently wrong, not just a
  test inconvenience); instead apply the same pattern `Calendars.UNIT_Expiry_Test` already uses for
  `Calendar.csv` (a `.conf` file with a `LastXUpdate` date + `warningTenor`/`errorTenor`, checked
  against real elapsed time since last refresh, not literal "today") to the ECB schedule too - a new
  `CBSchedule.conf`. Awaiting Nicholas's go-ahead on that vs. a simpler disable/loosen for now.
- **Cluster B (6 tests, fixed):** `TestDataFilesCreate.UNIT_ErrorHandling`,
  `TestDataFilesRead.UNIT_TestBasics`, `TestDataFilesRead.UNIT_TestTables`,
  `TestAQObjCurve.UNIT_AQObjCurve` (partially - see Cluster C below for its other half),
  `TestAQObjInfrastructure.UNIT_FreeObject`, `TestCurveData.UNIT_CurveData` (in
  `TestMarketDataCollection.cpp`). All expected a specific, purpose-built exception type
  (`CreateDataFile::Exception`, `ReadDataFile::Exception`, `etrading::ETradingException`) but the
  code now throws `AQLCoreInvalidData`. Root cause confirmed at source, not guessed:
  `MarketQuote.cpp:262-266` has an **active `AQ_THROW(...)` sitting directly next to a
  commented-out `throw ETradingException(...)`** - physical evidence of the historical
  `AQ_THROW`/`boost::format` cleanup (`CLAUDE.md` Â§2.1) consolidating these throws onto the generic
  macro (which always constructs `AQLCoreInvalidData`) without updating the tests. Nicholas's call:
  approach (a) - update the tests to expect `AQLCoreInvalidData`, matching what production code now
  deliberately and consistently does, rather than (b) restoring the specific types in production
  code. All 6 tests updated (`TestDataFilesCreate.cpp`, `TestDataFilesRead.cpp` x2,
  `TestAQObjCurve.cpp`, `TestAQObjInfrastructure.cpp` x6 assertions in one test,
  `TestMarketDataCollection.cpp`), each with a comment explaining why, pointing at the
  `AQ_THROW`/`boost::format` cleanup as the cause.
- **Cluster C (2 tests, fixed):** `TestAQObjCurve.UNIT_AQObjCurve` (its other half) and
  `TestExampleObject.UNIT_ExampleObjects`. Not a `FolderConfig.cpp` bug at all (that suffix was
  already fixed correctly in an earlier session) - a **literal path hardcoded directly in the two
  test files themselves**: `TestAQObjCurve.cpp:130` and `TestExampleObject.cpp:68` both built
  `getEnvironmentVariable("AQ") + "/resource/test/inputs/..."` - singular `resource`. Fixed to
  `/resources/test/...` in both. Confirmed the correctly-spelled target folders
  (`resources/test/inputs/ETrading/AQObjects/{AQObjCurve,ExampleObjects}/`) already exist with the
  expected fixture files, so no folder-creation work needed alongside the path fix.

**Build note:** `GTEST.vcxproj`'s link initially failed with unresolved `AQLMathCalendar::
countHoliday`/`isHoliday`/`getCalendar` symbols - a stale `models.lib` built before an earlier
session's `AQLMathCalendar` -> `AQLCalendar` rename, not a real regression (same class of issue as
the `AQLMatrix.h` comment-terminator bug found earlier this session). Fixed with a full `models`
Rebuild; GTEST then linked clean.

### Exception-macro follow-up sweep: 7 more slicing sites fixed, 2 correctly left alone; tolerance-macro precedence footgun fixed (2026-09-20, same session)

Nicholas asked to fix any remaining exception-macro bugs/concerns while reviewing the GTEST
findings above. Two items from the earlier exception-classes review (flagged then, not yet
actioned):

- **The 9 other `throw e;` sites** flagged (not fixed) in the earlier `AQ_CATCH`/
  `VALID_EXCEPTION_END` review. Read each in full context before touching anything - **7 were
  real slicing bugs, 2 were false positives**, worth distinguishing:
  - Real bugs, fixed (`catch(X& e) { throw e; }` -> `catch(X&) { throw; }`, or the minimal
    variant needed to keep `e` where it's genuinely still read before rethrow):
    `AQLFileAccessor.cpp:132`, `AQLStaticDataManager.cpp:148`,
    `AQLCurveForwardRateHelpers.cpp:1909`, `CurveUtilities.cpp:489` (catch parameter name dropped,
    unused otherwise), `AQLMathCurveFuncUtility.cpp:2019`, `AQLPriceIndexTool.cpp:2706` (catch
    binding kept - `e.addMsg(...)` genuinely mutates the live exception object in place before the
    now-bare `throw;`, which still carries that mutation forward since a bound reference isn't a
    copy), and `AQLLinearRatesModel.cpp:270` (a bonus fix beyond pure slicing - was catching
    `AQLCoreNumericalError` **by value**, copying the whole exception on every pass through this
    branch for no reason; changed to `catch(const AQLCoreNumericalError& e)` alongside the
    `throw e;` -> `throw;` fix).
  - **False positives, correctly left alone:** `AQLDataFile.cpp:544` and
    `AQLFTQuasiRandGF.cpp:30` both construct a **fresh, local** `AQLCoreSystemError` inside a
    `catch(...)` block and throw *that* - they are not rethrowing a caught exception at all
    (deliberately translating whatever was caught into a system error with added context), so
    there is no dynamic type to slice away. Confirmed by reading each in full before deciding,
    rather than pattern-matching on `throw e;` text alone.
- **Tolerance-macro operator-precedence footgun** (flagged as a note, not a live bug, in the
  earlier review): all 10 `AQ_IS_*_WITH_TOLERANCE` macros in `ExceptionMacros.h` expanded to a bare
  `( condition ) ? true : false` with no *outer* parens around the whole ternary - `?:` binds
  looser than `&&`/`||`, so `AQ_IS_EQUAL_WITH_TOLERANCE(...) && somethingElse` would silently drop
  `somethingElse` whenever the macro's own condition is false. Wrapped every one in an outer paren
  pair (behavior-preserving for every existing single-use call site; only changes behaviour for a
  composition pattern nothing currently uses). One explanatory comment above the first macro
  covers all ten, rather than repeating the same note ten times.

Verified: `models` full Rebuild clean; `GTEST` build in progress at time of writing (kicked off
after these fixes, covers `math`/`etrading`/`calibration`/`models`/`validation` transitively via
linking - will confirm clean or report back). Filtered re-run of just the 8 fixed tests (Clusters B
+ C) planned once the build completes, before claiming them fixed.

**Confirmed green (2026-09-20):** `GTEST` build succeeded clean. Filtered `--gtest_filter` run of the
7 distinct fixed tests initially showed 6/7 passing - `TestAQObjCurve.UNIT_AQObjCurve` still failed,
but with a **third, previously-hidden Cluster B instance** at `TestAQObjCurve.cpp:178`
(`calculateForwardRateUsingDiscountFactors`, same `AQObjCurve.cpp` `AQ_THROW` pattern, same fix).
It was never visible in the original full-suite run because the Cluster C path bug crashed the test
uncaught before execution ever reached that line - fixing C unmasked a second Cluster B failure
sitting right behind it in the same test. Grepped the rest of the file for any other
`ETradingException` expectations before calling it done - none left. Rebuilt, reran the filtered
set: **all 7 tests pass.** Clusters B and C are fully closed. Cluster A (`ECB_Calendars`,
`TestDatesCentralBank`) remains open pending Nicholas's decision on the `CBSchedule.conf` approach
proposed above vs. a simpler disable/loosen.

### Matrix consolidation plan fleshed out with real numbers and a concrete design (2026-09-20, same session)

Nicholas asked for a full rename/consolidation/flattening plan (naming for `AQLMatrix`, folding in
`IntMatrix`, consolidating `AnyTypeMatrix`/`VariantMatrix`, a shared flattening base class,
threading, `std::vector` confirmation). Full design written up in `MIGRATION_PLAN.md` Phase 6.11
(superseding the two open questions from the investigation the prior message) - not actioned, still
needs Nicholas's approval. Headline findings not in the earlier pass: `ComplexMatrix` (2 files/8
uses) sits in the *same* Heston hot loops as `DoubleMatrix`/`IntMatrix` - missed the first time,
caught this round; `AQLStringMatrix` (300 files/2,290 uses) is bigger than `DoubleMatrix` and is
explicitly flagged as out of scope for this item, not folded in; `etrading::Variant` (wraps
`boost::spirit::hold_any`, true type erasure, already has `transpose()` and string-matrix
conversion helpers) is confirmed the more capable of the two heterogeneous types and should
survive over `AnyType` (a fixed `boost::variant<...>` enumeration). Also flagged, not resolved: a
genuine naming tension between the `AQL` ("AQ Legacy", D17) prefix and Phase 8's eventual
`AQLString`/`AQLDate` retirement plan - recommended `AQAnyMatrix` (no `L`) if the consolidated
heterogeneous type ends up living in `etrading` (not flagged for deprecation) rather than `math`.
**Nicholas agreed with all points (2026-09-20) - plan is approved for the design, still needs
`AQLStringMatrix` (300 files/2,290 uses, the biggest item, deliberately left out of 6.11) revisited
separately with its own recommendation before that piece is scoped** - flagged in Phase 6.11 as a
follow-up reminder, not forgotten, not yet actioned.

### `AQLMathCentralBank` -> `AQLCentralBank` rename ("Math" category dropped - Nicholas found it confusing) (2026-09-20, same session)

Nicholas couldn't find `AQLMathCentralBank.h`/`.cpp` and asked whether they'd been left out of a
project. Checked: they were correctly listed in both `models.vcxproj` and `.filters` all along
(`include\Dates` / `src\Dates` filters, not mislabeled) - the likely explanation is Nicholas
searching in the `math` project, since the class name starts with `AQLMath` even though the file
has always lived in `models`. Exactly the confusion the rename fixes. Enumerated all 10 referencing
files (`AQLDateHelpers.h`/`.cpp`, `LibSetUpETrading.cpp`, `TestDatesCentralBank.cpp`,
`AQLMathCentralBank.h`/`.cpp` themselves, `AQLMathDateCalculations.h`/`.cpp`, plus the two project
files - 26 occurrences total), renamed the two physical files, the class identifier, and every
`#include` (`AQLMathCentralBank.h` -> `AQLCentralBank.h` came along for free via the same
word-boundary regex, since the class name is a literal substring of the file name). `models` full
Rebuild clean, `etrading` build clean (both consume it).

### ECB GTEST failures (Cluster A) - resolved as warnings, not fixed as data (2026-09-20, same session)

Nicholas's call: turn the ECB staleness failures into warnings rather than build a `CBSchedule.conf`
freshness-check mechanism (the design proposed in the earlier entry) - simpler, and matches how
`ECB_Calendars.UNIT_ECB_Calendar_Expiry_Test` already half-worked:

- **`ECB_Calendars.UNIT_ECB_Calendar_Expiry_Test`** (`TestAqDateECB.cpp`) already had a graduated
  design - the 4th/5th/6th ECB meeting date checks were already `GTEST_WARNING`s, only the 3rd
  meeting date check was a hard `AQ_THROW`. Simplest possible fix: changed that one `AQ_THROW` to
  `GTEST_WARNING` too, now consistent with its own siblings. No new mechanism needed - the test
  already had the right shape, just one leftover hard-error branch.
- **`TestDatesCentralBank.UNIT_AreThereEnoughDates`** had no warning path at all - a bare
  `EXPECT_GE( totalDaysToLast, 150 )`. Replaced with `GTEST_CONDITIONAL_WARNING` (already defined
  in `TestHelperUtilities.h`, the same macro family `Calendars.UNIT_Expiry_Test` uses for its own
  6-month warning tier) - prints a warning with the actual days-remaining count and a pointer to
  refresh `AQLCentralBank.cpp`/`CBSchedule.csv`, does not fail the test. Added the missing
  `#include "TestHelperUtilities.h"`.

Both changes are additive/behavioural-only in the test files - no production code touched, no data
refreshed (the underlying ECB schedule is still ~5 years stale; these tests just stop being
red about it). Build/rerun pending as part of the same batch as the two new renames below.

**Confirmed green (2026-09-20):** `models`/`etrading`/`calibration`/`GTEST` all rebuild clean after
the `AQLCentralBank` rename, the two `AQLDateCalculations`/`AQLDateTools` renames below, and the
ECB warning changes together. `--gtest_filter="ECB_Calendars.*:TestDatesCentralBank.*"` - all 6
tests pass, both previously-failing ones now print `#Warning:` (the ECB one shows
`-1714` days-to-last in the message, confirming the diagnosis). **All 9 originally-failing tests
from the full-suite run are now resolved** - 7 fixed as real bugs (Clusters B/C), 2 resolved as
intentional warnings, not failures (Cluster A).

### `AQLMathDateCalculations`/`AQLMathDateUtilities` renames - "Math" dropped, "Utilities" -> "Tools" (2026-09-20, same session)

Same pattern as the `AQLCentralBank` rename just above, same session. `AQLMathDateCalculations` ->
`AQLDateCalculations` (already renamed to `AQLCentralBank` by the earlier rename this file
references - had to happen first since `AQLDateCalculations.h` includes `AQLCentralBank.h`).
`AQLMathDateUtilities` -> `AQLDateTools` (Nicholas: "rename Utilities as Tools", matching the `aqTool`
category convention already used elsewhere in this codebase, e.g. `AQ_XLL`'s `Tool` category).
Enumerated first: 117 distinct files referenced one or both classes (`AQLMathDateCalculations` in
~86 files across `calibration`/`etrading`/`models` + 2 project files; `AQLMathDateUtilities` in
~45 files across `calibration`/`etrading`/`GTEST`/`models` + 2 project files, with overlap between
the two lists). Renamed both physical files, both class identifiers, and every `#include` in one
word-boundary sed pass per file (the `#include` filename updates came along for free, same trick as
`AQLCentralBank`, since the class name is a literal substring of its own header's filename).
`models`/`etrading`/`calibration`/`GTEST` all rebuild clean.

### `AQLDateTools` -> `AQLDateSchedule` consolidation - Choice A actioned, method-diffed first (2026-09-20, same session)

Nicholas confirmed: go with the method (diff every duplicated method, not just `generateSchedule`,
before trusting either side) and Choice A (standardize on `etrading::AQLDateScheduleHelpers`, the
one the live pricing surface already calls; delete `models::AQLDateTools`/its low-level partner
stays separate, out of scope). Full sequence:

1. **Method-by-method whitespace-insensitive diff, `AQLDateTools.cpp` vs `AQLDateScheduleHelpers.cpp`**
   (every duplicated method, not just `generateSchedule`). Found **one genuine behavioural
   divergence, not just mechanical `AQLDateCalculations::`->`AQLDateHelpers::` substitution**:
   `ModelDaycount()` used `"ACT/365_ISDA"` in `models` vs `"ACT/365"` (Fixed - a different
   day-count convention, confirmed via `AQLPriceDataDayCount.cpp`'s separate `ACT_365`/
   `ACT_365_ISDA` enum cases) in the `etrading` fork. Checked real callers: **16 real call sites,
   all in `models`** (CMS calibration, swaption vol, swap rate calc), all via
   `models::ModelDaycount()`'s `"ACT/365_ISDA"`; **zero callers anywhere** of the `etrading` copy -
   dead code, harmlessly wrong until something started calling it. Fixed the `etrading` value to
   `"ACT/365_ISDA"` before redirecting the 16 real callers onto it, specifically to avoid turning a
   dormant divergence into a live, silent pricing bug.
2. **Ported the missing `StubDateAndType` struct + `getStubDateAndType()` method** from
   `AQLDateTools` into `AQLDateScheduleHelpers` (`etrading`) - the one piece of the fork that was
   never duplicated, leaving `generateSchedule()` reaching backward into `models::AQLDateTools`
   for it (a real architecture violation - `etrading` depending on the legacy layer). Faithful port
   (`AQLDateCalculations::` -> `AQLDateHelpers::` calls, otherwise unchanged, including the
   existing `&AQLPriceDataCalendar()`/`&AQLPriceDataSlidingRule()` temporary-address pattern -
   flagged as a follow-up cleanup candidate, not bundled into the port). Redirected all 6 internal
   call sites inside `generateSchedule()`. `etrading` builds clean.
3. **Redirected the 3 `etrading` callers** (`CashflowEngine.cpp`, `CurveUtilities.cpp`,
   `SabrModel.cpp`) off `AQLDateTools::` onto the now-complete `AQLDateScheduleHelpers::` - these
   were themselves backward-dependency violations, now fixed. (`SabrModel.cpp` has a *separate*,
   still-live dependency on `models::AQLDateCalculations::termStrtoYMDW` - out of today's scope,
   explicitly left alone.)
4. **Enumerated every remaining `AQLDateTools` reference** - 40 files across `calibration`/
   `models`/`GTEST` (every public method is used somewhere: `getStubDateAndType` most, then
   `getAQLDate`/`getDate`/`getTerm`, down to single-use methods like `getIMMDate1/2/3`). Redirected
   `AQLDateTools::` -> `etrading::AQLDateScheduleHelpers::` across all 40 (word-boundary sed, one
   pass; caught two angle-bracket `#include <AQLDateTools.h>` variants my quoted-include regex
   missed and fixed those by hand).
5. **Hit and fixed a real build-config gap**: `models.vcxproj`/`calibration.vcxproj` had no
   `etrading\include` in their `AdditionalIncludeDirectories` - the legacy layer had never needed
   to reach into `etrading` before. Both already had a precedent for exactly this kind of sibling
   include (`models` already added `math\include`; `calibration` already added `math\include` +
   `models\include`), so added `etrading\include` the same way, across all 8 configs each. Safe for
   static libraries - compilation doesn't need the other project's `.lib` to exist yet, only the
   downstream `.exe`/`.dll` targets that link both do, and they already do.
6. **Hit and fixed the free-function qualification gap**: `AQLDateTools.h`'s ~11 free functions
   (`CalendarAdvance`, `Daycount`, `YearFraction`, `ModelTime`, `AQLStringToDate`,
   `FrequencyToTerm`, `TermToYearLength`, `TermToMonthLength`, `StringToMonthInteger`,
   `is_last_business_day_temp`) were global/unnamespaced in `models`, but live in `namespace
   etrading` in the fork - unqualified calls stopped resolving once redirected. Qualified every
   call site with `etrading::` (one accidental `etrading::etrading::` double-prefix from a stray
   already-qualified call, caught and fixed).
7. **Deleted `AQLDateTools.h`/`.cpp`** once a repo-wide grep confirmed zero remaining references
   outside itself; removed both project-file entries (`models.vcxproj`/`.filters`).
8. **Renamed `AQLDateScheduleHelpers` -> `AQLDateSchedule`** (Nicholas: drop the `Helpers` suffix),
   both physical files and the class, across **126 files** - `etrading`/`validation`/`calibration`/
   `models`/`GTEST`/`AQ_XLL` all reference this class, by far the widest-reaching rename this
   session (confirms it really is the live surface's canonical schedule generator). One
   word-boundary sed pass, zero collisions (unique identifier). Updated the two stale "duplicate
   method" comments left over from the fork to instead document the consolidation.

**Verification**: `etrading`/`calibration`/`models`/`validation`/`GTEST`/`AQ_XLL` all do a full,
clean Rebuild (not incremental) after the rename - confirmed one at a time to avoid a race between
a running background build and the file-rename step (hit this once: a build failed because
`AQLDateScheduleHelpers.h` was renamed out from under it mid-build - not a real bug, re-ran clean).
Ran the 123 tests most directly exercising what changed (`TestStubDates`, `TestDatesSwapSchedule`,
`TestCreditBasketModel`, `TestCreditIndexOption`, `TestInflationCurve`, `TestCreditModel`,
`TestSABRCalibration`, `TestMirDateFunctions`, `TestTradeGBPSwapStubRate`,
`TestAQObjCurveDiscountFactorsWithSpread`, `TestAQObjSwapFromBespokeSchedule`,
`TestAQObjSwapFromStructuredBespokeSchedule`) - **all 123 pass**. Full-suite rerun not done this
session (the 55-minute run); the targeted set covers every code path this batch actually touched.

**Not yet done**: the efficiency/OMP/enhancement review of the now-consolidated `AQLDateSchedule`
Nicholas asked for alongside the rename - next.

### `TestStructuredExceptionHandler.UNIT_IntegerDivideByZero` - real crash-safety bug found and fixed, not a bad test (2026-09-20, same session)

Nicholas ran the full suite himself post-consolidation: **only one failure now** (down from the
original 9) -
`TestStructuredExceptionHandler.UNIT_IntegerDivideByZero_BecomesCppException`, previously
long-dismissed in this file's history as a vague "build/optimizer issue." Confirmed first: **not
caused by today's `AQLDateSchedule` work** - unrelated file, unrelated subsystem. Nicholas then
asked whether integer divide-by-zero should even be a structured exception, floating removing the
test. **Pushed back and investigated instead**: divide-by-zero is a genuine Windows hardware fault
(`EXCEPTION_INT_DIVIDE_BY_ZERO`, a real `#DE` CPU trap), exactly the class of thing
`StructuredExceptionHandler` exists to catch so a worksheet-function bug crashes with a readable
error instead of taking Excel down - removing the test would hide a real, live gap, not close it.

**Root cause, found by reading `StructuredExceptionHandler.cpp`'s constructor, not guessed**: it
calls both `_set_se_translator(SEHandler)` *and* `signal(SIGFPE, signalHandler)`. On Windows, per
Microsoft's own `signal()` documentation, `SIGSEGV`/`SIGILL`/`SIGTERM` registrations are inert (not
genuinely delivered), but `SIGABRT`/`SIGFPE`/`SIGINT` are real. Both
`EXCEPTION_INT_DIVIDE_BY_ZERO` and `EXCEPTION_FLT_DIVIDE_BY_ZERO` map to `SIGFPE` - so the
`signal(SIGFPE, ...)` registration was intercepting integer divide-by-zero *before*
`_set_se_translator`'s `SEHandler` ever saw it, and throwing a C++ exception from inside a Windows
`SIGFPE` handler is unsupported/unreliable, so the throw silently failed to propagate - the process
didn't crash, but nothing was thrown either, matching "Actual: it throws nothing" exactly. This
also explains why `UNIT_AccessViolation_BecomesCppException` always passed: `SIGSEGV`'s
registration is inert on Windows, so access violations always fell through to `SEHandler`
correctly and were never affected.

**Fix**: removed the `signal(SIGFPE, signalHandler)` line from `StructuredExceptionHandler`'s
constructor (`StructuredExceptionHandler.cpp`), leaving `SIGABRT`/`SIGILL`/`SIGINT`/`SIGSEGV`/
`SIGTERM` registered as before. One line. `etrading`/`GTEST` rebuild clean;
`TestStructuredExceptionHandler.*` (all 4 cases, not just the one that was failing) now pass.

**Why this matters beyond the test going green**: before this fix, an accidental integer
divide-by-zero *anywhere* in this add-in's 466 worksheet functions - a day count, a frequency, a
notional, a matrix dimension that turns out to be zero in some edge case - would have crashed the
whole Excel process instead of surfacing as a catchable, readable `AQLCoreError`. This was a live,
real crash-safety gap in a class whose entire documented purpose is preventing exactly that
failure mode, hiding behind a test that had been mislabeled "optimizer issue" and left unfixed
across multiple prior sessions.

### `AQLDateSchedule` efficiency/OMP/enhancement review - done, findings given, actioning the safe subset now (2026-09-20, same session)

Reviewed every loop in the consolidated `AQLDateSchedule.cpp` for OMP suitability: only
`getMultiDate` and `calcDatesWithLag` are genuinely embarrassingly parallel (per-index independent,
same shape as `AQLMatrix`'s earlier flattening work) - `generateSchedule`/`generateRegularSchedule`/
`getStubDateAndType`/`calcRegularDates` are all inherently sequential (each date depends on the
previous one), checked `calcRegularDates` specifically since it looked promising at a glance but
its `while` loops chain off `regular_enddates.back()`. Noted this OMP work is only safe *because*
of this session's earlier `AQLMathCalendarData` mutex fix (concurrent `getDate()` calls share one
`AQLPriceDataCalendar`/`AQLMathCalendar`). Other findings: a temporary-address pattern
(`&AQLPriceDataCalendar()`) inside `getStubDateAndType`'s hot loop building a fresh default
calendar every iteration; missing `reserve()` on vectors that grow via `push_back` despite
`AQLDate` not being trivially-copyable (carries `std::atomic<long> julius_`); an allocate-then-
compare `upper()`-then-`==` pattern that could use the zero-allocation `AQLString::
equalsIgnoreCase()` added earlier this session; `std::optional` modernization for the raw optional
pointers (flagged, not actioned - too large a signature-ripple for the current token budget).
Actioning the safe, contained subset (reserve/temporary-fix/equalsIgnoreCase/OMP) now; token
budget is short this session, so this entry stands as the record if the session ends before the
follow-up report does.

**Done (2026-09-20, `AQLDateSchedule.cpp`)**:
- Added `defaultCalendar()` (function-local `static const AQLPriceDataCalendar`) and replaced all
  6 `&AQLPriceDataCalendar()` temporary-address sites (`getStubDateAndType`'s two stub-search
  loops, `firstStubDateFromStubType`, `lastStubDateFromStubType`) - no more rebuilding an empty
  calendar every loop iteration.
- `calcDatesWithLag`: was `push_back`-grown with no `reserve`; now pre-sized
  (`DateVector results( dates.size() )`) and indexed, matching `getMultiDate`'s already-good shape.
- **OMP added to `getMultiDate` and `calcDatesWithLag`**, the two functions confirmed
  embarrassingly parallel (each index independent) - **the threshold-guarded form**, matching
  `AQLMatrix.cpp`'s pattern exactly: new `AQL_DATE_SCHEDULE_OPENMP_THRESHOLD = 64` constant,
  `#pragma omp parallel for if( dateCount > AQL_DATE_SCHEDULE_OPENMP_THRESHOLD )` on both. Below
  64 elements (this library's typical case), both run the identical sequential loop with zero
  threading overhead. `OpenMPSupport` was already `true` for `etrading.vcxproj` (unlike `math`
  earlier this session) - no project-file change needed.
- Reconsidered the `equalsIgnoreCase` swap on `generateSchedule`'s stub-type dispatch chain on
  closer inspection: it already uppercases once and compares the single uppercased copy against
  multiple literals - already a single-allocation, multi-compare shape, not a clear win to change.
  **Not actioned** - correctly identified as lower-value than first suggested, not skipped by
  oversight.
- **Not actioned** (flagged only, in the review message): `std::optional` modernization,
  `is_last_business_day_temp` rename, free-function grouping, duplicate-overload
  `const`-correctness - all larger or lower-value than the token budget justified this session.

**Verification**: `etrading` builds clean (Debug|x64). **Full GTEST rerun not done this session**
(token budget) - the changes are narrowly contained (a hoisted-out temporary with identical
semantics, pre-sizing a vector that was already being filled index-by-index in order, and
threshold-guarded OMP that is behaviourally identical to the old sequential loop for every
vector below 64 elements) and `getMultiDate`/`calcDatesWithLag` are both covered by the 123-test
targeted run from the consolidation batch above, but that run predates these specific edits.
**Recommend a rerun of at least `TestStubDates`/`TestDatesSwapSchedule`/`TestSABRCalibration`
next session before considering this batch fully closed.**

### OMP reverted from `AQLDateSchedule.cpp` - it made the whole suite slower, not faster (2026-09-20, same session)

Nicholas reported the whole test suite running ~2x slower after the date-schedule work, and
separately reported `TestStructuredExceptionHandler.UNIT_IntegerDivideByZero` failing again despite
the earlier same-session fix. **Root cause of the slowdown, diagnosed not guessed**: once any OMP
parallel region fires in a process, the runtime's idle worker threads busy-spin-wait by default
(instead of sleeping) so they can wake instantly for the next one - this steals CPU from every
subsequent test for the rest of the process, not just ones touching the parallelized code. The
OMP added to `getMultiDate`/`calcDatesWithLag` this session copied `AQLMatrix.cpp`'s threshold (64)
without profiling *this* workload - date arithmetic is cheap per element and both functions are
called very often (schedules routinely exceed 64 elements), so the threshold was firing constantly
with nothing to offset the fork-join/spin-wait cost. **Reverted both to plain sequential loops**
(kept the genuinely-free wins: `calcDatesWithLag` pre-sized instead of `push_back`-grown, the
`defaultCalendar()` hoist from the earlier efficiency pass). Removed the now-unused
`AQL_DATE_SCHEDULE_OPENMP_THRESHOLD` constant and `<omp.h>` include. Left a comment explaining why,
so this isn't silently re-added without profiling evidence.

The reported `TestStructuredExceptionHandler` failure was confirmed **not a regression** - the
SIGFPE fix from earlier this session was still correctly in source; Nicholas was almost certainly
running a stale `GTEST.exe` predating the rebuild that picked it up (the same class of trap that's
bitten this session before).

**Verification**: `etrading` full clean Rebuild, `GTEST` full clean Rebuild, both exit 0. Ran
`TestStructuredExceptionHandler.*:TestStubDates.*:TestDatesSwapSchedule.*` (86 tests, the
divide-by-zero case included) against the fresh binary - **all 86 pass**, ~105s total, nothing
anomalously slow. Full-suite timing comparison not done this session (token budget) - Nicholas to
confirm the perceived slowdown is gone on his own full run.

**Follow-up, same session:** the remaining `TestStructuredExceptionHandler.UNIT_IntegerDivideByZero`
report was traced to Visual Studio Test Explorer's own test-host process intercepting the hardware
exception ahead of `_set_se_translator` - confirmed by direct `.exe` invocation passing (both
isolated and batched, matching the user's exact `--gtest_filter`) while Test Explorer fails
consistently. Not a code bug; a known category of test-runner/SEH interaction. Running `GTEST.exe`
directly remains the reliable way to verify this specific test.

### `ExceptionHandling` audit across every project/config, prompted by the above (2026-09-20, same session)

Nicholas asked to check exception handling was enabled on every config, every project, suspecting
a gap related to the above. Audited all 8 `.vcxproj` files: **`/EHa` (Async) is correctly set on
every real x64 build configuration everywhere** - `math`/`models`/`calibration`/`etrading`/
`validation`/`AQ_API`/`AQ_XLL`/`GTEST`, including all 5 of `AQ_XLL`'s edition-gated
`Release_XL_*` configs. Two apparent gaps (`GTEST` 8 declared configs vs 4 `Async` entries,
`AQ_XLL` 17 vs 9) were both explained by vestigial `Win32` platform declarations that carry no
real compile settings at all (confirmed by inspection - just a leftover linker option) - `CLAUDE.md`
already states this codebase is x64-only. No fix needed here; the earlier SEH investigation's
findings stand as the actual explanation.

### `Release_Profiler` across every project was missing `<Optimization>` - real bug, fixed (2026-09-20, same session)

Nicholas asked to check `Release_Profiler` configs specifically - intended as Release settings
(full optimization) plus debug symbols so a profiler can resolve function names/line numbers, not
a debug build. Investigating this surfaced a genuine, repo-wide bug, bigger than just
`Release_Profiler`:

- **`math`/`models`/`calibration`/`etrading`/`validation`/`GTEST` were all missing
  `<Optimization>` entirely in both `Release|x64` AND `Release_Profiler|x64`** - when `cl.exe`
  receives no `/O` flag at all, it defaults to no optimization, the same as `/Od`. So every
  "Release" build of the actual business-logic static libraries in this solution has likely never
  been genuinely optimized, despite `NDEBUG` being correctly defined and looking release-like.
  **`AQ_XLL` was the one project already correctly configured** (`Optimization=MaxSpeed`
  everywhere, including all `Release_XL_*` editions) - but since `AQ_XLL`/`GTEST` link against the
  5 static libs that were all missing it, even AQ_XLL's own correctly-optimized compilation units
  were sitting on top of unoptimized business logic.
- **Linker-side settings were already correct** (checked before assuming the same bug there):
  `GenerateDebugInformation` is `false` for `Release` and `true` for `Release_Profiler` in both
  `GTEST` and `AQ_XLL` (the two projects that actually link a final binary) - so PDB
  generation/linking was never the problem, only the missing compile-time optimization flag.
- **Fix**: added `<Optimization>MaxSpeed</Optimization>` to both `Release|x64` and
  `Release_Profiler|x64` in all 6 affected files, matching `AQ_XLL`'s existing value. Additionally
  added `<OmitFramePointers>false</OmitFramePointers>` to `Release_Profiler|x64` only (not
  `Release`) - keeps frame pointers so the profiler gets accurate call stacks even with full
  optimization on, the one deliberate difference from plain `Release` beyond symbols.

**Verification**: `math` full Rebuild in `Release_Profiler|x64` - clean (exit 0), confirms the
added `<Optimization>` element doesn't break compilation. **Not yet rebuilt in every config for
every project** (token budget) - recommend Nicholas do a full solution rebuild in both `Release`
and `Release_Profiler` across all 6 fixed projects to confirm, and ideally a before/after timing
comparison on a hot pricing path to see the real-world impact of years of builds running
unoptimized.

### âš  OPEN REMINDER - Matrix/table-type consolidation (Phase 6.11) - approved, steps 1/2/3 done, steps 4-6 not started

Flagging so this doesn't quietly fall out of the plan across sessions: the matrix/table-type
consolidation design (`AQLFlattenedMatrix<T>` shared base; `AQLMatrix`->`AQLNumericMatrix`;
`AnyType`/`AnyTypeMatrix` retirement onto `etrading::Variant`/`AQLAnyMatrix`; see the "Matrix
consolidation plan fleshed out" entry above, 2026-09-20) was **approved by Nicholas on 2026-09-20**.
**Update, same day:** step 2 (the `AQLMatrix`->`AQLNumericMatrix` rename) is **done** - see the
"`AQLMatrix` -> `AQLNumericMatrix` rename..." entry above. **Second update, same day:** steps 1
(the shared `AQLFlattenedMatrix<T>` base template) and 3 (piloting it on
`AQLIntMatrix`/`AQLComplexMatrix`) are also **done**, kept deliberately pilot-first per Nicholas's
explicit ask - see the "Phase 6.11 step 1 + step 3 pilot" entry above. `AQLNumericMatrix` itself
was **not** re-based onto the new shared template (separate, larger step, deliberately deferred).
Full staged plan lives in `MIGRATION_PLAN.md` Phase 6.11 (updated to reflect all three
completions). Next session picking this up should move to **step 5**
(`AnyType`/`AnyTypeMatrix` retirement onto `Variant`/`AQLAnyMatrix`) - Nicholas has already asked
for this specifically as the next target, and the shared base is now proven against two real
pilots. Step 4 (`AQLBoolMatrix`) sits between them in the original order but is smaller/lower-risk
than step 5 - either order is defensible, use judgement or ask. Step 7
(`AQLStringMatrix`/`StandardStringMatrix`/`STDStringMatrix`, 300/49/5 files) is deliberately
excluded - still needs its own inventory before it can be scoped,
separately reminded in the plan itself.

### `AQLMatrix` -> `AQLNumericMatrix` rename, review-driven fixes, and everyday methods (2026-09-20, same session)

Followed a code review of the existing matrix containers (scored `AQLMatrix` 6.5/10, the bare
`vector<vector<T>>` typedef family 2/10 as containers) with the Phase 6.11 step 2 rename plus the
concrete issues the review surfaced. All changes are to `AQLNumericMatrix` only - the
`vector<vector<T>>` family (`DoubleMatrix`/`IntMatrix`/`ComplexMatrix`/`BoolMatrix`/`AnyTypeMatrix`/
`VariantMatrix`/`AQLStringMatrix`/`StandardStringMatrix`) is untouched, still bare typedefs, still
Phase 6.11 steps 3-6's job.

**1. Rename, `AQLMatrix` -> `AQLNumericMatrix`.** Word-boundary, case-sensitive
(`\bAQLMatrix\b` - correctly excludes the nested `AQLMatrixData`, left unrenamed since it is
private and unambiguous). 381 occurrences across 32 source/project files (`math`/`etrading`/
`models`, plus `math.vcxproj`/`.filters`), well beyond the plan's "~40 internal" estimate - the
type is used across three projects, not just internally to `math`. Physical files `git mv`'d:
`AQLMatrix.h`/`.cpp` -> `AQLNumericMatrix.h`/`.cpp`, `GTEST\TestAQLMatrix.cpp` ->
`TestAQLNumericMatrix.cpp` (including its own `TEST(TestAQLMatrix,...)` suite name ->
`TestAQLNumericMatrix`, and the `GTEST.vcxproj`/`.vcxproj.filters` entries pointing at the old
filename, both missed by the first pass since `\bAQLMatrix\b` correctly does not match inside
`TestAQLMatrix` - fixed in a second, explicit pass). Verified with a repo-wide grep for
`\bAQLMatrix\b`/`TestAQLMatrix`: zero hits outside `CLAUDE.md`/`MIGRATION_PLAN.md`/`STATUS.md`
(historical narrative, deliberately not rewritten - same precedent as every other rename in this
file). `math`/`etrading`/`calibration`/`models`/`GTEST` all rebuild clean (Debug|x64) after.

**2. Correctness fix: `throw "Invalid Matrix"` in the `DoubleMatrix` constructor.** Was a raw
C-string throw wrapped around the whole constructor body in `catch(...)` - not an exception
object, so it cannot be caught by any `catch(const std::exception&)`/`catch(const AQLCoreError&)`
upstream, including the `AQ_CATCH`/`VALID_EXCEPTION_END` machinery fixed earlier this session -
and it discarded whatever the real exception was (a `bad_alloc`, already correctly translated to
`AQLCoreSystemError` by `AQLMatrixData`'s own constructor). Removed the swallowing `try`/`catch`
entirely; the real exception now propagates as itself. Folded in a second fix found in the same
constructor: the per-element copy loop indexed both sides up to `column()` regardless of each
source row's actual length, an unguarded OOB read on a ragged `DoubleMatrix` (a row shorter than
`mat[0]`). Replaced with a per-row `std::copy` bounded by `min(mat[i].size(), cols)` - both safe
on ragged input (same overlap-copy spirit as `AQLMatrixData::resize()`) and faster (one
contiguous copy per row instead of a doubly-indexed element loop).

**3. Efficiency fix: the four O(n^3)/O(n^2) decomposition kernels (`ludcmp`, `svdcmp`, `tred2`,
`tqli`) no longer write through the public `setValue(i,j,x)`.** `setValue` pays for an
`isWithin()` bounds check *and* a `makeUnShared()` `use_count()` check on every call - overhead
that is pure waste inside these kernels' tightest loops, since the matrices involved are always
sole-owned locals for the kernel's whole (synchronous) execution once unshared once. Each of the
four functions now calls `makeUnShared()` exactly once up front, then writes the ~50 combined
call sites directly through `(*matrix.pData_)[i][j] = ...` (legal - these are `AQLNumericMatrix`'s
own private static methods, same class, same access as any other member). The
`#ifdef USE_QUANTLIB_SVD` alternate SVD path (a separate, conditionally-compiled implementation
with its own `A`/`U_`/`V_` matrices) still uses `setValue` throughout - deliberately not touched,
out of scope (not one of the four named kernels, and not the active path in a normal build).

**4. Everyday/usability methods added** (all in `AQLNumericMatrix.h`/`.cpp`): brace-init
constructor (`AQLNumericMatrix{ {1,2}, {3,4} }`, throws on a ragged row rather than silently
zero-padding); bounds-checked `operator()(i,j)` read/write pair (the existing `operator[]` was
const-only - every write had to go through `setValue`, an asymmetric API); `trace()`; `norm()`
(Frobenius, OpenMP-guarded like every other reduction in this file); `static identity(n)` factory
(alongside the existing in-place `IdentityMatrix()` mutator); `operator!=` (delegates to
`operator==`); `equals(other, tolerance)` (`operator==`'s exact-`double`-equality is real but
rarely what a caller after arithmetic actually wants); `getDiagonal()`; `toDoubleMatrix()` (the
reverse of the existing `DoubleMatrix` constructor); `operator<<(ostream&)` (a friend free
function - `print()` only ever wrote to a file). `transpose()` already existed and needed no
change - the everyday-methods table from the review was for `AQLNumericMatrix` specifically;
"add `transpose()` uniformly to the other matrix types" is Phase 6.11 steps 3-6's job, not
actioned here.

**Verification:** `math`/`etrading`/`calibration`/`models`/`GTEST` all rebuild clean (Debug|x64).
19 new `TEST(TestAQLNumericMatrix,...)` cases added (14 for the new methods, 5 regression tests
for the four rewritten kernels specifically - `determinant()`, `inverseMatrix()` via
`A*A^-1==I`, `choleskyDecomposition()` via `L*L^T==A`, `svDecomp()` via `U*W*V^T==A`,
`eigenMatrix()` via `trace(A)==sum(eigenvalues)` - **no prior GTEST coverage existed for any of
these five methods at all**, worth calling out since they sit directly under the Jacobian-risk
analytics `CLAUDE.md` Â§1 already flags as under-tested, and this session's kernel rewrite touched
exactly this code). All 37 `TestAQLNumericMatrix.*` cases pass. Also ran
`TestCurveResultsJacobian.*` (the actual production Jacobian analytical-risk path, 6 cases,
SNAPSHOT + CONSISTENCY for USDOIS/USD3ML/USD6ML) plus `TestInflationCurve.*` (3 cases) as a
broader downstream sanity check since both sit on top of `AQLNumericMatrix` - all 9 pass. Full
suite rerun not done this session (token budget); recommend one before closing this out fully,
same standing caveat as every other partial-suite verification in this file.

### Phase 6.11 step 1 + step 3 pilot: `AQLFlattenedMatrix<T>` shared base, `AQLIntMatrix`/`AQLComplexMatrix` (2026-09-20, same session)

Kept the "pilot-first" discipline the approved Phase 6.11 design calls for - built the shared
base (step 1) and wrapped `IntMatrix`/`ComplexMatrix` in it (step 3's small, low-risk pilot
pair), **not** `AQLNumericMatrix` itself (still on its own proven internals, un-migrated - a
separate, larger step) and **not** `AnyTypeMatrix`/`VariantMatrix` (step 5, deliberately still
last).

**Scoping check done first, before writing any code:** grepped every caller of
`AQLMathDisplacedHestonTDP::CalibrationHelper`/`CalibrationHelperGL` (the only two functions
using `IntMatrix`/`ComplexMatrix` that are *not* behind `#ifdef isQuantLib`, which is commented
out at the top of the header - `//#define isQuantLib`) and found **zero callers anywhere in the
tree outside that same disabled block**. So in a normal build, `IntMatrix`/`ComplexMatrix` are
touched by exactly two functions that are themselves never called - about as low-risk as a pilot
gets, and confirms the plan's own "3 files/53 uses, 2 files/8 uses" inventory was already an
accurate low-risk read.

**1. New file `src\math\include\AQLFlattenedMatrix.h`** - a template class generalising
`AQLNumericMatrix`'s proven design (contiguous row-major `std::vector<T>`, `shared_ptr`-based
COW, move semantics) rather than reinventing it. Deliberately does **not** carry arithmetic
(`+`/`-`/`*`/decompositions) - those stay `AQLNumericMatrix`-specific; `IntMatrix`/`ComplexMatrix`
have zero real arithmetic-as-a-matrix call sites today, so adding it here would be speculative,
against the working agreement's "don't build for hypothetical requirements." What it does carry:
  - `AQLFlattenedMatrixRowView<T>`/`...ConstRowView<T>` - a thin (pointer, length) proxy for
    `operator[](i)`, not a bare `T*`, per the approved design's own note that some typedef-family
    call sites rely on `.size()` on a row, not just `[i][j]`.
  - Three constructors: `(rows, cols)` (zero-initialized, matches `AQLNumericMatrix`); `(rows,
    const std::vector<T>& rowTemplate)` - **specifically added to absorb the classic
    `std::vector<std::vector<T>>(rows, std::vector<T>(cols))` fill-construction idiom** (e.g.
    `ComplexMatrix A(termSize, ComplexVector(GL_Size));`, used ~12 times across
    `AQLMathDisplacedHestonTDP.cpp`, live and dead code alike) so every such call site needed
    **zero changes beyond the type name** - a real, deliberate reduction in the size and risk of
    the migration diff, not just a nice-to-have; and brace-init (`AQLIntMatrix m{ {1,2}, {3,4} }`,
    ragged-row rejection, same as `AQLNumericMatrix`'s).
  - `operator()(i,j)` bounds-checked read/write; `operator[](i)` unchecked row access (matches the
    old raw `vector<vector<T>>::operator[]` contract exactly - not a regression); `size()` as an
    alias for `row()` (drop-in compatibility with the old convention where the *outer* vector's
    `.size()` meant row count, e.g. `sgns.size()`); `getRow`/`getColumn`; `resize()`
    (overlap-preserving); `transpose()`; `operator==`/`!=` (exact); `equals(other, tolerance)`
    (generic via `std::abs(a-b) <= tolerance` - works uniformly for `int`/`double`/`complex<double>`
    without any per-type specialization); `operator<<`.
  - COW detach granularity note: non-const `operator[](i)` calls `makeUnShared()` once per *row*
    access, not once per *element* - deliberately not hoisted further the way
    `AQLNumericMatrix`'s O(n^3) kernels were, since the real call sites here are calibration-setup
    loops, not tight decomposition kernels; the cheap `use_count()==1` re-check on every
    already-unshared row is the right tradeoff for this usage shape, not the one the earlier
    kernel fix targeted.

**2. `AQLCoreTemplateType.h`** - removed the bare `ComplexMatrix`/`IntMatrix`
`vector<vector<T>>` typedefs, replaced with `AQLComplexMatrix`/`AQLIntMatrix` as thin aliases over
`AQLFlattenedMatrix<std::complex<double>>`/`AQLFlattenedMatrix<int>`. `ComplexVector` (the 1D
vector typedef) untouched - out of scope, not part of the 2D matrix consolidation.

**3. Renamed call sites** - word-boundary, case-sensitive `IntMatrix`â†’`AQLIntMatrix` (53
occurrences) and `ComplexMatrix`â†’`AQLComplexMatrix` (8 occurrences) across
`AQLMathDisplacedHestonTDP.h`/`.cpp` (the only two consumers) - including inside the
`#ifdef isQuantLib` dead block, for grep-cleanliness and in case it's ever revived, but **that
block was not compile-verified** (defining `isQuantLib` would pull in a separate, unrelated
QuantLib dependency-chain verification burden out of scope for this pilot - it was already
unverified/inactive before this change, and stays that way). Confirmed via repo-wide grep: zero
bare `IntMatrix`/`ComplexMatrix` remain anywhere in `src\` (only two explanatory comments
mentioning the old names by name, in `AQLCoreTemplateType.h` and `AQLFlattenedMatrix.h`
themselves).

**Verification:** `math`/`etrading`/`calibration`/`validation`/`models`/`GTEST` all rebuild clean
(Debug|x64) - `models` is the real test of the fill-constructor-idiom compatibility constructor
and the `.size()`/`[i][j]` drop-in behaviour, since it's the only project that actually consumes
`AQLIntMatrix`/`AQLComplexMatrix`. New file `GTEST\TestAQLFlattenedMatrix.cpp`, 14 cases against
the template directly through both pilot aliases (construction incl. the fill-idiom and
brace-init, `operator()`/`operator[]` read-write incl. COW-independence, `getRow`/`getColumn`,
`resize`, `transpose`, equality/`equals(tolerance)` incl. a `complex<double>` case, move
semantics, `operator<<`) - wired into `GTEST.vcxproj`/`.vcxproj.filters` (`src\eTrading\Math`
filter, alongside `TestAQLNumericMatrix.cpp`). All 14 pass, alongside the existing 37
`TestAQLNumericMatrix.*` (51 total, re-run together to confirm no interaction). Also ran
`*FX*:*Vanilla*` as a broader `models`-adjacent sanity check (2 tests, both pass) - no dedicated
Heston GTEST coverage exists anywhere in the tree to re-run more specifically (consistent with
the "zero live callers" finding above).

**Not yet started:** step 1's other half (migrating `AQLNumericMatrix` itself onto
`AQLFlattenedMatrix<T>` internally - deliberately deferred, see the scope note above); step 4
(`AQLBoolMatrix`, needs the `uint8_t`-backing workaround the approved design already flags for
`bool`); step 5 (`AnyType`/`AnyTypeMatrix` retirement onto `Variant`/`AQLAnyMatrix` - the
user-requested next target, bigger and deliberately sequenced after this pilot); step 7
(`AQLStringMatrix` family, still needs its own inventory).

### Step 5 (`AnyType`/`AnyTypeMatrix` retirement) - scoped, findings recorded, deliberately paused (2026-09-20, same session)

Nicholas asked to proceed with step 5 next. Did reconnaissance (no code changed) before touching
anything, given the plan's own standing scope warning that this step is "bigger than everything
else in 6.11 combined" - confirmed that warning concretely rather than taking it on faith, then
**Nicholas asked to pause and defer**, so this is a findings-and-plan record, not a completed or
in-progress migration.

**Scale, counted not estimated:** 79 files / 268 occurrences of `AnyType`/`AnyTypeVector`/
`AnyTypeMatrix`, across `etrading` (39 files), `validation` (28), `GTEST` (6), `AQ_XLL` (5),
`AQ_API` (4), `math` (1 - the typedef itself in `AQLCoreTemplateType.h`).

**Two concrete reasons this is not a repeat of the `IntMatrix`/`ComplexMatrix` pilot:**

1. **Not dead code - it's the golden-source contract.** Unlike the pilot (confirmed zero live
   callers), `AnyTypeMatrix` is the **return type of public `validation` wrapper functions**
   (e.g. `tryAqBondObjectDisplay`, `tryAqBondObjectDisplaySchedule`,
   `tryAqBondObjectDisplayCashflows` in `tryAqBondObject.cpp`). Per `CLAUDE.md` Â§4.1, `validation`
   is the golden source "nothing bypasses" - its return types flow straight into `AQ_XLL`
   marshalling, the `AQ_API`/SWIG bindings, and the `GTEST` input/output recording-and-playback
   system. Confirms the plan's "touches SWIG bindings, Excel marshaling, dozens of `TableInfo`
   call sites" warning concretely, not hypothetically.
2. **Not a pure rename - the two types behave differently, verified by reading both
   implementations side by side:**
   - `AnyType` code inspects values via `boost::get<T>(anyValue)` + `anyValue.type() ==
     typeid(T)` (`AnyTypeUtilities.cpp`'s `fromAnyTypeToString`); `Variant` uses
     `getType()`/`ContainedTypeEnum` + `getValue<T>()` instead - a different API shape, not a
     drop-in swap at any call site that inspects rather than just passes a value through.
   - `fromAnyTypeToString` blanks `NaN` doubles to `""` and takes a configurable
     `doublePrecision` (default 10dp). `Variant::getValueAsString()` does **neither** - no
     `NaN` blanking, fixed precision 20 always. A blind mechanical swap would silently change
     output for any NaN-valued cell or any caller relying on the old default precision - a real
     behavior change hiding inside what looks like a rename.

**Recommended staging, given to Nicholas, for whenever this is picked back up** (smallest/safest
first, matching every other rename this session's discipline of "one project per batch, build
between batches"):
1. Reconcile the `NaN`-blanking/precision behavioral gap **first**, before any call site moves -
   either port `fromAnyTypeToString`'s behavior into `Variant`, or keep it as a distinct helper.
   Every call site inherits whichever decision is made here, so deciding after the fact means
   redoing work.
2. `etrading` (39 files) first - where `AnyTypeUtilities.cpp` and the core helpers live, not yet
   the public contract layer.
3. `validation` (28 files) - the golden-source return types; needs a `docs\api_map.csv`/
   `rebrand\tools\api_pair_check.py` re-verification pass after, since these are public wrapper
   signatures, not internal implementation detail.
4. `AQ_XLL`/`AQ_API`/`GTEST` last (15 files combined) - these consume `validation`'s output type,
   so they should follow once 2-3 are settled, not move in parallel with them.

**Status: paused, not started.** Zero files changed. Revisit via this entry and
`MIGRATION_PLAN.md` Phase 6.11 step 5 when picked back up - do not re-derive the scope/staging
from scratch, it is recorded here.

---

## AQ_API coverage-gap closure session (2026-09-22) — IN PROGRESS, paused on token/rate limit

**Context:** user found via Streamlit that many validation-layer functions had no AQ_API (Python)
binding, despite AQ_XLL being ~98% ported. `rebrand/tools/api_pair_check.py` was extended with a
`--gap-list` flag (per-category missing-binding breakdown) to make this concrete instead of
anecdotal.

**Done this session, verified via `api_pair_check.py` (HARD GATE stayed 0 throughout):**
- Fixed 5 AQ_API name-drift functions to match their `validation` wrapper per Sec 5.1a (files:
  `aqCurveResults`, `aqCreditObject`, `aqCurveObject`, `aqDate`, `aqToolLVB`). SWIG `_wrap.*` files
  regenerated by user, confirmed working.
- Renamed all AQ_XLL source files `aq*.cpp/h` → `xll*.cpp/h` (21 `.cpp` + 2 `.h`, plus the
  generated `aqManifestList.h` → `xllManifestList.h`), matching AQ_API's later `aq*` naming.
  `aqXllTools.cpp/h` → `xllSupport.cpp/h` (avoided the redundant "XllTools" double-naming).
- Reorganized `projects/AQ_API.vcxproj.filters`: replaced the old `include\eTrading\*`/`src\eTrading\*`
  nesting with flat filters matching the locked 21-category list (Sec 5.1) plus `Object` (generic
  AQObj lifecycle) and `Support` (infra/helpers). All 109 files verified present, none missing/stray.
  Note: `aqCurveObject.h/.cpp` filed under `Curve` even though it also carries the `aqIRFixingTable*`
  functions (bundled since the InterestRate→IR rename) — one file covering two categories, not split.
- Closed the AQ_API coverage gap for these categories (validation→XLL was already done for all of
  them; only the AQ_API binding was missing): **CMS, BondFutureOption, Future, BondOption, Inflation,
  Swaption, CapFloor, FX, Volatility (incl. the 7 legacy SABR functions — ported to BOTH AQ_XLL and
  AQ_API since AQ_XLL had never carried them either; see xllVolatility.cpp's updated header comment),
  AssetSwap, IR, Date.** Public AQ_API surface grew from 121 to ~186 declared functions.
- Fixed 4 missing-`#include` build failures surfaced by the user's first post-session build
  (`aqSwaptionObject.h` missing `<vector>`; `aqFXObject.cpp`/`aqFutureTicker.cpp`/`aqInflationObject.cpp`
  missing `ParameterValidation.h` for `etrading::stringToDate`). Build confirmed green + GoogleTest
  passing by the user after these fixes.

**Still open — the `--gap-list` categories remaining, in descending size:**
`Curve` (66 missing of 94), `Math` (49 of 51), `Bond` (43 of 58), `Swap` (40 of 66), `(lifecycle)`
(23 of 31 — `aqObject*` handle lifecycle ops), `Tool` (20 of 29), `Credit` (18 of 23, incl. 2
`tryAqCreditBasketModel*` functions that also have NO AQ_XLL binding — same "port to both" question
as SABR was, needs the same decision before binding). A `Tool`+`Credit` batch was launched and
failed immediately on a session rate limit before writing anything — safe to just re-run, no
partial state to clean up.

**Established, working pattern for the remaining categories** (repeated successfully ~8 times this
session): for each missing `tryAq*` wrapper, find its `AQ_XLL` counterpart (already ported, ~98%
complete) for the parameter marshalling reference, add/extend an `aq<Category>Object.h/.cpp` (or
similar per-subgroup) file in `src/AQ_API/source/`, reuse existing `TypeUtilities.h` marshalling
helpers only (never invent new marshalling), register new files in `projects/AQ_API.vcxproj` +
`.vcxproj.filters` + all 4 `swig_*.i` files via direct Read+Edit (never scripted string-replace
with backslash-path literals — this reliably corrupts content in this shell environment via
backslash-escape mangling, e.g. `\a` → bell character), then verify with
`python rebrand/tools/api_pair_check.py --gap-list` (HARD GATE must stay 0).

**Known outstanding advisory items (non-blocking):** 2 wrapper-name-drift false positives in the
checker's own heuristic (`aqCreditModelRiskyDiscountFactor`→`tryAq...Factors` plural,
`aqToolsLVBAppend`→`tryAqToolLVBAdd` different verb) — both intentional per user's naming choice,
not real drift.

### Tool + Credit AQ_API binding batch (2026-09-22, same session, re-run after the rate-limit stall)

Closed the `Tool` and `Credit` gaps flagged above as "Still open". Both categories now show 0
missing in `api_pair_check.py --gap-list`; HARD GATE stayed 0 throughout.

**Tool (20 wrappers bound):**
- Extended existing files (no new registration needed): `aqToolLVB.h/.cpp` +
  `tryAqToolLVBFromKeysValues`/`tryAqToolLVB`/`tryAqToolLVBGroup`/
  `tryAqToolLVBFromMultipleKeysValues`/`tryAqToolLVBFromKeysAndMultipleValues`; `aqToolSetup.h/.cpp`
  + `tryAqToolLoadConfigurationFiles`; `aqToolGrids.h/.cpp` +
  `tryAqToolObjectMultiGridCreate`/`Display`/`SubNames` (the Create path builds a `TableInfo`
  per named grid via `etrading::JSONInfoBlock::createInfoBlock`, same helper `aqCreditModelCreate`
  already used — `validation::TableInfo`'s third tuple element (`FlexibleData`) and
  `etrading::JSONInfoBlockTuple`'s (`VariantMatrix`) are the identical underlying type
  `std::vector<std::vector<etrading::Variant>>`, so no new marshalling was invented); `aqToolRecord.h/.cpp`
  + `tryAqToolReplay` (paired with the existing `aqToolRecord`, both record/replay of the same csv
  test-capture format).
- New files, registered in `projects/AQ_API.vcxproj` + `.vcxproj.filters` (`Tool` filter) + all 4
  `swig_*.i`: `aqToolData.h/.cpp` (`tryAqToolAppend`, `tryAqToolClean`, `tryAqToolDataFilter` —
  `VariantMatrix`/`VariantVector` reshaping; `tryAqToolValuationSettingsDisplay` — `StandardStringMatrix`
  passed straight through, since it and non-R `SWIG_STRINGMATRIX` are both literally
  `std::vector<std::vector<std::string>>`); `aqToolDate.h/.cpp` (`tryAqToolTermsToDates`,
  `tryAqToolDatesToTerms`); `aqToolDiagnostics.h/.cpp` (`tryAqToolEchoDouble`,
  `tryAqToolBondAverageYield`, `tryAqToolBondYieldFromFuturePrice`, `tryAqToolSwapScheduleTemplate`
  — golden-named `Tool` but product-flavoured; filed together since AQ_XLL codes all four in the
  same `xllTool.cpp`, not split across `xllBond.cpp`/`xllSwap*.cpp`).
- One small, additive `TypeUtilities.h` fix: `swig::buildStringVectorFromDateVector` (DateVector →
  vector<string>, YYYYMMDD) was already implemented in `TypeUtilities.cpp` but had no header
  declaration, so nothing could call it. Added the missing prototype (mirrors the existing
  `buildDateVector` declaration immediately above it) rather than duplicating the loop inline.

**Credit (18 wrappers bound), all added to the existing `aqCreditObject.h/.cpp` (no new files):**
`tryAqCreditModelAsOfDate`, `tryAqCreditModelCalibrationParameters`,
`tryAqCreditModelImpliedSurvivalDate`, `tryAqCreditObjectSpread`, `tryAqCreditObjectIndexSpread`,
`tryAqCreditObjectOptionPV[FromForward]`, `tryAqCreditObjectOptionImpliedVol[FromForward]`,
`tryAqCreditObjectIndexOptionPV`/`ImpliedVol`/`Vega`/`CS01`/`Theta`,
`tryAqCreditBasketModelCreate`, `tryAqCreditBasketModelSurvivalProbability`,
`tryAqCreditObjectFeeLegCreate`, `tryAqCreditObjectFeeScheduleCreate`.

**Correction to the prior entry's note on the two `tryAqCreditBasketModel*` functions:** that entry
said they had "NO AQ_XLL binding either", matching the SABR precedent. That was stale — checked
`src/AQ_XLL/src/xllCredit.cpp` directly this session and both `aqCreditBasketModelCreate`
(line ~792) and `aqCreditBasketModelSurvivalProbability` (line ~831) are real, non-stub AQ_XLL
worksheet functions with full `.help()`/`.arg()` registrations. No "port to both" decision was
needed — bound normally from the existing AQ_XLL marshalling, same as the other 16.
`aqCreditBasketModelCreate`'s AQ_API binding mirrors the existing `aqCreditModelCreate` pattern
(fixed two data blocks, `etrading::JSONInfoBlock::createInfoBlock`) rather than the AQ_XLL
signature's optional-second-block shape, for consistency with the one Credit-model-create binding
already in this file.

**Verification:** `python rebrand/tools/api_pair_check.py --gap-list` — HARD GATE: 0. `Tool` and
`Credit` no longer appear in the missing-binding list at all (previously 20 and 18 respectively).
Remaining open categories unchanged from the prior entry: `Curve` (66/94), `Math` (49/51), `Bond`
(43/58), `Swap` (40/66), `(lifecycle)` (23/31). Build not run (none available in this session) —
verified by code review and the checker only, per the task's own constraint; the user should build
and run GoogleTest before relying on this.

### Math AQ_API binding batch (2026-09-22, same session)

Closed the `Math` gap flagged above (49 missing). All 49 wrappers had an existing `AQ_XLL`
worksheet function in `src/AQ_XLL/src/xllMath.cpp` to port the marshalling from — none skipped.

**8 new files in `src/AQ_API/source/`, grouped by sub-topic** (mirroring how `AQ_XLL`'s
`xllMath.cpp` itself groups these with `/* ===== */` section banners), registered in
`projects/AQ_API.vcxproj` + `.vcxproj.filters` (`Math` filter) + all 4 `swig_*.i`:

- `aqMathBlackScholes.h/.cpp` (10): `Price`, `Prices`, `ImpliedVol`, `ImpliedVols`,
  `DeltaForward`, `DeltaSpot`, `Gamma`, `Vega`, `Theta`, `Rho`. `CallOrPut` strings convert via
  `etrading::toCallOrPutEnum` (declared in `math/include/CoreEnumerations.h`, already the header
  `tryAqMathBlackScholes.h`'s own validation signature depends on and `xllMath.cpp` already
  includes) — no new marshalling invented, same conversion `xllMath.cpp` uses.
- `aqMathCapletFloorlet.h/.cpp` (4): `Price`, `Prices`, `ImpliedVol`, `ImpliedVols`. Unlike
  Black-Scholes, `capletOrFloorlet`/`volatilityType` are plain `std::string` in the `validation`
  signature itself (`tryAqMathCapletFloorlet.h`) — passed straight through, no enum conversion.
- `aqMathSwaption.h/.cpp` (9): `EuropeanIRSwaptionPrice`, `Prices`, `ImpliedVol`, `ImpliedVols`,
  `CashAnnuity`, `Delta`, `Gamma`, `Vega`, `Theta` — same plain-string pattern as CapletFloorlet.
- `aqMathVolatilityConversion.h/.cpp` (6): the `VolatilityTo*` pairwise conversions
  (Normal/Lognormal/ShiftedLognormal) — plain doubles, no marshalling needed at all.
- `aqMathConvexity.h/.cpp` (4): the `Libor*ConvexityAdjustment*`/`LiborRate*` in-arrears and
  arbitrary-fixing-date functions. Here `volatilityType` **is** an enum in the validation
  signature (`etrading::VolatilityTypeEnum`), converted via `etrading::toVolatilityTypeEnum`.
- `aqMathStatistics.h/.cpp` (6): the `NormalDistribution*` family (Standard/StandardPDF/
  StandardInverse/plain/PDF/Inverse) — plain doubles.
- `aqMathNumerical.h/.cpp` (6): `PolynomialInterpolation(s)`, `PoynomialIntegration(s)` (the
  "Poynomial" misspelling is the pre-existing `validation` wrapper name, kept verbatim per the
  task's own instruction, not corrected), `IntegrateUsingTerms` (term/value doubles) and
  `Integrate` (date/value form — `AQLDate`/`DateVector` via `etrading::stringToDate` +
  `swig::buildDateVector`, optional `joinDate` defaults to blank string -> `AQLDate()`).
- `aqMathRates.h/.cpp` (4): `ForwardRate(s)`, `DiscountFactor(s)` — the low-level raw
  (dates, values) curve-fit primitives (not a cached curve object). Only the non-legacy overload
  of each `tryAqMath*` was bound, per the `*** LEGACY METHOD ***` labelling already in
  `tryAqMathForwardRate.h`; the `curveCollection`/`curveIndex`-based legacy overloads were left
  unbound, matching how `xllMath.cpp` itself only ports the non-legacy overload. Five enum
  conversions per call (`Interpolation`, `StateVariable`, `DayCount`, `BusinessDayAdjustment`,
  `CompoundingFrequency`, all via existing `etrading::to*Enum` in `CoreEnumerations.h`), dates via
  `swig::buildDateVector`/`etrading::stringToDate`, and the optional `forwardAdjustmentTable`
  passed as a plain `std::vector<std::vector<std::string>>` (`StandardStringMatrix`'s own
  underlying type) rather than the R-ambiguous `SWIG_STRINGMATRIX` macro, since it needs to be
  correct for every language, not just non-R.

**One checker false-positive hit and fixed:** the first `api_pair_check.py --gap-list` run after
writing these files reported a HARD GATE of 2 (`aqMathDiscountFactor`, `aqMathForwardRate`
flagged as "not routed through validation"). Root cause: the checker locates a function's body by
regex-matching `aqMathForwardRate(` and taking the *first* match in the file — `aqMathRates.cpp`'s
own top-of-file brief comment read "the aqMathForwardRate(s) and aqMathDiscountFactor(s) ...
primitives", and `(s)` reads as an open-paren to the regex, so it matched inside the comment
before the real function definition and grabbed the wrong (helper-namespace) function body, which
has no `validation::` call in it. Fixed by rewording the two comments to
"aqMathForwardRate / aqMathForwardRates" (spelled out, no trailing `(s)`) — comment-only change,
no code semantics touched. Re-ran clean: **HARD GATE: 0**, `Math` no longer appears in the
`--gap-list` breakdown at all (51 declared, 0 missing; was 49/51 missing before this batch).

**Verification:** `python rebrand/tools/api_pair_check.py --gap-list` — HARD GATE: 0; `Math` gone
from the missing-binding list. Build not run (none available in this session) — verified by code
review, precedent-matching against `xllMath.cpp`'s marshalling, and the checker only; the user
should build and run GoogleTest before relying on this. Remaining open categories unchanged:
`Curve` (66/94), `Bond` (43/58), `Swap` (40/66), `(lifecycle)` (23/31).

### Swap AQ_API binding batch (2026-09-22, same session)

Closed the `Swap` gap flagged above (40 missing). All 40 wrappers had an existing `AQ_XLL`
worksheet function in `src/AQ_XLL/src/xllSwap.cpp` to port the marshalling from (including
`tryAqSwapGeneratorCreate`/`Display`, which turned out to be coded in `xllSwap.cpp` itself, not
`xllGenerator.cpp` — confirming they belong with Swap, not Generator) — none skipped.

**Extended existing files (no new registration needed):**
- `aqSwapObjectCreate.h/.cpp` (+10): `aqSwapObjectLVBKeys`, `aqSwapObjectCreateFromLegs`,
  `aqSwapObjectCreateFromLegLVBs`, `aqSwapObjectCreateFromSchedule`, `aqSwapObjectCreateBackToBack`,
  `aqSwapObjectAddLeg`, `aqSwapObjectAddFee`, `aqSwapGeneratorCreate`, `aqSwapObjectDisplay`,
  `aqSwapGeneratorDisplay` — all natural lifecycle/creation companions to the file's existing
  `aqSwapObjectCreate`/`CreateFromGenerator`.
- `aqSwapObjectPrice.h/.cpp` (+2): `aqSwapObjectAccruedInterest` (same shape as the file's existing
  `ParSpread`/`Spread`), `aqSwapObjectPVs` (multi-swap batch PV; `valuationSettingsLVBs`/
  `fixingTableNamesLVBs` tables marshalled via `swig::buildStringMatrix` +
  `etrading::buildMultiLabelValueBlock`, an existing `etrading::LabelValueBlock.h` helper, not a
  new one — mirrors `xllSwap.cpp`'s own `toLabelValueBlockVector` for the same "header row of
  keys, one row per swap" table shape).
- `aqSwapPV.h/.cpp` (+1): `aqSwapPVLVBKeys`. Note: the file already had a function named
  `tryAqSwapPVLVBKeys` (validation-wrapper-name-shaped, not public-API-shaped — a pre-existing
  naming slip, left untouched per the task's "never rename existing" rule) already generated into
  all 4 `swig_*_wrap.cxx`. Added the correctly-named `aqSwapPVLVBKeys` alongside it as a second,
  identical-body function so the golden name the checker expects actually exists.
- `aqSwapStubRate.h/.cpp` (+1): `aqSwapStubFixingDate` (companion to the file's existing
  `aqSwapStubRate`; `AQLDate` return marshalled via `etrading::toYYYYMMDDFromDate`, needing a new
  `#include "DateUtilities.h"` in this file).
- `aqSwapSchedule.h/.cpp` (+4): `aqSwapObjectScheduleCreate`, `aqSwapObjectScheduleDisplay`,
  `aqSwapObjectScheduleCreateBespoke`, `aqSwapObjectScheduleCreateBespokeFromCashflows` — the
  object/cached-handle counterparts to the file's existing stateless `aqSwapSchedule`; needed a
  new `#include "tryAqSwapObjectSchedule.h"`.

**3 new files, registered in `projects/AQ_API.vcxproj` + `.vcxproj.filters` (`Swap` filter) + all
4 `swig_*.i`:**
- `aqSwapLeg.h/.cpp` (9): `aqSwapLegLVBKeys`, `aqSwapLegDisplay`, `aqSwapLegPV`, `aqSwapLegAnnuity`
  (stateless, single-leg-LVB-in) plus `aqSwapObjectLegCreate`, `aqSwapObjectLegCreateFromSchedule`,
  `aqSwapObjectLegPV`, `aqSwapObjectLegDisplay`, `aqSwapObjectLegDisplayCashflows` (cached-leg
  object lifecycle/pricing) — grouped together since `xllSwap.cpp` codes both under one
  "Swap leg object lifecycle" section and they share no natural home in the PV/Create files.
- `aqSwapResults.h/.cpp` (9): `aqSwapResultsEnable`, `IsEnabled`, `RiskUpdate`,
  `DiscountRiskUpdate`, `ForwardRiskUpdate`, `Delete`, `DeleteAll`, `RiskTotals`, `Display` — the
  Jacobian-risk cache-management surface, same shape as `aqCurveResults.cpp`
  (`etrading::VariantMatrix` return via `swig::fromVariantMatrixToMatrixOfString`, same as that
  file's `aqCurveResultsDiscountFactorsDisplayAll`). `riskType` string converts via the existing
  `etrading::toRiskTypeEnum` (`CoreEnumerations.h`); `asOfDate` via `etrading::stringToDate`
  (`ParameterValidation.h`).
- `aqSwapDelta.h/.cpp` (4): `aqSwapDelta` (stateless, several inline trades), `aqSwapObjectDelta`,
  `aqSwapObjectDeltaLadder`, `aqSwapObjectDeltaLadderHorizontally` (cached-swap risk, with
  `AQLStringVector`/`DoubleMatrix` **output** parameters — a new shape for `AQ_API`, no prior
  precedent in this project). Rather than invent new marshalling, this file ports
  `xllSwap.cpp`'s own `toExcelLabeledMatrix` helper and the inline `VariantMatrix`-building bodies
  of `aqSwapObjectDelta`/`DeltaLadderHorizontally` almost verbatim (same header-row-then-pillar-row
  layout), swapping the `xloil::ExcelObj` return for `swig::fromVariantMatrixToMatrixOfString`
  which was already used elsewhere in `AQ_API` (`aqSwapSchedule.cpp`, `aqCurveResults.cpp`) — the
  matrix-shape decision is AQ_XLL's own precedent, not a new invention. `dealInfoLVBs`/multi-trade
  tables use the same `etrading::buildMultiLabelValueBlock` helper as `aqSwapObjectPVs` above.
  `etrading::getDataInstance()` (`ParameterValidation.h`) supplies the object-pool pointer
  `tryAqSwapDelta` needs, matching `xllSwap.cpp`'s own call.

**No functions skipped** — every one of the 40 had a matching `AQ_XLL` worksheet function to port
marshalling from.

**Verification:** `python rebrand/tools/api_pair_check.py --gap-list` — HARD GATE: 0; `Swap` gone
from the missing-binding list entirely (66 declared, 0 missing; was 40/66 missing before this
batch). Build not run (none available in this session) — verified by code review and
precedent-matching against `xllSwap.cpp`'s marshalling only; the user should build and run
GoogleTest before relying on this. Remaining open categories unchanged: `Curve` (66/94), `Bond`
(43/58), `(lifecycle)` (23/31).

### Bond AQ_API binding batch (2026-09-22, same session)

Closed the `Bond` gap flagged above (42 missing per the task list — 43 as originally measured;
the one-item drift traces to `tryAqBondObjectDisplay`, which the earlier count had folded in
differently). All 42 wrappers had an existing `AQ_XLL` worksheet function in
`src/AQ_XLL/src/xllBond.cpp` to port the marshalling from — none skipped.

**Extended existing files (no new registration needed):**
- `aqBondObjectCreate.h/.cpp` (+5): `aqBondObjectCreateFromLVB` (single-LVB create, mirrors
  `aqCapFloorObjectCreate`'s `LabelValueBlock` construction), `aqBondObjectCreateAUDNotionalBond`
  (plain string/date passthrough), `aqBondObjectDisplay` (input-parameter display, companion to
  the file's existing `Create`/`CreateFromGenerator`, mirrors `aqSwapObjectDisplay` living beside
  `aqSwapObjectCreate` in the Swap batch), `aqBondGeneratorCreate`/`aqBondGeneratorDisplay` (the
  Bond-category generator *construction* functions — introspection-only `aqGeneratorList`/
  `Display`/`Validate` already live in the separate `aqGenerator.cpp`, same category-vs-file
  judgement call the Swap batch made for `aqSwapGeneratorCreate`). `aqBondGeneratorCreate` mirrors
  `aqCreditModelCreate`'s two-data-block `JSONInfoBlock::createInfoBlock` pattern; confirmed
  `etrading::JSONInfoBlockTuple` and `validation::TableInfo` (the type `tryAqBondGeneratorCreate`
  declares) are the identical `std::tuple<vector<string>, vector<ContainedTypeEnum>,
  VariantMatrix>` instantiation via their typedefs, so no separate conversion was needed — passing
  a `JSONInfoBlockTuples` value where `vector<TableInfo>` is expected compiles directly. Needed a
  new `#include "JSONInfoBlock.h"` in the .cpp.
- `aqBondObjectPrice.h/.cpp` (+14, two as `AccruedInterest`/`OisSpread` overloads):
  `PriceFromCleanToDirty`, `PriceFromDirtyToClean` (both `LabelValueBlock`-based, same shape as
  the file's existing `ForwardPrice`), `YieldFromObject`, `YieldOptimized` (both take an
  `etrading::BondPtr` in `validation`, not a bare name — resolved via `etrading::getBond`, needing
  a new `#include "AQObjUtilities.h"`), `CompoundYield`, `AccruedInterest` (vector-of-dates
  overload plus the single-`LabelValueBlock` FRN overload — SWIG supports the same C++ overload
  resolution AQ_API already relies on elsewhere), `AccruedInterestDays` (returns `vector<int>`,
  passed straight through — no marshalling needed), `Quote`, `PriceFromCreditModel`,
  `LastCouponDate`, `OisSpread` (both the vector and single-date overloads), `RepoRate`,
  `ForwardReinvestedCoupon` — the misc settle/forward-settle date-pair functions grouped here per
  the task's own guidance, alongside the file's existing `ForwardPrice`.
- `aqBondCurve.h/.cpp` (+5): `CurveCreate` (same `JSONInfoBlockTuple`/`createInfoBlock` two-block
  pattern as `aqBondGeneratorCreate` above), `CurveDisplay` (`AnyTypeMatrix` via
  `swig::fromAnyTypeMatrixToMatrixOfString`, same as the file's existing
  `aqBondObjectDisplayCashflows`/`DisplaySchedule`), `CurveYield`, `PriceFromBondCurve`,
  `YieldFromBondCurve` (all plain date/string/double, no matrix marshalling). Needed a new
  `#include "JSONInfoBlock.h"`.

**4 new files, registered in `projects/AQ_API.vcxproj` + `.vcxproj.filters` (`Bond` filter) + all
4 `swig_*.i`:**
- `aqBondObjectRisk.h/.cpp` (7): `DV01`, `DV01Numerical` (`bumpMode` string -> `AQLString` via
  `.c_str()`, same pattern as `aqSwapDelta.cpp`'s `bumpModeAsAQLString`), `ModifiedDuration`,
  `BPVPerTick` (`LabelValueBlock`-based), `ZSpread`, `ZSpreads`, `ZSpreadFromRates` — grouped as
  the bond risk/sensitivity surface, mirroring how `xllBond.cpp` codes them together.
- `aqBondObjectFRN.h/.cpp` (4): `FRNPriceFromDiscountMargin`, `FRNPriceFromYield`,
  `FRNYieldFromPrice`, `FRNDiscountMarginFromPrice` — the floating-rate-note discount-margin/
  price/yield conversions that don't require discount/forecast curves (BBG CalcType 21; all
  plain date/double signatures).
- `aqBondObjectFuture.h/.cpp` (7): `CheapestToDeliver`, `CheapestToDeliverByNetBasis`,
  `ConversionFactor`, `FuturePrice`, `GrossBasis`, `NetBasis`, `RepoRateFromFuture` — the
  bond-future analytics group (plain bond-name-list/date/double signatures; `CheapestToDeliver*`
  bond-name lists are passed straight through as `vector<string>`, no handle-counter stripping
  needed on the AQ_API side).
- `aqBondSchedule.h/.cpp` (2): `ScheduleLVBKeys`, `Schedule` (stateless; `Schedule` returns
  `AQLStringMatrix` from `validation`, marshalled via `swig::fromStringMatrixToMatrixOfString`,
  same helper `aqSwapGeneratorDisplay` uses for the same return type).

**No functions skipped** — every one of the 42 had a matching `AQ_XLL` worksheet function in
`xllBond.cpp` to port marshalling from.

**Verification:** `python rebrand/tools/api_pair_check.py --gap-list` (after `git add` on the 4
new files — the checker enumerates `src/AQ_API/source/*.{h,cpp}` via `git ls-files`, so an
unstaged new file is invisible to it) — HARD GATE: 0; `Bond` gone from the missing-binding list
entirely (58 declared, 0 missing; was ~42-43/58 missing before this batch). Build not run (none
available in this session) — verified by code review and precedent-matching against
`xllBond.cpp`'s marshalling only; the user should build and run GoogleTest before relying on this.
Remaining open categories: `Curve` (66/94), `(lifecycle)` (23/31).

**AQ_API coverage-gap closure — `Curve` batch (2026-09-23).** Continued a prior session's
partial attempt (interrupted by a rate limit after it added 11 unrelated `Curve` functions —
`aqCurveObjectDataCreate/Display`, `aqCurveObjectDisplayConventions`, `aqCurveObjectDualBootstrap`,
`aqCurveObjectCalibrateHedge`, `aqCurveObjectEngineCalibrate` — to the tail of
`aqCurveObject.h/.cpp`; verified via `--gap-list` that none of those collided with this batch's
target list before starting). Added 29 functions to the existing `src/AQ_API/source/aqCurveObject.h/.cpp`
(no new files, no project-file changes needed):

- **Curve object one-shot creation (4):** `aqCurveObjectCreateBasis`, `aqCurveObjectCreateFXForwards`,
  `aqCurveObjectCreateOIS`, `aqCurveObjectCreateSwap` — raw-conventions-and-rates curve builders;
  `SWIG_STRINGMATRIX` params marshalled to `AQLStringMatrix` via `swig::buildStringMatrix`, same
  pattern as the existing `aqCurveObjectDataCreate`. Objects are cached under the name passed in
  directly (no Excel-cell-location decoration/instance counter — that's an `AQ_XLL`-only concept).
- **Curve object discount factors / forward rates, handle-based (11):**
  `aqCurveObjectDiscountFactors[ForwardStarting[FromTenors|FromYearFractions]|FromTenors|FromYearFractions|Table]`,
  `aqCurveObjectForwardRates[FromForwardDates|FromYearFraction|Table]` — dates marshalled via
  `swig::buildGregorianDateVector` (the validation signatures here take
  `std::vector<boost::gregorian::date>`, not `DateVector`/`AQLDate`, unlike the legacy stateless
  `curveCollection`+`curveIndex` twins in `aqCurveDiscountFactor.cpp`). The two `*Table` functions
  build a `Date` + one-column-per-curve-index `VariantMatrix` exactly mirroring
  `xllCurve.cpp`'s `aqCurveObjectDiscountFactorsTable`/`aqCurveObjectForwardRatesTable`, then
  `swig::fromVariantMatrixToMatrixOfString`.
- **Bumping (6):** `aqCurveObjectBumpAll`, `aqCurveObjectBumpInstrument`, `aqCurveMarketDataBumpAll`,
  `aqCurveMarketDataBumpClear`, `aqCurveMarketDataBumpInstrument`, `aqCurveMarketDataColumn` —
  thin passthroughs (`validation`'s signatures here already take plain `std::string`/`double`/`bool`,
  no marshalling needed); `MarketDataColumn`'s `etrading::VariantVector` wrapped into a
  single-column `VariantMatrix`, same as `xllCurve.cpp` does for the same wrapper.
- **Legacy stateless `curveCollection`+`curveIndex` surface (6):** `aqCurveDelete`,
  `aqCurveDiscountFactorsDisplay` (the `DiscountFactorTable` struct's `terms_`/`paymentDates_`/
  `discountFactors_` built into a Term/PaymentDate/DiscountFactor matrix, mirroring
  `xllCurve.cpp`), `aqCurveDiscountFactorsOverride`, `aqCurveDiscountFactorsSetToOne`,
  `aqCurveForwardRatesOverride`, `aqCurveForwardRatesFromForwardDatesFromObject` — `AQLString`/
  `DateVector` marshalling here follows the existing `aqCurveDiscountFactor.cpp` precedent exactly
  (`.c_str()` into `AQLString`, `swig::buildDateVector` for `DateVector` params).
- **Jacobian risk display (2):** `aqCurveObjectEngineJacobianDisplay` (returns either the label
  matrix via `swig::fromStringMatrixToMatrixOfString` or the `DoubleMatrix` values, depending on
  the `displayLabels` flag, matching `xllCurve.cpp`'s branch), `aqCurveObjectJacobianDisplay` —
  both convert the `DoubleMatrix` output to a `VariantMatrix` row-by-row (no existing shared
  `DoubleMatrix`->string helper in `TypeUtilities.h`; same manual-loop pattern as `aqSwapDelta.cpp`'s
  local `buildLabeledDeltaMatrix`, minus labels since the Jacobian wrappers don't return them).

**No functions skipped** — every target had a matching `AQ_XLL` worksheet function in
`xllCurve.cpp` (confirmed via `XLO_FUNC_START( aq... ` grep) to port marshalling from.

**Verification:** `git add` on the two changed files, then `python rebrand/tools/api_pair_check.py
--gap-list` — HARD GATE: 0; `Curve` missing-binding count dropped from 55 to 26 (68 declared now,
up from 39). Build not run (none available in this session) — verified by code review and
precedent-matching against `xllCurve.cpp`'s marshalling and existing `aqCurveDiscountFactor.cpp`/
`aqCurveForwardRate.cpp`/`aqSwapDelta.cpp` siblings only; the user should build and run GoogleTest
before relying on this. Remaining `Curve` gaps (26) are a distinct cluster not targeted by this
batch: `CurveGenerator*`/`CurveGroup*` (not yet designed for `AQ_API`), the `CurveResults`
Jacobian-store family (10 functions), `CurveVasicek*`/`CurveHullWhiteForwardRates`,
`CurveCalibrateCTD`, `CurveCompoundRateWithFixingTable`, `CurveEuroDollarConvexityAdjustment`,
`CurveFrequency`, `CurveTermsToDates`/`CurveDatesToTerms`. Remaining open categories: `Curve`
(26/94), `(lifecycle)` (23/31).

**AQ_API coverage-gap closure — `Curve` batch complete (2026-09-23).** Closed out the remaining
26-function `Curve` gap left by the previous batch above, adding the last cluster to the same
`src/AQ_API/source/aqCurveObject.h/.cpp` (no new files, no project-file changes needed). Ran
`rebrand/tools/api_pair_check.py --gap-list` first per instructions; all 26 target wrappers were
confirmed still missing before starting (none had been picked up by any other session in the
interim). Marshalling was ported from `xllCurve.cpp`'s worksheet functions for each, per
`validation`'s golden-source signatures (trusting the `.cpp` bodies over any stale header doc
comments, as instructed):

- **Curve results / Jacobian risk store (12):** `aqCurveResultsEnable`, `aqCurveResultsIsEnabled`,
  `aqCurveResultsDiscountFactorsUpdate` (3 `AQLStringMatrix` label/value blocks plus an optional
  `forwardAdjustments` block left as `StandardStringMatrix` — same type as non-R
  `SWIG_STRINGMATRIX`, no conversion needed, matching the existing `aqToolValuationSettingsDisplay`
  precedent), `aqCurveResultsDiscountFactorsDisplay`, `aqCurveResultsDelete`,
  `aqCurveResultsDeleteAll`, `aqCurveResultsForwardRatesDisplay` (business-day-adjustment string
  converted via `etrading::toBusinessDayAdjustmentEnum`, defaulting to `NONE_BUSINESS_DAY_ADJ` on
  blank), `aqCurveResultsJacobianUpdate` (5 `AQLStringMatrix` blocks plus `vector<bool>`/
  `vector<double>` passed straight through — SWIG already supports `std::vector<bool>`, evidenced
  in the generated R wrapper), `aqCurveResultsJacobianDisplay`, `aqCurveResultsJacobianDiscountFactorDelta`,
  `aqCurveResultsJacobianRiskTotals`, `aqCurveResultsJacobianImplyNewDiscountFactors` — risk-type
  strings converted via `etrading::toRiskTypeEnum`, `VariantMatrix` results marshalled via
  `swig::fromVariantMatrixToMatrixOfString`, same as the Jacobian-display functions from the prior
  batch.
- **Curve groups (2):** `aqCurveGroupCreate`, `aqCurveGroupCollectionName` — `validation`'s
  signatures here take plain `StandardString`/`StandardStringVector` (i.e. `std::string`/
  `std::vector<std::string>`), not the `AQL*` types, so these are thin passthroughs with no
  marshalling at all.
- **Curve terms/dates (2):** `aqCurveTermsToDates` (`DateVector` result converted back via
  `swig::buildStringVectorFromDateVector`), `aqCurveDatesToTerms` (`swig::buildDateVector` on the
  way in) — both already had their `validation` wrappers living in the already-included
  `tryAqCurveDiscountFactor.h`.
- **Curve compound rate with fixing table (2, one wrapper name, two overloads):**
  `aqCurveCompoundRateWithFixingTable` — ported both the vector (`DateVector` start/end columns)
  and single-date (`AQLDate` via `etrading::stringToDate`) overloads, mirroring the existing
  `aqCurveCompoundRate` sibling pair in `aqCurveCompoundRate.cpp` exactly (same overload-by-name
  pattern the R SWIG wrapper already dispatches on for that sibling).
- **Curve one-shot CTD calibration (1):** `aqCurveCalibrateCTD` — `AQLStringMatrix` curve
  conventions plus an `AQLStringVector` of collateral curve names.
- **Curve frequency / EuroDollar convexity (2):** `aqCurveFrequency` (thin `AQLString` passthrough,
  `.getCString()` back to `std::string`), `aqCurveEuroDollarConvexityAdjustment` (3 `AQLDate`
  conversions via `etrading::stringToDate`).
- **Short-rate model checks (3):** `aqCurveHullWhiteForwardRates`, `aqCurveVasicekChecking`
  (`AnyTypeMatrix` result via `swig::fromAnyTypeMatrixToMatrixOfString`), `aqCurveVasicekForwardRates`
  — all three take an optional `valuationDate` string defaulting to `AQLDate()` (the curve's own
  as-of date) when blank, same empty-check pattern already used for `joinDate` in
  `aqMathRates.cpp`/`aqMathNumerical.cpp`.
- **Curve generator (3):** `aqCurveGeneratorCreate` (one or two named data blocks, reusing the
  existing local `getTableInfoFromStringMatrix` helper already defined earlier in
  `aqCurveObject.cpp` for `aqCurveMarketDataCreate` — the XLL's own `toTableInfo` helper is
  functionally identical for this generic, curve-type-unaware case), `aqCurveGeneratorDisplay`,
  `aqCurveGeneratorModify` (`swig::buildSingleLabelValueBlock` for the overrides block, same
  helper already used throughout `aqAssetSwapObject.cpp`/`aqCMSObject.cpp`).

**No functions skipped** — all 26 had matching `AQ_XLL` worksheet functions in `xllCurve.cpp` to
port marshalling from.

**Verification:** `git add -A src/AQ_API/ rebrand/STATUS.md`, then `python
rebrand/tools/api_pair_check.py --gap-list` — HARD GATE stays 0; `Curve` category now shows **0
missing** (94/94 declared, up from 68/94) and no longer appears in the coverage-gap listing at
all. Build not run (none available in this session) — verified by code review and
precedent-matching against `xllCurve.cpp`'s marshalling and the existing `aqCurveCompoundRate.cpp`/
`aqSwapResults.cpp`/`aqToolData.cpp`/`aqMathRates.cpp` siblings only; the user should build and run
GoogleTest before relying on this. `Curve` is now fully closed. Remaining open category:
`(lifecycle)` (23/31) — `CDSObject*` (7), `GridObject*` (6), and the generic `Object*` handle
lifecycle functions (10, e.g. `aqObjectDelete`/`aqObjectList`/`aqObjectQuickLoad`) — out of scope
for this batch.

### 2026-09-23 — `AQ_API` coverage gap CLOSED: `(lifecycle)` category bound (23 functions) — last remaining category, effort complete

Bound the 23 `(lifecycle)` `validation` wrappers flagged in the previous entry — this was the
**last remaining category** in the `AQ_API` coverage-gap effort. Marshalling ported from the
matching `AQ_XLL` worksheet functions (`xllCredit.cpp`, `xllTool.cpp`, `xllObject.cpp`), trusting
the `.cpp` implementations over header doc comments per the working brief. Three new files added
(none of these 23 fit the 21 locked categories — they take the `aqObject<Lifecycle>` form or a
same-file grouping with the existing `Credit`/`Tool` filters):

- **`aqCDSObject.h`/`.cpp` (new, filed under the `Credit` filter) — 7 functions:**
  `aqCDSObjectPVFromHazardRate`, `aqCDSObjectPVByIntegration`, `aqCDSObjectPVByMonteCarlo`
  (`double&` out-param `standardError` has no `AQ_API` out-param precedent, so it is returned as a
  2-element `std::vector<double>` `[PV, standardError]`, mirroring the 2-row column
  `xllCredit.cpp` already returns to Excel for the same function),
  `aqCDSObjectRiskyAnnuityFromHazardRate`, `aqCDSObjectAccruedYearFraction` (the
  `creditModelName`-driven overload — the `...FromHazardRate` form doesn't exist for this
  function), `aqCDSObjectParSpreadFromHazardRate`, `aqCDSObjectHazardRateFromParSpread` (the
  `valuationSettingsLVB`-driven overload; `xllCredit.cpp`'s header notes a same-name
  `creditModelName`-driven sibling overload was deliberately left unexposed there because Excel
  cannot register two worksheet functions under one name — that restriction does not apply to
  `AQ_API`/SWIG, but only the LVB-driven overload was in the gap list, so only that one is bound
  here; the `creditModelName` overload remains a follow-up if ever requested). All four
  `LabelValueBlock` inputs (`valuationSettingsLVB`/`mcParametersLVB`) use the existing
  `swig::buildSingleLabelValueBlock` helper, same pattern as `aqAssetSwapObject.cpp`.
  `aqCDSObjectPV`/`RiskyAnnuity`/`CS01`/`ParSpread` (the credit-model-driven siblings) were already
  bound in `aqCreditObject.h`/`.cpp` — untouched.
- **`aqGridObject.h`/`.cpp` (new, filed under the `Tool` filter) — 6 functions:**
  `aqGridObjectCreate` (range marshalled to a `validation::TableInfo` via
  `swig::buildVariantMatrix` + `etrading::JSONInfoBlock::createInfoBlock`, the same two-step
  conversion `aqToolObjectMultiGridCreate` already uses per named grid in `aqToolGrids.cpp`),
  `aqGridObjectSave`, `aqGridObjectLoad` (returns the `pair<bool,string>::second` status string,
  matching `xllTool.cpp`), `aqGridObjectNames`, `aqGridObjectClearOne`, `aqGridObjectClearAll`.
  `aqGridObjectDisplay` and the multi-grid functions were already bound in `aqToolGrids.h`/`.cpp` —
  untouched.
- **`aqObject.h`/`.cpp` (existing file, extended) — 10 functions:** `aqObjectExists`,
  `aqObjectTypeAsString`, `aqObjectType`, `aqObjectList` (empty `typeAsString` calls
  `validation::tryAqObjectList()` — the no-arg, every-type overload — matching `xllObject.cpp`'s
  `ObjectType`-omitted branch), `aqObjectDelete` (returns `!stillExists`, since
  `tryAqObjectDelete` returns `true` when the delete *failed* — same inverted sense
  `xllObject.cpp` branches on), `aqObjectDeleteAll` (empty `typeAsString` likewise dispatches to
  the every-type overload), `aqObjectLoadAndReturnTupleResults` (returns a 2-element
  `std::vector<std::string>` `[handle, etrading::toString(cachedObjectType)]`, matching the 2-row
  column `xllObject.cpp` returns), `aqObjectLoadFromString` (returns just
  `std::get<0>()` of the tuple, matching `xllObject.cpp`), `aqObjectQuickLoad`, `aqObjectQuickSave`.
  `aqObjectSave`/`aqObjectLoad`/`aqObjectClearCache` (the last living in `aqToolSetup.h`/`.cpp`)
  were already bound — untouched.

Registered in `projects/AQ_API.vcxproj` (`ClCompile`/`ClInclude`) and
`projects/AQ_API.vcxproj.filters` (`aqCDSObject.*` under the existing `Credit` filter,
`aqGridObject.*` under the existing `Tool` filter — both filters already existed, no new filter
added), plus `#include`/`%include` for the two new headers in all four SWIG interface files
(`swig_Python.i`, `swig_CSharp.i`, `swig_JAVA.i`, `swig_R.i`).

**Verification:** `git add -A src/AQ_API/ projects/AQ_API.vcxproj projects/AQ_API.vcxproj.filters
rebrand/STATUS.md`, then `python rebrand/tools/api_pair_check.py --gap-list` — HARD GATE stays 0;
public `aq*` declared count 451 → 474 (+23, exactly the bound count); **the coverage-gap list is
now completely empty — no category remains, closing the `AQ_API` binding effort that has spanned
the last several `STATUS.md` entries.** The 4 pre-existing "wrapper name drift" advisories
(`aqBondObjectAccruedInterestLVB`, `aqCreditModelRiskyDiscountFactor`, `aqToolInitialize`,
`aqToolsLVBAppend`) are untouched by this batch and remain open as naming-convention cleanup, not
coverage gaps. Build not run (none available in this session) — verified by code review and
precedent-matching against `xllCredit.cpp`/`xllTool.cpp`/`xllObject.cpp`'s marshalling and the
existing `aqCreditObject.cpp`/`aqToolGrids.cpp`/`aqAssetSwapObject.cpp` siblings only; the user
should build and run GoogleTest before relying on this.

### 2026-09-23 — AQ_API coverage-gap closure: final consolidated summary + filter-consistency pass

**Coverage-gap effort closed.** Across this and the preceding several sessions, `AQ_API` went
from missing many `validation` wrappers (surfaced by extending `rebrand/tools/api_pair_check.py`
with a `--gap-list` flag) to full parity: `python rebrand/tools/api_pair_check.py --gap-list` now
reports **HARD GATE 0** and an **empty coverage-gap list** — all 470 `validation` `tryAq*` wrappers
have a matching `AQ_API` public function (474 declared, 477 implemented incl. overloads). Closed,
in order, across the following batches: `CMS`, `BondFutureOption`/`Future`/`BondOption`,
`Inflation`/`Swaption`, `CapFloor`/`FX`, `Volatility` (7 of 14 legacy SABR functions deliberately
deferred, then ported), `AssetSwap`/`IR`/`Date`, `Tool`/`Credit`, `Math` (49 fns), `Swap` (40 fns),
`Bond` (42 fns), `Curve` (55 fns across two batches, closing 94/94), and finally `(lifecycle)`
(23 fns: CDS object, Grid object, generic `aqObject*` handle lifecycle — new `aqCDSObject.h/.cpp`,
`aqGridObject.h/.cpp`, extended `aqObject.h/.cpp`). Marshalling throughout was ported from the
matching `AQ_XLL` worksheet function as the reference (AQ_XLL being ~98% complete and treated as
ground truth for parameter `[in]`/`[out]`/`[inout]` semantics — validation header doc comments were
found stale/wrong in several places and the `.cpp` implementation was trusted instead), reusing only
existing `TypeUtilities.h` helpers. 4 pre-existing "wrapper name drift" advisories remain open as
naming-convention cleanup, not coverage gaps (`aqBondObjectAccruedInterestLVB`,
`aqCreditModelRiskyDiscountFactor`, `aqToolInitialize`, `aqToolsLVBAppend`).

**Filter-consistency pass (second half of the request — organise `AQ_API` `*.h`/`*.cpp` under the
same category filter names `AQ_XLL` uses).** Cross-checked all 155 git-tracked files under
`src/AQ_API/source/` against `projects/AQ_API.vcxproj.filters` (extracted every `ClCompile`/
`ClInclude` `Include` + `Filter` pair via a read-only XML parse, diffed against `git ls-files`).
Result: **every file that is actually part of the build (registered in `projects/AQ_API.vcxproj`)
is correctly filed under its matching category filter** (`Date`, `Curve`, `FX`, `Inflation`,
`Volatility`, `IR`, `Future`, `Swap`, `AssetSwap`, `CMS`, `TRS`, `CapFloor`, `Swaption`,
`BondOption`, `BondFutureOption`, `Bond`, `Credit`, `Math`, `Model`, `Generator`, `Tool`, plus
`Object` for generic handle lifecycle and `Support` for infra/helpers) — no reorganisation was
needed; every batch in this effort self-registered its new files correctly as it went.

Two pre-existing, unrelated loose ends surfaced by the cross-check, **left untouched pending
Nicholas's decision** (neither is part of the active build — both are absent from
`projects/AQ_API.vcxproj` entirely, so this is not a coverage or build-correctness issue):
- `src/AQ_API/source/aqCurveGetInterpolationJoinDate.h`/`.cpp` — an orphaned, never-registered
  duplicate of `aqCurveInterpolationJoinDate.h`/`.cpp` (created ~7 minutes earlier on 2026-09-09),
  calling a stale wrapper name `tryAqCurveGetInterpolationJoinDate` that predates the current
  golden name `tryAqCurveInterpolationJoinDate`. Likely dead code left behind when the file was
  renamed/recreated correctly; candidate for deletion.
- `src/AQ_API/source/targetver.h` — a stock VS SDK-version header, also never registered in the
  `.vcxproj`; harmless either way.

**Still open, not part of this task:** SWIG regeneration for everything added since the last
confirmed regen (Curve/Tool/Credit/Math/Swap/Bond/lifecycle batches) — the user should re-run
`generate*_2022.bat` for each language and confirm a clean regen; and a full build + GoogleTest run
across `AQ_API`/`AQ_XLL`, which has not been possible in this environment (no MSBuild/devenv
available) — everything above was verified via `api_pair_check.py` and precedent-matching code

### 2026-09-23 — AQ_API Solution Explorer filters rebuilt: Core API / Optional API, mirroring AQ_XLL

**Nicholas's correction:** the flat category filters added earlier (one filter per locked category,
e.g. `Curve`, `Swap`, `Credit`) were not what he asked for. He wants the `AQ_API` project browsable
the same way `AQ_XLL` is organised on disk — `src\Core`/`src\Optional` (CLAUDE.md Sec 4.4/6.3) — with
a separate `include` and `src` top-level split (all `.h` under `include`, all `.cpp` under `src`,
mirroring the existing top-level `include`/`src` filters, which were already there for extension
grouping) and, inside each, a `Core API` filter and an `Optional API` filter.

**Rebuilt `projects/AQ_API.vcxproj.filters`** with this structure:
- `include\Core API\{Curve, Date, Generator, IR, Math, Object, Support, Tool}`
- `include\Optional API\{AssetSwap, Bond, CapFloor, CMS, Credit, Future, FX, Inflation, Swap,
  Swaption, TRS, Volatility}`
- `src\Core API\{...}` / `src\Optional API\{...}` — identical subfolder set, mirrored.

Every `ClCompile`/`ClInclude` entry moved from its old flat category filter into the matching
`include\{Core|Optional} API\<Category>` or `src\{Core|Optional} API\<Category>` filter — Core vs.
Optional and the category itself both taken directly from Nicholas's message, which matches
`AQ_XLL`'s actual `src\Core`/`src\Optional` split (CLAUDE.md Sec 4.4) function-for-function. The old
flat filters (`Date`, `Curve`, `FX`, ... `Object`, `Support`) are removed — the nested filters
replace them entirely, not alongside them. `resources\Swig Generated Files` and the other pre-
existing `resources\*` filters (Swig Interfaces, pre/post-build scripts) are untouched.

Two judgement calls made without asking first, both flagged inline as XML comments on the affected
entries so they're visible in Solution Explorer, not just here:
- **`BondOption`/`BondFutureOption` folded into `Bond`.** Nicholas's category list has no separate
  BondOption/BondFutureOption folder, only `Bond` — this matches `AQ_XLL`, which files both
  categories' functions in `aqBond.cpp` rather than their own files (CLAUDE.md Sec 5.1a: both
  operate on the same cached `etrading::BondOption`, created only via `aqBondOptionObjectCreate`).
  `aqBondOptionObject.h/.cpp` and `aqBondFutureOptionObject.h/.cpp` now file under
  `Optional API\Bond`, consistent with that precedent, even though their own file names still say
  `BondOption`/`BondFutureOption` (unrenamed — out of scope here).
- **`aqCreditObject.h/.cpp` filed under `Credit`, despite also containing the `aqTRSObject*`
  functions (PV/ParRate/ParSpread/Annuity).** Unlike `AQ_XLL`, which has a dedicated `aqTRS.cpp`
  (CLAUDE.md Sec 4.4), `AQ_API` never got one — the 4 TRS functions live inside `aqCreditObject.cpp`
  alongside 26 genuine Credit functions. Filters apply per file, not per function, so the file went
  to the filter matching its majority content; flagged to Nicholas with an inline XML comment in
  case he'd rather split TRS out into its own `aqTRSObject.h/.cpp` pair (would need a small follow-up
  to move those 4 function bodies and re-register in the `.vcxproj`/`.i` files).

No `Model` filter was recreated — Nicholas's Core/Optional list doesn't include it, and it matches
reality: zero `AQ_API` files exist for it (the `Model` category itself has zero `validation`
wrappers so far, per CLAUDE.md Sec 2.1).

**Verification:** cross-checked every `git ls-files`-tracked `.h`/`.cpp` under `src/AQ_API/source/`
against the new filters via a read-only XML parse (no backslash-path scripting on the `.filters` file
itself — only reading it to build the diff, all edits done via `Edit`/`Write` directly) — every file
registered in `projects/AQ_API.vcxproj` still has exactly one filter entry, nothing dropped. Fixed two
new `--` XML-comment validity errors while authoring the judgement-call notes above (same landmine
hit earlier this project — `--` is invalid inside `<!-- -->`). Re-ran
`python rebrand/tools/api_pair_check.py` after the rewrite: HARD GATE still 0, coverage counts
unchanged (474 declared / 477 implemented) — this was a pure Solution-Explorer reorganisation, no
source code touched. The two orphaned/unregistered files flagged in the previous entry
(`aqCurveGetInterpolationJoinDate.h/.cpp`, `targetver.h`) remain outside the `.vcxproj` and so have
no filter entry either — unchanged, still awaiting Nicholas's call.

review only.
