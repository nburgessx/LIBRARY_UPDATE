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

### 2.1 Current state (2026-09)

The **toolchain** migration is done. The **code rebrand has not started**:

- Namespaces still `validation_api`, `etrading`.
- ~25,900 `LA*` / `MA*` / `MB*` identifiers untouched.
- ~340 `me*` / `mir*` / `tryMe*` source files untouched.
- `AQ_XLL` is a 5-file, ~140-line xlOil proof of concept. The ~82k-line,
  653-function XLL+ add-in in `.APPLES\APPLE\src\MLIBQ_ADDIN` is **not ported**.

Done: VS2022/v143/C++17, static runtime, `/MP`; projects `validation_api→validation`,
`MLIB_CLIENT_API→AQ_BINDINGS`, `MLIBQ_ADDIN→AQ_XLL`; `ZEROMQ` dropped; external
deps upgraded and routed through `$(AQ_EXTERNAL_LIB_PATH)`; xlOil static build
wired (`AlgoQuantLib.xll` registers, hello-world functions return values).

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

**AlgoQuantLib's own licence:** proprietary, no redistribution. A frozen per-file
header names the `LICENSE` file but quotes no terms; an interim `LICENSE`
placeholder goes in early; the final EULA replaces `LICENSE` later with **zero
header churn**. See `..\..\CLAUDE.md` §7 for the exact header text.

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

### 4.1 `validation` is the contract

Every public call — bindings and XLL — routes through `validation`. Nothing
bypasses it. It (1) validates inputs once so every language returns identical
results, and (2) records inputs/outputs, from which `GTEST` cases are
generated.

Functions here carry a `try` prefix: `try` + library + category + function, e.g.
`tryAqDatesYearFraction`. (Today they are `tryMe…` in namespace `validation_api`
— both change in the rebrand.)

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
  conventions. Consumed by the AQObj object layer: generator + overrides → handle.
  Seeds: `resources\config\{SWAP,BOND,CURVE}_GENERATOR`.

Generator JSON and calendar config get rebranded like code (legacy names,
client-specific conventions stripped). Both deploy paths must package `config`.

### 4.4 Editions

`AlgoQuantLib` ships as **Swaps / Bonds / Credit / Full** — one artefact per
language, **gated at runtime**, not per-edition builds. Working design (Phase 4a):
one binary; `config\editions.json` maps edition → categories; `config\licence.json`
(or a key) names the entitlement, read behind one function so enforcement can
harden later without touching registration; at `xlAutoOpen` / import, register
only the entitled categories; `aqToolsEdition()` reports the active edition.
Editions cut **across** categories, so the gate is category-level.

---

## 5. Naming conventions

### 5.1 Public API functions

`library prefix + Category + FunctionName` — `aq` + `Dates` + `YearFraction`
→ **`aqDatesYearFraction`**. Gives an IntelliSense-style grouped feel in Excel and
in every binding: type `aqDates` and the date functions surface together.

**Canonical category list — LOCKED (20):**

`Dates`, `Curves`, `FX`, `Inflation`, `Vols`, `Rates`, `Swaps`, `AssetSwap`,
`ConstantMaturitySwap`, `TotalReturnSwap`, `CapFloor`, `Swaption`, `BondOption`,
`BondFutureOption`, `Bonds`, `Credit`, `Math`, `Models`, `Generators`, `Tools`

`Curves` is the rates yield-curve framework only — *not* bond or credit curves;
`Bonds` includes bond-curve fitting, `Credit` includes hazard/survival curves.
`Math` is low-level building blocks for own-calculation / result replication.
`Models` may be sparse initially. **No** `Products` category.

- **No `Options` category.** An option product is its own category —
  `CapFloor`, `Swaption`, `BondOption`, `BondFutureOption`. Same principle for
  swaps: `AssetSwap`, `ConstantMaturitySwap` and `TotalReturnSwap` are their own
  categories, not members of `Swaps`; `Swaps` is the vanilla swap plus its legs
  and schedules. `Inflation` is likewise its own category, not part of `Curves`.
  A user reaches for the product, not the umbrella.

Use these 20, identically in `validation` / `AQ_XLL` / `AQ_API` / `GTEST`.
Detail: `MIGRATION_PLAN.md` §2.2.

`Objects` is **no longer a category** either — the handle API is distinguished by the
`aqObj` *prefix* instead, so the same product categories serve both surfaces:

| Form | Meaning | Example |
|---|---|---|
| `aq<Category><Function>` | stateless — data in, value out | `aqSwapsParRate` |
| `aqObj<Category><Function>` | handle API — object handle in | `aqObjSwapsParRate` |
| `aqObj<Lifecycle>` | handle lifecycle, no category word | `aqObjLoad`, `aqObjSave`, `aqObjClearCache` |

`FX` is its own category (not folded into `Curves`): FX forwards and FX swaps are
derived from discount / xccy curves, but `FX` is what a user reaches for, and
discoverability wins over taxonomy here.

### 5.2 Two orthogonal groupings — do not conflate

- **Discoverability** = category prefix (`aqCurves…`, `aqSwaps…`).
- **Gating** = shipped edition (Swaps / Bonds / Credit / Full). Editions cut
  *across* categories and are enforced by a **runtime edition manifest with gated
  registration** (§4.4), not separate builds. Protection is light by design — the
  library is not useful without the shipped examples and templates.

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
| LWO (light-weight objects) — **C++ classes** | **`AQObj`** prefix: `AQObjCurve`, `AQObjUtilities`, `AQObjCurveDayAdjustment`, `AQObjHandleEnums`; screaming-snake macros take `AQOBJ_` (`AQOBJ_KEY`); free predicate `isLWOObject → isAQObject`. Matches the public `aqObj` prefix. |
| LWO — **public function names** (`meLWO…`) | `aqObj` + the **same** category as the stateless twin: `aqObjSwapsPV`, `aqObjCurvesMarketDataDisplay`. Lifecycle ops drop the category: `aqObjLoad`, `aqObjSave`, `aqObjClearCache`. The prefix — not a separate `Objects` category — is what separates the two surfaces. |
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

`meDatesYearFraction` → `aqDatesYearFraction` turns every saved sheet into
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

Dates → Tools → Curves → Swaps → Products → Models. Each ported function needs a
`validation` wrapper (§4.1) **and** a GoogleTest case before it counts as done.

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

---

## 10. Pointers

- `MIGRATION_PLAN.md` — the phased plan; the authority on sequence and decisions.
- `..\..\CLAUDE.md` — repo-wide context and the rebrand rationale.
- `.APPLES\APPLE\APPLE_2017.sln` — the VS2017 original, port source.
- `readme.md` — end-user / new-developer entry point.