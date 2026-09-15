# CLAUDE.md — AlgoQuantLib (AQ_LIB)

Solution-scoped guidance for Claude Code working inside `LIBRARY_UPDATE\AQ_LIB`.
The repo-wide context lives in `..\..\CLAUDE.md`; this file is the one that
applies while you are actually editing, building and testing the library.

---

## 0. Read/write boundaries

| Path | Access |
|---|---|
| `LIBRARY_UPDATE\AQ_LIB\` **and all subfolders** | **READ / WRITE** — this is the tree being rebranded |
| `REPO\.APPLES\` | **READ ONLY** — VS2017 branded original; the port source of truth |
| `REPO\.ALGO_QUANT_LIB\` | **READ ONLY** — manual backup of AQ_LIB + the external libs. Never write here. It is a safety copy, not a build tree. |
| `$(AQ_EXTERNAL_LIB_PATH)` (external dependency tree) | **READ ONLY** — except the two documented xlOil patches (§3) |
| Anything under `REPO\` outside the three paths above | **READ ONLY** |

This tree is authoritative. When the rebrand is complete it will be copied into a
fresh folder and committed to a new Git repo; the current Git stash repo is
temporary and will be deleted. Keep history clean enough to copy out.

---

## 1. What this library is

AlgoQuantLib (**AQ**) is a C++ quantitative finance library for **fixed income,
interest rates and credit**:

- Yield-curve framework calibrated on the **forward rate as the state variable**,
  monotone-preserving interpolation to keep forwards smooth, forwards integrated
  to discount factors — removes forward-curve kinks and the arbitrage they imply.
- **Analytical risk via the Jacobian approach** — IR sensitivities without
  bump-and-revalue. Not exercised for a while; needs testing and extension.
- Language bindings for **Python, C#, Java, R** via SwigWin.
- An **Excel add-in (XLL)**, migrating from Planatech XLL+ to **xlOil**.

Design principles the code is held to:

- Simple over clever. Human-readable first, laid out in steps; rely on inlining
  and `constexpr` so verbosity is free at compile time.
- Thread-safe and parallel-friendly; cache what matters.
- Stateless functional APIs in some layers, object-based stateful methods in others.
- Calendars and holidays updatable **without recompiling**.
- Full build is minutes, not hours.

---

## 2. Layout

```
AQ_LIB\
├── AlgoQuantLib-VS22.sln                    the solution
├── SetEnvironmentVariables.bat    one-time env-var setup (see §3.1)
├── Visualizer.natvis             debug visualisers for Julian-day dates (keep current)
├── readme.md                     library readme
├── MIGRATION_PLAN.md             the rebrand / xlOil port plan — READ THIS
├── projects\                     all .vcxproj / .filters / .user live here
├── src\
│   ├── math\                     LEGACY — deprecate / extract (§7)
│   ├── models\                   LEGACY — near-empty (~1.3k lines)
│   ├── calibration\              LEGACY — deprecate / extract (§7)
│   ├── etrading\                 core pricing / analytics + LWO handle framework
│   ├── validation\               single entry / contract layer (§4)  [namespace still validation_api]
│   ├── AQ_BINDINGS\              SWIG bindings — RENAME PENDING → AQ_API (§5.4)
│   ├── AQ_XLL\                   xlOil Excel add-in — POC only, port pending (§6)
│   └── GTEST\              test suite, sees the whole library
├── resources\                    end-user spreadsheets, toolkits, research guides
└── targets\                      build output (git-ignored, regenerated)
```

### 2.1 Current state (2026-09-11)

**This section was stale for a long time — it used to say "the code rebrand
has not started," which stopped being true many sessions ago. Corrected
here; treat `STATUS.md` and `rebrand\STATUS.md` as the live source, not this
paragraph, if the two ever drift again.**

- **Toolchain migration: done.** VS2022/v143/C++17, static runtime, `/MP`;
  projects `validation_api→validation`, `MLIB_CLIENT_API/AQ_BINDINGS→AQ_API`,
  `MLIBQ_ADDIN→AQ_XLL`; `ZEROMQ` dropped; external deps upgraded and routed
  through `$(AQ_EXTERNAL_LIB_PATH)`.
- **Phase 3 (identifier rebrand): done.** `validation_api`→`validation`,
  `me*`/`mir*`/`LA*`/`MA*`/`MB*` renamed per §6, `mir*` deleted wholesale,
  `LWO`→`AQObj`.
- **Phase 4 (xlOil XLL port): essentially complete.** `AQ_XLL` is no longer a
  proof of concept — **466 worksheet functions, covering 458 of 467
  `validation` wrappers (98%)**, build green in every configuration,
  GoogleTest passing. The 9 unmatched wrappers are deliberate exclusions
  (a false-positive name collision, one in-place mutator that doesn't fit a
  worksheet-function shape, and 7 legacy SABR functions decided superseded),
  not gaps. The only category-level work left is `Model`/`Generator`, which
  have **zero `validation` wrappers today** — new design-and-build work, not
  a port. Full detail and the re-runnable gap-audit script: `STATUS.md` §2
  and `rebrand\STATUS.md`.
- **Phase 4a (editions): done (2026-09-15).** `AQ_XLL` ships five build
  configurations — `Release` (Full) plus `Release_XL_Bond`/
  `Release_XL_Swap`/`Release_XL_Credit`/`Release_XL_Curve` — each built from
  the `src\Core`/`src\Optional` file split (§4.4). The `AQ_API` runtime
  edition manifest once planned for this phase is **dropped, not
  deferred** — `AQ_API` ships one full binary per language, no edition
  concept there.
- **Not yet started:** Phase 5 (bindings verification, SWIG regen); Phase 6
  (legacy extraction, licence headers, resources audit); Phase 7
  (Linux/CMake, clang-format, clean repo).

### 2.2 Visualizer.natvis

Dates are Julian day numbers — unreadable in the debugger without this file.
When a date/object type is renamed, **update the natvis entry in the same commit**
or debugging silently degrades.

---

## 3. Build

- **Toolchain:** Visual Studio 2022, platform toolset **v143**, **x64 only**.
- **Language standard:** `/std:c++17`. (There is no C++15 — VS2015 shipped C++14.)
- **Runtime:** static (`/MT`, `/MTd`). Every dependency — Boost, QuantLib, xlOil —
  must match. A `/MD` dependency links but misbehaves.
- **Parallelism:** `/MP` (`<MultiProcessorCompilation>true`) on every project. Do
  not remove it — it is the difference between a 1–2 minute and a 15 minute build.
- **Project settings live in the `.vcxproj` directly**, not in imported `.props`.
  Deliberate — one place. Do not "tidy" this into property sheets.
- **Linux MAKE/CMAKE build** exists but lags Windows. **In scope**, lower
  priority (late phase) — it must be re-validated *and* fully rebranded: the end
  state is that no file anywhere, Windows or Linux, carries a legacy name.

### 3.1 Environment variables

Set once by `SetEnvironmentVariables.bat` in this folder, then **restart Visual
Studio** (MSBuild reads the environment at launch). Used by project properties
*and* the binding build scripts — never hard-code these paths in either.

| Variable | Meaning |
|---|---|
| `AQ` | root of this source tree (derived from the script location) |
| `AQ_EXTERNAL_LIB_PATH` | root of the external dependency tree (`...\library\...`). Canonical spelling, no space. |
| `AQ_PYTHON_ROOT` | 64-bit Python the bindings are **compiled against** (needs `Include\Python.h` and `libs\`). Only the `*Python` configurations need it. The vendored Python 2.7/3.6/3.7 copies under the external libs are legacy and will not load on a modern machine. |

The script derives its defaults as folders *next to* `AQ_LIB`. If your external
libs live elsewhere (e.g. the backup tree), override at the prompt — the script
validates the path before writing.

### 3.2 Known build landmines (fixes are known if they resurface)

- **MAX_PATH.** Boost `b2` intermediate paths blow past 260 chars on a deep tree.
  Boost was built at `C:\temp\boost_1_91_0` for this reason. External libs may be
  relocated freely.
- **xlOil static build.** Needs `XLOIL_STATIC_LIB` + `SPDLOG_COMPILED_LIB`
  defines, plus `xlOilHelpers.lib` and `Oleacc.lib` as link inputs.
  `xlOilHelpers` has no upstream static config — it was made by hand.
- **xlOil patch (not upstream — must survive any xlOil upgrade).**
  `src\xlOil-COM\AppObjects.cpp` (~line 916) has an added
  `template class Collection<ExcelWorkbook, Excel::Workbooks>;` inside
  `namespace xloil`. The static build does not force template member
  instantiation the way the dllexport DLL build does.
- **Reference project:** xlOil's `TestAddin` is the known-good static-XLL sample.
  When `AQ_XLL` fails to link, diff its resolved compiler/linker command lines
  against `TestAddin`'s with `/VERBOSE:LIB`.
- **Dead XLL+ paths.** `projects\AQ_XLL.vcxproj` still lists
  `XllPlus\7.0\VS14.0\include` and `...\lib\x64` on its include/library paths.
  Remove when touching that project.
- **AQ_XLL: every xlOil function must have exactly as many `.arg()` calls as the
  `XLO_FUNC_START` signature has parameters.** One extra `.arg()` makes
  `xlAutoOpen` throw `xloil::Exception<std::runtime_error>` ("Too many args for
  function"), which Excel surfaces as an unhandled C++ exception and the whole
  add-in fails to load — the XLL itself is a valid binary, so this looks like a
  "corrupt / unsupported version" problem but is not. It builds green (the
  mismatch is a runtime registration check, not a compile error). When the add-in
  will not load, audit param-vs-`.arg()` counts across `src/AQ_XLL/src/*.cpp`
  first. (Hit once: `aqObjSave` shipped with 3 params and 4 `.arg()` in commit
  `a8b50b47`.)
- **C++17 fixes already applied elsewhere and expected again in ports:**
  `_HAS_STD_BYTE` / `byte` ambiguity, `register` keyword, `auto_ptr`→`unique_ptr`,
  `Disposable` removal, `mem_fun_ref` / `bind2nd`→lambdas, and an ICE in
  `LAPriceCMSSpreadCalibration.cpp` needing `/Od` on that one file.

### 3.3 Dependency licences (the library is sold — this matters)

| Dependency | Version | Licence | Note |
|---|---|---|---|
| Boost | 1.91 | BSL-1.0 | permissive |
| QuantLib | 1.43 | modified BSD | permissive |
| xlOil | 0.19.0 | Apache-2.0 | **we ship a patched copy — Apache-2.0 requires marking modified files** |
| Eigen | (Eigen3) | MPL-2.0 | file-level copyleft. Linking is fine. **Never patch Eigen — wrap it.** |
| Adept | 2.0.3 | Apache-2.0 | permissive |
| GoogleTest | 1.17 | BSD | test-only, not shipped |

Action carried in the plan: add `NOTICE` / `THIRD_PARTY_LICENSES.md` at this root.

**AlgoQuantLib's own licence:** proprietary, no redistribution. Copyright and
licence text live only in the root `LICENSE` file — never in source-file or
console banners; an interim `LICENSE` placeholder goes in early, the final EULA
replaces `LICENSE` later with **zero source churn**. See `..\..\CLAUDE.md` §7
for the licence decision and §7.1 for the banner-content rule (no copyright, no
company/person names, no tool/migration references — technical help only,
research citations allowed).

### 3.4 Lint & formatting

- **`clang-format`** replaces the legacy AStyle utility. `.clang-format` at this
  root, tuned to the current style. The single bulk reformat happens **after**
  the mass renames (plan Phase 6), never during — mid-rename reformatting buries
  the real diff.
- **`clang-tidy`** — `readability-identifier-naming` as a rebrand-completeness
  guard (AQ convention: camelCase, types Upper, members trailing `_`); other
  checks advisory. Config in Phase 6, CI gate from Phase 7.
- Until the rebrand is done, linters are **guards, not gates**.

---

## 4. Architecture — how code flows

```
   AQ_API (SWIG: Py/C#/Java/R)        AQ_XLL (xlOil add-in)        GTEST
                \                        /                          (sees all)
                 \                      /                              |
                  ▼                    ▼                               |
              ┌──────────────────────────────┐                         |
              │          validation          │ ◄───────────────────────┘
              │  single entry / contract     │
              │  `try` prefix                │
              └──────────────┬───────────────┘
                             ▼
              ┌──────────────────────────────┐
              │           etrading           │  core pricing / analytics
              │        (+ LWO handles)       │
              └──────────────┬───────────────┘
                             ▼
        ┌────────────────────────────────────────┐
        │  LEGACY — deprecate / extract (§7)      │
        │  calibration │ models │ math           │
        └────────────────────────────────────────┘
```

### 4.1 `validation` is the contract — and the GOLDEN SOURCE for names

Every public call — the Excel XLL and the Python / C# / Java / R bindings —
routes through `validation`. Nothing bypasses it. It (1) validates inputs once
so every language returns identical results, and (2) records inputs/outputs,
from which `GTEST` cases are generated.

**The `validation` wrapper name is the single source of truth for every public
function name.** A wrapper is `try` + `<GoldenName>`; the XLL function, every
binding method (Python / C# / Java / R) and the `GTEST` case name are all
`<GoldenName>` verbatim. So:

- To name or rename a function, name the `validation` wrapper first; the other
  surfaces follow.
- `tryAqDateYearFraction` ⇔ `aqDateYearFraction` in the XLL, `aqDateYearFraction`
  in Python/C#/Java/R, `…AqDateYearFraction…` in the test name.
- `docs\api_map.csv` and `rebrand\tools\api_pair_check.py` enforce the pairing.

(Today the wrappers are `tryMe…` in namespace `validation_api` — both change in
the rebrand.)

An `etrading` function is **not public** until it has a `validation` wrapper. Do
not expose `etrading` symbols directly to `AQ_API` or `AQ_XLL`.

### 4.2 Naming carries meaning — preserve it

- **Lowercase project name = statically linked; Uppercase = dynamically linked.**
  `math`, `models`, `calibration`, `etrading`, `validation` are static;
  `AQ_API`, `AQ_XLL`, `GTEST` are dynamic.
- **Project name ≠ output name.** All consumer artefacts are named
  **AlgoQuantLib** so users write `import AlgoQuantLib`:
  `AlgoQuantLib.pyd` / `AlgoQuantLib.xll` / `AlgoQuantLib.dll`.

### 4.3 The `config` folder — calendars & generators

`config` ships with **both** the `AQ_API` and `AQ_XLL` packages — the no-recompile
customisation surface:

- **Holiday / calendar files** (MarketWire / SwapsWire) — loaded at runtime;
  end-users edit holidays without a refreshed add-in. `config` *is* the mechanism
  behind "calendars without recompiling".
- **Generators** — JSON **instrument & model static-data templates**. Static data
  is ~90% of a booking; a generator carries the conventions so a user books a USD
  swap from `notional, effective, maturity, rate, pay/receive` (or a UST from
  `notional, issue/settle, maturity, coupon`) instead of hand-building
  conventions. Consumed by the object layer: generator + overrides → handle.
  Seeds: `resources\config\{SWAP,BOND,CURVE}_GENERATOR`.

Generator JSON and calendar config get rebranded like code (legacy names,
client-specific conventions stripped). Both deploy paths must package `config`.

### 4.4 Editions — DONE (2026-09-15), AQ_XLL-only

`AlgoQuantLib` ships as **Bonds / Swaps / Credit / Curves / Full** editions.
**Decided (Nicholas, 2026-09-15): editions are an `AQ_XLL`-only concept,
gated at compile time — there is no runtime edition gate, and the
`AQ_API` runtime-manifest design once planned for this phase is dropped,
not deferred.** `AQ_API` (Python/C#/Java/R) ships one full binary per
language with every category always registered; editions are not a concept
that surface has.

**`AQ_XLL` mechanism (decided, Nicholas 2026-09-11; build configurations
shipped 2026-09-15):** `projects\AQ_XLL.vcxproj.filters` organises every
`src\AQ_XLL\src\*.cpp` file under two Solution Explorer filters:

- **`src\Core`** — files that must build into **every** edition
  (`aqXllTools.cpp`, `aqMain.cpp`, `aqDate.cpp`, `aqObject.cpp`, `aqMath.cpp`,
  `aqTool.cpp`, plus **`aqCurve.cpp` and `aqIR.cpp` (was `aqInterestRate.cpp`,
  renamed 2026-09-12 — Nicholas: `InterestRate`→`IR`), promoted 2026-09-12**:
  every priced product depends on discount/forward curves and fixing tables,
  so these two are infrastructure too, not a product edition of their own —
  see §5.1a for the category rename detail and `MIGRATION_PLAN.md` §4a.5 for
  why they moved out of `Optional`.
- **`src\Optional`** — one file per product category (`aqBond.cpp`,
  `aqSwaption.cpp`, `aqCMS.cpp`, `aqTRS.cpp`, `aqCapFloor.cpp`, `aqFX.cpp`,
  `aqFuture.cpp`, `aqInflation.cpp`, `aqVolatility.cpp`, `aqAssetSwap.cpp`,
  `aqCredit.cpp`, `aqSwap.cpp`, and every new category file going forward).
  **Every new `AQ_XLL` category file is added to `src\Optional`, never
  `src\Core`** — unless it turns out to be cross-cutting infrastructure like
  `Curve`/`IR`, in which case flag it for `Core` rather than assuming.

**Five build configurations now exist** alongside
`Debug`/`DebugEditAndContinue`/`ReleaseProfiler`: **`Release`** (the `Full`
edition — excludes nothing, every `src\Optional` file builds into it) and
**`Release_XL_Bond`**, **`Release_XL_Swap`**, **`Release_XL_Credit`**,
**`Release_XL_Curve`** — each compiles every `src\Core` file plus only the
`src\Optional` file(s) its edition needs; excluded files are marked
"Excluded From Build" for that configuration in the `.vcxproj`, same
mechanism as any normal per-configuration file exclusion. A separate
**`Release_XL_Manifest`** configuration layers a further, function-level cut
on top of any of these via `resources\manifest\activeList.txt` (renamed
2026-09-15 from `active.txt`; `demo.txt` renamed to `demoList.txt`
alongside it; see
`rebrand\STATUS.md` for the manifest-generator tooling) — that is a finer
cut than the edition mechanism, not a sixth edition. This is specific to
`AQ_XLL` (a native binary per edition is cheap to produce for one add-in);
`AQ_API` bindings (Python/C#/Java/R) build once per language and register
every category unconditionally — no per-language edition SKUs, since
building N native artefacts per language was the combinatorial blow-up
CLAUDE.md §5.2 always ruled out, and the runtime-gate alternative to avoid
that blow-up is the piece that got dropped.

---

## 5. Naming conventions

### 5.1 Public API functions

`library prefix + Category + FunctionName` — `aq` + `Date` + `YearFraction`
→ **`aqDateYearFraction`**. Gives an IntelliSense-style grouped feel in Excel and
in every binding: type `aqDate` and the date functions surface together.

**Categories are SINGULAR.** `aqDate`, not `aqDates`; `aqSwap`, not `aqSwaps`.

**Canonical category list — LOCKED (21):**

`Date`, `Curve`, `FX`, `Inflation`, `Volatility`, `IR`, `Future`, `Swap`, `AssetSwap`,
`CMS`, `TRS`, `CapFloor`, `Swaption`, `BondOption`,
`BondFutureOption`, `Bond`, `Credit`, `Math`, `Model`, `Generator`, `Tool`

`Curve` is the rates yield-curve framework only — *not* bond or credit curves;
`Bond` includes bond-curve fitting, `Credit` includes hazard/survival curves.
`Math` is low-level building blocks for own-calculation / result replication.
`Model` may be sparse initially. **No** `Product` category.

- **No `Option` category.** An option product is its own category —
  `CapFloor`, `Swaption`, `BondOption`, `BondFutureOption`. Same principle for
  swaps: `AssetSwap`, `CMS` and `TRS` are their own
  categories, not members of `Swap`; `Swap` is the vanilla swap plus its legs
  and schedules. `Inflation` is likewise its own category, not part of `Curve`.
  A user reaches for the product, not the umbrella.
- **`Ois` is NOT its own category (decided, Nicholas 2026-09-11) — it lives
  inside `Swap`.** Unlike `AssetSwap`/`CapFloor`/`Swaption` etc., an OIS is not
  a structurally distinct product — it is a vanilla swap whose floating leg
  compounds an overnight index instead of a term rate. So it is a **product
  qualifier inside `Swap`**, not a category: `aqSwapOisPV`, `aqSwapOisParRate`
  (stateless; the object form would be `aqSwapOisObject<Function>` if/when
  needed) — see the form table below. Code-wise, `aqSwapOis*` lives in
  `aqSwap.cpp` alongside `aqSwap*`, **not** a separate `aqOis.cpp`; the
  validation wrappers live in `src\validation\Swap\` alongside the vanilla-swap
  wrappers. **Done (2026-09-11):** the pre-existing `tryAqOisPV`/
  `tryAqOisParRate` wrappers (+ `*LVBKeys` companions) — which predated the
  current category scheme — were renamed to `tryAqSwapOisPV`/
  `tryAqSwapOisParRate`, files included, along with the matching `AQ_API`
  (`aqSwapOisPV`/`aqSwapOisParRate`) and `GTEST`
  (`TryAqTestTradeEUROISParRate.cpp`) renames. See `rebrand\STATUS.md` for
  the detail.
- **`InterestRate` renamed to `IR`** (decided, Nicholas 2026-09-12) — shorter,
  matches how the category reads in Excel's function list. Full-stack rename
  (`validation`'s `tryAqInterestRate{FixingTable,FutureFra,ObjectFra}.{h,cpp}`
  → `tryAqIR*`; `AQ_XLL`'s `aqInterestRate.cpp` → `aqIR.cpp`, all 13 functions;
  `AQ_API`'s `aqInterestRateFixingTable*` bindings in `aqCurveObject.{h,cpp}`
  → `aqIRFixingTable*`; the matching `GTEST` call sites and 3 fixture files).
  Detail: `rebrand\STATUS.md`.

Use these 21, identically in `validation` / `AQ_XLL` / `AQ_API` / `GTEST`.
Detail: `MIGRATION_PLAN.md` §2.2.

**Stateless vs object (stateful).** Most product functions take a cached object
name; a minority are stateless (data in, value out). The two surfaces share the
category list and are told apart by the word `Object`:

| Form | Meaning | Example |
|---|---|---|
| `aq<Category><Function>` | stateless — data in, value out | `aqBondScheduleKeys` |
| `aq<Category>Object<Function>` | operates on a cached instance of the category's product | `aqBondObjectDirtyPrice`, `aqSwapObjectParRate` |
| `aq<Category><SubObject><Function>` | operates on a cached *named* sub-object (Curve, Generator, MarketData, Model, FixingTable); the sub-object already denotes an object, so `Object` is not repeated | `aqBondCurveYield`, `aqBondGeneratorCreate`, `aqIRFixingTableValues` |
| `aq<Category><Variant><Function>` | a same-category **product variant** that doesn't earn its own top-level category — stateless (`<Variant>` before `Object`, no `Object` word) or, if ever needed, stateful (`<Variant>Object<Function>`) | `aqSwapOisPV`, `aqSwapOisParRate` (stateless OIS-swap forms; not `aqSwapObjectOis*`) |
| `aqObject<Lifecycle>` | generic handle lifecycle, no category | `aqObjectLoad`, `aqObjectSave`, `aqObjectExists`, `aqObjectClearCache` |

`FX` is its own category (not folded into `Curve`): FX forwards and FX swaps are
derived from discount / xccy curves, but `FX` is what a user reaches for, and
discoverability wins over taxonomy here.

### 5.1a Category-migration sequence — always in this order

Whenever a category (or a variant like `Ois`) is migrated or newly exposed, do
the four surfaces **in this order** — never start with `AQ_XLL`:

1. **`validation`** — confirm or rename the wrapper to the golden-source name
   (`tryAq<Category>[<Variant>][Object]<Function>`). Legacy wrappers that
   predate the current category scheme (like `tryAqOis*`) get renamed here
   first; this is the one and only place a public name is decided.
2. **`GTEST`** — update call sites, and file/suite names where they encode the
   old wrapper name, to match.
3. **`AQ_API`** — update the SWIG-bound method name (and source file name) in
   every language binding to match.
4. **`AQ_XLL`** — write or rename the worksheet function to the identical name
   (minus `try`); `.arg()` count must equal the parameter count.

Then run `rebrand\tools\api_pair_check.py` (HARD GATE must be 0) and rebuild
`validation → AQ_API → AQ_XLL → GTEST`. Any drift found between the four
surfaces during a migration gets cleaned up as part of that migration, not
deferred — `validation` is always the tie-breaker.

**File-per-category is the default, not a hard rule.** `AQ_XLL` category files
are normally `aq<Category>.cpp` — but `BondOption` and `BondFutureOption`
(decided, Nicholas 2026-09-11) are code-organized together in `aqBond.cpp`
rather than their own files: `aqBondOptionObjectCreate` is the *only* creator
for both (`tryAqBondFutureOptionObjectPV`/`Greeks` `dynamic_pointer_cast` the
very same cached `etrading::BondOption` to price it against a bond-future price
instead of a bond spot price — there is no `aqBondFutureOptionObjectCreate`).
The category names themselves are unchanged — `BondOption` and
`BondFutureOption` stay separate entries in the locked list, and their
functions keep the `aqBondOptionObject*` / `aqBondFutureOptionObject*` names;
only the file they live in moved. Consolidate a category's file into a sibling
category's file when they operate on the same underlying cached object like
this; don't do it merely because two category names share a prefix.

### 5.2 Two orthogonal groupings — do not conflate

- **Discoverability** = category prefix (`aqCurve…`, `aqSwap…`).
- **Gating** = shipped edition (Bonds / Swaps / Credit / Curves / Full),
  **`AQ_XLL`-only** (§4.4). Editions cut *across* categories, enforced at
  compile time via the `src\Core`/`src\Optional` filter split and the
  `Release_XL_*` build configurations — a native `.xll` per edition is cheap
  to produce for one add-in. `AQ_API` has no edition concept: one binary per
  language, every category always registered. Protection on `AQ_XLL` is
  light by design — the library is not useful without the shipped examples
  and templates.

### 5.3 C++ style (do not "improve" this)

- Self-describing names: `spot_`, `strike_`, `vol_`, `timeToExpiry_`,
  `interestRate_` — never `s`, `k`, `v`, `t`, `r`, `x`, `z1`.
- camelCase. Classes/objects start uppercase; functions/variables lowercase;
  **class members end with a trailing underscore**.
- Prefer enumerated types over strings.
- Readable steps over one-liners.

### 5.4 Rebrand mapping

| Old | New |
|---|---|
| Mizuho / client names | AlgoQuantLib |
| MLIB | AQ |
| `me` (function prefix) | `aq` |
| `mir` (function prefix) | **deprecate — do not migrate** (produce call-graph first) |
| `LA`, `LB` ("Legacy Analytics" type/object prefixes) | **`AQL`** ("AQ Legacy") — marks legacy-to-deprecate, greppable vs new `AQ*` |
| `MA`, `MB` (type/object prefixes) | `AQ` — confirm per project (`math` had zero real ones) |
| `MLIB_*` macros | `AQ_*` |
| LWO (light-weight objects) — **C++ classes** | **`AQObj`** prefix: `AQObjCurve`, `AQObjUtilities`, `AQObjCurveDayAdjustment`, `AQObjHandleEnums`; screaming-snake macros take `AQOBJ_` (`AQOBJ_KEY`); free predicate `isLWOObject → isAQObject`. |
| LWO — **public function names** (`meLWO…`) | `aq<Category>Object<Function>` — the `Object` word after the singular category is what separates the handle API from the stateless twin: `aqSwapObjectPV`, `aqBondObjectDirtyPrice`. Named sub-objects don't repeat it: `aqCurveMarketDataDisplay`. Generic lifecycle ops: `aqObjectLoad`, `aqObjectSave`, `aqObjectClearCache`. |
| `mir*` (whole stack: `AQ_API\mir*` 58 files, `validation\tryMir*` ~35, `LAXL.cpp` 156 fns) | **delete wholesale** — self-contained, no inbound `aq`/`me` deps (0.5 call-graph) |
| `msc*`, `LoanCalculations`, `SupervisoryRules`, `CashflowClient` + securitisation cluster | **delete — client-specific**; keep only what `Credit` genuinely needs (0.6 removal map) |
| project `AQ_BINDINGS` | **`AQ_API`** (agreed) — update `.vcxproj`/`.filters`/`.user`, `.sln`, folder, SWIG `.i`, the 8 `generate*`/`deploy*` batch files, and the pre/post-build `<Command>` lines |
| `validation_api` (namespace) | `validation` |
| calendar holiday-centre delimiter `:` | **`+`** — one named constant + one helper; **accept `:` too during transition** unless a `:` parsing collision is found (plan Phase 0.8), then `+` only |
| Excel function names | **clean break** — every `me*`, hidden aliases included, becomes `aq*`; **no forwarding aliases**; removed names go in the release notes |

**End-state gate:** when the rebrand is done, a tree-wide case-sensitive grep for
`Mizuho|MLIB|\bme[A-Z]|\bmir[A-Z]|\b(LA|MA|MB|LB)[A-Z]|validation_api|XllPlus`
over **everything** — source, `projects\`, scripts, `resources\`, `examples\`,
docs, the Linux build — returns nothing but third-party headers and genuine
string data. Legacy-branded or client-specific resources/examples are rewritten
or removed, not carried.

### 5.5 The `me` prefix is the biggest hazard

`me` is a substring of ordinary English and C++: *some, name, element, time,
come, theme, parameter, MEMORY, gamma…*. A naive replace **will** corrupt the
codebase in ways that compile.

- **Never** run an unanchored, case-insensitive replace of `me`, `MA`, `MB`,
  `LA`, `LB`.
- Word-boundary, case-sensitive, prefix-anchored patterns only — e.g.
  `\bme(?=[A-Z])`.
- `MA`/`MB`/`LA`/`LB` anchored to a following uppercase letter **and** checked
  against the extracted symbol list. The real convention is `LA[A-Z][a-z]` /
  `LA[0-9]D`; `MAX`/`MASK`/`MATRIX`/`LABEL`/`LAST`/`LAPACK` etc. are false
  positives to skip.
- **`LA` → `AQL`** (not `AQ`) — "Legacy Analytics" → "AQ Legacy", keeps the
  legacy tree marked and greppable. `MA`/`MB` → `AQ`.
- **Build the identifier list first (`rebrand\tools\prefix_census.py`), get
  sign-off, rename from the approved list.**
- Reviewable batches — one project or category at a time. Build between each.
- Never rename inside string literals, third-party headers, or `.APPLES`.
- `LWO` → `AQObj`; predicate stays `isAQObject()`. Never write `AQObjbject` —
  when renaming `AQO`→`AQObj`, exclude tokens that already contain `AQObject`
  (`AQObjects`, `IsAQObject`, `isAQObject`).

### 5.6 Renaming Excel functions — clean break

`meDatesYearFraction` → `aqDateYearFraction` turns every saved sheet into
`#NAME?`. **Decided:** clean break — no `me*` spelling survives, no forwarding
aliases (hidden or otherwise). Fresh product, fresh clients, no old workbooks to
protect. As you rename, collect the removed public names into a `RELEASE_NOTES`
"renamed / removed functions" list.

---

## 6. The Excel layer (`AQ_XLL`)

**Architecture decision:** xlOil for **registration and marshalling only**. Keep
the existing object/handle framework underneath — do **not** use xlOil's cache.

The handle framework is a deliberate feature and matters to the product:

- A `map<int,string>`: the string is the cell address, the int a
  monotonically-increasing counter that never repeats (no collision risk).
- The int is a compact hash of the cell location — short numeric sheet handles,
  not long embedded cell references.
- The counter suffix updates on every recalculation, so the handle string
  changes and Excel's dependency tree correctly fires downstream functions.

Port it from `.APPLES\APPLE\src\etrading` (`LWOCurve`, `IsLWOObject`,
`HandleEnums`, `LWOUtilities`, `StructuredExceptionHandler` — shells already
present in `src\etrading`), rebrand `LWO→AQObj`, keep the behaviour exactly.

### 6.1 Port source

`.APPLES\APPLE\src\MLIBQ_ADDIN` — 40 files, ~82k lines, **653** exported Excel
functions (`IMPLEMENT_XLLFN4`). Each old function is: a FunctionWizard
registration blob + `_4`/`_12` C export wrappers + a hand-written `_Impl` body.
xlOil deletes the first two (replaced by `XLO_FUNC_START/END` + `.help()/.arg()`)
and owns the SEH + exception→string that `MLIB_START/END` did. **Only the `_Impl`
body carries across**, and it shrinks (`ExcelObj`/`ExcelArray` vs
`CXlOper`/`CXlStringArg`).

This is **not** a verbatim 653-function copy — it is the reference for *which*
functions survive into the agreed category surface and *how each argument
marshals*. Build the keep/drop/rename inventory first (plan Phase 1).

`XllPlusTips*` (~4.6k lines, ~40 helpers on `CXlOper`/`COper`): audit
keep/drop before porting. Most `getVectorOf<T>` / `populateExcelArrayWith…` /
`fromXToXloResult` map onto xlOil primitives and should be dropped. Keep the
genuinely-ours bits: `AQDate`/`AQString`/`Variant` conversions, the handle I/O
(`OutputObject`/`ChangeObject`), CSV loaders, `xlDumpMemory`.

`msc*` add-in files (structured credit, other client) — **do not port; delete.**

### 6.2 Porting order

Date → Tool → Curve → Swap → products → Model. Each ported function needs a
`validation` wrapper (§4.1) **and** a GoogleTest case before it counts as done.

### 6.3 File organisation for editions — `src\Core` / `src\Optional`

Every `AQ_XLL` category `.cpp` under `projects\AQ_XLL.vcxproj.filters` sits in
one of two filters — see §4.4 for the full edition-gating design:

- **`src\Core`** — always-built infrastructure (`aqXllTools.cpp`,
  `aqMain.cpp`, `aqDate.cpp`, `aqObject.cpp`, `aqMath.cpp`, `aqTool.cpp`,
  `aqCurve.cpp`, `aqIR.cpp`).
- **`src\Optional`** — one file per product category. **New category files go
  here, not `Core`** — unless it's cross-cutting infrastructure every product
  depends on, like `Curve`/`IR` turned out to be.

**Done (2026-09-15).** The per-edition build configurations
`Release_XL_Bond`/`Release_XL_Swap`/`Release_XL_Credit`/`Release_XL_Curve`
exist alongside `Debug`/`DebugEditAndContinue`/`ReleaseProfiler`/`Release` —
each builds `Core` plus only the `Optional` file(s) its edition needs,
everything else marked excluded from that configuration. `Release` (full)
excludes nothing.

---

## 7. Legacy — `calibration`, `math`, `models`

Direction: **deprecate as much as possible.** There is no `mapping` project — the
set is `calibration`, `math`, `models` (`models` is already near-empty).

Complication: live date logic, string logic and other genuinely-used code sits in
here. So the approach is **extraction, not deletion**:

1. Build a usage map — what in `calibration`/`math`/`models` is reachable from
   `validation` or `etrading`?
2. Extract the live pieces into a clean `core` / `utils` project under
   `etrading`, rebranded to `AQ`.
3. Point callers at the new home.
4. Delete the dead remainder, one project at a time, building between each.

Do not delete outright — check what is needed first.

---

## 8. Testing (`GTEST`)

GoogleTest 1.17, top of the stack. Cases are generated from the `validation`
input/output recordings.

**Before renaming anything, capture a full input/output recording set on the
current build.** Renames are behaviour-preserving by definition — after each
stage, re-run and diff. Any numerical difference is a bug from that stage.

Priority coverage to add:

- Yield-curve framework + Jacobian risk — test against analytic cases and against
  bump-and-revalue as the reference for the Jacobian.
- Fixed-income price/yield — the standing claim is Bloomberg-exact both ways;
  protect it with a regression test.
- Date logic — it is being extracted out of legacy.

---

## 9. Working agreement

**Changes**

- Minimal. Change only what is required.
- **Never delete code, comments or existing formatting** unless asked. Comments
  are the author's.
- **Propose before restructuring.** Describe the better solution and ask.
- Show the full file or full diff after each change.
- Present options rather than making silent decisions.

**Renaming**

- Enumerate the symbols and show the list for approval **before** applying.
- Anchored, case-sensitive patterns only (§5.5).
- One project/category per batch. Build between batches.
- Never touch `.APPLES`, `.ALGO_QUANT_LIB`, or third-party headers.

**Verification**

- After any change: the solution builds in all configurations.
- After any rename: GoogleTest produces numerically identical output to the
  pre-rename baseline.

**Communication**

- Be direct. Say when something is unknown rather than guessing.
- Surface tensions and disagreements explicitly.

**Migration record**

- **Always update `rebrand\STATUS.md` during this migration project.** Every time
  code is ported, renamed, deleted or a decision is taken, record it in
  `STATUS.md` in the same session — what changed, what is verified, what is
  deferred. `STATUS.md` is the live running record; a change that is not in it
  did not happen as far as the next session is concerned.

---

## 10. Pointers

- `MIGRATION_PLAN.md` — the phased plan; the authority on sequence and decisions.
- `..\..\CLAUDE.md` — repo-wide context and the rebrand rationale.
- `.APPLES\APPLE\APPLE_2017.sln` — the VS2017 original, port source.
- `readme.md` — end-user / new-developer entry point.