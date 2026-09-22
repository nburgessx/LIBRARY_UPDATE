# AlgoQuantLib â€” Migration & Rebrand Plan

Rebrand the VS2017 client library (`..\..\.APPLES\APPLE`) into a modern,
vendor-neutral **AlgoQuantLib** product in `LIBRARY_UPDATE\AQ_LIB`, port the
Excel add-in from Planatech XLL+ to xlOil, and leave a tree clean enough to lift
into a fresh Git repo.

Status legend: â˜ not started Â· â— in progress Â· â˜‘ done

---

## Context and goals

- The library was built for one client; several clients now want it, so it is
  being rebranded into Nicholas Burgess / AlgoQuantHub's own name and shipped as
  a commercial product with full source.
- `LIBRARY_UPDATE\AQ_LIB` is **authoritative**. `.ALGO_QUANT_LIB` is a manual
  backup â€” read-only, never a build target. `.APPLES` is the read-only port
  source.
- Current Git repo is a temporary stash. When the rebrand is green, the tree is
  copied to a new folder and committed to a **new** repo for a clean base with no
  client-name pollution; the stash repo is then deleted.
- Toolchain migration (VS2022 / v143 / C++17 / static runtime / xlOil wired) is
  **done**. The code-level rebrand and the XLL port are **not started**.

### Decisions taken

| # | Decision |
|---|---|
| D1 | `.ALGO_QUANT_LIB` is a backup â€” leave untouched. |
| D2 | `msc*` functions (structured credit, other client) â€” **delete**, with all downstream references. |
| D3 | `AQ_BINDINGS` â†’ **`AQ_API`** â€” **done**. Project files, `.sln`, folder, SWIG `.i`, and all 8 `generate*`/`deploy*` batch files + pre/post-build commands all renamed. |
| D4 | The LWO handle/object framework stays â€” port as-is, rebrand `LWOâ†’AQObj`, keep the cell-location counter/hash behaviour. Do **not** adopt xlOil's cache. |
| D5 | Calendar holiday-centre delimiter `:` â†’ **`+`**. |
| D6 | Function-signature **categories** across validation / XLL / API / tests to be reviewed and standardised â€” professional, clear, concise. (Phase 2 â€” Â§2.2 is the table to review.) |
| D7 | `readme.md` to be rebranded and made current. (Phase 6.) |
| D8 | Env vars set once via `SetEnvVars.bat`, then restart VS. |
| D9 | **Q1 â€” Excel names: clean break.** Migrate every `me*` spelling (including any currently-hidden `me*` aliases) to `aq*`. **No** backward-compatibility aliases; old names simply cease to exist. Removed names are listed in the release notes. |
| D10 | **Q2 â€” Licence: proprietary, no redistribution, as a drop-in placeholder.** A short interim `LICENSE` (proprietary notice + "AS IS" + "no redistribution"), written so the final EULA text replaces `LICENSE` in full later **without touching any source file**. The contractor/client IP-provenance check with a solicitor is still advised before commercial release but does not block the rebrand. |
| D10.1 | **Q2.1 â€” Banners are minimal and technical-only (supersedes the earlier frozen per-file header).** No file or console banner carries copyright text, a company/person name (`AlgoQuantHub`, `Nicholas Burgess`, `Claude`, `Anthropic`, any client/contractor), or a reference to the old/new library name, the rebrand, or the migration effort. Banners are restricted to technical help (what the code does, params, units, preconditions); citations to external research sources are allowed. Copyright/licence text lives only in `LICENSE`. See `CLAUDE.md` Â§7.1. |
| D11 | **Q3 â€” `AQ_API` confirmed.** |
| D12 | **Q4 â€” holiday-centre join is `+` only** (Nicholas, this session). Reason: clean break (D9), fresh clients, no legacy user sheets to protect; the only `:`-form data that ships is the ~103 generator JSON, which we migrate ourselves; and `:` is heavily overloaded (`DATA_COLL_DEL`, curve-name lists in the same generator files). The `splitCalendarCentres()` helper is written so accepting `:` again is a **one-line toggle** if field feedback ever demands it â€” but it ships `+`-only. Migrate the 103 JSON calendar fields to `+`. |
| D13 | **Q5 â€” Linux / CMake build is in scope**, lower priority (late phase). **End-state gate: not one file anywhere in the tree â€” source, Makefiles, `make.*`, CMake, `.sln`/`.vcxproj`, scripts, resources, examples, docs â€” may contain a legacy client name or an old prefix.** Many `resources\` and `examples\` items will be rewritten or removed for the final version. |
| D14 | **Q6 â€” category taxonomy locked (21, singular).** See Â§2.2 / `CLAUDE.md` Â§5.1. (Was 22 briefly with `Ois` as its own category; folded into `Swap` as a product variant 2026-09-11 â€” Â§2.7.) |
| D15 | **LWO â†’ `AQObj`** for the C++ object-framework **classes** (`AQObjCurve`, `AQObjUtilities`, â€¦); screaming-snake macros take `AQOBJ_`; free predicate `isLWOObject â†’ isAQObject`. Public **function** names carrying `LWO` become **`aq<Category>Object<Fn>`** (`aqSwapObjectPV`, `aqBondObjectDirtyPrice`); named sub-objects skip the `Object` word (`aqBondCurveYield`); lifecycle ops drop the category (`aqObjectLoad`, `aqObjectSave`). Supersedes the earlier `aqObj<Category>` / `aqObjects*` schemes â€” see Phase 3.2, step 8 and step 11. |
| D17 | **`LA` â†’ `AQL`** (not `AQ`). `LA` = "Legacy Analytics" â€” the whole `LA*` tree is legacy-to-deprecate; the `AQL` ("AQ Legacy") prefix keeps it visually distinct and greppable against new `AQ*` code. Applies to identifiers, files (`LAString.h â†’ AQLString.h`), include-guard macros, error-string text. `MA`/`MB` â†’ `AQ`, confirmed per project. `LAObject â†’ AQLObject`, `LAMath â†’ AQLMath`. |
| D16 | **Navigation:** category names are a public-API concern and are **not** propagated into `etrading`/`math` file or class names (those stay domain-oriented). The bridge is the `validation` layer: every wrapper is the identical public name plus a `try` prefix, foldered by category (Phase 3.5), plus a live `docs\api_map.csv` (Phase 3.6). Judged acceptable â€” see Â§"Navigation" note below Â§2.5. |
| D18 | **Category scheme (step 11):** categories are **singular** (`aqDate`, not `aqDates`); the handle marker is the word **`Object`** after the category (`aqBondObjectDirtyPrice`), a named sub-object skips it (`aqBondCurveYield`), generic lifecycle is `aqObject<Lifecycle>`. `Vols â†’ Volatility`. `AQ_XLL` category files are `aq<Category>.{cpp,h}` (`aqBond.cpp`, `aqDate.cpp`, `aqObject.cpp`, `aqTool.cpp`; `aqXllTools` is the XLL-layer utility, not a category) â€” **default, not absolute: `BondOption`/`BondFutureOption` are code-organized together in `aqBond.cpp`** (2026-09-11, `CLAUDE.md` Â§5.1a) because `aqBondOptionObjectCreate` is the only creator for both; consolidate a category's file into a sibling's only when they share the same underlying cached object, not merely a name prefix. `validation`, `AQ_API`, `GTEST` **test names** and `resources\test` fixtures follow â€” task 2.6. |
| D19 | **The `validation` wrapper name is the GOLDEN SOURCE** for every public function name. Wrapper = `try` + `<GoldenName>`; the XLL function, every binding method (Python / C# / Java / R) and the `GTEST` case name are `<GoldenName>` verbatim. Rename the wrapper first; the other surfaces follow. Enforced by `docs\api_map.csv` + `api_pair_check.py`. |
| D20 | **Editions are `AQ_XLL`-only (Nicholas, 2026-09-15).** Phase 4a closed via `Release`/`Release_XL_Bond`/`Release_XL_Swap`/`Release_XL_Credit`/`Release_XL_Curve` build configurations, gated at compile time. The `AQ_API` runtime edition-gate (`config\editions.json`/`licence.json`, a module-import registration gate, `aqToolEdition()`) is **dropped, not deferred** â€” `AQ_API` ships one full binary per language with every category always registered. |
| D21 | **C#/Java/R binding testing shelved permanently, not just blocked (Nicholas, 2026-09-15).** Reason: no current requirement to use these languages, and no test environment available to Nicholas (.NET/JDK/R toolchains not installed). Not a Phase 5 exit criterion any more â€” Phase 5 can close on Python alone. Demoted to a standing nice-to-have, revisited only if/when a client or use case actually requires one of these languages; not scheduled, not tracked against any phase gate. Python remains the one verified, supported binding. |

Open questions: none blocking. Phase 4 waits on the xlOil worked examples;
Phase 6.0 (Credit untangle) needs Nicholas's domain call on `CreditResults`.

### Pulled forward (done ahead of sequence, at Nicholas's request)

Commits on top of the `baseline` tag â€” `git reset --hard baseline` reverts all:

| Commit | What |
|---|---|
| `6c262aa` | Planning docs + Phase 0 inventories + `LICENSE`/`THIRD_PARTY_LICENSES.md`; `Mizuho International`/`MHI`/`MHSC` â†’ `AlgoQuantHub` in `src` comments (1359 files); `etrading.nuspec` rebranded. |
| `50cad24` | Removed 733 contractor file-header blocks (`@Author`/`@Department`/`ISD Front Office Development` + copyright) from `src/**/*.{cpp,h}`; re-encoded 90 Shift-JIS files to UTF-8; stripped ~4,800 Japanese chars from comments; 129 full-width punctuation â†’ ASCII. **Two string-literal fixes â€” verify on first build:** `LAPriceSZCalibration.cpp:330` `"LEVENBERG-MARQUARDT_METHOD"` (was a full-width minus â†’ the branch never matched; now it can â€” behavioural), and `LAMathCorrelation.cpp:1102` error text (full-width space â†’ space). |
| `767aa4e` | Neutralised hard-coded client config paths in `LACurveProperties.cpp` / `FolderConfig.cpp` (`M:\â€¦5858_mlibir\â€¦`, `S:\mizuho\â€¦`, `D:\appl\â€¦` â†’ `.\config\â€¦`). |

**None of the above touched a code identifier** (comments/encoding/dead-path
strings only, plus the two flagged lines). The systematic `MLIB*`/`me*`/`LA*`
identifier rename is still Phase 1â€“3 and still needs the baseline build + diff.

---

## Phase 0 â€” Baseline & safety net  â˜

Nothing else starts until this is green.

- â˜ **0.1** Confirm the authoritative tree builds **all 12 configurations**
  (`Debug|Release|Debug_EditAndContinue|Release_Profiler` + the per-language
  `*CSharp|*Java|*Python|*R` configs), x64. Record any config that does not, and
  why.
- â˜ **0.2** Confirm `AQ_EXTERNAL_LIB_PATH` resolves to a real external-libs tree
  from `LIBRARY_UPDATE\AQ_LIB` (the setup script's derived default assumes a
  sibling folder that is not there â€” override at the prompt if needed, and record
  the path used).
- â˜ **0.3** **Capture the pre-rebrand test baseline.** Build current `validation`
  with input/output recording on; run the full existing exercise set (bindings
  smoke tests, `GTEST`, the `resources\test` batch and Excel tests); save
  the recordings as `baseline/` outside the build tree. This is the diff target
  for every later stage. *(Nicholas â€” needs a build; see `rebrand\00_phase0_status.md`.)*
- â˜ **0.4** Git: commit current state, tag `baseline`. Adopt branch-per-stage +
  tag-before-stage for everything below.
- â˜‘ **0.5 `mir` call-graph** â€” `rebrand\callgraph_mir.md`. Result: `mir*` is a
  self-contained legacy binding stack (`AQ_API\mir*` â†’ `validation\tryMir*` â†’
  core). **No inbound calls from `me`/`aq` code** (the `tryMe*`â†’`mir*` hits are
  stale doc-comment headers only). Disposition: **remove wholesale** â€” 58
  `AQ_API\mir*` files, ~35 `validation\tryMir*` files, `mir*` entries in the 4
  `swig_*.i` + `swig_R_wrap.cpp`, and 156 `mir*` functions in the XLL
  `LAXL.cpp` (never ported â†’ simply not carried).
- â˜‘ **0.6 Removal map** â€” `rebrand\removal_map_client_specific.md`. Covers
  `msc*` (6 XLL files), `LoanCalculations`, `SupervisoryRules`, `CashflowClient`
  and the securitisation cluster in `etrading` they depend on
  (`CashflowEngine/Model`, `LoanPortfolio`, `SyntheticExcessSpread`, `Trigger`,
  `Tranche`, `PremiumLeg/Schedule`?, `CreditResults*`?). Ambiguous items
  (PremiumLeg / CreditResults â€” possibly shared with CDS `Credit`) are flagged
  for review, **not** auto-deleted.
- â˜‘ **0.7 653-function XLL inventory** â€” `rebrand\xll_function_inventory.csv`
  (653 rows: `old_name, old_file, name_stub, validation_wrapper_called,
  proposed_new_category, keep_drop_merge, notes`). 401 already have a `try*`
  wrapper; 252 do not (154 of those are `mir*` in `LAXL.cpp` â†’ drop). Category
  and keep/drop columns are for the Phase 2.3 pass.
- â˜‘ **0.8 Calendar-centre delimiter** â€” `rebrand\calendar_delimiter_sites.md`.
  The token is `:` via `LAString::toToken(':')` on `CURVEINPUT_CALENDAR` /
  `CURVEINPUT_BASISCALENDAR` (8 sites in `etrading\LACurveCalibrationHelpers.cpp`)
  and a `LAPriceDataCalendar` set. **It ships in generator JSON** â€”
  `SWAP_GENERATOR` values like `"SYB:LNB"`. It is **distinct** from the global
  `MULTI_STATIC_DATA_DELIMITER=':'` in `calibration\LADefinitions.h` (leave that
  alone). Collision risk with `:` inside calendar strings: none found so far â†’
  D12 "accept both" is viable; confirm during 3.4.
- â˜‘ **0.9** Interim `LICENSE` + `THIRD_PARTY_LICENSES.md` written at the library
  root (placeholder content; filenames + the header wording that references them
  are final).
- â˜‘ **0.10 Config & generator inventory** â€” `rebrand\inventory_config_generators.md`.
  `resources\config` holds **269 generator JSON** (132 SWAP, ~113 CURVE, 24
  BOND) + `.conf` registries, plus holiday data (`Calendar.csv/.conf`,
  `CBSchedule.csv`) and `.properties`. Format: `CACHED_OBJECT_TYPE` +
  `STRUCTURED_KEYS` column blocks (the AQObj serialisation format). Legacy
  artefacts to remove: a bundled `MLIBQ_ADDIN.xll`, `ir.properties`,
  `irsvr_excel.conf`. Python deploy already copies `resources\config`; XLL does
  not yet.

**Exit:** all 12 configs build, baseline recordings saved, tag `baseline`,
inventories 0.5â€“0.10 reviewed and signed off (0.5â€“0.10 drafted â€” awaiting
Nicholas's review), `LICENSE` placeholder in place (done).

---

## Phase 1 â€” Structural rebrand: projects, folders, macros, scripts  â˜

Mechanical, low-risk-per-step, high blast radius. Build + baseline-diff after
each numbered item.

- â˜‘ **1.1 Macros** `MLIB_* â†’ AQ_*` â€” commit `927a66d`. 380 files, 4867
  replacements, word-boundary anchored, generated wrappers excluded. Awaiting
  Nicholas's build + GoogleTest diff vs `baseline` (expect numerically identical).
- â˜‘ **1.2 `AQ_BINDINGS` â†’ `AQ_API`** (D3) â€” commit `988a3c1`. Folder + 3 project
  files (`git mv`, 168 src files), `.sln`, `<ProjectName>`/`<RootNamespace>`, 8
  batch files + their vars, pre/post-build commands. All vcxproj file refs verified
  on disk. Awaiting Nicholas's rebuild of the `*Python/*CSharp/*Java/*R` configs
  + generate/deploy. Original detail:
  - `projects\AQ_BINDINGS.vcxproj` (+ `.filters`, `.user`) â†’ rename file,
    `<ProjectName>`, `<RootNamespace>`.
  - `AlgoQuantLib-VS22.sln` â€” project name and path (GUID unchanged).
  - `src\AQ_BINDINGS\` â†’ `src\AQ_API\`.
  - `src\AQ_API\source\{generate,deploy}{Python,CSharp,Java,R}_2022.bat` â€” the
    hard-coded `...\targets\%1\%2\AQ_BINDINGS` path segment, the
    `src\AQ_BINDINGS\source` path, and the `AQ_BINDINGS_TARGET` /
    `AQ_BINDINGS_OUT` variable names.
  - `.vcxproj` `<PreBuildEvent>` / `<PostBuildEvent>` `<Command>` lines
    (every config) that call those scripts.
  - `AdditionalIncludeDirectories` entries pointing at the old folder.
  - SWIG `.i` files if they reference the path.
  - `Visualizer.natvis` if it names the project.
  - Rebuild every `*Python/*CSharp/*Java/*R` config; regenerate and redeploy.
- â— **1.3 File/folder renames** â€” commit `1377330`. Done: `InitializeMLibETrading.*`
  â†’ `InitializeAQETrading.*`, `InitializeMLibGoogleTest.*` â†’ `InitializeAQGoogleTest.*`
  (class + 124 includes + `etrading`/`GTEST` vcxproj), `mlib_ReadMe.txt`
  deleted, `getMLIBQEnvironmentVariable*` â†’ `getGoogleTestInputPath` /
  `getConfigFolderPath`, `MLib`/`MLIBQ` product refs in comments/messages â†’
  `AlgoQuantLib`. **Deferred:** `tryMe*` (206) and `me*` (78) file renames â€” folded
  into Phase 3 (the `meâ†’aq` identifier rename) to avoid churning `#include`s
  twice; `mir*` files go in 1.4. Awaiting Nicholas's build.
- âœ— **1.4 Remove `mir*`** â€” **DEFERRED, not a Phase 1 batch.** The revised 0.5
  call-graph (`rebrand\callgraph_mir.md`) found **49 `GTEST` files** and
  `etrading\Replay.cpp` depend on `tryMir*` (22 distinct functions). This is the
  flagship curve/trade regression suite â€” deleting `mir` means porting those
  tests to the `aq` equivalents and re-baselining each. Moved to **Phase 3c**
  (after the `meâ†’aq` rename gives the target names).
- âœ— **1.4a Remove client-specific credit modules** â€” **DEFERRED.** The revised
  0.6 map found the securitisation cluster (`CashflowEngine/Model`,
  `LoanPortfolio/Calculations/Validation`, `SyntheticExcessSpread`, `Trigger`,
  `Tranche`) is **entangled with `CreditResults`** (on the KEEP list):
  `CreditResults.h` includes six of these headers and holds a
  `shared_ptr<Trigger>`. Needs a "what is the `Credit` category" design pass
  first. Moved to **Phase 6.0** (Credit untangle). `SupervisoryRules` /
  `CashflowClient` / `LoanCalculations` `validation` wrappers go with it.
- â˜ **1.5 Delete `msc*`** (D2) â€” the 6 `msc*` add-in files live only in
  `.APPLES\...\MLIBQ_ADDIN`; the new-tree `AQ_XLL` is a POC that never had them.
  "Deletion" = **do not port them** in Phase 4. Nothing to remove now. Any
  `msc*` GoogleTest / resource references are handled in Phase 6.0 with the
  credit untangle.
- â˜‘ **1.6 Namespaces** `validation_api â†’ validation` â€” commit `cdceb61`. 524
  files, 2067 refs, word-boundary; no collision; wrap/.i files unaffected.
  Awaiting Nicholas's build + GoogleTest diff.
- â— **1.7 Config path resolution.** Client absolute paths in
  `LACurveProperties.cpp` / `FolderConfig.cpp` already neutralised â†’ `.\config\â€¦`
  (commit `767aa4e`). **Still to do, needs a build:** the `MLIBQ` env-var name
  (read via `getenv("MLIBQ")` / `toPath("MLIBQ", â€¦)` in both files, and named in
  `exposed_functions.cpp` error text) â€” rename to `AQ` or a dedicated
  `AQ_CONFIG`, and reconcile the two path conventions (`\resource\config\â€¦` in
  `FolderConfig` vs `\config\â€¦` / bare filename elsewhere). Also delete the
  stale `resources\config\MLIBQ_ADDIN.xll`. `LACurveProperties` and
  `FolderConfig` are near-duplicates â€” consider collapsing to one.
- â˜‘ **1.8 `.clang-format`** â€” commit (see below). `.clang-format` at `AQ_LIB`
  root, tuned to observed style: Allman braces, `UseTab: ForIndentation` /
  width 4 (tabs were the plurality â€” 59k vs 33k 4-space lines), `SpacesInParentheses`
  (`foo( arg )`, ~3:1 in the code), `PointerAlignment: Left`,
  **`ColumnLimit: 0`** and `SortIncludes: false` so it only normalises
  whitespace/braces and never rewraps â€” keeps the Phase 6.8 bulk-reformat diff
  small. `resources\.clang-format` with `DisableFormat: true` exempts examples /
  toolkits / the vendored AStyle tree. **No reformat run** â€” clang-format is a
  guide for edited files until Phase 6.8. `resources\utilities\ArtisticStyle`
  (AStyle, not wired to anything) is removed in the Phase 6.4 resources audit.

**Exit:** solution builds all configs; `grep -r "MLIB\|Mizuho\|validation_api\|msc"`
in `src\` is empty except third-party and string literals; baseline-diff clean.

---

## Phase 2 â€” Category taxonomy review  â˜  (D6, D14)

A design step, not a code step. Produce, circulate, agree â€” *then* Phases 3â€“5
apply it. **Â§2.2 below is the table Nicholas asked to review.**

- â˜ **2.1** Inventory every category token in use today:
  - XLL POC: `aqDate`, `aqTool`, `aqMath`.
  - CLAUDE.md target list: `AQ_dates`, `AQ_tools`, `AQ_curves`, `AQ_swaps`,
    `AQ_products`, `AQ_models`.
  - Old add-in de-facto categories (by file): `Dates`, `Curves`, `Swap`,
    `Products`, `Vols`, `Math`, `Utilities`, `LWO*`, `LoanCalculations`,
    `SupervisoryRules`, `CashflowClient`.
  - Old bindings: `Curve`, `Date`, `LWO`, `Product`, `Utility`, plus `mir*`.
  - `validation` wrappers: `tryMe` + `<Category>` + `<Func>`.
  - `GTEST`: test-suite naming.
- â˜‘ **2.2 Canonical category list â€” LOCKED (Nicholas).** **20** categories,
  **SINGULAR**, applied identically in `validation`, `AQ_XLL`, `AQ_API` and
  `GTEST` (test names included). Revised at step 9 (Options umbrella removed,
  sub-types promoted) and again at step 11 (plural â†’ singular; the handle marker
  moved from an `aqObj` prefix to the word `Object` after the category):

  | Category | Covers | Example |
  |---|---|---|
  | `Date` | calendars, schedules, IMM / central-bank dates, year fractions | `aqDateYearFraction` |
  | `Curve` | **rates yield-curve framework only** â€” build, calibration, interpolation, DF / zero / forward queries, cross-currency. *Not* bond or credit curves. | `aqCurveObjectForwardRate` |
  | `Volatility` | vol surfaces, surface SABR calibration, cap / swaption vol utilities | `aqVolatilitySabrImplied` |
  | `IR` (was `InterestRate`, renamed 2026-09-12) | Fixing tables, FRAs, rate-future <-> FRA conversion | `aqIRFixingTableValues` |
  | `Future` | futures ticker / date-code helpers, futures conventions | `aqFutureTickerNext` |
  | `Swap` | vanilla / OIS / basis / cross-currency â€” creation, legs, schedules, pricing, risk. **OIS is a product variant here, not its own category** â€” `aqSwapOis<Function>` (Â§2.2 note below); Asset / CMS / total-return swaps *do* have their own categories. | `aqSwapObjectParRate`, `aqSwapOisParRate` |
  | `Bond` | bond creation, price / yield, repo / basis / CTD, **bond-curve fitting** (govie / spread curves) | `aqBondObjectYield`, `aqBondCurveYield` |
  | `Credit` | CDS, **hazard-rate / survival (credit) curves**, par spreads | `aqCreditParSpread` |
  | `CapFloor` | caps / floors â€” creation, PV, greeks (incl. analytical) | `aqCapFloorObjectPV` |
  | `Swaption` | swaptions â€” PV, delta / gamma / theta / vega, implied vol | `aqSwaptionObjectPV` |
  | `BondOption` | options on bonds â€” PV, greeks | `aqBondOptionObjectPV` |
  | `BondFutureOption` | options on bond futures â€” PV, greeks | `aqBondFutureOptionObjectPV` |
  | `AssetSwap` | asset-swap spread, par-par, fixed-equivalent coupon | `aqAssetSwapObjectSpread` |
  | `CMS` | CMS pricing with convexity adjustment | `aqCMSObjectPVUsingConvexityAdjustment` |
  | `TRS` | Total return swap â€” PV, annuity, par rate / spread | `aqTRSObjectPV` |
  | `Inflation` | inflation curve build, CPI, zero-coupon inflation swaps | `aqInflationObjectZCSwapPV` |
  | `Math` | **low-level building blocks** â€” distributions, interpolation, root-finding, matrix ops | `aqMathNormalCdf` |
  | `Model` | term-structure / stochastic models, calibration sets, model-based / exotic / CMS-spread pricing, analytic (Jacobian) risk. May be sparse initially. | `aqModelHullWhiteCalibrate` |
  | `Generator` | list / describe / validate the JSON instrument & model static-data templates | `aqGeneratorList` |
  | `FX` | FX forwards and FX swaps derived from discount / xccy curves | `aqFXForward` |
  | `Tool` | echo, build stamp, edition report, diagnostics, memory dump, CSV load | `aqToolBuildTime` |

  **Name forms:** `aq<Category><Fn>` stateless Â· `aq<Category>Object<Fn>` handle
  API on the category's product Â· `aq<Category><SubObject><Fn>` handle API on a
  named sub-object (Curve, Generator, MarketData, Model, FixingTable) â€” the
  sub-object already denotes an object, so `Object` is not repeated Â·
  `aq<Category><Variant><Fn>` a same-category **product variant** that doesn't
  earn its own category (`aqSwapOisPV`, `aqSwapOisParRate` â€” `Object` still not
  repeated for the stateless form) Â·
  `aqObject<Lifecycle>` generic handle lifecycle (`aqObjectLoad`, `aqObjectSave`,
  `aqObjectClearCache`).

  **Decisions folded in:** `Curve` = rates yield-curve framework only, bond-curve
  fitting â†’ `Bond`, hazard/survival â†’ `Credit`. **No `Options` umbrella** and
  **no `Object` category** â€” the handle API is the `Object` word after the
  category. **`Volatility`** (full word; supersedes the earlier `Vols` and the interim `Vol`). **`Math` kept.**
  **`Model` added.** **`Products` dropped.** `LoanCalculations`,
  `SupervisoryRules` and `msc*` are **removed** (client-specific â€” Phase 1.4a).
  **`Ois` folded into `Swap` as a product variant** (2026-09-11, superseding the
  brief period it was its own category â€” see 2.7 below).
- â˜ **2.3** Record the locked list in `CLAUDE.md` Â§5.1 (done). Map every row of
  the 0.7 inventory (`rebrand\xll_function_inventory.csv`) to one of the 20
  categories or to drop/merge.
- â˜‘ **2.4 Edition â†’ category map.** Realised as `AQ_XLL` build configurations
  (Phase 4a, done 2026-09-15) â€” `Release_XL_Swap`/`Release_XL_Bond`/
  `Release_XL_Credit`/`Release_XL_Curve`/`Release` (Full), each `Core` plus
  the `Optional` files below:

  | Edition | Registers |
  |---|---|
  | Swap | Date, Curve, Volatility, IR, Swap, Math, Generator, Object, Tool |
  | Bond | Date, Curve, IR, Bond, Math, Generator, Object, Tool |
  | Credit | Date, Curve, Credit, Bond, Math, Generator, Object, Tool |
  | Curve | Date, Curve, IR, Math, Generator, Object, Tool |
  | Full | all 21, incl. options and Model |

  No `AQ_API` equivalent â€” that half of the original straw man was dropped
  (Phase 4a).

- â˜‘ **2.5 Generator categories** (Nicholas). `Generator` category is
  **introspection only** â€” `aqGeneratorList` / `â€¦Describe` / `â€¦Validate`. The
  construction methods (`aqSwapFromGenerator`, `aqBondFromGenerator`, â€¦) live in
  their **asset categories**, not in `Generator`.
- â˜‘ **2.6 Apply the singular scheme across all four surfaces.** *(DONE â€”
  built green with GTest passing, Nicholas 2026-09-09; uncommitted pending the
  staged commit.)* `validation` wrappers, `AQ_API` (SWIG `.i`), `GTEST` **test
  names**, `docs\api_map.csv` and every `resources\test` fixture file renamed to
  the 22-category singular golden-source scheme (`Vol`â†’`Volatility`, `Future` +
  `Ois` added). `AQ_XLL` call sites re-synced. Follow-on in the same delta:
  the AQ_XLL `Tool` + `Object`-lifecycle port from `.APPLES\...\meUtilities.cpp`
  (~55 `XLO_FUNC` functions across `aqObject.cpp` / `aqTool.cpp` + `aqXllTools`
  marshalling helpers), and `Interpolation` / `PCA` moved `Tool` â†’ `Math`
  (`aqMathInterpolation` / `aqMathPCA`, `tryAqMath*`). Detail + resume plan in
  `rebrand\STATUS.md`.
- â˜‘ **2.7 `Ois` folded into `Swap` as a product variant** (Nicholas,
  2026-09-11) â€” supersedes the `Ois` category added in 2.6; category count
  22 â†’ **21**. `Ois` is a vanilla swap with an overnight-compounded floating
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
  `aqSwapOisPV`/`aqSwapOisParRate`, per Â§5.1a's validation â†’ GTEST â†’ API â†’
  XLL order (the `AQ_XLL` functions themselves land with the rest of `Swap`).
  Detail: `rebrand\STATUS.md`.

**Exit:** agreed category list + editionâ†’category map in `CLAUDE.md`; 0.7
inventory fully categorised.

### Navigation â€” category names vs internal names

**The concern (Nicholas):** the public XLL/API categories (`Dates`, `Curves`,
`Swaps`, â€¦) are *not* propagated into the `etrading` / `math` file and class
names (`LACurve*`, `LADate*`, engines, models), so it is hard to go from a
function seen in Excel to the code that implements it.

**Assessment: real friction, not a blocker â€” and forcing categories down into
the implementation layer would make things worse, not better.** Reasons:

- `etrading` / `math` are organised by *C++ design* â€” a curve class, a
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
2. **`docs\api_map.csv`** â€” live, shipped: `public name | category | validation
   wrapper | etrading entry point | test`. One `grep` from any name to its code.
3. **CI guard** (Phase 7.4): every `aq*` public function must have a matching
   `tryAq*` wrapper row â€” keeps the index honest.
4. Solution Explorer **filters** in `AQ_XLL` / `AQ_API` stay category-grouped
   (they already are).

`etrading` / `math` file and class names get the `LAâ†’AQ` prefix swap and nothing
else â€” they stay domain-oriented.

---

## Phase 3 â€” Identifier rebrand + calendar delimiter  â˜

The big one. **`LA` â†’ `AQL`** ("Legacy Analytics" â†’ "AQ Legacy" â€” keeps the
legacy tree marked and greppable), `MA` / `MB` â†’ `AQ` (confirm per project),
plus `me*â†’aq*` function bodies, plus `LWOâ†’AQObj`. Anchored, case-sensitive, from an
approved list only (CLAUDE.md Â§5.5). One project per batch, in dependency order:
`math â†’ models â†’ calibration â†’ etrading â†’ validation â†’ AQ_API`. Build +
baseline-diff between **every** batch.

- â— **3.1** For each project: `rebrand\tools\prefix_census.py <project>` â†’
  review â†’ sign-off â†’ `git mv` the `LA*` files to `AQL*`, apply the approved
  identifier map (identifiers + include-guard macros + error-string text), fix
  `#include "LA*.h"` **tree-wide**, update `.vcxproj`/`.filters` + natvis + SWIG
  `.i` â†’ build all projects â†’ baseline-diff.
  - â˜‘ **`math`** â€” commits `268ed8e` + `608a79f` fix. 204 `LAâ†’AQL`, 235 files.
    Build-verified green (clean rebuild). One leak fixed: `LACoreComponentManager`
    (a `models` file used tree-wide) â€” its `git mv` was missed; done in `608a79f`.
  - â˜‘ **`models`** â€” commit `15790a98`. **578 `LAâ†’AQL`** (approved map =
    every `LA*` file stem âˆª owned tokens â€” the fix for the `math` leak class),
    **552 files** `git mv`, tree-wide **744 files / ~20,250 replacements**. 11
    vcxproj-casing merges (`PayOff` vs git's `Payoff`). SKIP `MANUAL`/`MANAGER`
    (English `#define`s). Verified: 0 broken includes / missing vcxproj / artifacts.
    Awaiting Nicholas's clean rebuild + GTest diff.
  - **Follow-up (Phase 3 guard sweep):** `__LAxxx_H__` / `#ifndef LAxxx_h`-with-`_`
    style include guards are not caught by the `\bLA` anchor â€” cosmetic, functional;
    normalise in one pass after all project batches.
  - Tooling now: `rebrand\tools\prefix_census.py` (stemâˆªtoken, fast one-pass) +
    `rebrand\tools\run_batch.py` (git mv + `\b`-replace + vcxproj-casing merge +
    verify gate that aborts+reverts on any lock/mismatch).
- â˜ **3.2 `LWO â†’ AQObj`** (D4, D15). Two distinct things:
  - **C++ object-framework classes** get the `AQObj` prefix (AlgoQuant Object):
    `LWOCurve â†’ AQObjCurve`, `LWOCurveDayAdjustment â†’ AQObjCurveDayAdjustment`,
    `LWOUtilities â†’ AQObjUtilities`, `HandleEnums â†’ AQObjHandleEnums`,
    `IsLWOObject.{h,cpp}` â†’ `AQObjectPredicates.{h,cpp}` with the free function
    `isLWOObject() â†’ isAQObject()`. Folded into the `etrading` batch. Handle
    behaviour (counter, cell-hash, recalc suffix) byte-for-byte unchanged.
    **Never `AQObjbject`** â€” when renaming `AQO`â†’`AQObj`, exclude tokens that
    already contain `AQObject` (`AQObjects`, `IsAQObject`, `isAQObject`). Note the legacy
    `math` class `LAObject` becomes `AQLObject` (distinct from the framework's
    `AQObject` / `AQObj*`), so no clash.
  - **Public function names carrying `LWO`** become `aq<Category>Object<Fn>` â€”
    the word `Object` after the SINGULAR category is what separates the handle
    API from its stateless twin:
    - object-lifecycle ops (`meLWOLoad/Save/Copy/Modify/Delete/Clear/List`) â†’
      **`aqObjectâ€¦`** with no category word (`aqObjectLoad`, `aqObjectCopy`, â€¦)
    - handle-based pricing/creation (`meLWOSwapPV`, `meLWOBondPrice`, â€¦) â†’
      **`aq<Category>Object<Fn>`** (`aqSwapObjectPV`, `aqBondObjectPrice`)
    - functions on a named sub-object (`meLWOCurveMarketDataDisplay`,
      `meLWOBondCurveâ€¦`) â†’ **`aq<Category><SubObject><Fn>`**, no `Object` word
      (`aqCurveMarketDataDisplay`, `aqBondCurveYield`)
  - `tryMeLWO*` validation wrappers follow the same rule with a `try` prefix.
- â˜ **3.3** Function prefixes `me* â†’ aq*` / `tryMe* â†’ tryAq*` in `validation` and
  `AQ_API`, plus SWIG `.i`. This is the **clean break** (D9): no `me*` spelling
  survives anywhere, no forwarding aliases. Collect the removed public names into
  a `RELEASE_NOTES` "renamed / removed functions" list as you go.
- â˜ **3.5 `validation` layer as the category index** (navigation â€” see below).
  Rename every wrapper to `tryAq<Category>â€¦<Function>` (the identical public name
  plus `try`) and move it into `src\validation\<category>\` (folder + `.filters`),
  where `<category>` is one of the locked 20 (singular). This makes
  `validation\Curve\` etc. the browsable
  table-of-contents from any public name to its `etrading` entry point.
  `etrading` / `math` file names stay **domain-oriented** (organised by class /
  engine / model, not by API category) â€” do not force them to match.
- â˜ **3.6 Live API map** â€” regenerate `rebrand\xll_function_inventory.csv` (rename
  it `docs\api_map.csv`, keep it shipped) with the final columns:
  `public name | category | validation wrapper | etrading entry point | test`.
  One `grep` from "where is `aqCurveObjectForwardRate`". A CI check (Phase 7.4)
  asserts every `aq*` public function has a matching `tryAq*` wrapper row.
- â˜ **3.4 Calendar delimiter** (D5, D12 â€” `+` only): add
  `constexpr char CALENDAR_CENTRE_DELIMITER = '+';` and a single
  `splitCalendarCentres()` helper (structured so re-enabling `:` is a one-line
  toggle, but shipping `+`-only). Route through it: the 16
  `chgrow(...CALENDAR...).toToken(':')` sites in
  `etrading\LACurveCalibrationHelpers.cpp` + `models\LAMathCurveGenerateFuncUtility.cpp`,
  `LAPriceDataCalendar::convertFromString`, and the NYB-strip rebuild in
  `LAMathDateCalculations.cpp` (rebuild with `+`). **Leave every other
  `toToken(':')` alone** â€” `:` stays the generic list separator. Migrate the
  ~103 generator JSON files' **calendar fields only** to `+` (curve-name-list
  fields keep `:`). `GTEST`: `"SYB+LNB"` builds the expected combined
  holiday set. Details: `rebrand\calendar_delimiter_sites.md`.

**Exit:** `grep -rE "\bLA[A-Z][a-z]|\bLA[0-9]D|\bme[A-Z]"` in `src\` returns only
third-party / literals (`mir*` still present â€” retired in 3c; `AQL*` is the
intended legacy marker and is expected); all configs build; baseline-diff
**numerically identical** (renames are behaviour-preserving â€” any delta is a bug,
and the calendar change is the only intentional behaviour change, covered by its
own new test).

---

## Phase 3c â€” Retire `mir*`  â˜

`mir*` is the legacy Interest-Rates binding stack. 58 `AQ_API\mir*.{cpp,h}`,
65 `validation\tryMir*.*`, `mir*` blocks in 4 `swig_*.i` + `swig_R_wrap.cpp`.
**49 `GTEST` files** and `etrading\Replay.cpp` use 22 distinct `tryMir*`
functions (details: `rebrand\callgraph_mir.md`).

- â˜ **3c.1** For each of the 22 `tryMir*` functions, map to the `tryAq*`
  equivalent (post-Phase-3 names): `tryAqProductSwapPv`, `tryAqProductOisParRate`,
  `tryAqCurveCalibrate*`, `tryAqDate*`, `tryAqObjectsCurve*` etc. Any with no
  equivalent â†’ promote that one `mir` function to `aq` (rename + keep), don't
  drop the capability.
- â˜ **3c.2** Migrate the 49 `GTEST` files to the `tryAq*` calls, **one
  file at a time, re-baselining each** â€” numbers must match the pre-migration
  run for that test.
- â˜ **3c.3** `Replay.cpp` â€” drop the four `"tryMirSetUpâ€¦Curve"` `functionList`
  keys (the `"tryAqCurveCalibrate*"` keys already point at the same
  `replayCurve*` functions).
- â˜ **3c.4** Delete all `mir*` / `tryMir*` files + swig `.i` blocks +
  `.vcxproj`/`.filters` entries. Regenerate `swig_R_wrap.cpp`. Reword the two
  `mirCurveGenerate` error strings.

**Exit:** `grep -rE "\bmir[A-Z]|tryMir"` in `src\` is empty; suite green and
baseline-identical.

---

## Phase 4 â€” xlOil XLL port  â˜‘ (essentially complete, 2026-09-11)

**Status (2026-09-15): 470 `AQ_XLL` functions ported, covering 460 of 470
`validation` wrappers.** The 9 unmatched wrappers are deliberate exclusions
(1 false-positive name collision, 1 in-place mutator that doesn't fit a
worksheet-function shape, 7 legacy SABR functions Nicholas decided to leave
out as superseded) â€” not gaps; a 10th apparent miss is a script artifact
(trailing-space regex quirk), not a real one. Detail and the re-runnable
audit script: `rebrand\STATUS.md` and `STATUS.md` Â§1a/Â§2. `Generator` was
built from scratch this session (Â§4.7 below). `Model` is the only category
with **zero validation wrappers today**, and stays that way until a design
brief exists â€” unlike `Generator`, there is no legacy source and nothing on
disk to introspect.

Wait for Nicholas's xlOil worked examples before starting â€” they define the
canonical marshalling / handle-I/O / array-return / error-convention pattern.

- â˜ **4.1** Port the **AQObj handle framework** from `.APPLES\...\etrading`
  (`LWOCurve`, `IsLWOObject`, `HandleEnums`, `LWOUtilities`,
  `StructuredExceptionHandler`) into `src\etrading`, rebranded, behaviour
  preserved. Unit-test the counter/hash/recalc-suffix logic directly.
- â˜ **4.2** Audit `XllPlusTips*` â†’ keep/drop list (CLAUDE.md Â§6.1). Port the
  kept helpers onto `ExcelObj` / `ExcelArray`. Delete the XLL+ boilerplate.
- â˜ **4.3** Write the porting template doc from the examples: old `_Impl` â†’
  `XLO_FUNC_START/END` + `.help()/.arg()`, plus the required `validation`
  wrapper + `GTEST` case per function.
- â˜ **4.4** Remove the dead `XllPlus\7.0\...` include/lib paths from
  `AQ_XLL.vcxproj`.
- â˜‘ **4.5 Editions & manifest gating** â€” see **Phase 4a** below, done
  2026-09-15. `AQ_XLL`-only, not shared with `AQ_API` â€” that half was
  dropped, not built (see Phase 4a).
- â˜ **4.6** **Clean break confirmed at registration** (D9): register `aq*` names
  only; no `me*` aliases, hidden or otherwise. Finalise the `RELEASE_NOTES`
  renamed/removed-function list started in 3.3 so users can find replacements.
- â˜‘ **4.7** Port functions **category by category** in the CLAUDE.md Â§9.4 order
  (Date â†’ Tool â†’ Curve â†’ Swap â†’ Bond â†’ Credit â†’ IR â†’ Volatility â†’ options â†’
  Math â†’ Model â†’ Generator â†’ Object), driven by the 0.7 inventory and the
  locked Phase 2 categories. Per function: confirm it is a "keep" â†’ port `_Impl`
  â†’ `validation` wrapper â†’ `GTEST` case â†’ tick the inventory row. Build +
  run suite per category. **Done (2026-09-11) for every category with an
  existing `validation` surface** â€” `Date, Curve, FX, Inflation, Volatility,
  IR, Future, Swap, AssetSwap, CMS, TRS, CapFloor, Swaption,
  BondOption, BondFutureOption, Bond, Credit, Math, Tool, Object(-lifecycle)`
  all ported, build green, tests pass. **`Generator` done (2026-09-15)** â€”
  `tryAqGeneratorList`/`Display`/`Validate` designed and written from
  scratch (no `validation` surface existed to port from) across all four
  surfaces; see `STATUS.md` Â§1a and `rebrand\STATUS.md`'s top entry.
  `Model` remains: **0 `validation` wrappers**, no legacy source, no design
  brief â€” blocked on Nicholas's input (which model types, what parameters),
  not on engineering time.
- â˜‘ **4.8 Generator + config wiring** â€” introspection (`List`/`Display`/
  `Validate`) done 2026-09-15; see `STATUS.md` Â§1a. Any remaining
  `config`-packaging work for the XLL/API deploy paths stays open under
  **Phase 4b**.
- â˜‘ **4.11 `AQ_XLL` file naming** (Nicholas): every category file is
  `aq<Category>.{cpp,h}` â€” `aqBond.cpp`, `aqDate.cpp`, `aqMath.cpp`,
  `aqTool.cpp`, `aqObject.cpp`, `aqMain.{cpp,h}` â€” with **one exception**,
  `aqXllTools.{h,cpp}`, the shared Excel-side helper library (marshalling +
  AQObj handle decoration). `XllTools` is not a category â€” the `Xll` infix
  marks it as the XLL-layer utility file, distinct from the `Tool` category
  file `aqTool.cpp`. Do the same in `AQ_API` where practical.
- â˜ **4.9 Record / playback / rebase on EVERY validation function**  (Nicholas,
  2026-09-06). The infrastructure already exists and is proven â€” `RecordMacros.h`
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
    `AQ_RECORD_OUTPUTS` needs no rebase logic â€” do not add any.
  - Coverage today: 57 of 101 validation source files use the macros, ~52 carry
    hand-rolled `if (CreateDataFile::recordEnabled()) { ... }` blocks, and some
    functions have neither.
  - âš  **18 hand-rolled INPUT recordings use a key that is NOT the parameter name**
    (`key=staticDataTable, expr=curveName`; `key=convexityAdjConv,
    expr=tenorBasisConv`; ...). Converting those to the macro silently changes
    the fixture key and breaks the test at run time, build still green. Convert
    them by hand, or rename the parameter to match the key, and re-run
    `rebrand/tools/fixture_key_check.py`. The other ~453 hand-rolled pairs
    already agree and convert mechanically.
  - ðŸž Found while auditing: `tryAqObjRatesFixingTable.cpp` writes
    `file.write("fixingValues", fixingDates)` and
    `file.write("fixingDates", fixingValues)` â€” **the two keys are swapped**.
    Pre-existing; the recorded fixtures have dates under the values key. Fix the
    code and rebase those fixtures together, never separately.
  - Audit the existing `tryAq*` surface and add the macros where they are absent;
    that is what turns a recorded workbook into a `GTEST` case for free, and it is
    the mechanism behind the whole fixture suite.
  - âš  **The recorded key is the STRINGIFIED C++ PARAMETER NAME** â€”
    `WRITE_PARAMETER(P)` expands to `file.write("P", P)`. Renaming a parameter
    silently breaks every fixture that records it: the build stays green and the
    test fails at run time with `ReadDataFile::Load: unknown key`. This bit us in
    step 8B. Guard: `rebrand/tools/fixture_key_check.py`, run after any rename
    that touches validation parameters.
- â˜ **4.12 Structured exception handling for `AQ_API` too** (Nicholas asked
  2026-09-06 whether SEH covers the API as well as Excel â€” it does not, fully).
  `VALID_EXCEPTION_START` installs the handler on every `tryAq*`, so **any**
  caller â€” XLL, Python, C#, Java, R, GTEST â€” is protected for the part that runs
  inside `validation`. But `AQ_API_START` (`src/AQ_API/source/APISetUp.h`) is a
  bare `try {`: **no `StructuredExceptionHandler`**. So a fault in AQ_API's own
  marshalling â€” SWIG conversion, building result vectors â€” is unprotected and
  will take down the host process (the Python interpreter, the CLR, the JVM).
  Fix symmetrically with `AQ_XLL_GUARD`: add the handler to `AQ_API_START`.
  Note `AQ_API` must build `/EHa` for this to work â€” check before relying on it.
- â˜ **4.10 Structured exception handling across `AQ_XLL`**  (Nicholas, 2026-09-06).
  An access violation, stack overflow or divide-by-zero inside a worksheet
  function is a Windows structured exception, not a C++ one; unhandled it takes
  Excel down with it. `etrading::StructuredExceptionHandler` installs a
  `_set_se_translator` for its lifetime and turns those into C++ exceptions that
  xlOil's `XLO_FUNC_START/END` returns to the cell as an error string.
  - `validation` is already covered â€” `VALID_EXCEPTION_START` installs the
    handler (plus a `ThreadGuard`) on every `tryAq*` call.
  - The gap was the **marshalling** either side of the validation call, which
    `validation` never sees: reading a malformed Excel range, or building the
    result array.
  - **Rule going forward: `AQ_XLL_GUARD` (in `aqXllTools.h`) is the first line of
    every AQ_XLL worksheet function.** Applied to `aqDateFromTenor` and
    `aqDateFromYearFraction` as the reference pattern.
  - Requires `/EHa`; all projects already set `<ExceptionHandling>Async`. Do not
    change that â€” the translator silently stops working under `/EHsc`.
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

## Phase 4a â€” Editions & manifest gating  â˜‘ (done, 2026-09-15)

`AlgoQuantLib` ships as **Bonds / Swaps / Credit / Curves / Full** editions.
**Decided (Nicholas, 2026-09-15): `AQ_XLL`-only, gated at compile time. The
`AQ_API` runtime-gate half of this phase (below) is dropped, not
deferred** â€” `AQ_API` ships one full binary per language with every
category always registered; there is no edition concept on that surface.

**`AQ_XLL` (decided, Nicholas 2026-09-11; shipped 2026-09-15) â€” per-edition
build configurations, gated at compile time.** A native `.xll` per edition
is cheap for one add-in: `projects\AQ_XLL.vcxproj.filters` splits every
`src\AQ_XLL\src\*.cpp` file into **`src\Core`** (always-built:
`aqXllTools.cpp`, `aqMain.cpp`, `aqDate.cpp`, `aqObject.cpp`, `aqMath.cpp`,
`aqTool.cpp`, plus `aqCurve.cpp` and `aqIR.cpp` â€” promoted 2026-09-12, see
4a.5) and **`src\Optional`** (one file per product category â€” every new
category file goes here). Five build configurations exist alongside
`Debug`/`Debug_EditAndContinue`/`Release_Profiler`: **`Release`** (Full,
excludes nothing) and **`Release_XL_Bond`**, **`Release_XL_Swap`**,
**`Release_XL_Credit`**, **`Release_XL_Curve`** â€” each compiles `Core` plus
only the `Optional` file(s) that edition needs (excluded from build for
every other configuration). A further `Release_XL_Manifest` configuration
layers a function-level cut on top via `resources\manifest\activeList.txt`
(renamed 2026-09-15 from `active.txt`, alongside `demo.txt`â†’`demoList.txt`;
see `rebrand\STATUS.md` for that tooling) â€” a finer cut than the edition
mechanism, not a sixth edition. Detail: `CLAUDE.md` (both) Â§4.4/Â§4.5,
`AQ_LIB\CLAUDE.md` Â§6.3.

- â˜‘ **4a.5** `AQ_XLL` per-edition build configurations
  (`Release_XL_Bond`/`Release_XL_Swap`/`Release_XL_Credit`/`Release_XL_Curve`),
  each excluding the `src\Optional` files its edition doesn't need â€” **done,
  confirmed building green**.
  **Dependency check flagged 2026-09-12, resolved same day:** `aqCurve.cpp`
  sat in `src\Optional`, but every priced product category depends on it
  (bonds/swaps/caps all discount off a curve) â€” a `ReleaseBonds` config that
  excluded it would ship a Bond edition that can't build or calibrate the
  curve it needs to price against. **Fixed: Nicholas promoted `aqCurve.cpp`
  and `aqIR.cpp` (was `aqInterestRate.cpp` â€” fixing tables/FRA conversions
  cross-cut the same way) into `src\Core`**, committed. A "Curves-only" SKU
  falls out naturally as a `Core`-only build with no `Optional` files added.

**Dropped, not carried forward (was 4a.1â€“4a.4):** the `AQ_API` runtime
edition manifest â€” `config\editions.json` mapping edition â†’ categories,
`config\licence.json` entitlement, a module-import registration gate, and
an `aqToolEdition()` reporting the active edition/category list at runtime.
None of this is being built; `AQ_API`'s bindings register every category
unconditionally, in every language, always.

---

## Phase 4b â€” Config folder & generators  â˜

The `config` folder ships with **both** the `AQ_API` and `AQ_XLL` packages and is
the library's no-recompile customisation surface:

- **Holiday / calendar files** (from MarketWire / SwapsWire) â€” loaded at runtime;
  end-users correct / add / modify holidays without a refreshed add-in. This is
  the existing "calendars without recompilation" property; the `config` folder is
  the mechanism.
- **Generators** â€” JSON **instrument & model static-data templates**. Static data
  is ~90% of a trade booking; a generator carries the conventions (calendars, day
  counts, rolls, fixing / payment lags, currency defaults, model params) so a
  user books e.g. a USD swap from `notional, effective, maturity, fixed rate,
  pay/receive`, or a US Treasury from `notional, issue/settle, maturity, coupon`,
  instead of hand-assembling conventions most users are unfamiliar with.
  Generators are consumed by the AQObj object layer: generator name + a few
  overrides â†’ object handle. Existing seeds live under
  `resources\config\{SWAP,BOND,CURVE}_GENERATOR`.

- â˜ **4b.1** Document the generator JSON schema; add a schema-validation
  `GTEST` over the shipped set (the `tryAqGeneratorValidate` function built in
  4b.2 below re-uses the object framework's own deserialization + key
  validation rather than re-implementing a schema check â€” a hand-written
  schema doc plus a dedicated schema-conformance `GTEST` sweep across the
  full shipped set, beyond the three seed files exercised in `TestAqGenerator
  *.cpp`, is still open).
- â˜‘ **4b.2** `Generator` category (2.5, singular per Â§5.1 â€” corrects this
  line's earlier `Generators` typo): `aqGeneratorList` / `â€¦Display` /
  `â€¦Validate` **done 2026-09-15** (`Display`, renamed same-session from an
  initial `Describe` to match the pre-existing per-asset `*GeneratorDisplay`
  naming) across `validation`/`GTEST`/`AQ_API`/
  `AQ_XLL` â€” see `STATUS.md` Â§1a. The per-asset `aqSwapFromGenerator` /
  `aqBondFromGenerator` construction functions already existed pre-session
  and are unaffected.
- â˜ **4b.3** Confirm `config` is packaged by **both** deploy paths. Python
  already does (`deployPython_2022.bat` copies `resources\config`); add the same
  for `AQ_XLL` and the other languages.
- â˜ **4b.4** Rebrand the generator JSON and calendar config â€” legacy names,
  client-specific conventions and defaults (folded into the 6.4 resources audit).
- â˜ **4b.5** One end-to-end test per asset: book from a generator + minimal
  fields, price, check against the baseline.

---

## Phase 5 â€” Bindings & test coverage  â˜

- â˜‘ **5.1a** `AQ_BINDINGSâ†’AQ_API` rename (D3) â€” done, project renamed and
  Python verified green end-to-end.
- ~~â˜ **5.1b** Exercise C#, R, Java~~ â€” **shelved permanently (D21,
  2026-09-15), not a Phase 5 exit criterion.** No current requirement to use
  these languages and no test environment available. Revisit only if/when a
  client or use case actually needs one of them â€” at that point, generate,
  build, deploy and run the relevant `resources\api\*` test app before
  calling that language verified. Until then this is a standing nice-to-have,
  not tracked work.
- â˜ **5.2** ~~Wire `AQ_API` module import to the Phase 4a edition gate~~ â€”
  **dropped with the `AQ_API` edition gate itself (Phase 4a, 2026-09-15)**.
  Ship `config` (calendars + generators, no `editions.json`/`licence.json`)
  with every language package (4b.3).
- â˜ **5.3** Fill priority `GTEST` gaps:
  - Yield-curve framework + Jacobian risk vs bump-and-revalue.
  - Fixed-income price/yield vs Bloomberg (both directions) â€” regression lock.
  - Date logic (it moved out of legacy in Phase 6).
  - Generator round-trips per asset (4b.5).
- â˜ **5.4** Wire the `validation`-recording â†’ `GTEST` generation so new
  wrappers get cases automatically.

**Exit:** Python builds and passes (D21 â€” C#/Java/R shelved, not a gate); new
coverage merged; suite green against baseline.

---

## Phase 6 â€” Legacy extraction, resources, docs, licence  â˜

- â˜ **6.0 Credit untangle** (was 1.4a). Decide what the `Credit` category is in
  the rebranded library, then act. `etrading\CreditResults.h` (CDS
  result-reporting) `#include`s six securitisation-cluster headers
  (`LoanPortfolio`, `LoanCalculations`, `SyntheticExcessSpread`, `CashflowEngine`,
  `Trigger`, `Tranche`) and holds a `shared_ptr<Trigger>`; `Trigger.h` includes
  `LoanCalculations.h`; `validation\tryMeUtilitySetup.cpp` uses
  `CreditResultsContainer`. Two outcomes (Nicholas's call):
  (a) `CreditResults` is securitisation code mis-labelled CDS â†’ delete
  `CreditResults*` + the whole cluster + `tryMeCashflowClient/LoanCalculations/
  SupervisoryRules` wrappers, adjust `tryMeUtilitySetup`;
  (b) it is real CDS reporting with vestigial loan/trigger/tranche members â†’
  strip those members, then the cluster + client wrappers delete cleanly.
  Either way: `SupervisoryRules`, `CashflowClient`, `LoanCalculations` go; `msc*`
  GoogleTest / resource references go. Build + baseline-diff after.
- â˜ **6.1** Usage-map `calibration` / `math` / `models` â€” what is reachable from
  `validation` / `etrading`?
- â˜ **6.2** Extract the live pieces (dates, strings, math utilities) into a clean
  `core` / `utils` project under `etrading`, rebranded. Point callers at it.
- â˜ **6.3** Delete the dead remainder â€” one project at a time, build between.
- â˜ **6.4** **`resources\` and `examples\` audit** (D13). Every spreadsheet,
  toolkit, config, workshop and research guide: rebrand, rewrite, or remove.
  Includes the `config` folder â€” calendar/holiday files and generator JSON
  (4b.4): strip legacy names and client-specific conventions. Nothing
  legacy-branded survives; broken or client-specific examples are cut, not
  carried. **Also: regenerate `Calendar.csv` / `Calendar.conf`** â€” currently
  a stopgap (`LastCalendarUpdate` bumped 2026-09-15 to silence
  `Calendars.UNIT_Expiry_Test` without actually refreshing `Calendar.csv`,
  still dated 2022-07-22; ~10 GTEST failures reported 2026-09-15, likely
  from this staleness â€” see `STATUS.md` top section). The `CDWCalendarUpdate.bat`
  tool this line used to point to was **never ported into `AQ_LIB`** â€” it
  only exists in `.APPLES\APPLE\resource\utilities\BatchFiles\CDWCalendarUpdate\`,
  and is a Python script pulling from a paid MarketWire/SwapsWire endpoint.
- â˜ **6.4a Calendar sourcing strategy** (new, 2026-09-15 â€” Nicholas: SwapsWire/
  MarketWire aren't free; look for a public/cheaper alternative before
  committing to a refresh mechanism). Needs a decision before 6.4's
  `Calendar.csv` regeneration can be more than a one-off stopgap:
  - **Candidate: QuantLib's built-in `Calendar` classes** (`ql/time/calendars/*`)
    â€” already a linked dependency (`AQ_EXTERNAL_LIB_PATH\...\QuantLib-1.43`),
    permissive licence, no new cost. Algorithmic rules (Easter-based, nth-
    weekday) self-extend forever; hardcoded exception lists only update via
    a QuantLib version bump (community-maintained, lags reality â€” see
    `STATUS.md`/conversation 2026-09-15 for the full explanation). Covers a
    meaningful subset of `Calendar.csv`'s ~110 centre codes (major
    currencies/markets) but not the niche settlement/clearing codes
    (`BMA`, `CMF`, `KdQ`, `SyF`, ...) that look SwapsWire-specific.
  - **Candidate: public sources per centre** (central bank / exchange
    published holiday lists â€” ECB TARGET calendar, Federal Reserve, gov.uk,
    exchange sites) for the codes QuantLib doesn't cover. Manual/scraped,
    not a single feed.
  - **Open question:** first inventory which of the ~110 `Calendar.csv`
    centre codes actually matter (are used by any shipped generator /
    booked product) vs. dead weight from the original client's book â€” no
    point sourcing calendars nobody needs.
  - Whatever is chosen, keep the existing wide-CSV format and the "calendars
    without recompiling" property (`CLAUDE.md` Â§9.2/Â§4.4) â€” this is a data-
    source swap, not an architecture change.
- â˜ **6.5** Rewrite `readme.md` (D7) â€” draft exists; refresh once names are final.
  Add the config-folder / generators / editions sections.
- â˜ **6.6** Finalise `THIRD_PARTY_LICENSES.md` / `NOTICE` (Boost, QuantLib,
  **patched** xlOil â€” mark modified files, Eigen, Adept; GoogleTest test-only).
- â˜ **6.7** **Banner cleanup** (D10.1). Bulk-remove the ~1,470 `Mizuho` / `MHI` /
  `MHSC` header and `@Department` lines across `src\` and replace with, at most,
  a minimal technical banner â€” no copyright, no company/person name, no
  tool/migration reference. Do **not** insert a copyright or licence statement
  into source files; that text lives only in `LICENSE`. Replace `LICENSE`
  placeholder content when the EULA is ready.
- â˜ **6.8 Bulk `clang-format` pass** (see Tooling appendix). One normalization
  commit now that the mass renames are done. Then it stays green.
- â˜ **6.9 `clang-tidy` config.** Add `.clang-tidy` with
  `readability-identifier-naming` tuned to the AQ convention (camelCase; types
  Upper; functions/vars lower; members trailing `_`) as a **rename-completeness
  guard**, plus `bugprone-*` / `performance-*` / selected `modernize-*` as
  **advisory** (not build-breaking yet). Needs a `compile_commands.json` (MSBuild
  can emit one).
- â˜ **6.10 Delete the `AQL Classic` `.vcxproj.filters` filter contents**
  (Nicholas, 2026-09-12). Scoped, not actioned, this session â€” small and
  tractable, unlike 8.1/8.2 below. **17 files, `GTEST.vcxproj.filters`
  only:** 6 curve-fixture `.cpp`/`.h` pairs (`Curve{Accessors,Ois,FwdFxConst,
  Std,TenorBasis,XccyBasis}`), `TestDatesCentralBank.cpp`,
  `TestDatesSwapSchedule.cpp`, `TestRiskSwapDeltaLadder.cpp`,
  `TestRiskTenorBasisCurve.cpp`, and `TestMirDateFunctions.cpp` (check this
  one first â€” a `mir`-named test may already be dead per the "`mir*` stack
  deleted wholesale" work; confirm before deleting rather than assuming).
  `AQ_API.vcxproj.filters` declares the same filter with **zero files under
  it** â€” delete the empty `<Filter Include="...AQL Classic">` declarations
  there too. Detail: `rebrand\STATUS.md`.
- â˜ **6.11 Matrix/table-type consolidation** (new, 2026-09-19; design proposed
  2026-09-20 â€” Nicholas asked how to make `AnyMatrix`/`VariantMatrix`-style
  API utilities consistent with `AQLMatrix`'s flattened storage, then asked
  for a concrete rename/consolidation/flattening plan). **Design approved
  2026-09-20 â€” steps 1, 2 and 3 done same day, steps 4â€“6 not yet started.**

  **Step 2 done (2026-09-20):** `AQLMatrix` â†’ `AQLNumericMatrix`, 381
  occurrences across 32 files, both physical files and the GTEST suite
  renamed too. Alongside the rename, a code review of the existing matrix
  containers surfaced and fixed a real correctness bug (`throw "Invalid
  Matrix"` â€” a raw string throw that no `catch(const std::exception&)` could
  catch) and an efficiency issue (`ludcmp`/`svdcmp`/`tred2`/`tqli` writing
  through the bounds-checked, COW-checked `setValue()` in their O(nÂ³) inner
  loops), plus added the everyday methods the review recommended
  (`operator()`, `trace()`, `norm()`, `equals()`, `operator!=`, static
  `identity()`, `getDiagonal()`, `toDoubleMatrix()`, `operator<<`,
  brace-init construction). 19 new GTEST cases, all passing; full detail in
  `rebrand\STATUS.md`.

  **Steps 1 and 3 done (2026-09-20, same session, pilot-first discipline
  kept per Nicholas's explicit ask):** built the shared
  `AQLFlattenedMatrix<T>` base (`src\math\include\AQLFlattenedMatrix.h`) and
  piloted it on `AQLIntMatrix`/`AQLComplexMatrix` â€” both now thin aliases in
  place of the old bare `vector<vector<T>>` typedefs. `AQLNumericMatrix`
  itself was deliberately **not** re-based onto the new shared template
  (its own internals are already proven and freshly fixed â€” re-basing it is
  a separate, larger step). 14 new GTEST cases, all pass; full design
  writeup and scope rationale in `rebrand\STATUS.md`.

  **Step 5 scoped 2026-09-20, then deliberately PAUSED at Nicholas's request â€” do not resume
  without rereading this first.** Reconnaissance only, zero files changed. Confirmed the scope
  warning below concretely rather than on faith: **79 files / 268 occurrences**
  (`etrading` 39, `validation` 28, `GTEST` 6, `AQ_XLL` 5, `AQ_API` 4, `math` 1), and this is
  **not** a repeat of the `IntMatrix`/`ComplexMatrix` pilot on two counts â€” (1) `AnyTypeMatrix`
  is the return type of public `validation` wrapper functions (e.g. `tryAqBondObjectDisplay`),
  i.e. the golden-source contract itself, not dead code; (2) `AnyType`'s `boost::get<T>`/`typeid`
  inspection style and `fromAnyTypeToString`'s `NaN`-blanking + configurable precision have **no**
  equivalent in `Variant`/`getValueAsString()` (no `NaN` blanking, fixed precision 20 always) â€” a
  mechanical swap would silently change output, so this needs real reconciliation work, not a
  rename pass. **Recommended staging when resumed** (full detail and rationale in
  `rebrand\STATUS.md`'s "Step 5 ... scoped, findings recorded, deliberately paused" entry â€” read
  that first, don't re-derive): (1) reconcile the `NaN`/precision gap before any call site moves;
  (2) `etrading` (39 files); (3) `validation` (28 files, needs an `api_map.csv`/
  `api_pair_check.py` re-verification pass after â€” public wrapper signatures, not internal
  detail); (4) `AQ_XLL`/`AQ_API`/`GTEST` last (15 files, consume `validation`'s output type).

  **âš  REMINDER â€” steps 4 and 5 remain open, 5 is explicitly paused (not forgotten):**
  `AQLBoolMatrix` (step 4, needs the `uint8_t`-backing workaround already flagged for `bool`) is
  unstarted and smaller/lower-risk than step 5 â€” a reasonable thing to pick up first when this is
  revisited, though Nicholas should confirm which he wants next rather than assuming. Step 7
  (`AQLStringMatrix`/`StandardStringMatrix`/`STDStringMatrix`)
  is intentionally excluded from this â€” it still needs its own inventory
  before it can be scoped (see the note further down). Check
  `rebrand\STATUS.md` for whether any of this has been actioned since this
  plan entry was last touched.

  **Full inventory, with real usage counts (2026-09-20):**

  | Type | Files / uses | Role | Disposition |
  |---|---|---|---|
  | `AQLNumericMatrix` (was `AQLMatrix`, renamed 2026-09-20) | 381 occurrences / 32 files (not ~40 â€” used across `math`/`etrading`/`models`, not just internally) | numeric linear algebra (LU/SVD/Cholesky), already flattened | **renamed, done** â€” still needs migrating onto the shared base (below) once it exists |
  | `DoubleMatrix` | 274 / 1,713 | numeric, real hot loops (Heston calibration, swaption vol, yield-curve pro) | candidate for the shared base, **gate on profiling**, see below |
  | `AQLIntMatrix` (was `IntMatrix`, renamed 2026-09-20) | 3 / 53 | numeric, always paired with `DoubleMatrix` in the same Heston calibration files (dead code today â€” see `rebrand\STATUS.md`, zero live callers) | **piloted on `AQLFlattenedMatrix<T>`, done** |
  | `AQLComplexMatrix` (was `ComplexMatrix`, renamed 2026-09-20) | 2 / 8 | numeric, found in the *same* Heston hot loops as `DoubleMatrix`/`IntMatrix` (`AQLMathDisplacedHestonTDP.cpp`) | **piloted on `AQLFlattenedMatrix<T>`, done** |
  | `BoolMatrix` | 12 / 29 | calibration flag/config grids (`getCalibTargetIRVolGrids`, `AQLDataBoolMatrix`), not arithmetic | rename `AQLBoolMatrix`, flatten for consistency (low risk), not for speed |
  | `DateMatrix` | 14 / 19 | small, likely marshaling-adjacent | low priority â€” check whether it can fold into `AQLAnyMatrix` instead of getting its own type |
  | `AnyTypeMatrix` | 79 files / 268 occurrences (recounted 2026-09-20, was 79/202) | heterogeneous marshaling, `boost::variant`-backed (`AnyType`); **is the golden-source `validation` return type in 28 of those files, not dead code** | **retire onto `VariantMatrix`, scoped but PAUSED 2026-09-20** â€” see the staging plan above and `rebrand\STATUS.md` |
  | `VariantMatrix` | 120 / 706 | heterogeneous marshaling, `etrading::Variant`-backed (wraps `boost::spirit::hold_any` â€” true type erasure, already has `transpose()` and `AQLStringMatrix`/`StandardStringMatrix` conversion helpers) | **survives** â€” more capable of the two; rename `AQLAnyMatrix` |
  | `AQLStringMatrix` | 300 / 2,290 | string marshaling â€” **the single largest matrix-type user in the tree, bigger than `DoubleMatrix`** | **out of scope for now** â€” flag only, see below |
  | `StandardStringMatrix` | 49 / 277 | string marshaling, smaller sibling of the above | **out of scope for now** â€” flag only |
  | `STDStringMatrix` | 5 / 17 | string marshaling, likely a near-duplicate spelling of the above two | **out of scope for now** â€” flag only |
  | Eigen `MatrixXd` | `PolynomialInterpolation`/`PiecewisePolynomialInterpolation` only | already contiguous, SIMD-optimized third-party | leave untouched |

  **The redundancy worth collapsing is confirmed**: `AnyType` (`boost::variant<int,double,bool,
  std::string,AQLString,const char*>`, a fixed 6-type enumeration) is strictly narrower than
  `etrading::Variant` (true type erasure via `boost::spirit::hold_any`, and already has a
  `transpose()` free function plus `AQLStringMatrix`/`StandardStringMatrix` conversion helpers).
  `Variant`/`VariantMatrix` should survive; `AnyType`/`AnyTypeMatrix` should be retired onto it.

  **Proposed design â€” one shared base template, not four separate flattening implementations:**
  a template, `AQLFlattenedMatrix<T>`, generalising the existing `AQLMatrix::AQLMatrixData`
  design (contiguous row-major `std::vector<T>`, a `RowView<T>` proxy - not a bare pointer, since
  the typedef-family call sites genuinely rely on `.size()` on a row and nested-vector-style
  construction, not just `[i][j]`) into something every consolidated type composes (not inherits,
  matching `AQLMatrix`'s existing ownership shape) rather than reimplementing. One footnote:
  `std::vector<bool>` is bit-packed and can't back a pointer/reference row-view, so
  `AQLFlattenedMatrix<bool>` needs `uint8_t` storage underneath, presented as `bool` through the
  view - the only element type needing this workaround.
  - `AQLNumericMatrix` (double), `AQLIntMatrix`, `AQLComplexMatrix`, `AQLBoolMatrix`: thin
    wrappers around `AQLFlattenedMatrix<T>`, each adding only the operations that make sense for
    its `T` (no LU/SVD/Cholesky on `AQLIntMatrix`/`AQLBoolMatrix` - meaningless there).
  - `AQLAnyMatrix` (ex-`VariantMatrix`): also composes `AQLFlattenedMatrix<Variant>` - `Variant`
    is a regular value type, so no special handling needed the way `bool` needs one. Gains
    `transpose()` (already exists as a free function on `VariantMatrix` today - becomes a member,
    or stays free, either way trivial data movement independent of `T`) at near-zero extra design
    cost once the base template exists.
  - **Naming tension flagged, not resolved**: `AQL` = "AQ Legacy" (D17) - it deliberately marks
    code in the `math` project as legacy-to-deprecate (Phase 8 already scopes retiring
    `AQLString`/`AQLDate` eventually). `AQLAnyMatrix`/`VariantMatrix`'s natural home is `etrading`
    (not `math`), and `etrading` is **not** flagged for deprecation - carrying the `AQL` prefix
    there would misleadingly mark permanent code as legacy. Recommend `AQAnyMatrix` (no `L`) if it
    ends up living in `etrading`; keep the `AQL` prefix only for the types staying in `math`
    (`AQLNumericMatrix`/`AQLIntMatrix`/`AQLComplexMatrix`/`AQLBoolMatrix`), consistent with their
    neighbours (`AQLDate`/`AQLString`/`AQLCalendar`) until Phase 6.1/6.2's legacy extraction moves
    them into the new `core`/`utils` project, at which point they'd graduate to `AQ*` too.
  - Centralize the OpenMP `if(tripCount > OPENMP_SIZE_THRESHOLD)` pattern once, inside
    `AQLFlattenedMatrix<T>`'s embarrassingly-parallel operations (fill/clear/scalar-multiply/
    transpose/`getRow`/`getColumn`), so every consolidated type gets it for free rather than
    re-deriving the threshold per type. `dotRow`/`dotCol` only apply where `T` is arithmetic.
    **Not yet done (2026-09-20)** â€” the base built for steps 1/3 below doesn't carry
    fill/clear/scalar-multiply/`dotRow`/`dotCol` at all yet (no real call site needed them for the
    `AQLIntMatrix`/`AQLComplexMatrix` pilot â€” see `rebrand\STATUS.md`), so this centralization
    has nothing to attach to yet; revisit once a consolidated type actually needs one of these.

  **Proposed staged order** (smallest/safest first, `DoubleMatrix`'s 274-file blast radius last
  and gated on evidence, not assumption):
  1. **Done (2026-09-20).** Built `AQLFlattenedMatrix<T>` (`src\math\include\AQLFlattenedMatrix.h`)
     â€” contiguous storage, COW, `operator()`/`operator[]` (row-view), `getRow`/`getColumn`,
     `resize`, `transpose`, `operator==`/`!=`/`equals`, `operator<<`, brace-init, plus a
     compatibility constructor absorbing the classic `vector<vector<T>>(rows, vector<T>(cols))`
     fill idiom. **Migrating `AQLNumericMatrix`'s own `AQLMatrixData` onto it internally is
     deliberately deferred** â€” `AQLNumericMatrix` already has its own proven, freshly-fixed
     internals (see the rename/fixes entry above) and rich arithmetic surface; re-basing it onto
     the shared base is a separate, larger step with its own risk, not bundled into building the
     base. See `rebrand\STATUS.md` for the full design writeup and scope rationale.
  2. **Done (2026-09-20).** Renamed `AQLMatrix` â†’ `AQLNumericMatrix` (381 occurrences / 32 files,
     not ~40 â€” see `rebrand\STATUS.md`).
  3. **Done (2026-09-20).** Piloted `AQLIntMatrix`/`AQLComplexMatrix` on `AQLFlattenedMatrix<T>` â€”
     both now thin aliases (`AQLCoreTemplateType.h`) in place of the old bare typedefs. Scoping
     check done first: the only two functions using them outside the commented-out
     `#ifdef isQuantLib` block (`AQLMathDisplacedHestonTDP::CalibrationHelper`/`CalibrationHelperGL`)
     have **zero live callers anywhere in the tree** â€” about as low-risk as a pilot gets. 14 new
     GTEST cases (`TestAQLFlattenedMatrix.cpp`), all pass; `math`/`etrading`/`calibration`/
     `validation`/`models`/`GTEST` all rebuild clean. Full detail in `rebrand\STATUS.md`.
  4. `AQLBoolMatrix` rename + flatten (12 files, low risk, consistency not speed).
  5. `AnyType` â†’ `Variant` retirement + `AQLAnyMatrix`/`AQAnyMatrix` rename + flatten + transpose -
     its own scoped project (SWIG bindings, Excel marshaling, dozens of `TableInfo` call sites).
  6. `DoubleMatrix` â†’ fold into `AQLNumericMatrix<double>` **only if profiling actually shows a
     hot, large-matrix bottleneck** - many of its 1,713 uses are small (tens of tenors/strikes)
     calibration matrices where the numerical work (Heston characteristic functions, Gauss-
     Laguerre quadrature) likely dominates over memory layout; don't convert 274 files on faith.
  7. **Explicitly deferred, not scoped yet**: the `AQLStringMatrix`/`StandardStringMatrix`/
     `STDStringMatrix` family - at 300/49/5 files respectively, `AQLStringMatrix` alone is larger
     than `DoubleMatrix`. Needs its own inventory and decision later, not folded into this item.

  **Nicholas approved this design (2026-09-20)** â€” steps 1â€“6 above are ready to start whenever
  picked up. **âš  Reminder â€” do not lose this:** step 7 (`AQLStringMatrix`/`StandardStringMatrix`/
  `STDStringMatrix`) still needs its own inventory and a proposed approach before it can be
  scoped â€” bigger than everything else in this item combined, deliberately not designed yet.
  Revisit explicitly, don't let it quietly fall out of the plan.

  **Agreed, no further discussion needed**: containers stay `std::vector`-based throughout: no
  custom allocators, no raw arrays, matching `AQLString`'s and `AQLMatrix`'s existing direction.

**Exit:** legacy projects gone or reduced to a documented `core`; resources,
examples and config fully rebranded or removed; every header carries the
proprietary notice; `clang-format` clean; `clang-tidy` naming check clean;
`AQL Classic` filter contents gone; docs current.

---

## Phase 7 â€” Linux build, final sweep, clean repo  â˜

- â˜ **7.1 Linux / CMake** (D13). Re-validate the MAKE/CMAKE build on the
  rebranded tree; retarget to the current dependency set; rebrand its
  `Makefile` / `make.header` / `make.footer` / `make.sh` and any CMake files.
  Lower priority than 4â€“6 but **in scope** â€” it is part of "no legacy names
  anywhere".
- â˜ **7.2 Final legacy sweep.** Tree-wide, case-sensitive:
  `grep -rIE "Mizuho|MLIB|\bme[A-Z]|\bmir[A-Z]|\bLA[A-Z][a-z]|\bLA[0-9]D|\bMA[A-Z][a-z]|validation_api|XllPlus|MLIBQ"`
  (`AQL*` is the intended legacy prefix â€” expected, not a hit)
  over the whole `AQ_LIB` tree (source, projects, scripts, resources, examples,
  docs, Linux build). Expected result: **zero hits** outside third-party headers
  and genuine string data. Anything left is fixed or removed.
- â˜ **7.3** Full green build, all 12 Windows configs + Linux; full suite green;
  baseline-diff clean (bar the intended calendar change).
- â˜ **7.4 CI** â€” a pipeline that builds the key configs, runs `GTEST`, and
  runs `clang-format --dry-run --Werror` + the `clang-tidy` naming check as
  gates; the rest of `clang-tidy` as a non-blocking report. Wire it against the
  new clean repo.
- â˜ **7.5** Copy `AQ_LIB` to the new folder, `git init`, first commit = clean
  base. Delete the stash repo.
- â˜ **7.6** Commodities library integration â€” **only** once the above is green
  (separate plan; copy in, retarget v143/C++17 standalone, expect the same C++17
  fixes, then rebrand and add a `Commodities` category).

---

## Phase 8 â€” Retire `AQLString` / `AQLDate` (scoped 2026-09-12, NOT started)  â˜

**Deliberately its own phase, after Phase 7, not folded into Phase 6.** This
is a genuinely separate initiative from the rebrand: it is a type-replacement
across the whole codebase, not a rename, and the blast radius is an order of
magnitude past anything else in this plan. Do not start it opportunistically
mid-batch the way small fixes have been folded in elsewhere â€” it needs its
own dedicated pass, tooling and regression budget.

**Scale (measured 2026-09-12):** `AQLString` â€” **~36,962 occurrences across
1,108 files.** `AQLDate` â€” **~9,904 occurrences across 585 files.** For
comparison, the entire `me*`â†’`aq*` identifier rebrand (Phase 3, the biggest
thing done so far) touched a fraction of this.

**Why retire them at all (Nicholas asked directly â€” answered here):**
- **`AQLString`** (`src\math\include\AQLString.h` + `.cpp`, ~1,550 lines) is
  a hand-rolled, atomic-refcounted, copy-on-write string class â€” solving a
  problem (expensive string copies) that C++11's move semantics and small-
  string optimisation already solved natively in `std::string`, which this
  codebase has had available since the C++17 retarget. No architectural
  reason found to keep it; its extra convenience (numeric constructors,
  `getDoubleValue()`/`getIntValue()`) is a thin, trivially-replaceable layer
  over `std::to_string`/`std::stod`/`std::stoi`. **Target: `std::string`.**
- **`AQLDate`** (`src\math\include\AQLDate.h`, 112 lines + 695-line `.cpp`) is
  a hand-rolled, virtual, Julian-day calendar class. **Target:
  `boost::gregorian::date`** â€” already a direct dependency (BSL-1.0, no
  encumbrance), already the canonical date type taken directly by the newer
  `Curve` validation headers, and the bridge functions
  `etrading::toGregorianDateFromAQLDate`/`toAQLDateFromGregorianDate` already
  exist and are proven correct in production use â€” the hard
  conversion-correctness work is already done.
- â˜ **8.1 Tooling first.** A libclang/clang-tidy-based codemod (not `sed`/
  regex) that rewrites declarations, call sites and includes together,
  batchable by project or by file, with a dry-run diff mode. Manual
  batch-and-review (the Phase 3 approach) does not scale to 37k occurrences.
- â˜ **8.2 `AQLString` â†’ `std::string`**, one project at a time
  (`math` â†’ `etrading` â†’ `validation` â†’ `AQ_API`/`AQ_XLL` â†’ `GTEST`), full
  build + baseline-diff between each. Expect signature ripples through every
  `validation` wrapper that takes `AQLString`/`AQLStringVector`/
  `AQLStringMatrix` â€” i.e. most of them.
- â˜ **8.3 `AQLDate` â†’ `boost::gregorian::date`**, same batch discipline.
  Retire the now-redundant `toGregorianDateFromAQLDate`/
  `toAQLDateFromGregorianDate` bridge once nothing calls it.
- â˜ **8.4** Once both are gone: `git grep -w AQLString` / `AQLDate` return
  zero hits (bar `.APPLES`, read-only reference); delete
  `src\math\{include,src}\AQLString.*` / `AQLDate.*`.

**Exit:** `std::string` and `boost::gregorian::date` used throughout; no
`AQLString`/`AQLDate` symbol remains in `AQ_LIB`; full green build and
baseline-diff clean.

---

## Suggested order of execution

```
0 â”€â–¶ 1 â”€â–¶ 2 â”€â–¶ 3 â”€â–¶ 4 (+4a +4b) â”€â–¶ 5 â”€â–¶ 6 â”€â–¶ 7 â”€â–¶ 8
         â”‚                          â–²
         â””â”€â”€ Phase 2 output feeds 3, 4, 4a, 4b and 5
```

Phase 8 (`AQLString`/`AQLDate` retirement) is deliberately last and separate
â€” a post-launch modernisation initiative, not a rebrand blocker. It could in
principle start any time after Phase 3, but scheduling it after 7 keeps the
rebrand's own timeline and regression baseline undisturbed by a
much-larger-scale, higher-risk type change.

**Status (2026-09-15): Phases 0-3 done. Phase 4 done** (470 `AQ_XLL`
functions, 460/470 `validation` wrappers covered â€” see Phase 4's status note
above, including the new `Generator` category built this session); `Model`
still needs a design brief before any `validation` wrapper can be written (0
exist today, no legacy source, nothing on disk to introspect unlike
`Generator`). **Phase 4a done** â€” `AQ_XLL` ships
`Release`/`Release_XL_Bond`/`Release_XL_Swap`/`Release_XL_Credit`/
`Release_XL_Curve`; the `AQ_API` runtime edition manifest is dropped, not
built. Phases 4b (bar the `Generator` introspection slice, done), 5, 6, 7 not
started. Phase 7.1 (Linux) can slot in any time
after Phase 3 but is lowest priority.

---

## Appendix â€” Tooling & lint

**Recommendation: yes, adopt lint â€” two tools, phased so they never fight the
rename.**

| Tool | Role | When |
|---|---|---|
| **`clang-format`** | deterministic formatting; replaces the legacy AStyle utility. Editor + CI integration, one `.clang-format` at the root. | **Config in Phase 1.7**, tuned to current style so the eventual reformat is small. **Bulk apply once in Phase 6.8**, after all mass renames â€” reformatting mid-rename buries the real diff and breaks the baseline-diff discipline. Green thereafter. |
| **`clang-tidy`** | static lint. `readability-identifier-naming` **enforces** the AQ naming convention â€” a real completeness guard for the rebrand, complementing the Phase 7.2 grep. `bugprone-*` / `performance-*` / curated `modernize-*` catch latent issues. | **Config in Phase 6.9.** Naming check as a gate; the rest advisory (non-blocking) until the tree is clean, then tighten. Needs `compile_commands.json` (emit from MSBuild, or use the LLVM VS integration). |

Not recommended: per-file `cpplint` (Google-style, wrong fit); MSVC `/analyze` as
a gate (too noisy on this codebase â€” fine as an occasional manual sweep). IWYU is
optional but genuinely useful *during* Phase 1/3 include churn â€” run it manually,
don't gate on it.

Rule for the whole rebrand: **linters are guards, not gates, until Phase 6.**
Turning them on earlier means every rename batch argues with the linter instead
of with the compiler.
