# AlgoQuantLib — Migration & Rebrand Plan

Rebrand the VS2017 client library (`..\..\.APPLES\APPLE`) into a modern,
vendor-neutral **AlgoQuantLib** product in `LIBRARY_UPDATE\AQ_LIB`, port the
Excel add-in from Planatech XLL+ to xlOil, and leave a tree clean enough to lift
into a fresh Git repo.

Status legend: ☐ not started · ◐ in progress · ☑ done

---

## Context and goals

- The library was built for one client; several clients now want it, so it is
  being rebranded into Nicholas Burgess / AlgoQuantHub's own name and shipped as
  a commercial product with full source.
- `LIBRARY_UPDATE\AQ_LIB` is **authoritative**. `.ALGO_QUANT_LIB` is a manual
  backup — read-only, never a build target. `.APPLES` is the read-only port
  source.
- Current Git repo is a temporary stash. When the rebrand is green, the tree is
  copied to a new folder and committed to a **new** repo for a clean base with no
  client-name pollution; the stash repo is then deleted.
- Toolchain migration (VS2022 / v143 / C++17 / static runtime / xlOil wired) is
  **done**. The code-level rebrand and the XLL port are **not started**.

### Decisions taken

| # | Decision |
|---|---|
| D1 | `.ALGO_QUANT_LIB` is a backup — leave untouched. |
| D2 | `msc*` functions (structured credit, other client) — **delete**, with all downstream references. |
| D3 | `AQ_BINDINGS` → **`AQ_API`** — **done**. Project files, `.sln`, folder, SWIG `.i`, and all 8 `generate*`/`deploy*` batch files + pre/post-build commands all renamed. |
| D4 | The LWO handle/object framework stays — port as-is, rebrand `LWO→AQObj`, keep the cell-location counter/hash behaviour. Do **not** adopt xlOil's cache. |
| D5 | Calendar holiday-centre delimiter `:` → **`+`**. |
| D6 | Function-signature **categories** across validation / XLL / API / tests to be reviewed and standardised — professional, clear, concise. (Phase 2 — §2.2 is the table to review.) |
| D7 | `readme.md` to be rebranded and made current. (Phase 6.) |
| D8 | Env vars set once via `SetEnvironmentVariables.bat`, then restart VS. |
| D9 | **Q1 — Excel names: clean break.** Migrate every `me*` spelling (including any currently-hidden `me*` aliases) to `aq*`. **No** backward-compatibility aliases; old names simply cease to exist. Removed names are listed in the release notes. |
| D10 | **Q2 — Licence: proprietary, no redistribution, as a drop-in placeholder.** A short interim `LICENSE` (proprietary notice + "AS IS" + "no redistribution"), written so the final EULA text replaces `LICENSE` in full later **without touching any source file**. The contractor/client IP-provenance check with a solicitor is still advised before commercial release but does not block the rebrand. |
| D10.1 | **Q2.1 — Banners are minimal and technical-only (supersedes the earlier frozen per-file header).** No file or console banner carries copyright text, a company/person name (`AlgoQuantHub`, `Nicholas Burgess`, `Claude`, `Anthropic`, any client/contractor), or a reference to the old/new library name, the rebrand, or the migration effort. Banners are restricted to technical help (what the code does, params, units, preconditions); citations to external research sources are allowed. Copyright/licence text lives only in `LICENSE`. See `CLAUDE.md` §7.1. |
| D11 | **Q3 — `AQ_API` confirmed.** |
| D12 | **Q4 — holiday-centre join is `+` only** (Nicholas, this session). Reason: clean break (D9), fresh clients, no legacy user sheets to protect; the only `:`-form data that ships is the ~103 generator JSON, which we migrate ourselves; and `:` is heavily overloaded (`DATA_COLL_DEL`, curve-name lists in the same generator files). The `splitCalendarCentres()` helper is written so accepting `:` again is a **one-line toggle** if field feedback ever demands it — but it ships `+`-only. Migrate the 103 JSON calendar fields to `+`. |
| D13 | **Q5 — Linux / CMake build is in scope**, lower priority (late phase). **End-state gate: not one file anywhere in the tree — source, Makefiles, `make.*`, CMake, `.sln`/`.vcxproj`, scripts, resources, examples, docs — may contain a legacy client name or an old prefix.** Many `resources\` and `examples\` items will be rewritten or removed for the final version. |
| D14 | **Q6 — category taxonomy locked (21, singular).** See §2.2 / `CLAUDE.md` §5.1. (Was 22 briefly with `Ois` as its own category; folded into `Swap` as a product variant 2026-09-11 — §2.7.) |
| D15 | **LWO → `AQObj`** for the C++ object-framework **classes** (`AQObjCurve`, `AQObjUtilities`, …); screaming-snake macros take `AQOBJ_`; free predicate `isLWOObject → isAQObject`. Public **function** names carrying `LWO` become **`aq<Category>Object<Fn>`** (`aqSwapObjectPV`, `aqBondObjectDirtyPrice`); named sub-objects skip the `Object` word (`aqBondCurveYield`); lifecycle ops drop the category (`aqObjectLoad`, `aqObjectSave`). Supersedes the earlier `aqObj<Category>` / `aqObjects*` schemes — see Phase 3.2, step 8 and step 11. |
| D17 | **`LA` → `AQL`** (not `AQ`). `LA` = "Legacy Analytics" — the whole `LA*` tree is legacy-to-deprecate; the `AQL` ("AQ Legacy") prefix keeps it visually distinct and greppable against new `AQ*` code. Applies to identifiers, files (`LAString.h → AQLString.h`), include-guard macros, error-string text. `MA`/`MB` → `AQ`, confirmed per project. `LAObject → AQLObject`, `LAMath → AQLMath`. |
| D16 | **Navigation:** category names are a public-API concern and are **not** propagated into `etrading`/`math` file or class names (those stay domain-oriented). The bridge is the `validation` layer: every wrapper is the identical public name plus a `try` prefix, foldered by category (Phase 3.5), plus a live `docs\api_map.csv` (Phase 3.6). Judged acceptable — see §"Navigation" note below §2.5. |
| D18 | **Category scheme (step 11):** categories are **singular** (`aqDate`, not `aqDates`); the handle marker is the word **`Object`** after the category (`aqBondObjectDirtyPrice`), a named sub-object skips it (`aqBondCurveYield`), generic lifecycle is `aqObject<Lifecycle>`. `Vols → Volatility`. `AQ_XLL` category files are `aq<Category>.{cpp,h}` (`aqBond.cpp`, `aqDate.cpp`, `aqObject.cpp`, `aqTool.cpp`; `aqXllTools` is the XLL-layer utility, not a category) — **default, not absolute: `BondOption`/`BondFutureOption` are code-organized together in `aqBond.cpp`** (2026-09-11, `CLAUDE.md` §5.1a) because `aqBondOptionObjectCreate` is the only creator for both; consolidate a category's file into a sibling's only when they share the same underlying cached object, not merely a name prefix. `validation`, `AQ_API`, `GTEST` **test names** and `resources\test` fixtures follow — task 2.6. |
| D19 | **The `validation` wrapper name is the GOLDEN SOURCE** for every public function name. Wrapper = `try` + `<GoldenName>`; the XLL function, every binding method (Python / C# / Java / R) and the `GTEST` case name are `<GoldenName>` verbatim. Rename the wrapper first; the other surfaces follow. Enforced by `docs\api_map.csv` + `api_pair_check.py`. |
| D20 | **Editions are `AQ_XLL`-only (Nicholas, 2026-09-15).** Phase 4a closed via `Release`/`Release_XL_Bond`/`Release_XL_Swap`/`Release_XL_Credit`/`Release_XL_Curve` build configurations, gated at compile time. The `AQ_API` runtime edition-gate (`config\editions.json`/`licence.json`, a module-import registration gate, `aqToolEdition()`) is **dropped, not deferred** — `AQ_API` ships one full binary per language with every category always registered. |
| D21 | **C#/Java/R binding testing shelved permanently, not just blocked (Nicholas, 2026-09-15).** Reason: no current requirement to use these languages, and no test environment available to Nicholas (.NET/JDK/R toolchains not installed). Not a Phase 5 exit criterion any more — Phase 5 can close on Python alone. Demoted to a standing nice-to-have, revisited only if/when a client or use case actually requires one of these languages; not scheduled, not tracked against any phase gate. Python remains the one verified, supported binding. |

Open questions: none blocking. Phase 4 waits on the xlOil worked examples;
Phase 6.0 (Credit untangle) needs Nicholas's domain call on `CreditResults`.

### Pulled forward (done ahead of sequence, at Nicholas's request)

Commits on top of the `baseline` tag — `git reset --hard baseline` reverts all:

| Commit | What |
|---|---|
| `6c262aa` | Planning docs + Phase 0 inventories + `LICENSE`/`THIRD_PARTY_LICENSES.md`; `Mizuho International`/`MHI`/`MHSC` → `AlgoQuantHub` in `src` comments (1359 files); `etrading.nuspec` rebranded. |
| `50cad24` | Removed 733 contractor file-header blocks (`@Author`/`@Department`/`ISD Front Office Development` + copyright) from `src/**/*.{cpp,h}`; re-encoded 90 Shift-JIS files to UTF-8; stripped ~4,800 Japanese chars from comments; 129 full-width punctuation → ASCII. **Two string-literal fixes — verify on first build:** `LAPriceSZCalibration.cpp:330` `"LEVENBERG-MARQUARDT_METHOD"` (was a full-width minus → the branch never matched; now it can — behavioural), and `LAMathCorrelation.cpp:1102` error text (full-width space → space). |
| `767aa4e` | Neutralised hard-coded client config paths in `LACurveProperties.cpp` / `FolderConfig.cpp` (`M:\…5858_mlibir\…`, `S:\mizuho\…`, `D:\appl\…` → `.\config\…`). |

**None of the above touched a code identifier** (comments/encoding/dead-path
strings only, plus the two flagged lines). The systematic `MLIB*`/`me*`/`LA*`
identifier rename is still Phase 1–3 and still needs the baseline build + diff.

---

## Phase 0 — Baseline & safety net  ☐

Nothing else starts until this is green.

- ☐ **0.1** Confirm the authoritative tree builds **all 12 configurations**
  (`Debug|Release|DebugEditAndContinue|ReleaseProfiler` + the per-language
  `*CSharp|*Java|*Python|*R` configs), x64. Record any config that does not, and
  why.
- ☐ **0.2** Confirm `AQ_EXTERNAL_LIB_PATH` resolves to a real external-libs tree
  from `LIBRARY_UPDATE\AQ_LIB` (the setup script's derived default assumes a
  sibling folder that is not there — override at the prompt if needed, and record
  the path used).
- ☐ **0.3** **Capture the pre-rebrand test baseline.** Build current `validation`
  with input/output recording on; run the full existing exercise set (bindings
  smoke tests, `GTEST`, the `resources\test` batch and Excel tests); save
  the recordings as `baseline/` outside the build tree. This is the diff target
  for every later stage. *(Nicholas — needs a build; see `rebrand\00_phase0_status.md`.)*
- ☐ **0.4** Git: commit current state, tag `baseline`. Adopt branch-per-stage +
  tag-before-stage for everything below.
- ☑ **0.5 `mir` call-graph** — `rebrand\callgraph_mir.md`. Result: `mir*` is a
  self-contained legacy binding stack (`AQ_API\mir*` → `validation\tryMir*` →
  core). **No inbound calls from `me`/`aq` code** (the `tryMe*`→`mir*` hits are
  stale doc-comment headers only). Disposition: **remove wholesale** — 58
  `AQ_API\mir*` files, ~35 `validation\tryMir*` files, `mir*` entries in the 4
  `swig_*.i` + `swig_R_wrap.cpp`, and 156 `mir*` functions in the XLL
  `LAXL.cpp` (never ported → simply not carried).
- ☑ **0.6 Removal map** — `rebrand\removal_map_client_specific.md`. Covers
  `msc*` (6 XLL files), `LoanCalculations`, `SupervisoryRules`, `CashflowClient`
  and the securitisation cluster in `etrading` they depend on
  (`CashflowEngine/Model`, `LoanPortfolio`, `SyntheticExcessSpread`, `Trigger`,
  `Tranche`, `PremiumLeg/Schedule`?, `CreditResults*`?). Ambiguous items
  (PremiumLeg / CreditResults — possibly shared with CDS `Credit`) are flagged
  for review, **not** auto-deleted.
- ☑ **0.7 653-function XLL inventory** — `rebrand\xll_function_inventory.csv`
  (653 rows: `old_name, old_file, name_stub, validation_wrapper_called,
  proposed_new_category, keep_drop_merge, notes`). 401 already have a `try*`
  wrapper; 252 do not (154 of those are `mir*` in `LAXL.cpp` → drop). Category
  and keep/drop columns are for the Phase 2.3 pass.
- ☑ **0.8 Calendar-centre delimiter** — `rebrand\calendar_delimiter_sites.md`.
  The token is `:` via `LAString::toToken(':')` on `CURVEINPUT_CALENDAR` /
  `CURVEINPUT_BASISCALENDAR` (8 sites in `etrading\LACurveCalibrationHelpers.cpp`)
  and a `LAPriceDataCalendar` set. **It ships in generator JSON** —
  `SWAP_GENERATOR` values like `"SYB:LNB"`. It is **distinct** from the global
  `MULTI_STATIC_DATA_DELIMITER=':'` in `calibration\LADefinitions.h` (leave that
  alone). Collision risk with `:` inside calendar strings: none found so far →
  D12 "accept both" is viable; confirm during 3.4.
- ☑ **0.9** Interim `LICENSE` + `THIRD_PARTY_LICENSES.md` written at the library
  root (placeholder content; filenames + the header wording that references them
  are final).
- ☑ **0.10 Config & generator inventory** — `rebrand\inventory_config_generators.md`.
  `resources\config` holds **269 generator JSON** (132 SWAP, ~113 CURVE, 24
  BOND) + `.conf` registries, plus holiday data (`Calendar.csv/.conf`,
  `CBSchedule.csv`) and `.properties`. Format: `CACHED_OBJECT_TYPE` +
  `STRUCTURED_KEYS` column blocks (the AQObj serialisation format). Legacy
  artefacts to remove: a bundled `MLIBQ_ADDIN.xll`, `ir.properties`,
  `irsvr_excel.conf`. Python deploy already copies `resources\config`; XLL does
  not yet.

**Exit:** all 12 configs build, baseline recordings saved, tag `baseline`,
inventories 0.5–0.10 reviewed and signed off (0.5–0.10 drafted — awaiting
Nicholas's review), `LICENSE` placeholder in place (done).

---

## Phase 1 — Structural rebrand: projects, folders, macros, scripts  ☐

Mechanical, low-risk-per-step, high blast radius. Build + baseline-diff after
each numbered item.

- ☑ **1.1 Macros** `MLIB_* → AQ_*` — commit `927a66d`. 380 files, 4867
  replacements, word-boundary anchored, generated wrappers excluded. Awaiting
  Nicholas's build + GoogleTest diff vs `baseline` (expect numerically identical).
- ☑ **1.2 `AQ_BINDINGS` → `AQ_API`** (D3) — commit `988a3c1`. Folder + 3 project
  files (`git mv`, 168 src files), `.sln`, `<ProjectName>`/`<RootNamespace>`, 8
  batch files + their vars, pre/post-build commands. All vcxproj file refs verified
  on disk. Awaiting Nicholas's rebuild of the `*Python/*CSharp/*Java/*R` configs
  + generate/deploy. Original detail:
  - `projects\AQ_BINDINGS.vcxproj` (+ `.filters`, `.user`) → rename file,
    `<ProjectName>`, `<RootNamespace>`.
  - `AlgoQuantLib-VS22.sln` — project name and path (GUID unchanged).
  - `src\AQ_BINDINGS\` → `src\AQ_API\`.
  - `src\AQ_API\source\{generate,deploy}{Python,CSharp,Java,R}_2022.bat` — the
    hard-coded `...\targets\%1\%2\AQ_BINDINGS` path segment, the
    `src\AQ_BINDINGS\source` path, and the `AQ_BINDINGS_TARGET` /
    `AQ_BINDINGS_OUT` variable names.
  - `.vcxproj` `<PreBuildEvent>` / `<PostBuildEvent>` `<Command>` lines
    (every config) that call those scripts.
  - `AdditionalIncludeDirectories` entries pointing at the old folder.
  - SWIG `.i` files if they reference the path.
  - `Visualizer.natvis` if it names the project.
  - Rebuild every `*Python/*CSharp/*Java/*R` config; regenerate and redeploy.
- ◐ **1.3 File/folder renames** — commit `1377330`. Done: `InitializeMLibETrading.*`
  → `InitializeAQETrading.*`, `InitializeMLibGoogleTest.*` → `InitializeAQGoogleTest.*`
  (class + 124 includes + `etrading`/`GTEST` vcxproj), `mlib_ReadMe.txt`
  deleted, `getMLIBQEnvironmentVariable*` → `getGoogleTestInputPath` /
  `getConfigFolderPath`, `MLib`/`MLIBQ` product refs in comments/messages →
  `AlgoQuantLib`. **Deferred:** `tryMe*` (206) and `me*` (78) file renames — folded
  into Phase 3 (the `me→aq` identifier rename) to avoid churning `#include`s
  twice; `mir*` files go in 1.4. Awaiting Nicholas's build.
- ✗ **1.4 Remove `mir*`** — **DEFERRED, not a Phase 1 batch.** The revised 0.5
  call-graph (`rebrand\callgraph_mir.md`) found **49 `GTEST` files** and
  `etrading\Replay.cpp` depend on `tryMir*` (22 distinct functions). This is the
  flagship curve/trade regression suite — deleting `mir` means porting those
  tests to the `aq` equivalents and re-baselining each. Moved to **Phase 3c**
  (after the `me→aq` rename gives the target names).
- ✗ **1.4a Remove client-specific credit modules** — **DEFERRED.** The revised
  0.6 map found the securitisation cluster (`CashflowEngine/Model`,
  `LoanPortfolio/Calculations/Validation`, `SyntheticExcessSpread`, `Trigger`,
  `Tranche`) is **entangled with `CreditResults`** (on the KEEP list):
  `CreditResults.h` includes six of these headers and holds a
  `shared_ptr<Trigger>`. Needs a "what is the `Credit` category" design pass
  first. Moved to **Phase 6.0** (Credit untangle). `SupervisoryRules` /
  `CashflowClient` / `LoanCalculations` `validation` wrappers go with it.
- ☐ **1.5 Delete `msc*`** (D2) — the 6 `msc*` add-in files live only in
  `.APPLES\...\MLIBQ_ADDIN`; the new-tree `AQ_XLL` is a POC that never had them.
  "Deletion" = **do not port them** in Phase 4. Nothing to remove now. Any
  `msc*` GoogleTest / resource references are handled in Phase 6.0 with the
  credit untangle.
- ☑ **1.6 Namespaces** `validation_api → validation` — commit `cdceb61`. 524
  files, 2067 refs, word-boundary; no collision; wrap/.i files unaffected.
  Awaiting Nicholas's build + GoogleTest diff.
- ◐ **1.7 Config path resolution.** Client absolute paths in
  `LACurveProperties.cpp` / `FolderConfig.cpp` already neutralised → `.\config\…`
  (commit `767aa4e`). **Still to do, needs a build:** the `MLIBQ` env-var name
  (read via `getenv("MLIBQ")` / `toPath("MLIBQ", …)` in both files, and named in
  `exposed_functions.cpp` error text) — rename to `AQ` or a dedicated
  `AQ_CONFIG`, and reconcile the two path conventions (`\resource\config\…` in
  `FolderConfig` vs `\config\…` / bare filename elsewhere). Also delete the
  stale `resources\config\MLIBQ_ADDIN.xll`. `LACurveProperties` and
  `FolderConfig` are near-duplicates — consider collapsing to one.
- ☑ **1.8 `.clang-format`** — commit (see below). `.clang-format` at `AQ_LIB`
  root, tuned to observed style: Allman braces, `UseTab: ForIndentation` /
  width 4 (tabs were the plurality — 59k vs 33k 4-space lines), `SpacesInParentheses`
  (`foo( arg )`, ~3:1 in the code), `PointerAlignment: Left`,
  **`ColumnLimit: 0`** and `SortIncludes: false` so it only normalises
  whitespace/braces and never rewraps — keeps the Phase 6.8 bulk-reformat diff
  small. `resources\.clang-format` with `DisableFormat: true` exempts examples /
  toolkits / the vendored AStyle tree. **No reformat run** — clang-format is a
  guide for edited files until Phase 6.8. `resources\utilities\ArtisticStyle`
  (AStyle, not wired to anything) is removed in the Phase 6.4 resources audit.

**Exit:** solution builds all configs; `grep -r "MLIB\|Mizuho\|validation_api\|msc"`
in `src\` is empty except third-party and string literals; baseline-diff clean.

---

## Phase 2 — Category taxonomy review  ☐  (D6, D14)

A design step, not a code step. Produce, circulate, agree — *then* Phases 3–5
apply it. **§2.2 below is the table Nicholas asked to review.**

- ☐ **2.1** Inventory every category token in use today:
  - XLL POC: `aqDate`, `aqTool`, `aqMath`.
  - CLAUDE.md target list: `AQ_dates`, `AQ_tools`, `AQ_curves`, `AQ_swaps`,
    `AQ_products`, `AQ_models`.
  - Old add-in de-facto categories (by file): `Dates`, `Curves`, `Swap`,
    `Products`, `Vols`, `Math`, `Utilities`, `LWO*`, `LoanCalculations`,
    `SupervisoryRules`, `CashflowClient`.
  - Old bindings: `Curve`, `Date`, `LWO`, `Product`, `Utility`, plus `mir*`.
  - `validation` wrappers: `tryMe` + `<Category>` + `<Func>`.
  - `GTEST`: test-suite naming.
- ☑ **2.2 Canonical category list — LOCKED (Nicholas).** **20** categories,
  **SINGULAR**, applied identically in `validation`, `AQ_XLL`, `AQ_API` and
  `GTEST` (test names included). Revised at step 9 (Options umbrella removed,
  sub-types promoted) and again at step 11 (plural → singular; the handle marker
  moved from an `aqObj` prefix to the word `Object` after the category):

  | Category | Covers | Example |
  |---|---|---|
  | `Date` | calendars, schedules, IMM / central-bank dates, year fractions | `aqDateYearFraction` |
  | `Curve` | **rates yield-curve framework only** — build, calibration, interpolation, DF / zero / forward queries, cross-currency. *Not* bond or credit curves. | `aqCurveObjectForwardRate` |
  | `Volatility` | vol surfaces, surface SABR calibration, cap / swaption vol utilities | `aqVolatilitySabrImplied` |
  | `IR` (was `InterestRate`, renamed 2026-09-12) | Fixing tables, FRAs, rate-future <-> FRA conversion | `aqIRFixingTableValues` |
  | `Future` | futures ticker / date-code helpers, futures conventions | `aqFutureTickerNext` |
  | `Swap` | vanilla / OIS / basis / cross-currency — creation, legs, schedules, pricing, risk. **OIS is a product variant here, not its own category** — `aqSwapOis<Function>` (§2.2 note below); Asset / CMS / total-return swaps *do* have their own categories. | `aqSwapObjectParRate`, `aqSwapOisParRate` |
  | `Bond` | bond creation, price / yield, repo / basis / CTD, **bond-curve fitting** (govie / spread curves) | `aqBondObjectYield`, `aqBondCurveYield` |
  | `Credit` | CDS, **hazard-rate / survival (credit) curves**, par spreads | `aqCreditParSpread` |
  | `CapFloor` | caps / floors — creation, PV, greeks (incl. analytical) | `aqCapFloorObjectPV` |
  | `Swaption` | swaptions — PV, delta / gamma / theta / vega, implied vol | `aqSwaptionObjectPV` |
  | `BondOption` | options on bonds — PV, greeks | `aqBondOptionObjectPV` |
  | `BondFutureOption` | options on bond futures — PV, greeks | `aqBondFutureOptionObjectPV` |
  | `AssetSwap` | asset-swap spread, par-par, fixed-equivalent coupon | `aqAssetSwapObjectSpread` |
  | `CMS` | CMS pricing with convexity adjustment | `aqCMSObjectPVUsingConvexityAdjustment` |
  | `TRS` | Total return swap — PV, annuity, par rate / spread | `aqTRSObjectPV` |
  | `Inflation` | inflation curve build, CPI, zero-coupon inflation swaps | `aqInflationObjectZCSwapPV` |
  | `Math` | **low-level building blocks** — distributions, interpolation, root-finding, matrix ops | `aqMathNormalCdf` |
  | `Model` | term-structure / stochastic models, calibration sets, model-based / exotic / CMS-spread pricing, analytic (Jacobian) risk. May be sparse initially. | `aqModelHullWhiteCalibrate` |
  | `Generator` | list / describe / validate the JSON instrument & model static-data templates | `aqGeneratorList` |
  | `FX` | FX forwards and FX swaps derived from discount / xccy curves | `aqFXForward` |
  | `Tool` | echo, build stamp, edition report, diagnostics, memory dump, CSV load | `aqToolBuildTime` |

  **Name forms:** `aq<Category><Fn>` stateless · `aq<Category>Object<Fn>` handle
  API on the category's product · `aq<Category><SubObject><Fn>` handle API on a
  named sub-object (Curve, Generator, MarketData, Model, FixingTable) — the
  sub-object already denotes an object, so `Object` is not repeated ·
  `aq<Category><Variant><Fn>` a same-category **product variant** that doesn't
  earn its own category (`aqSwapOisPV`, `aqSwapOisParRate` — `Object` still not
  repeated for the stateless form) ·
  `aqObject<Lifecycle>` generic handle lifecycle (`aqObjectLoad`, `aqObjectSave`,
  `aqObjectClearCache`).

  **Decisions folded in:** `Curve` = rates yield-curve framework only, bond-curve
  fitting → `Bond`, hazard/survival → `Credit`. **No `Options` umbrella** and
  **no `Object` category** — the handle API is the `Object` word after the
  category. **`Volatility`** (full word; supersedes the earlier `Vols` and the interim `Vol`). **`Math` kept.**
  **`Model` added.** **`Products` dropped.** `LoanCalculations`,
  `SupervisoryRules` and `msc*` are **removed** (client-specific — Phase 1.4a).
  **`Ois` folded into `Swap` as a product variant** (2026-09-11, superseding the
  brief period it was its own category — see 2.7 below).
- ☐ **2.3** Record the locked list in `CLAUDE.md` §5.1 (done). Map every row of
  the 0.7 inventory (`rebrand\xll_function_inventory.csv`) to one of the 20
  categories or to drop/merge.
- ☑ **2.4 Edition → category map.** Realised as `AQ_XLL` build configurations
  (Phase 4a, done 2026-09-15) — `Release_XL_Swap`/`Release_XL_Bond`/
  `Release_XL_Credit`/`Release_XL_Curve`/`Release` (Full), each `Core` plus
  the `Optional` files below:

  | Edition | Registers |
  |---|---|
  | Swap | Date, Curve, Volatility, IR, Swap, Math, Generator, Object, Tool |
  | Bond | Date, Curve, IR, Bond, Math, Generator, Object, Tool |
  | Credit | Date, Curve, Credit, Bond, Math, Generator, Object, Tool |
  | Curve | Date, Curve, IR, Math, Generator, Object, Tool |
  | Full | all 21, incl. options and Model |

  No `AQ_API` equivalent — that half of the original straw man was dropped
  (Phase 4a).

- ☑ **2.5 Generator categories** (Nicholas). `Generator` category is
  **introspection only** — `aqGeneratorList` / `…Describe` / `…Validate`. The
  construction methods (`aqSwapFromGenerator`, `aqBondFromGenerator`, …) live in
  their **asset categories**, not in `Generator`.
- ☑ **2.6 Apply the singular scheme across all four surfaces.** *(DONE —
  built green with GTest passing, Nicholas 2026-09-09; uncommitted pending the
  staged commit.)* `validation` wrappers, `AQ_API` (SWIG `.i`), `GTEST` **test
  names**, `docs\api_map.csv` and every `resources\test` fixture file renamed to
  the 22-category singular golden-source scheme (`Vol`→`Volatility`, `Future` +
  `Ois` added). `AQ_XLL` call sites re-synced. Follow-on in the same delta:
  the AQ_XLL `Tool` + `Object`-lifecycle port from `.APPLES\...\meUtilities.cpp`
  (~55 `XLO_FUNC` functions across `aqObject.cpp` / `aqTool.cpp` + `aqXllTools`
  marshalling helpers), and `Interpolation` / `PCA` moved `Tool` → `Math`
  (`aqMathInterpolation` / `aqMathPCA`, `tryAqMath*`). Detail + resume plan in
  `rebrand\STATUS.md`.
- ☑ **2.7 `Ois` folded into `Swap` as a product variant** (Nicholas,
  2026-09-11) — supersedes the `Ois` category added in 2.6; category count
  22 → **21**. `Ois` is a vanilla swap with an overnight-compounded floating
  leg, not a structurally distinct product, so it does not earn a top-level
  category the way `AssetSwap`/`CapFloor` do. Golden-source form:
  `aqSwapOis<Function>` (stateless; object form would be
  `aqSwapOisObject<Function>` if ever needed), living in `aqSwap.cpp` /
  `src\validation\Swap\`, not a separate `aqOis.cpp` / category folder.
  **Rename done (2026-09-11):** the pre-existing `tryAqOisPV` /
  `tryAqOisParRate` (+ `*LVBKeys`) wrappers, their `AQ_API` bindings
  (`aqOisPV`/`aqOisParRate`, shipped to Python/C#/Java/R via SWIG) and
  `GTEST` (`TryAqTestTradeEUROISParRate.cpp`) predated this decision and have
  been renamed to `tryAqSwapOisPV`/`tryAqSwapOisParRate` /
  `aqSwapOisPV`/`aqSwapOisParRate`, per §5.1a's validation → GTEST → API →
  XLL order (the `AQ_XLL` functions themselves land with the rest of `Swap`).
  Detail: `rebrand\STATUS.md`.

**Exit:** agreed category list + edition→category map in `CLAUDE.md`; 0.7
inventory fully categorised.

### Navigation — category names vs internal names

**The concern (Nicholas):** the public XLL/API categories (`Dates`, `Curves`,
`Swaps`, …) are *not* propagated into the `etrading` / `math` file and class
names (`LACurve*`, `LADate*`, engines, models), so it is hard to go from a
function seen in Excel to the code that implements it.

**Assessment: real friction, not a blocker — and forcing categories down into
the implementation layer would make things worse, not better.** Reasons:

- `etrading` / `math` are organised by *C++ design* — a curve class, a
  calibration engine, a model. One impl file often serves several API
  categories; one API category is often served by several impl files. A
  category-named file layout there would fight the code's actual structure.
- Public "category" is a *presentation* choice that will keep evolving (editions,
  new asset classes). Pinning implementation file names to it creates churn every
  time the taxonomy shifts.
- Precedent: QuantLib, Boost, Eigen all keep public grouping separate from
  internal file layout.

**What we do instead** (Phase 3.5 / 3.6):

1. **`validation` is the category index.** After Phase 3 every wrapper is
   `tryAq<Category><Function>` and sits in `src\validation\<category>\`. Open
   `validation\Curve\`, see every curve function; each wrapper is ~20 lines and
   names its `etrading` entry point. This is the browsable map.
2. **`docs\api_map.csv`** — live, shipped: `public name | category | validation
   wrapper | etrading entry point | test`. One `grep` from any name to its code.
3. **CI guard** (Phase 7.4): every `aq*` public function must have a matching
   `tryAq*` wrapper row — keeps the index honest.
4. Solution Explorer **filters** in `AQ_XLL` / `AQ_API` stay category-grouped
   (they already are).

`etrading` / `math` file and class names get the `LA→AQ` prefix swap and nothing
else — they stay domain-oriented.

---

## Phase 3 — Identifier rebrand + calendar delimiter  ☐

The big one. **`LA` → `AQL`** ("Legacy Analytics" → "AQ Legacy" — keeps the
legacy tree marked and greppable), `MA` / `MB` → `AQ` (confirm per project),
plus `me*→aq*` function bodies, plus `LWO→AQObj`. Anchored, case-sensitive, from an
approved list only (CLAUDE.md §5.5). One project per batch, in dependency order:
`math → models → calibration → etrading → validation → AQ_API`. Build +
baseline-diff between **every** batch.

- ◐ **3.1** For each project: `rebrand\tools\prefix_census.py <project>` →
  review → sign-off → `git mv` the `LA*` files to `AQL*`, apply the approved
  identifier map (identifiers + include-guard macros + error-string text), fix
  `#include "LA*.h"` **tree-wide**, update `.vcxproj`/`.filters` + natvis + SWIG
  `.i` → build all projects → baseline-diff.
  - ☑ **`math`** — commits `268ed8e` + `608a79f` fix. 204 `LA→AQL`, 235 files.
    Build-verified green (clean rebuild). One leak fixed: `LACoreComponentManager`
    (a `models` file used tree-wide) — its `git mv` was missed; done in `608a79f`.
  - ☑ **`models`** — commit `15790a98`. **578 `LA→AQL`** (approved map =
    every `LA*` file stem ∪ owned tokens — the fix for the `math` leak class),
    **552 files** `git mv`, tree-wide **744 files / ~20,250 replacements**. 11
    vcxproj-casing merges (`PayOff` vs git's `Payoff`). SKIP `MANUAL`/`MANAGER`
    (English `#define`s). Verified: 0 broken includes / missing vcxproj / artifacts.
    Awaiting Nicholas's clean rebuild + GTest diff.
  - **Follow-up (Phase 3 guard sweep):** `__LAxxx_H__` / `#ifndef LAxxx_h`-with-`_`
    style include guards are not caught by the `\bLA` anchor — cosmetic, functional;
    normalise in one pass after all project batches.
  - Tooling now: `rebrand\tools\prefix_census.py` (stem∪token, fast one-pass) +
    `rebrand\tools\run_batch.py` (git mv + `\b`-replace + vcxproj-casing merge +
    verify gate that aborts+reverts on any lock/mismatch).
- ☐ **3.2 `LWO → AQObj`** (D4, D15). Two distinct things:
  - **C++ object-framework classes** get the `AQObj` prefix (AlgoQuant Object):
    `LWOCurve → AQObjCurve`, `LWOCurveDayAdjustment → AQObjCurveDayAdjustment`,
    `LWOUtilities → AQObjUtilities`, `HandleEnums → AQObjHandleEnums`,
    `IsLWOObject.{h,cpp}` → `AQObjectPredicates.{h,cpp}` with the free function
    `isLWOObject() → isAQObject()`. Folded into the `etrading` batch. Handle
    behaviour (counter, cell-hash, recalc suffix) byte-for-byte unchanged.
    **Never `AQObjbject`** — when renaming `AQO`→`AQObj`, exclude tokens that
    already contain `AQObject` (`AQObjects`, `IsAQObject`, `isAQObject`). Note the legacy
    `math` class `LAObject` becomes `AQLObject` (distinct from the framework's
    `AQObject` / `AQObj*`), so no clash.
  - **Public function names carrying `LWO`** become `aq<Category>Object<Fn>` —
    the word `Object` after the SINGULAR category is what separates the handle
    API from its stateless twin:
    - object-lifecycle ops (`meLWOLoad/Save/Copy/Modify/Delete/Clear/List`) →
      **`aqObject…`** with no category word (`aqObjectLoad`, `aqObjectCopy`, …)
    - handle-based pricing/creation (`meLWOSwapPV`, `meLWOBondPrice`, …) →
      **`aq<Category>Object<Fn>`** (`aqSwapObjectPV`, `aqBondObjectPrice`)
    - functions on a named sub-object (`meLWOCurveMarketDataDisplay`,
      `meLWOBondCurve…`) → **`aq<Category><SubObject><Fn>`**, no `Object` word
      (`aqCurveMarketDataDisplay`, `aqBondCurveYield`)
  - `tryMeLWO*` validation wrappers follow the same rule with a `try` prefix.
- ☐ **3.3** Function prefixes `me* → aq*` / `tryMe* → tryAq*` in `validation` and
  `AQ_API`, plus SWIG `.i`. This is the **clean break** (D9): no `me*` spelling
  survives anywhere, no forwarding aliases. Collect the removed public names into
  a `RELEASE_NOTES` "renamed / removed functions" list as you go.
- ☐ **3.5 `validation` layer as the category index** (navigation — see below).
  Rename every wrapper to `tryAq<Category>…<Function>` (the identical public name
  plus `try`) and move it into `src\validation\<category>\` (folder + `.filters`),
  where `<category>` is one of the locked 20 (singular). This makes
  `validation\Curve\` etc. the browsable
  table-of-contents from any public name to its `etrading` entry point.
  `etrading` / `math` file names stay **domain-oriented** (organised by class /
  engine / model, not by API category) — do not force them to match.
- ☐ **3.6 Live API map** — regenerate `rebrand\xll_function_inventory.csv` (rename
  it `docs\api_map.csv`, keep it shipped) with the final columns:
  `public name | category | validation wrapper | etrading entry point | test`.
  One `grep` from "where is `aqCurveObjectForwardRate`". A CI check (Phase 7.4)
  asserts every `aq*` public function has a matching `tryAq*` wrapper row.
- ☐ **3.4 Calendar delimiter** (D5, D12 — `+` only): add
  `constexpr char CALENDAR_CENTRE_DELIMITER = '+';` and a single
  `splitCalendarCentres()` helper (structured so re-enabling `:` is a one-line
  toggle, but shipping `+`-only). Route through it: the 16
  `chgrow(...CALENDAR...).toToken(':')` sites in
  `etrading\LACurveCalibrationHelpers.cpp` + `models\LAMathCurveGenerateFuncUtility.cpp`,
  `LAPriceDataCalendar::convertFromString`, and the NYB-strip rebuild in
  `LAMathDateCalculations.cpp` (rebuild with `+`). **Leave every other
  `toToken(':')` alone** — `:` stays the generic list separator. Migrate the
  ~103 generator JSON files' **calendar fields only** to `+` (curve-name-list
  fields keep `:`). `GTEST`: `"SYB+LNB"` builds the expected combined
  holiday set. Details: `rebrand\calendar_delimiter_sites.md`.

**Exit:** `grep -rE "\bLA[A-Z][a-z]|\bLA[0-9]D|\bme[A-Z]"` in `src\` returns only
third-party / literals (`mir*` still present — retired in 3c; `AQL*` is the
intended legacy marker and is expected); all configs build; baseline-diff
**numerically identical** (renames are behaviour-preserving — any delta is a bug,
and the calendar change is the only intentional behaviour change, covered by its
own new test).

---

## Phase 3c — Retire `mir*`  ☐

`mir*` is the legacy Interest-Rates binding stack. 58 `AQ_API\mir*.{cpp,h}`,
65 `validation\tryMir*.*`, `mir*` blocks in 4 `swig_*.i` + `swig_R_wrap.cpp`.
**49 `GTEST` files** and `etrading\Replay.cpp` use 22 distinct `tryMir*`
functions (details: `rebrand\callgraph_mir.md`).

- ☐ **3c.1** For each of the 22 `tryMir*` functions, map to the `tryAq*`
  equivalent (post-Phase-3 names): `tryAqProductSwapPv`, `tryAqProductOisParRate`,
  `tryAqCurveCalibrate*`, `tryAqDate*`, `tryAqObjectsCurve*` etc. Any with no
  equivalent → promote that one `mir` function to `aq` (rename + keep), don't
  drop the capability.
- ☐ **3c.2** Migrate the 49 `GTEST` files to the `tryAq*` calls, **one
  file at a time, re-baselining each** — numbers must match the pre-migration
  run for that test.
- ☐ **3c.3** `Replay.cpp` — drop the four `"tryMirSetUp…Curve"` `functionList`
  keys (the `"tryAqCurveCalibrate*"` keys already point at the same
  `replayCurve*` functions).
- ☐ **3c.4** Delete all `mir*` / `tryMir*` files + swig `.i` blocks +
  `.vcxproj`/`.filters` entries. Regenerate `swig_R_wrap.cpp`. Reword the two
  `mirCurveGenerate` error strings.

**Exit:** `grep -rE "\bmir[A-Z]|tryMir"` in `src\` is empty; suite green and
baseline-identical.

---

## Phase 4 — xlOil XLL port  ☑ (essentially complete, 2026-09-11)

**Status (2026-09-15): 470 `AQ_XLL` functions ported, covering 460 of 470
`validation` wrappers.** The 9 unmatched wrappers are deliberate exclusions
(1 false-positive name collision, 1 in-place mutator that doesn't fit a
worksheet-function shape, 7 legacy SABR functions Nicholas decided to leave
out as superseded) — not gaps; a 10th apparent miss is a script artifact
(trailing-space regex quirk), not a real one. Detail and the re-runnable
audit script: `rebrand\STATUS.md` and `STATUS.md` §1a/§2. `Generator` was
built from scratch this session (§4.7 below). `Model` is the only category
with **zero validation wrappers today**, and stays that way until a design
brief exists — unlike `Generator`, there is no legacy source and nothing on
disk to introspect.

Wait for Nicholas's xlOil worked examples before starting — they define the
canonical marshalling / handle-I/O / array-return / error-convention pattern.

- ☐ **4.1** Port the **AQObj handle framework** from `.APPLES\...\etrading`
  (`LWOCurve`, `IsLWOObject`, `HandleEnums`, `LWOUtilities`,
  `StructuredExceptionHandler`) into `src\etrading`, rebranded, behaviour
  preserved. Unit-test the counter/hash/recalc-suffix logic directly.
- ☐ **4.2** Audit `XllPlusTips*` → keep/drop list (CLAUDE.md §6.1). Port the
  kept helpers onto `ExcelObj` / `ExcelArray`. Delete the XLL+ boilerplate.
- ☐ **4.3** Write the porting template doc from the examples: old `_Impl` →
  `XLO_FUNC_START/END` + `.help()/.arg()`, plus the required `validation`
  wrapper + `GTEST` case per function.
- ☐ **4.4** Remove the dead `XllPlus\7.0\...` include/lib paths from
  `AQ_XLL.vcxproj`.
- ☑ **4.5 Editions & manifest gating** — see **Phase 4a** below, done
  2026-09-15. `AQ_XLL`-only, not shared with `AQ_API` — that half was
  dropped, not built (see Phase 4a).
- ☐ **4.6** **Clean break confirmed at registration** (D9): register `aq*` names
  only; no `me*` aliases, hidden or otherwise. Finalise the `RELEASE_NOTES`
  renamed/removed-function list started in 3.3 so users can find replacements.
- ☑ **4.7** Port functions **category by category** in the CLAUDE.md §9.4 order
  (Date → Tool → Curve → Swap → Bond → Credit → IR → Volatility → options →
  Math → Model → Generator → Object), driven by the 0.7 inventory and the
  locked Phase 2 categories. Per function: confirm it is a "keep" → port `_Impl`
  → `validation` wrapper → `GTEST` case → tick the inventory row. Build +
  run suite per category. **Done (2026-09-11) for every category with an
  existing `validation` surface** — `Date, Curve, FX, Inflation, Volatility,
  IR, Future, Swap, AssetSwap, CMS, TRS, CapFloor, Swaption,
  BondOption, BondFutureOption, Bond, Credit, Math, Tool, Object(-lifecycle)`
  all ported, build green, tests pass. **`Generator` done (2026-09-15)** —
  `tryAqGeneratorList`/`Display`/`Validate` designed and written from
  scratch (no `validation` surface existed to port from) across all four
  surfaces; see `STATUS.md` §1a and `rebrand\STATUS.md`'s top entry.
  `Model` remains: **0 `validation` wrappers**, no legacy source, no design
  brief — blocked on Nicholas's input (which model types, what parameters),
  not on engineering time.
- ☑ **4.8 Generator + config wiring** — introspection (`List`/`Display`/
  `Validate`) done 2026-09-15; see `STATUS.md` §1a. Any remaining
  `config`-packaging work for the XLL/API deploy paths stays open under
  **Phase 4b**.
- ☑ **4.11 `AQ_XLL` file naming** (Nicholas): every category file is
  `aq<Category>.{cpp,h}` — `aqBond.cpp`, `aqDate.cpp`, `aqMath.cpp`,
  `aqTool.cpp`, `aqObject.cpp`, `aqMain.{cpp,h}` — with **one exception**,
  `aqXllTools.{h,cpp}`, the shared Excel-side helper library (marshalling +
  AQObj handle decoration). `XllTools` is not a category — the `Xll` infix
  marks it as the XLL-layer utility file, distinct from the `Tool` category
  file `aqTool.cpp`. Do the same in `AQ_API` where practical.
- ☐ **4.9 Record / playback / rebase on EVERY validation function**  (Nicholas,
  2026-09-06). The infrastructure already exists and is proven — `RecordMacros.h`
  (`RECORD_INPUTS_n` / `RECORD_OUTPUTS`), `CreateDataFile::recordEnabled()`,
  `ReadDataFile::Load`, and the rebase switch `CreateDataFile::setRebaseResultsFlag()`
  / `rebaseResultsEnabled()` which overwrites the stored outputs instead of
  comparing to them, so a test set can be reset on demand. What is missing is
  **coverage**: the macros were applied to some functions and not others.
  - **Rule going forward: every `validation` function gets `RECORD_DECORATED_INPUTS`
    / `RECORD_INPUTS_n` and `RECORD_OUTPUTS`. No new `tryAq*` lands without them.**
  - **The macros exist and are named `AQ_RECORD_INPUTS(...)` / `AQ_RECORD_OUTPUTS(result)`**
    (variadic, up to 20 args; `AQ_RECORD_DECORATED_INPUTS` for object functions
    that need a filename prefix/suffix). `__FUNCTION__` supplies the file name and
    the `generatorFunction` field, so nothing is hand-written.
  - **Rebase is a TEST-side concern, not a validation one.** `validation` only
    records; `GTEST/ResultsProcessor.cpp` decides whether to compare against the
    stored outputs or overwrite them, keyed off
    `CreateDataFile::rebaseResultsEnabled()` (set by `RunTests.cpp`). So
    `AQ_RECORD_OUTPUTS` needs no rebase logic — do not add any.
  - Coverage today: 57 of 101 validation source files use the macros, ~52 carry
    hand-rolled `if (CreateDataFile::recordEnabled()) { ... }` blocks, and some
    functions have neither.
  - ⚠ **18 hand-rolled INPUT recordings use a key that is NOT the parameter name**
    (`key=staticDataTable, expr=curveName`; `key=convexityAdjConv,
    expr=tenorBasisConv`; ...). Converting those to the macro silently changes
    the fixture key and breaks the test at run time, build still green. Convert
    them by hand, or rename the parameter to match the key, and re-run
    `rebrand/tools/fixture_key_check.py`. The other ~453 hand-rolled pairs
    already agree and convert mechanically.
  - 🐞 Found while auditing: `tryAqObjRatesFixingTable.cpp` writes
    `file.write("fixingValues", fixingDates)` and
    `file.write("fixingDates", fixingValues)` — **the two keys are swapped**.
    Pre-existing; the recorded fixtures have dates under the values key. Fix the
    code and rebase those fixtures together, never separately.
  - Audit the existing `tryAq*` surface and add the macros where they are absent;
    that is what turns a recorded workbook into a `GTEST` case for free, and it is
    the mechanism behind the whole fixture suite.
  - ⚠ **The recorded key is the STRINGIFIED C++ PARAMETER NAME** —
    `WRITE_PARAMETER(P)` expands to `file.write("P", P)`. Renaming a parameter
    silently breaks every fixture that records it: the build stays green and the
    test fails at run time with `ReadDataFile::Load: unknown key`. This bit us in
    step 8B. Guard: `rebrand/tools/fixture_key_check.py`, run after any rename
    that touches validation parameters.
- ☐ **4.12 Structured exception handling for `AQ_API` too** (Nicholas asked
  2026-09-06 whether SEH covers the API as well as Excel — it does not, fully).
  `VALID_EXCEPTION_START` installs the handler on every `tryAq*`, so **any**
  caller — XLL, Python, C#, Java, R, GTEST — is protected for the part that runs
  inside `validation`. But `AQ_API_START` (`src/AQ_API/source/APISetUp.h`) is a
  bare `try {`: **no `StructuredExceptionHandler`**. So a fault in AQ_API's own
  marshalling — SWIG conversion, building result vectors — is unprotected and
  will take down the host process (the Python interpreter, the CLR, the JVM).
  Fix symmetrically with `AQ_XLL_GUARD`: add the handler to `AQ_API_START`.
  Note `AQ_API` must build `/EHa` for this to work — check before relying on it.
- ☐ **4.10 Structured exception handling across `AQ_XLL`**  (Nicholas, 2026-09-06).
  An access violation, stack overflow or divide-by-zero inside a worksheet
  function is a Windows structured exception, not a C++ one; unhandled it takes
  Excel down with it. `etrading::StructuredExceptionHandler` installs a
  `_set_se_translator` for its lifetime and turns those into C++ exceptions that
  xlOil's `XLO_FUNC_START/END` returns to the cell as an error string.
  - `validation` is already covered — `VALID_EXCEPTION_START` installs the
    handler (plus a `ThreadGuard`) on every `tryAq*` call.
  - The gap was the **marshalling** either side of the validation call, which
    `validation` never sees: reading a malformed Excel range, or building the
    result array.
  - **Rule going forward: `AQ_XLL_GUARD` (in `aqXllTools.h`) is the first line of
    every AQ_XLL worksheet function.** Applied to `aqDateFromTenor` and
    `aqDateFromYearFraction` as the reference pattern.
  - Requires `/EHa`; all projects already set `<ExceptionHandling>Async`. Do not
    change that — the translator silently stops working under `/EHsc`.
  - Note `ThreadGuard` is deliberately NOT used at the XLL boundary: it throws if
    two threads enter, which is validation's single-threaded contract, and xlOil
    may call `.threadsafe()` functions concurrently. validation applies it itself.

**Exit:** every "keep" row in the inventory is ported, wrapped and tested;
every ported function carries `AQ_XLL_GUARD` and its `validation` wrapper carries
the record/playback macros;
`AlgoQuantLib.xll` loads and every category returns correct values against the
baseline; dropped functions are recorded with a reason; editions gate correctly;
generators load from the shipped `config` folder.

---

## Phase 4a — Editions & manifest gating  ☑ (done, 2026-09-15)

`AlgoQuantLib` ships as **Bonds / Swaps / Credit / Curves / Full** editions.
**Decided (Nicholas, 2026-09-15): `AQ_XLL`-only, gated at compile time. The
`AQ_API` runtime-gate half of this phase (below) is dropped, not
deferred** — `AQ_API` ships one full binary per language with every
category always registered; there is no edition concept on that surface.

**`AQ_XLL` (decided, Nicholas 2026-09-11; shipped 2026-09-15) — per-edition
build configurations, gated at compile time.** A native `.xll` per edition
is cheap for one add-in: `projects\AQ_XLL.vcxproj.filters` splits every
`src\AQ_XLL\src\*.cpp` file into **`src\Core`** (always-built:
`aqXllTools.cpp`, `aqMain.cpp`, `aqDate.cpp`, `aqObject.cpp`, `aqMath.cpp`,
`aqTool.cpp`, plus `aqCurve.cpp` and `aqIR.cpp` — promoted 2026-09-12, see
4a.5) and **`src\Optional`** (one file per product category — every new
category file goes here). Five build configurations exist alongside
`Debug`/`DebugEditAndContinue`/`ReleaseProfiler`: **`Release`** (Full,
excludes nothing) and **`Release_XL_Bond`**, **`Release_XL_Swap`**,
**`Release_XL_Credit`**, **`Release_XL_Curve`** — each compiles `Core` plus
only the `Optional` file(s) that edition needs (excluded from build for
every other configuration). A further `Release_XL_Manifest` configuration
layers a function-level cut on top via `resources\manifest\activeList.txt`
(renamed 2026-09-15 from `active.txt`, alongside `demo.txt`→`demoList.txt`;
see `rebrand\STATUS.md` for that tooling) — a finer cut than the edition
mechanism, not a sixth edition. Detail: `CLAUDE.md` (both) §4.4/§4.5,
`AQ_LIB\CLAUDE.md` §6.3.

- ☑ **4a.5** `AQ_XLL` per-edition build configurations
  (`Release_XL_Bond`/`Release_XL_Swap`/`Release_XL_Credit`/`Release_XL_Curve`),
  each excluding the `src\Optional` files its edition doesn't need — **done,
  confirmed building green**.
  **Dependency check flagged 2026-09-12, resolved same day:** `aqCurve.cpp`
  sat in `src\Optional`, but every priced product category depends on it
  (bonds/swaps/caps all discount off a curve) — a `ReleaseBonds` config that
  excluded it would ship a Bond edition that can't build or calibrate the
  curve it needs to price against. **Fixed: Nicholas promoted `aqCurve.cpp`
  and `aqIR.cpp` (was `aqInterestRate.cpp` — fixing tables/FRA conversions
  cross-cut the same way) into `src\Core`**, committed. A "Curves-only" SKU
  falls out naturally as a `Core`-only build with no `Optional` files added.

**Dropped, not carried forward (was 4a.1–4a.4):** the `AQ_API` runtime
edition manifest — `config\editions.json` mapping edition → categories,
`config\licence.json` entitlement, a module-import registration gate, and
an `aqToolEdition()` reporting the active edition/category list at runtime.
None of this is being built; `AQ_API`'s bindings register every category
unconditionally, in every language, always.

---

## Phase 4b — Config folder & generators  ☐

The `config` folder ships with **both** the `AQ_API` and `AQ_XLL` packages and is
the library's no-recompile customisation surface:

- **Holiday / calendar files** (from MarketWire / SwapsWire) — loaded at runtime;
  end-users correct / add / modify holidays without a refreshed add-in. This is
  the existing "calendars without recompilation" property; the `config` folder is
  the mechanism.
- **Generators** — JSON **instrument & model static-data templates**. Static data
  is ~90% of a trade booking; a generator carries the conventions (calendars, day
  counts, rolls, fixing / payment lags, currency defaults, model params) so a
  user books e.g. a USD swap from `notional, effective, maturity, fixed rate,
  pay/receive`, or a US Treasury from `notional, issue/settle, maturity, coupon`,
  instead of hand-assembling conventions most users are unfamiliar with.
  Generators are consumed by the AQObj object layer: generator name + a few
  overrides → object handle. Existing seeds live under
  `resources\config\{SWAP,BOND,CURVE}_GENERATOR`.

- ☐ **4b.1** Document the generator JSON schema; add a schema-validation
  `GTEST` over the shipped set (the `tryAqGeneratorValidate` function built in
  4b.2 below re-uses the object framework's own deserialization + key
  validation rather than re-implementing a schema check — a hand-written
  schema doc plus a dedicated schema-conformance `GTEST` sweep across the
  full shipped set, beyond the three seed files exercised in `TestAqGenerator
  *.cpp`, is still open).
- ☑ **4b.2** `Generator` category (2.5, singular per §5.1 — corrects this
  line's earlier `Generators` typo): `aqGeneratorList` / `…Display` /
  `…Validate` **done 2026-09-15** (`Display`, renamed same-session from an
  initial `Describe` to match the pre-existing per-asset `*GeneratorDisplay`
  naming) across `validation`/`GTEST`/`AQ_API`/
  `AQ_XLL` — see `STATUS.md` §1a. The per-asset `aqSwapFromGenerator` /
  `aqBondFromGenerator` construction functions already existed pre-session
  and are unaffected.
- ☐ **4b.3** Confirm `config` is packaged by **both** deploy paths. Python
  already does (`deployPython_2022.bat` copies `resources\config`); add the same
  for `AQ_XLL` and the other languages.
- ☐ **4b.4** Rebrand the generator JSON and calendar config — legacy names,
  client-specific conventions and defaults (folded into the 6.4 resources audit).
- ☐ **4b.5** One end-to-end test per asset: book from a generator + minimal
  fields, price, check against the baseline.

---

## Phase 5 — Bindings & test coverage  ☐

- ☑ **5.1a** `AQ_BINDINGS→AQ_API` rename (D3) — done, project renamed and
  Python verified green end-to-end.
- ~~☐ **5.1b** Exercise C#, R, Java~~ — **shelved permanently (D21,
  2026-09-15), not a Phase 5 exit criterion.** No current requirement to use
  these languages and no test environment available. Revisit only if/when a
  client or use case actually needs one of them — at that point, generate,
  build, deploy and run the relevant `resources\api\*` test app before
  calling that language verified. Until then this is a standing nice-to-have,
  not tracked work.
- ☐ **5.2** ~~Wire `AQ_API` module import to the Phase 4a edition gate~~ —
  **dropped with the `AQ_API` edition gate itself (Phase 4a, 2026-09-15)**.
  Ship `config` (calendars + generators, no `editions.json`/`licence.json`)
  with every language package (4b.3).
- ☐ **5.3** Fill priority `GTEST` gaps:
  - Yield-curve framework + Jacobian risk vs bump-and-revalue.
  - Fixed-income price/yield vs Bloomberg (both directions) — regression lock.
  - Date logic (it moved out of legacy in Phase 6).
  - Generator round-trips per asset (4b.5).
- ☐ **5.4** Wire the `validation`-recording → `GTEST` generation so new
  wrappers get cases automatically.

**Exit:** Python builds and passes (D21 — C#/Java/R shelved, not a gate); new
coverage merged; suite green against baseline.

---

## Phase 6 — Legacy extraction, resources, docs, licence  ☐

- ☐ **6.0 Credit untangle** (was 1.4a). Decide what the `Credit` category is in
  the rebranded library, then act. `etrading\CreditResults.h` (CDS
  result-reporting) `#include`s six securitisation-cluster headers
  (`LoanPortfolio`, `LoanCalculations`, `SyntheticExcessSpread`, `CashflowEngine`,
  `Trigger`, `Tranche`) and holds a `shared_ptr<Trigger>`; `Trigger.h` includes
  `LoanCalculations.h`; `validation\tryMeUtilitySetup.cpp` uses
  `CreditResultsContainer`. Two outcomes (Nicholas's call):
  (a) `CreditResults` is securitisation code mis-labelled CDS → delete
  `CreditResults*` + the whole cluster + `tryMeCashflowClient/LoanCalculations/
  SupervisoryRules` wrappers, adjust `tryMeUtilitySetup`;
  (b) it is real CDS reporting with vestigial loan/trigger/tranche members →
  strip those members, then the cluster + client wrappers delete cleanly.
  Either way: `SupervisoryRules`, `CashflowClient`, `LoanCalculations` go; `msc*`
  GoogleTest / resource references go. Build + baseline-diff after.
- ☐ **6.1** Usage-map `calibration` / `math` / `models` — what is reachable from
  `validation` / `etrading`?
- ☐ **6.2** Extract the live pieces (dates, strings, math utilities) into a clean
  `core` / `utils` project under `etrading`, rebranded. Point callers at it.
- ☐ **6.3** Delete the dead remainder — one project at a time, build between.
- ☐ **6.4** **`resources\` and `examples\` audit** (D13). Every spreadsheet,
  toolkit, config, workshop and research guide: rebrand, rewrite, or remove.
  Includes the `config` folder — calendar/holiday files and generator JSON
  (4b.4): strip legacy names and client-specific conventions. Nothing
  legacy-branded survives; broken or client-specific examples are cut, not
  carried. **Also: regenerate `Calendar.csv` / `Calendar.conf`** — currently
  a stopgap (`LastCalendarUpdate` bumped 2026-09-15 to silence
  `Calendars.UNIT_Expiry_Test` without actually refreshing `Calendar.csv`,
  still dated 2022-07-22; ~10 GTEST failures reported 2026-09-15, likely
  from this staleness — see `STATUS.md` top section). The `CDWCalendarUpdate.bat`
  tool this line used to point to was **never ported into `AQ_LIB`** — it
  only exists in `.APPLES\APPLE\resource\utilities\BatchFiles\CDWCalendarUpdate\`,
  and is a Python script pulling from a paid MarketWire/SwapsWire endpoint.
- ☐ **6.4a Calendar sourcing strategy** (new, 2026-09-15 — Nicholas: SwapsWire/
  MarketWire aren't free; look for a public/cheaper alternative before
  committing to a refresh mechanism). Needs a decision before 6.4's
  `Calendar.csv` regeneration can be more than a one-off stopgap:
  - **Candidate: QuantLib's built-in `Calendar` classes** (`ql/time/calendars/*`)
    — already a linked dependency (`AQ_EXTERNAL_LIB_PATH\...\QuantLib-1.43`),
    permissive licence, no new cost. Algorithmic rules (Easter-based, nth-
    weekday) self-extend forever; hardcoded exception lists only update via
    a QuantLib version bump (community-maintained, lags reality — see
    `STATUS.md`/conversation 2026-09-15 for the full explanation). Covers a
    meaningful subset of `Calendar.csv`'s ~110 centre codes (major
    currencies/markets) but not the niche settlement/clearing codes
    (`BMA`, `CMF`, `KdQ`, `SyF`, ...) that look SwapsWire-specific.
  - **Candidate: public sources per centre** (central bank / exchange
    published holiday lists — ECB TARGET calendar, Federal Reserve, gov.uk,
    exchange sites) for the codes QuantLib doesn't cover. Manual/scraped,
    not a single feed.
  - **Open question:** first inventory which of the ~110 `Calendar.csv`
    centre codes actually matter (are used by any shipped generator /
    booked product) vs. dead weight from the original client's book — no
    point sourcing calendars nobody needs.
  - Whatever is chosen, keep the existing wide-CSV format and the "calendars
    without recompiling" property (`CLAUDE.md` §9.2/§4.4) — this is a data-
    source swap, not an architecture change.
- ☐ **6.5** Rewrite `readme.md` (D7) — draft exists; refresh once names are final.
  Add the config-folder / generators / editions sections.
- ☐ **6.6** Finalise `THIRD_PARTY_LICENSES.md` / `NOTICE` (Boost, QuantLib,
  **patched** xlOil — mark modified files, Eigen, Adept; GoogleTest test-only).
- ☐ **6.7** **Banner cleanup** (D10.1). Bulk-remove the ~1,470 `Mizuho` / `MHI` /
  `MHSC` header and `@Department` lines across `src\` and replace with, at most,
  a minimal technical banner — no copyright, no company/person name, no
  tool/migration reference. Do **not** insert a copyright or licence statement
  into source files; that text lives only in `LICENSE`. Replace `LICENSE`
  placeholder content when the EULA is ready.
- ☐ **6.8 Bulk `clang-format` pass** (see Tooling appendix). One normalization
  commit now that the mass renames are done. Then it stays green.
- ☐ **6.9 `clang-tidy` config.** Add `.clang-tidy` with
  `readability-identifier-naming` tuned to the AQ convention (camelCase; types
  Upper; functions/vars lower; members trailing `_`) as a **rename-completeness
  guard**, plus `bugprone-*` / `performance-*` / selected `modernize-*` as
  **advisory** (not build-breaking yet). Needs a `compile_commands.json` (MSBuild
  can emit one).
- ☐ **6.10 Delete the `AQL Classic` `.vcxproj.filters` filter contents**
  (Nicholas, 2026-09-12). Scoped, not actioned, this session — small and
  tractable, unlike 8.1/8.2 below. **17 files, `GTEST.vcxproj.filters`
  only:** 6 curve-fixture `.cpp`/`.h` pairs (`Curve{Accessors,Ois,FwdFxConst,
  Std,TenorBasis,XccyBasis}`), `TestDatesCentralBank.cpp`,
  `TestDatesSwapSchedule.cpp`, `TestRiskSwapDeltaLadder.cpp`,
  `TestRiskTenorBasisCurve.cpp`, and `TestMirDateFunctions.cpp` (check this
  one first — a `mir`-named test may already be dead per the "`mir*` stack
  deleted wholesale" work; confirm before deleting rather than assuming).
  `AQ_API.vcxproj.filters` declares the same filter with **zero files under
  it** — delete the empty `<Filter Include="...AQL Classic">` declarations
  there too. Detail: `rebrand\STATUS.md`.

**Exit:** legacy projects gone or reduced to a documented `core`; resources,
examples and config fully rebranded or removed; every header carries the
proprietary notice; `clang-format` clean; `clang-tidy` naming check clean;
`AQL Classic` filter contents gone; docs current.

---

## Phase 7 — Linux build, final sweep, clean repo  ☐

- ☐ **7.1 Linux / CMake** (D13). Re-validate the MAKE/CMAKE build on the
  rebranded tree; retarget to the current dependency set; rebrand its
  `Makefile` / `make.header` / `make.footer` / `make.sh` and any CMake files.
  Lower priority than 4–6 but **in scope** — it is part of "no legacy names
  anywhere".
- ☐ **7.2 Final legacy sweep.** Tree-wide, case-sensitive:
  `grep -rIE "Mizuho|MLIB|\bme[A-Z]|\bmir[A-Z]|\bLA[A-Z][a-z]|\bLA[0-9]D|\bMA[A-Z][a-z]|validation_api|XllPlus|MLIBQ"`
  (`AQL*` is the intended legacy prefix — expected, not a hit)
  over the whole `AQ_LIB` tree (source, projects, scripts, resources, examples,
  docs, Linux build). Expected result: **zero hits** outside third-party headers
  and genuine string data. Anything left is fixed or removed.
- ☐ **7.3** Full green build, all 12 Windows configs + Linux; full suite green;
  baseline-diff clean (bar the intended calendar change).
- ☐ **7.4 CI** — a pipeline that builds the key configs, runs `GTEST`, and
  runs `clang-format --dry-run --Werror` + the `clang-tidy` naming check as
  gates; the rest of `clang-tidy` as a non-blocking report. Wire it against the
  new clean repo.
- ☐ **7.5** Copy `AQ_LIB` to the new folder, `git init`, first commit = clean
  base. Delete the stash repo.
- ☐ **7.6** Commodities library integration — **only** once the above is green
  (separate plan; copy in, retarget v143/C++17 standalone, expect the same C++17
  fixes, then rebrand and add a `Commodities` category).

---

## Phase 8 — Retire `AQLString` / `AQLDate` (scoped 2026-09-12, NOT started)  ☐

**Deliberately its own phase, after Phase 7, not folded into Phase 6.** This
is a genuinely separate initiative from the rebrand: it is a type-replacement
across the whole codebase, not a rename, and the blast radius is an order of
magnitude past anything else in this plan. Do not start it opportunistically
mid-batch the way small fixes have been folded in elsewhere — it needs its
own dedicated pass, tooling and regression budget.

**Scale (measured 2026-09-12):** `AQLString` — **~36,962 occurrences across
1,108 files.** `AQLDate` — **~9,904 occurrences across 585 files.** For
comparison, the entire `me*`→`aq*` identifier rebrand (Phase 3, the biggest
thing done so far) touched a fraction of this.

**Why retire them at all (Nicholas asked directly — answered here):**
- **`AQLString`** (`src\math\include\AQLString.h` + `.cpp`, ~1,550 lines) is
  a hand-rolled, atomic-refcounted, copy-on-write string class — solving a
  problem (expensive string copies) that C++11's move semantics and small-
  string optimisation already solved natively in `std::string`, which this
  codebase has had available since the C++17 retarget. No architectural
  reason found to keep it; its extra convenience (numeric constructors,
  `getDoubleValue()`/`getIntValue()`) is a thin, trivially-replaceable layer
  over `std::to_string`/`std::stod`/`std::stoi`. **Target: `std::string`.**
- **`AQLDate`** (`src\math\include\AQLDate.h`, 112 lines + 695-line `.cpp`) is
  a hand-rolled, virtual, Julian-day calendar class. **Target:
  `boost::gregorian::date`** — already a direct dependency (BSL-1.0, no
  encumbrance), already the canonical date type taken directly by the newer
  `Curve` validation headers, and the bridge functions
  `etrading::toGregorianDateFromAQLDate`/`toAQLDateFromGregorianDate` already
  exist and are proven correct in production use — the hard
  conversion-correctness work is already done.
- ☐ **8.1 Tooling first.** A libclang/clang-tidy-based codemod (not `sed`/
  regex) that rewrites declarations, call sites and includes together,
  batchable by project or by file, with a dry-run diff mode. Manual
  batch-and-review (the Phase 3 approach) does not scale to 37k occurrences.
- ☐ **8.2 `AQLString` → `std::string`**, one project at a time
  (`math` → `etrading` → `validation` → `AQ_API`/`AQ_XLL` → `GTEST`), full
  build + baseline-diff between each. Expect signature ripples through every
  `validation` wrapper that takes `AQLString`/`AQLStringVector`/
  `AQLStringMatrix` — i.e. most of them.
- ☐ **8.3 `AQLDate` → `boost::gregorian::date`**, same batch discipline.
  Retire the now-redundant `toGregorianDateFromAQLDate`/
  `toAQLDateFromGregorianDate` bridge once nothing calls it.
- ☐ **8.4** Once both are gone: `git grep -w AQLString` / `AQLDate` return
  zero hits (bar `.APPLES`, read-only reference); delete
  `src\math\{include,src}\AQLString.*` / `AQLDate.*`.

**Exit:** `std::string` and `boost::gregorian::date` used throughout; no
`AQLString`/`AQLDate` symbol remains in `AQ_LIB`; full green build and
baseline-diff clean.

---

## Suggested order of execution

```
0 ─▶ 1 ─▶ 2 ─▶ 3 ─▶ 4 (+4a +4b) ─▶ 5 ─▶ 6 ─▶ 7 ─▶ 8
         │                          ▲
         └── Phase 2 output feeds 3, 4, 4a, 4b and 5
```

Phase 8 (`AQLString`/`AQLDate` retirement) is deliberately last and separate
— a post-launch modernisation initiative, not a rebrand blocker. It could in
principle start any time after Phase 3, but scheduling it after 7 keeps the
rebrand's own timeline and regression baseline undisturbed by a
much-larger-scale, higher-risk type change.

**Status (2026-09-15): Phases 0-3 done. Phase 4 done** (470 `AQ_XLL`
functions, 460/470 `validation` wrappers covered — see Phase 4's status note
above, including the new `Generator` category built this session); `Model`
still needs a design brief before any `validation` wrapper can be written (0
exist today, no legacy source, nothing on disk to introspect unlike
`Generator`). **Phase 4a done** — `AQ_XLL` ships
`Release`/`Release_XL_Bond`/`Release_XL_Swap`/`Release_XL_Credit`/
`Release_XL_Curve`; the `AQ_API` runtime edition manifest is dropped, not
built. Phases 4b (bar the `Generator` introspection slice, done), 5, 6, 7 not
started. Phase 7.1 (Linux) can slot in any time
after Phase 3 but is lowest priority.

---

## Appendix — Tooling & lint

**Recommendation: yes, adopt lint — two tools, phased so they never fight the
rename.**

| Tool | Role | When |
|---|---|---|
| **`clang-format`** | deterministic formatting; replaces the legacy AStyle utility. Editor + CI integration, one `.clang-format` at the root. | **Config in Phase 1.7**, tuned to current style so the eventual reformat is small. **Bulk apply once in Phase 6.8**, after all mass renames — reformatting mid-rename buries the real diff and breaks the baseline-diff discipline. Green thereafter. |
| **`clang-tidy`** | static lint. `readability-identifier-naming` **enforces** the AQ naming convention — a real completeness guard for the rebrand, complementing the Phase 7.2 grep. `bugprone-*` / `performance-*` / curated `modernize-*` catch latent issues. | **Config in Phase 6.9.** Naming check as a gate; the rest advisory (non-blocking) until the tree is clean, then tighten. Needs `compile_commands.json` (emit from MSBuild, or use the LLVM VS integration). |

Not recommended: per-file `cpplint` (Google-style, wrong fit); MSVC `/analyze` as
a gate (too noisy on this codebase — fine as an occasional manual sweep). IWYU is
optional but genuinely useful *during* Phase 1/3 include churn — run it manually,
don't gate on it.

Rule for the whole rebrand: **linters are guards, not gates, until Phase 6.**
Turning them on earlier means every rename batch argues with the linter instead
of with the compiler.
