# AlgoQuantLib

A C++ quantitative finance library for **fixed income, interest rates and
credit** â€” yield-curve construction, instrument pricing and risk â€” with language
bindings for Python, C#, Java and R, and an Excel add-in.

**Author:** [Nicholas Burgess](mailto:nburgessx@gmail.com) â€” AlgoQuantHub

> **Status:** the library is mid-rebrand from its original VS2017 form. Internal
> identifiers, namespaces and some project names are still being migrated. See
> `MIGRATION_PLAN.md` for the sequence and `CLAUDE.md` for working guidance.

---

## What it does

- **Yield curves** calibrated on the *forward rate* as the state variable, with
  monotone-preserving interpolation so the forward curve stays smooth and free of
  the kinks that imply arbitrage. Discount factors are obtained by integrating
  the forwards.
- **Analytical risk** via the Jacobian approach â€” interest-rate sensitivities
  without bump-and-revalue.
- **Instruments:** bonds, vanilla and OIS swaps, basis and cross-currency swaps,
  FRAs and futures, caps/floors, CDS, and vanilla options.
- **Consumers:** a single validated core, exposed identically through Python, C#,
  Java, R (SWIG) and Excel (xlOil), all shipping as **`AlgoQuantLib`**.

---

## Repository layout

```
AQ_LIB\
â”œâ”€â”€ AlgoQuantLib-VS22.sln                  Visual Studio 2022 solution
â”œâ”€â”€ SetEnvVars.bat  one-time environment setup
â”œâ”€â”€ Visualizer.natvis            debugger visualisers for date types
â”œâ”€â”€ CLAUDE.md                    working guidance
â”œâ”€â”€ MIGRATION_PLAN.md            rebrand / xlOil-port plan
â”œâ”€â”€ projects\                    all .vcxproj files
â”œâ”€â”€ src\
â”‚   â”œâ”€â”€ math\  models\  calibration\   legacy â€” being deprecated / extracted
â”‚   â”œâ”€â”€ etrading\                       core pricing and analytics
â”‚   â”œâ”€â”€ validation\                     single entry / contract layer
â”‚   â”œâ”€â”€ AQ_API\                        SWIG bindings (Py/C#/Java/R)
â”‚   â”œâ”€â”€ AQ_XLL\                         xlOil Excel add-in
â”‚   â””â”€â”€ GTEST\                    test suite
â”œâ”€â”€ resources\                   end-user spreadsheets, pricing toolkits, guides
â””â”€â”€ targets\                     build output (generated; not in source control)
```

### Layer model

```
  AQ_API (Py/C#/Java/R)         AQ_XLL (Excel)          GTEST
                 \                 /                     (sees everything)
                  â–¼               â–¼                            |
              â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”                     |
              â”‚       validation         â”‚ â—„â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
              â”‚  validates every input   â”‚
              â”‚  once; records I/O for   â”‚
              â”‚  test generation         â”‚
              â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
                           â–¼
              â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”
              â”‚        etrading          â”‚  core pricing / analytics
              â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
                           â–¼
              â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”
              â”‚  calibration Â· math Â·    â”‚  legacy â€” deprecate / extract
              â”‚  models                  â”‚
              â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
```

Every public call â€” binding or spreadsheet â€” passes through `validation`, so all
languages return identical results for identical inputs. Nothing bypasses it.

---

## Building

### Prerequisites

- Visual Studio 2022 with the **v143** toolset, Windows x64.
- The external dependency tree (Boost 1.91, QuantLib 1.43, Eigen3, Adept 2.0.3,
  GoogleTest 1.17, SWIG 4.0, xlOil 0.19) â€” see `CLAUDE.md` Â§3 for how it is
  built. The dependencies use the **static** runtime (`/MT`, `/MTd`); the library
  matches.
- For the Python binding only: a 64-bit Python install with `Include\Python.h`
  and `libs\`.

### One-time setup

From the solution folder:

```
SetEnvVars.bat
```

It sets, in your user environment:

| Variable | Meaning |
|---|---|
| `AQ` | this source tree (derived from the script location) |
| `AQ_EXTERNAL_LIB_PATH` | root of the external dependency tree |
| `AQ_PYTHON_ROOT` | Python the bindings compile against (optional; `*Python` configs only) |

Accept the suggested paths or type your own; the script validates them. Then
**restart Visual Studio** â€” MSBuild reads the environment at launch.

### Compile

Open `AlgoQuantLib-VS22.sln` and build. All projects are x64. Configurations:

| Configuration | Purpose |
|---|---|
| `Debug`, `Release` | the core library and Excel add-in |
| `Debug_EditAndContinue` | Debug with Edit-and-Continue |
| `Release_Profiler` | Release with profiling instrumentation |
| `Debug/Release` + `Python` `CSharp` `Java` `R` | build the corresponding language binding |

Project properties are kept in each `.vcxproj` directly (not in shared property
sheets) â€” deliberately, so every setting is in one place.

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

The Excel add-in (`AQ_XLL`) ships as **Bond**, **Swap**, **Credit**, **Curve**
and **Full** editions â€” separate `.xll` builds (`Release_XL_Bond`,
`Release_XL_Swap`, `Release_XL_Credit`, `Release_XL_Curve`, `Release`), each
containing only the worksheet functions its edition needs. There is no
runtime edition switch and no entitlement file â€” the edition is fixed at
build time by which `.xll` you load.

The language bindings (`AQ_API` â€” Python/C#/Java/R) do **not** have editions:
one binary per language, every category always available.

---

## Config folder & generators

The `config` folder ships with the Excel and API packages and is the
customisation surface that needs **no recompilation**:

- **Calendars / holidays** â€” holiday dates (from MarketWire / SwapsWire) are
  loaded at runtime. Correct, add or amend them in `config` and reload; the
  add-in does not need rebuilding or reinstalling.
- **Generators** â€” JSON templates that carry an instrument's or model's static
  data (calendars, day counts, rolls, fixing and payment lags, currency
  defaults). Static data is most of a trade booking, so with a generator a user
  books, for example, a USD swap from just notional, start, maturity, fixed rate
  and direction, or a US Treasury from notional, dates and coupon. Starter
  generators live under `resources\config`.

---

## Testing

`GTEST` sits at the top of the stack and can reach every layer. Its cases
are generated from the input/output recordings captured in `validation`, so the
test surface tracks the API surface.

Run the `GTEST` project, or the batch and spreadsheet test harnesses under
`resources\test`.

---

## Conventions

- **Public functions:** `aq` + `Category` + `FunctionName`, e.g.
  `aqDatesYearFraction`, `aqCurvesForwardRate`. The category list is being
  standardised â€” see `MIGRATION_PLAN.md` Phase 2.
- **Project name case:** lowercase = statically linked, Uppercase = dynamically
  linked.
- **C++ style:** descriptive names, camelCase, classes uppercase, members with a
  trailing underscore, enums over strings, readable steps over one-liners.
  Enforced by `clang-format` and `clang-tidy` (`.clang-format` / `.clang-tidy` at
  the root).
- **Calendars** update from MarketWire / SwapsWire holiday files without
  recompiling â€” see *Config folder & generators*.

---

## Licence

**Proprietary. All rights reserved.** AlgoQuantLib is distributed under the
AlgoQuantHub End User Licence Agreement â€” see the `LICENSE` file (an interim
placeholder until the full EULA is issued). No right to use, copy, modify or
redistribute is granted except under a separate written agreement with
AlgoQuantHub.

Third-party components bundled with AlgoQuantLib (Boost, QuantLib, xlOil, Eigen,
Adept) retain their own licences â€” see `THIRD_PARTY_LICENSES.md`. GoogleTest is
used for testing only and is not distributed.
