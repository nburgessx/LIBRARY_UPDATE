# AlgoQuantLib

A C++ quantitative finance library for **fixed income, interest rates and
credit** — yield-curve construction, instrument pricing and risk — with language
bindings for Python, C#, Java and R, and an Excel add-in.

**Author:** [Nicholas Burgess](mailto:nburgessx@gmail.com) — AlgoQuantHub

> **Status:** the library is mid-rebrand from its original VS2017 form. Internal
> identifiers, namespaces and some project names are still being migrated. See
> `MIGRATION_PLAN.md` for the sequence and `CLAUDE.md` for working guidance.

---

## What it does

- **Yield curves** calibrated on the *forward rate* as the state variable, with
  monotone-preserving interpolation so the forward curve stays smooth and free of
  the kinks that imply arbitrage. Discount factors are obtained by integrating
  the forwards.
- **Analytical risk** via the Jacobian approach — interest-rate sensitivities
  without bump-and-revalue.
- **Instruments:** bonds, vanilla and OIS swaps, basis and cross-currency swaps,
  FRAs and futures, caps/floors, CDS, and vanilla options.
- **Consumers:** a single validated core, exposed identically through Python, C#,
  Java, R (SWIG) and Excel (xlOil), all shipping as **`AlgoQuantLib`**.

---

## Repository layout

```
AQ_LIB\
├── AlgoQuantLib-VS22.sln                  Visual Studio 2022 solution
├── SetEnvironmentVariables.bat  one-time environment setup
├── Visualizer.natvis            debugger visualisers for date types
├── CLAUDE.md                    working guidance
├── MIGRATION_PLAN.md            rebrand / xlOil-port plan
├── projects\                    all .vcxproj files
├── src\
│   ├── math\  models\  calibration\   legacy — being deprecated / extracted
│   ├── etrading\                       core pricing and analytics
│   ├── validation\                     single entry / contract layer
│   ├── AQ_API\                        SWIG bindings (Py/C#/Java/R)
│   ├── AQ_XLL\                         xlOil Excel add-in
│   └── GOOGLE_TEST\                    test suite
├── resources\                   end-user spreadsheets, pricing toolkits, guides
└── targets\                     build output (generated; not in source control)
```

### Layer model

```
  AQ_API (Py/C#/Java/R)         AQ_XLL (Excel)          GOOGLE_TEST
                 \                 /                     (sees everything)
                  ▼               ▼                            |
              ┌──────────────────────────┐                     |
              │       validation         │ ◄───────────────────┘
              │  validates every input   │
              │  once; records I/O for   │
              │  test generation         │
              └────────────┬─────────────┘
                           ▼
              ┌──────────────────────────┐
              │        etrading          │  core pricing / analytics
              └────────────┬─────────────┘
                           ▼
              ┌──────────────────────────┐
              │  calibration · math ·    │  legacy — deprecate / extract
              │  models                  │
              └──────────────────────────┘
```

Every public call — binding or spreadsheet — passes through `validation`, so all
languages return identical results for identical inputs. Nothing bypasses it.

---

## Building

### Prerequisites

- Visual Studio 2022 with the **v143** toolset, Windows x64.
- The external dependency tree (Boost 1.91, QuantLib 1.43, Eigen3, Adept 2.0.3,
  GoogleTest 1.17, SWIG 4.0, xlOil 0.19) — see `CLAUDE.md` §3 for how it is
  built. The dependencies use the **static** runtime (`/MT`, `/MTd`); the library
  matches.
- For the Python binding only: a 64-bit Python install with `Include\Python.h`
  and `libs\`.

### One-time setup

From the solution folder:

```
SetEnvironmentVariables.bat
```

It sets, in your user environment:

| Variable | Meaning |
|---|---|
| `AQ` | this source tree (derived from the script location) |
| `AQ_EXTERNAL_LIB_PATH` | root of the external dependency tree |
| `AQ_PYTHON_ROOT` | Python the bindings compile against (optional; `*Python` configs only) |

Accept the suggested paths or type your own; the script validates them. Then
**restart Visual Studio** — MSBuild reads the environment at launch.

### Compile

Open `AlgoQuantLib-VS22.sln` and build. All projects are x64. Configurations:

| Configuration | Purpose |
|---|---|
| `Debug`, `Release` | the core library and Excel add-in |
| `DebugEditAndContinue` | Debug with Edit-and-Continue |
| `ReleaseProfiler` | Release with profiling instrumentation |
| `Debug/Release` + `Python` `CSharp` `Java` `R` | build the corresponding language binding |

Project properties are kept in each `.vcxproj` directly (not in shared property
sheets) — deliberately, so every setting is in one place.

### Outputs

All consumer artefacts are named `AlgoQuantLib`, regardless of project name:

| Target | Artefact | Consume as |
|---|---|---|
| `AQ_XLL` | `AlgoQuantLib.xll` | load in Excel |
| `AQ_API` (Python) | `AlgoQuantLib.pyd` | `import AlgoQuantLib` |
| `AQ_API` (C#/Java/R) | `AlgoQuantLib.dll` + generated wrappers | per-language import |

The binding `generate*` / `deploy*` scripts under `src\AQ_API\source` run
automatically as pre- and post-build steps for the language configurations.

### Editions

`AlgoQuantLib` ships as **Swaps**, **Bonds**, **Credit** and **Full** editions.
There is one binary per language; the edition is selected at load time from an
edition manifest and an entitlement file in the `config` folder — no separate
per-edition build. `aqToolsEdition()` reports which edition is active.

---

## Config folder & generators

The `config` folder ships with the Excel and API packages and is the
customisation surface that needs **no recompilation**:

- **Calendars / holidays** — holiday dates (from MarketWire / SwapsWire) are
  loaded at runtime. Correct, add or amend them in `config` and reload; the
  add-in does not need rebuilding or reinstalling.
- **Generators** — JSON templates that carry an instrument's or model's static
  data (calendars, day counts, rolls, fixing and payment lags, currency
  defaults). Static data is most of a trade booking, so with a generator a user
  books, for example, a USD swap from just notional, start, maturity, fixed rate
  and direction, or a US Treasury from notional, dates and coupon. Starter
  generators live under `resources\config`.

---

## Testing

`GOOGLE_TEST` sits at the top of the stack and can reach every layer. Its cases
are generated from the input/output recordings captured in `validation`, so the
test surface tracks the API surface.

Run the `GOOGLE_TEST` project, or the batch and spreadsheet test harnesses under
`resources\test`.

---

## Conventions

- **Public functions:** `aq` + `Category` + `FunctionName`, e.g.
  `aqDatesYearFraction`, `aqCurvesForwardRate`. The category list is being
  standardised — see `MIGRATION_PLAN.md` Phase 2.
- **Project name case:** lowercase = statically linked, Uppercase = dynamically
  linked.
- **C++ style:** descriptive names, camelCase, classes uppercase, members with a
  trailing underscore, enums over strings, readable steps over one-liners.
  Enforced by `clang-format` and `clang-tidy` (`.clang-format` / `.clang-tidy` at
  the root).
- **Calendars** update from MarketWire / SwapsWire holiday files without
  recompiling — see *Config folder & generators*.

---

## Licence

**Proprietary. All rights reserved.** AlgoQuantLib is distributed under the
AlgoQuantHub End User Licence Agreement — see the `LICENSE` file (an interim
placeholder until the full EULA is issued). No right to use, copy, modify or
redistribute is granted except under a separate written agreement with
AlgoQuantHub.

Third-party components bundled with AlgoQuantLib (Boost, QuantLib, xlOil, Eigen,
Adept) retain their own licences — see `THIRD_PARTY_LICENSES.md`. GoogleTest is
used for testing only and is not distributed.
