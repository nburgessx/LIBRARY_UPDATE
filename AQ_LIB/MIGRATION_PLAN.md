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
| D3 | `AQ_BINDINGS` → **`AQ_API`** (confirmed). Update project files, `.sln`, folder, SWIG `.i`, and all 8 `generate*`/`deploy*` batch files + pre/post-build commands. `RootNamespace` is currently `swigUseCase` — tidy to `AQ_API` in the same pass. |
| D4 | The LWO handle/object framework stays — port as-is, rebrand `LWO→AQO`, keep the cell-location counter/hash behaviour. Do **not** adopt xlOil's cache. |
| D5 | Calendar holiday-centre delimiter `:` → **`+`**. |
| D6 | Function-signature **categories** across validation / XLL / API / tests to be reviewed and standardised — professional, clear, concise. (Phase 2 — §2.2 is the table to review.) |
| D7 | `readme.md` to be rebranded and made current. (Phase 6.) |
| D8 | Env vars set once via `SetEnvironmentVariables.bat`, then restart VS. |
| D9 | **Q1 — Excel names: clean break.** Migrate every `me*` spelling (including any currently-hidden `me*` aliases) to `aq*`. **No** backward-compatibility aliases; old names simply cease to exist. Removed names are listed in the release notes. |
| D10 | **Q2 — Licence: proprietary, no redistribution, as a drop-in placeholder.** A short interim `LICENSE` (proprietary notice + "AS IS" + "no redistribution") and a per-file header that points at `LICENSE` by name, both written so the final EULA text replaces `LICENSE` in full later **without touching a single source header**. Header wording is frozen now; only `LICENSE` / `EULA.md` content changes later. The contractor/client IP-provenance check with a solicitor is still advised before commercial release but does not block the rebrand. |
| D11 | **Q3 — `AQ_API` confirmed.** |
| D12 | **Q4 — holiday-centre join is `+` only** (Nicholas, this session). Reason: clean break (D9), fresh clients, no legacy user sheets to protect; the only `:`-form data that ships is the ~103 generator JSON, which we migrate ourselves; and `:` is heavily overloaded (`DATA_COLL_DEL`, curve-name lists in the same generator files). The `splitCalendarCentres()` helper is written so accepting `:` again is a **one-line toggle** if field feedback ever demands it — but it ships `+`-only. Migrate the 103 JSON calendar fields to `+`. |
| D13 | **Q5 — Linux / CMake build is in scope**, lower priority (late phase). **End-state gate: not one file anywhere in the tree — source, Makefiles, `make.*`, CMake, `.sln`/`.vcxproj`, scripts, resources, examples, docs — may contain a legacy client name or an old prefix.** Many `resources\` and `examples\` items will be rewritten or removed for the final version. |
| D14 | **Q6 — category taxonomy: review the straw-man in §2.2**, then it is locked into `CLAUDE.md` §5.1. |

All open questions from the first draft are now answered. Nothing in the plan is
blocked on a decision; Phase 4 still waits on the xlOil worked examples.

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
  smoke tests, `GOOGLE_TEST`, the `resources\test` batch and Excel tests); save
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
  `STRUCTURED_KEYS` column blocks (the AQO serialisation format). Legacy
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
- ☐ **1.2 `AQ_BINDINGS` → `AQ_API`** (D3). In one batch:
  - `projects\AQ_BINDINGS.vcxproj` (+ `.filters`, `.user`) → rename file,
    `<ProjectName>`, `<RootNamespace>`.
  - `AQ_2022.sln` — project name and path (GUID unchanged).
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
- ☐ **1.3 File/folder renames** — drop `Mizuho` / `MLIB` / `me` / `mir` from
  names. `InitializeMLibETrading.*`, `InitializeMLibGoogleTest.*`,
  `mlib_ReadMe.txt`, the 206 `tryMe*` files, 78 `me*` files. Fix every `#include`
  in the same commit. Update `.vcxproj` / `.filters` / natvis / SWIG `.i` in the
  same commit. One project per sub-batch, build between.
- ☐ **1.4 Remove `mir*` wholesale** — per the 0.5 call-graph (no inbound `aq`/`me`
  deps). Delete the 58 `AQ_API\mir*` files, ~35 `validation\tryMir*` files, and
  the `mir*` lines in `swig_*.i` / `swig_R_wrap.cpp`. Build clean.
- ☐ **1.4a Remove client-specific modules** — per the 0.6 map: `LoanCalculations`,
  `SupervisoryRules`, `CashflowClient`, and the confirmed-dead securitisation
  cluster in `etrading`. Their `validation\tryMe*` wrappers and `GOOGLE_TEST`
  cases go too. **STOP-gate:** the "VERIFY — KEEP FOR NOW" rows in
  `rebrand\removal_map_client_specific.md` (Serialize*/SchemaObject,
  CreditResultsContainer include, DataFrame) — Nicholas has asked to keep these
  for now and review them at this step. Do not delete them without his explicit
  go-ahead; if in doubt, keep and move on.
- ☐ **1.5 Delete `msc*`** (D2) — per the 0.6 map. Add-in files, downstream
  symbols, resources, tests. Build clean with them gone.
- ☐ **1.6 Namespaces** `validation_api → validation`. Single mechanical rename;
  update the friend/using declarations and the bindings + XLL call sites (the
  POC `AQ_XLL\src\math.cpp` already calls `validation_api::tryMe…`).
- ◐ **1.7 Config path resolution.** Client absolute paths in
  `LACurveProperties.cpp` / `FolderConfig.cpp` already neutralised → `.\config\…`
  (commit `767aa4e`). **Still to do, needs a build:** the `MLIBQ` env-var name
  (read via `getenv("MLIBQ")` / `toPath("MLIBQ", …)` in both files, and named in
  `exposed_functions.cpp` error text) — rename to `AQ` or a dedicated
  `AQ_CONFIG`, and reconcile the two path conventions (`\resource\config\…` in
  `FolderConfig` vs `\config\…` / bare filename elsewhere). Also delete the
  stale `resources\config\MLIBQ_ADDIN.xll`. `LACurveProperties` and
  `FolderConfig` are near-duplicates — consider collapsing to one.
- ☐ **1.8 `.clang-format`** — add one at the `AQ_LIB` root, tuned to the *current*
  style (derive from a sample so a future reformat is minimal). Retire the old
  `resources\utilities\ArtisticStyle` (AStyle) setup. **Config only now — do not
  bulk-reformat** (that waits until after Phase 3; see Tooling appendix).

**Exit:** solution builds all configs; `grep -r "MLIB\|Mizuho\|validation_api\|msc"`
in `src\` is empty except third-party and string literals; baseline-diff clean.

---

## Phase 2 — Category taxonomy review  ☐  (D6, D14)

A design step, not a code step. Produce, circulate, agree — *then* Phases 3–5
apply it. **§2.2 below is the table Nicholas asked to review.**

- ☐ **2.1** Inventory every category token in use today:
  - XLL POC: `aqDates`, `aqTools`, `aqMath`.
  - CLAUDE.md target list: `AQ_dates`, `AQ_tools`, `AQ_curves`, `AQ_swaps`,
    `AQ_products`, `AQ_models`.
  - Old add-in de-facto categories (by file): `Dates`, `Curves`, `Swap`,
    `Products`, `Vols`, `Math`, `Utilities`, `LWO*`, `LoanCalculations`,
    `SupervisoryRules`, `CashflowClient`.
  - Old bindings: `Curve`, `Date`, `LWO`, `Product`, `Utility`, plus `mir*`.
  - `validation` wrappers: `tryMe` + `<Category>` + `<Func>`.
  - `GOOGLE_TEST`: test-suite naming.
- ☑ **2.2 Canonical category list — LOCKED (Nicholas, this session).** 13
  categories, applied identically in `validation`, `AQ_XLL`, `AQ_API` and
  `GOOGLE_TEST`:

  | Category | Covers | Prefix example |
  |---|---|---|
  | `Dates` | calendars, schedules, IMM / central-bank dates, year fractions | `aqDatesYearFraction` |
  | `Curves` | **rates yield-curve framework only** — build, calibration, interpolation, DF / zero / forward queries, cross-currency. *Not* bond or credit curves. | `aqCurvesForwardRate` |
  | `Vols` | vol surfaces, surface SABR calibration, cap / swaption vol utilities | `aqVolsSabrImplied` |
  | `Rates` | FRAs, futures, cap / floor, stub rates, compounding | `aqRatesCompound` |
  | `Swaps` | vanilla / OIS / basis / cross-currency — creation, schedule, pricing, risk | `aqSwapsParRate` |
  | `Bonds` | bond creation, price / yield, asset-swap spread, **bond-curve fitting** (govie / spread curves) | `aqBondsYield` |
  | `Credit` | CDS, **hazard-rate / survival (credit) curves**, par spreads | `aqCreditParSpread` |
  | `Options` | vanilla option pricing / greeks (Black-Scholes etc.) | `aqOptionsBlackScholesPrice` |
  | `Math` | **low-level building blocks** — distributions, interpolation, root-finding, matrix ops — for users doing their own calculations or replicating results | `aqMathNormalCdf` |
  | `Models` | term-structure / stochastic models (Hull-White, LMM, Piterbarg, SABR-as-model), model calibration sets, model-based / exotic / CMS-spread pricing, analytic (Jacobian) risk. May be sparse initially. | `aqModelsHullWhiteCalibrate` |
  | `Generators` | list / describe / validate the JSON instrument & model static-data templates; build instruments from a generator + a few overrides | `aqGeneratorsList` |
  | `Objects` | AQO handle framework — create / copy / modify / delete / clear / list | `aqObjectsDelete` |
  | `Tools` | echo, build stamp, edition report, diagnostics, memory dump, CSV load | `aqToolsBuildTime` |

  **Decisions folded in:** `Curves` = rates yield-curve framework only, bond-curve
  fitting → `Bonds`, hazard/survival → `Credit` (each asset owns its curve).
  **`Vols`** (not `Vol`, not `Volatility`). **`Math` kept** — low-level surface.
  **`Models` added** (may be thin at first). **`Products` dropped** — not a
  category; it is the union of `Swaps` / `Bonds` / `Credit`. **`Generators`
  kept.** `LoanCalculations`, `SupervisoryRules` and `msc*` are **removed**
  (client-specific — Phase 1.4a).
- ☐ **2.3** Record the locked list in `CLAUDE.md` §5.1 (done). Map every row of
  the 0.7 inventory (`rebrand\xll_function_inventory.csv`) to one of the 13
  categories or to drop/merge.
- ☐ **2.4 Edition → category map.** Straw man (needs sign-off):

  | Edition | Registers |
  |---|---|
  | Swaps | Dates, Curves, Vols, Rates, Swaps, Math, Generators, Objects, Tools |
  | Bonds | Dates, Curves, Rates, Bonds, Math, Generators, Objects, Tools |
  | Credit | Dates, Curves, Credit, Bonds, Math, Generators, Objects, Tools |
  | Full | all 13, incl. Options and Models |

- ☐ **2.5 Generator categories.** Confirm whether generator support is its own
  `Generators` category plus per-asset constructors (`aqSwapsFromGenerator`,
  `aqBondsFromGenerator`), or folded entirely into `Objects`. Recommendation:
  a small `Generators` category for introspection (list / describe / validate)
  **and** per-asset `…FromGenerator` constructors in the asset categories.

**Exit:** agreed category list + edition→category map in `CLAUDE.md`; 0.7
inventory fully categorised.

---

## Phase 3 — Identifier rebrand + calendar delimiter  ☐

The big one. `LA` / `MA` / `MB` → `AQ` across ~25,900 identifiers, plus `me*→aq*`
function bodies, plus `LWO→AQO`. Anchored, case-sensitive, from an approved list
only (CLAUDE.md §5.5). One project per batch, in dependency order:
`math → models → calibration → etrading → validation → AQ_API`. Build +
baseline-diff between **every** batch.

- ☐ **3.1** For each project: extract the symbol list matching the prefix
  pattern → review → rename from the approved list → build → diff. Update natvis
  and SWIG `.i` in the same commit as the project they describe.
- ☐ **3.2** `LWO → AQO` rename, folded into the `etrading` batch. Keep the handle
  behaviour byte-for-byte (D4).
- ☐ **3.3** Function prefixes `me* → aq*` / `tryMe* → tryAq*` in `validation` and
  `AQ_API`, plus SWIG `.i`. This is the **clean break** (D9): no `me*` spelling
  survives anywhere, no forwarding aliases. Collect the removed public names into
  a `RELEASE_NOTES` "renamed / removed functions" list as you go.
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
  fields keep `:`). `GOOGLE_TEST`: `"SYB+LNB"` builds the expected combined
  holiday set. Details: `rebrand\calendar_delimiter_sites.md`.

**Exit:** `grep -rE "\b(LA|MA|MB|LB|me|mir)[A-Z]"` in `src\` returns only
third-party / literals; all configs build; baseline-diff **numerically identical**
(renames are behaviour-preserving — any delta is a bug, and the calendar change
is the only intentional behaviour change, covered by its own new test).

---

## Phase 4 — xlOil XLL port  ☐

Wait for Nicholas's xlOil worked examples before starting — they define the
canonical marshalling / handle-I/O / array-return / error-convention pattern.

- ☐ **4.1** Port the **AQO handle framework** from `.APPLES\...\etrading`
  (`LWOCurve`, `IsLWOObject`, `HandleEnums`, `LWOUtilities`,
  `StructuredExceptionHandler`) into `src\etrading`, rebranded, behaviour
  preserved. Unit-test the counter/hash/recalc-suffix logic directly.
- ☐ **4.2** Audit `XllPlusTips*` → keep/drop list (CLAUDE.md §6.1). Port the
  kept helpers onto `ExcelObj` / `ExcelArray`. Delete the XLL+ boilerplate.
- ☐ **4.3** Write the porting template doc from the examples: old `_Impl` →
  `XLO_FUNC_START/END` + `.help()/.arg()`, plus the required `validation`
  wrapper + `GOOGLE_TEST` case per function.
- ☐ **4.4** Remove the dead `XllPlus\7.0\...` include/lib paths from
  `AQ_XLL.vcxproj`.
- ☐ **4.5 Editions & manifest gating** — see **Phase 4a** below; the mechanism is
  shared between XLL and bindings, so build it once here and wire `AQ_API` to it
  in Phase 5.
- ☐ **4.6** **Clean break confirmed at registration** (D9): register `aq*` names
  only; no `me*` aliases, hidden or otherwise. Finalise the `RELEASE_NOTES`
  renamed/removed-function list started in 3.3 so users can find replacements.
- ☐ **4.7** Port functions **category by category** in the CLAUDE.md §9.4 order
  (Dates → Tools → Curves → Swaps → Bonds → Credit → Rates → Vols → Options →
  Math → Models → Generators → Objects), driven by the 0.7 inventory and the
  locked Phase 2 categories. Per function: confirm it is a "keep" → port `_Impl`
  → `validation` wrapper → `GOOGLE_TEST` case → tick the inventory row. Build +
  run suite per category.
- ☐ **4.8 Generator + config wiring** — see **Phase 4b**.

**Exit:** every "keep" row in the inventory is ported, wrapped and tested;
`AlgoQuantLib.xll` loads and every category returns correct values against the
baseline; dropped functions are recorded with a reason; editions gate correctly;
generators load from the shipped `config` folder.

---

## Phase 4a — Editions & manifest gating  ☐

Nicholas wants `AlgoQuantLib` to ship as **Swaps / Bonds / Credit / Full**
editions, one artefact per language, gated at runtime. Design (for discussion —
not locked):

- **Single binary per language, built once.** No per-edition builds (the build ×
  config × language matrix is already large; CLAUDE.md §5.2 rules this out).
  Physical code is all present — protection is intentionally light.
- **Edition manifest** — `config\editions.json`: named editions → the categories
  (and optionally specific functions) each registers. This is the 2.4 table,
  shipped as data so editions can be re-cut without a rebuild.
- **Entitlement** — which edition(s) this install may use. v1: a plain
  `config\licence.json` (or a short key string) naming the edition. Keep the
  read behind one function so enforcement can harden later (signed key, expiry,
  machine binding) **without touching registration code**.
- **Registration** — at `xlAutoOpen` (XLL) and module import (bindings): load
  manifest → resolve entitled edition → register only those categories.
  Unentitled functions are **not registered** (clean `#NAME?` / `AttributeError`).
  Add `aqToolsEdition()` returning the active edition and category list.
- ☐ **4a.1** Agree manifest + entitlement file shape and the enforcement seam.
- ☐ **4a.2** Implement the gate in the shared registration layer (used by both
  `AQ_XLL` and `AQ_API`).
- ☐ **4a.3** `config\editions.json` from the 2.4 table; a `Full` `licence.json`
  for dev.
- ☐ **4a.4** `GOOGLE_TEST` / smoke: each edition registers exactly its categories
  and nothing else; `aqToolsEdition()` agrees.

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
  Generators are consumed by the AQO object layer: generator name + a few
  overrides → object handle. Existing seeds live under
  `resources\config\{SWAP,BOND,CURVE}_GENERATOR`.

- ☐ **4b.1** Document the generator JSON schema; add a schema-validation
  `GOOGLE_TEST` over the shipped set.
- ☐ **4b.2** `Generators` category (2.5): `aqGeneratorsList` / `…Describe` /
  `…Validate`, plus per-asset `aqSwapsFromGenerator` / `aqBondsFromGenerator` /
  etc.
- ☐ **4b.3** Confirm `config` is packaged by **both** deploy paths. Python
  already does (`deployPython_2022.bat` copies `resources\config`); add the same
  for `AQ_XLL` and the other languages.
- ☐ **4b.4** Rebrand the generator JSON and calendar config — legacy names,
  client-specific conventions and defaults (folded into the 6.4 resources audit).
- ☐ **4b.5** One end-to-end test per asset: book from a generator + minimal
  fields, price, check against the baseline.

---

## Phase 5 — Bindings & test coverage  ☐

- ☐ **5.1** `AQ_API`: verify Python still green end-to-end after the rename +
  identifier rebrand. Then exercise **C#, R, Java** (currently unverified) —
  generate, build, deploy, run each `resources\api\*` test app.
- ☐ **5.2** Wire `AQ_API` module import to the Phase 4a edition gate; ship
  `config` (editions + calendars + generators) with every language package
  (4b.3). Smoke each edition per language.
- ☐ **5.3** Fill priority `GOOGLE_TEST` gaps:
  - Yield-curve framework + Jacobian risk vs bump-and-revalue.
  - Fixed-income price/yield vs Bloomberg (both directions) — regression lock.
  - Date logic (it moved out of legacy in Phase 6).
  - Generator round-trips per asset (4b.5).
- ☐ **5.4** Wire the `validation`-recording → `GOOGLE_TEST` generation so new
  wrappers get cases automatically.

**Exit:** all four languages build and pass; new coverage merged; suite green
against baseline.

---

## Phase 6 — Legacy extraction, resources, docs, licence  ☐

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
  carried.
- ☐ **6.5** Rewrite `readme.md` (D7) — draft exists; refresh once names are final.
  Add the config-folder / generators / editions sections.
- ☐ **6.6** Finalise `THIRD_PARTY_LICENSES.md` / `NOTICE` (Boost, QuantLib,
  **patched** xlOil — mark modified files, Eigen, Adept; GoogleTest test-only).
- ☐ **6.7** **Copyright / licence headers** (D10). Bulk-replace the ~1,470
  `Mizuho` / `MHI` / `MHSC` header and `@Department` lines across `src\` with the
  frozen proprietary header — `Copyright (c) 2026 Nicholas Burgess / AlgoQuantHub`
  + "distributed under the terms of the AlgoQuantHub End User Licence Agreement,
  see LICENSE" + "AS IS, no warranty". The header names `LICENSE`; it does not
  quote licence terms, so dropping in the final EULA later is a one-file change
  to `LICENSE` with **no header churn**. Replace `LICENSE` placeholder content
  when the EULA is ready.
- ☐ **6.8 Bulk `clang-format` pass** (see Tooling appendix). One normalization
  commit now that the mass renames are done. Then it stays green.
- ☐ **6.9 `clang-tidy` config.** Add `.clang-tidy` with
  `readability-identifier-naming` tuned to the AQ convention (camelCase; types
  Upper; functions/vars lower; members trailing `_`) as a **rename-completeness
  guard**, plus `bugprone-*` / `performance-*` / selected `modernize-*` as
  **advisory** (not build-breaking yet). Needs a `compile_commands.json` (MSBuild
  can emit one).

**Exit:** legacy projects gone or reduced to a documented `core`; resources,
examples and config fully rebranded or removed; every header carries the
proprietary notice; `clang-format` clean; `clang-tidy` naming check clean; docs
current.

---

## Phase 7 — Linux build, final sweep, clean repo  ☐

- ☐ **7.1 Linux / CMake** (D13). Re-validate the MAKE/CMAKE build on the
  rebranded tree; retarget to the current dependency set; rebrand its
  `Makefile` / `make.header` / `make.footer` / `make.sh` and any CMake files.
  Lower priority than 4–6 but **in scope** — it is part of "no legacy names
  anywhere".
- ☐ **7.2 Final legacy sweep.** Tree-wide, case-sensitive:
  `grep -rIE "Mizuho|MLIB|\bme[A-Z]|\bmir[A-Z]|\b(LA|MA|MB|LB)[A-Z]|validation_api|XllPlus|MLIBQ"`
  over the whole `AQ_LIB` tree (source, projects, scripts, resources, examples,
  docs, Linux build). Expected result: **zero hits** outside third-party headers
  and genuine string data. Anything left is fixed or removed.
- ☐ **7.3** Full green build, all 12 Windows configs + Linux; full suite green;
  baseline-diff clean (bar the intended calendar change).
- ☐ **7.4 CI** — a pipeline that builds the key configs, runs `GOOGLE_TEST`, and
  runs `clang-format --dry-run --Werror` + the `clang-tidy` naming check as
  gates; the rest of `clang-tidy` as a non-blocking report. Wire it against the
  new clean repo.
- ☐ **7.5** Copy `AQ_LIB` to the new folder, `git init`, first commit = clean
  base. Delete the stash repo.
- ☐ **7.6** Commodities library integration — **only** once the above is green
  (separate plan; copy in, retarget v143/C++17 standalone, expect the same C++17
  fixes, then rebrand and add a `Commodities` category).

---

## Suggested order of execution

```
0 ─▶ 1 ─▶ 2 ─▶ 3 ─▶ 4 (+4a +4b) ─▶ 5 ─▶ 6 ─▶ 7
         │                          ▲
         └── Phase 2 output feeds 3, 4, 4a, 4b and 5
```

All decisions are made bar the Phase 2 sign-off. Phases 0–3 can start now.
Phase 4 (and 4a/4b) waits only on the xlOil worked examples. Phase 7.1 (Linux)
can slot in any time after Phase 3 but is lowest priority.

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
