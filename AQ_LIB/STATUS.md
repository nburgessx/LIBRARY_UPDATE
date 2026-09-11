# AlgoQuantLib — project status

**As at 2026-09-10 (pause point).** Overview of where the rebrand stands and what
is left. Committed **HEAD is `18328864`**. Since the old `3809f148` baseline,
`22212bc5` committed **task 2.6** (the `validation` / `AQ_API` / `GTEST` /
fixture rename to the 22-category singular golden-source scheme — `Vol`→
`Volatility`, `Future` + `Ois` added), the **AQ_XLL Tool + Object-lifecycle
port** from `.APPLES\...\meUtilities.cpp`, and **Interpolation + PCA re-homed
`Tool`→`Math`** (`tryAqMath*`); `18328864` added a natvis refresh (and,
inadvertently, a generated `swig_Python_wrap.cxx` — flag to fix). All built green
with GoogleTest passing.

**Uncommitted working-tree delta (small):**
- `src/AQ_XLL/src/{aqBond,aqDate,aqMath,aqTool}.cpp` — the remaining Bond / Tool
  / Date / Math worksheet functions. **Built green, GTest passing (Nicholas).**
- `src/AQ_XLL/src/aqRate.cpp` (new) + vcxproj/filters — the `Rate` category
  (fixing table, FRA object, future↔FRA). **NOT yet built.**
- `rebrand/phase2_validation_rename_MAP.csv` (untracked, the task-2.6 map) + doc
  updates.

Two open items before resuming: (1) rebuild `AQ_XLL` with `aqRate.cpp`;
(2) decide whether the **`Rate` category is renamed** to `IR` / `InterestRate`
(`aqRate*` reads oddly) — a golden-source category rename if it goes ahead. Then
commit the small delta. Detail + resume plan: `rebrand/STATUS.md` top section.

- The **phase-by-phase plan** is `MIGRATION_PLAN.md`.
- The **detailed running record** of the rebrand (per-commit, per-step) is
  `rebrand/STATUS.md`. This file is the summary above it; that file is the
  evidence beneath it.

---

## 1. Headline

**Roughly 55–60% through the plan by effort.** The hard, risky part — the mass
identifier rebrand — is done. The largest single piece of remaining work is the
XLL port; its harness and first tranche now work in Excel.

| Phase | Scope | State |
|---|---|---|
| 0 | Baseline, inventories, safety net | **done** |
| 1 | Structural: projects, folders, macros, scripts | **done** |
| 2 | Category taxonomy (20 categories, locked) | **done** |
| 3 | Identifier rebrand + calendar delimiter | **done** |
| 3c | Retire `mir*` | **done** (deleted wholesale) |
| 4 | xlOil XLL port | **~35%** — Date / Tool / Bond / Object-lifecycle / Math complete; Rate (`aqRate.cpp`) written, not yet built; Curve / Swap / FX / Inflation / Volatility / product-option / Model / Generator still to do; see §3.1 |
| 4a | Editions & manifest gating | not started |
| 4b | Config folder & generators audit | not started |
| 5 | Bindings (C#/Java/R) & test coverage | not started |
| 6 | Legacy extraction, resources, docs, licence | not started |
| 7 | Linux build, final sweep, clean repo | not started |

**What "done" means for Phase 3:** `src/` and `projects/` contain zero
`Mizuho`, `MLIB`, `me*`, `mir*`, `LA*`/`MA*`/`MB*`, `validation_api`,
`XllPlus`, person names, or copyright banners. Build green, GoogleTest green.

---

## 2. State of the working tree right now

**Committed HEAD is `18328864`.** `22212bc5` (task 2.6 + AQ_XLL Tool/Object port
+ Interpolation/PCA→Math) and `18328864` (natvis) are committed and green. The
**uncommitted delta is now small** — the Bond/Tool/Date/Math XLL fill-in (green)
plus the new `aqRate.cpp` (not yet built) plus doc/map files. See the PAUSE
POINT block at the top of `rebrand/STATUS.md` for the exact file list.

Do **not** `git add -A` blindly: `src/AQ_API/source/swig_*_wrap.*` are
SWIG-generated and keep showing as phantom modifications; never commit them —
and note `swig_Python_wrap.cxx` already slipped into `18328864` (candidate for
`git rm --cached`).

The `ExcelObj`-argument rule for the static `XLO_FUNC_START` path: every arg is
`const ExcelObj&`, unpacked in the body (`.get<double>()`, `.isMissing()`,
`toAQLString(...)`), and **`.arg()` count must equal the parameter count** or the
add-in throws at `xlAutoOpen` and Excel reports a "corrupt" XLL. Every ported
function is audited for this.

---

## 3. Outstanding work

### 3.1 Phase 4 — xlOil XLL port  ← the critical path

The port source is `.APPLES\APPLE\src\MLIBQ_ADDIN`: 40 files, ~82k lines,
**653 exported Excel functions** (`rebrand/xll_function_inventory.csv` — 653
rows, decision columns still to fill per file). ~460 are `me*`/`meLWO*` in the
etrading filter (the port set); `LAXL.cpp` has 161 `mir*` (deprecate or `aql*`);
~29 `msc*`/client (delete).

**State of `src/AQ_XLL` (2026-09-07, `e5c57046`):**

    include/  aqMain.h  aqXllTools.h
    src/      aqBond.cpp   aqDate.cpp   aqMain.cpp  aqMath.cpp
              aqObject.cpp aqTool.cpp   aqXllTools.cpp

Naming scheme (step 11): categories SINGULAR; handle API is `aq<Category>Object<Fn>`
(named sub-object skips `Object`); lifecycle `aqObject<Lifecycle>`. AQ_XLL renamed;
`validation` / `AQ_API` / `GTEST` test names to follow (MIGRATION_PLAN 2.6).

Working in Excel (Nicholas-verified, pre-rename names): `aqToolsInitialize`,
`aqToolsResize`, `aqDatesFromTenor`, `aqDatesFromYearFraction`, `aqObjBondsCreate`,
`aqObjBondsDisplay`, `aqObjExists`, `aqObjLoad`, `aqObjSave` — now
`aqToolInitialize`, `aqToolResize`, `aqDateFromTenor`, `aqDateFromYearFraction`,
`aqBondObjectCreate`, `aqBondObjectDisplay`, `aqObjectExists`, `aqObjectLoad`,
`aqObjectSave`.
Library auto-inits in the add-in constructor; `AQ_INITIALIZE` is the per-function
lazy guard. Marshalling helpers (LVB, AnyTypeMatrix, numeric-aware string→number,
column/matrix builders, caller-range) live in `aqXllTools`.

Outstanding:

- [ ] **`aqObjSave` multi-row output blocked** — `AQ_IS_ARRAY_OUTPUT` returns
      false even under Ctrl+Shift+Enter (Excel not reporting the array range).
      `AsArray` explicit override + `aqToolsCallerInfo` diagnostic coded,
      uncommitted. Build, run the diagnostic, then decide: keep auto-detect or
      go explicit-only across all multi-output functions. See
      `rebrand/STATUS.md` → "RESUME HERE — AQ_XLL port".
- [ ] **Port the remaining tranches**, agreed order Date → Tool → Curve →
      Swap → products → Model. Fill the inventory decision columns per file,
      starting `meDates` (36).
- [ ] **4.10 — `AQ_XLL_GUARD` + `AQ_INITIALIZE` on every new XLL function.**
      Held so far.
- [x] **4.11 — file naming.** `aq<Category>.{cpp,h}` (`aqBond.cpp`, `aqDate.cpp`,
      `aqObject.cpp`, `aqTool.cpp`, `aqMath.cpp`); `aqXllTools.{h,cpp}` is the
      XLL-layer utility (not a category).
- [ ] **4.12 — SEH for `AQ_API`.** `AQ_API_START` in
      `src/AQ_API/source/APISetUp.h` is a bare `try {`. **Not yet fixed.**
      Also: `AQ_API` has no auto-init — callers must call `setUpAQL()`.
- [ ] Remove the dead XLL+ include/library paths still listed in
      `projects/AQ_XLL.vcxproj`.
- [ ] GoogleTest run to re-confirm green (last green tag `61795829`).

### 3.2 Phase 4.9 — validation recording rollout

Every function exposed through XLL or API should record its inputs and outputs
via the `AQ_RECORD_*` macros, so GoogleTest cases can be generated and rebased.

Current numbers across `src/validation/src` (101 files, 478 `tryAq*` names):

| Measure | Count |
|---|---|
| `AQ_RECORD_*` call sites in place | 681 across 72 files |
| Hand-rolled `if (recordEnabled())` blocks still to convert | **119** |
| `decorateFilename(...)` call sites still hand-rolled | **27** |

Remaining passes:

- [ ] **81 variable-filename blocks** — the filename is computed, so each needs
      reading individually. No bulk conversion.
- [ ] **Pass (b): 18 recordings whose fixture key ≠ parameter name.** Converting
      these mechanically **would break the fixtures**. Rename the parameter to
      match the key, then re-run `rebrand/tools/fixture_key_check.py`.
- [ ] **Pass (c): functions with no recording at all** — the largest group;
      needs new fixtures generated and reviewed.

### 3.3 Phase 5 — bindings & tests

- [ ] **C#, R and Java bindings are unverified.** Only Python is confirmed
      end-to-end. Each needs a build and a smoke test.
- [ ] SWIG regeneration after the rename (never commit `swig_*_wrap.*`).
- [ ] Coverage gaps called out in `CLAUDE.md` §8: yield-curve framework +
      Jacobian risk (vs bump-and-revalue), fixed-income price/yield
      (the Bloomberg-exact claim needs a regression test), date logic.
- [ ] **`tryAqToolsReplay` has no test at all** — untested public API.

### 3.4 Phase 6 — legacy, licence, docs

- [ ] **Third-party provenance scan — REQUIRED BEFORE SALE.** See §5.1.
- [ ] Extract the live pieces of `calibration` / `math` / `models` into a clean
      `core`/`utils` home, then delete the dead remainder one project at a time.
- [ ] Apply the frozen per-file copyright header (wording is settled in
      `../../CLAUDE.md` §7). Not yet applied to any file.
- [ ] `docs/api_map.csv` — the shipped `public name | category | wrapper |
      etrading entry | test` index.
- [ ] `RELEASE_NOTES` list of renamed/removed public function names.
- [ ] The bulk `clang-format` pass (must come **after** all renames).
- [ ] Resources audit: generator JSON and calendar config still carry
      client-specific conventions.

### 3.5 Phase 7 — clean repo

- [ ] Linux MAKE/CMake build re-validated and fully rebranded.
- [ ] `clang-tidy` naming guard wired into CI.
- [ ] Copy the tree to a fresh folder, `git init`, single clean base commit,
      delete the temporary stash repo.

---

## 4. Known defects carried forward

- [ ] **`tryAqObjRatesFixingTable.cpp` writes swapped keys** —
      `file.write("fixingValues", fixingDates)` and
      `file.write("fixingDates", fixingValues)`. Pre-existing, not introduced by
      the rebrand. **Fix the code and rebase those fixtures together**, never
      separately.
- [ ] `lwoer` (a typo for "lower" in a comment in
      `src/math/src/AQLFunctionUtilities.cpp`) is the one remaining `lwo`
      string under `src/` — deliberately left.
- [ ] 8 `mir*` references remain in `AQLCurveCalibrationHelpers.cpp`,
      `InitializeETrading.cpp`, `Replay.cpp`.
- [ ] 8 `validation_api` references in Makefiles / `add_functions.txt`.
- [ ] 6 copyright/ownership lines in `src/etrading/etrading.nuspec`.

---

## 5. Hazards — these outrank routine cleanup

### 5.1 Undeclared third-party code (blocks sale)

Two components in `src/math`, both found by accident, **neither listed in
`THIRD_PARTY_LICENSES.md`**:

- **`AQLSobol.cpp`** — Peter Jäckel. The notice contains a **preservation
  clause**.
- **`AQLNl2sol.cpp`** — NL2SOL, 6,541 lines, Dennis/Welsch/Gay/Peters,
  NSF-funded, ACM TOMS.

Both are **excluded by name from every sweep. Never strip their attribution.**
A full provenance scan of `math` / `models` / `calibration` is required before
the library is sold — if two were found by accident, others may be there.

Separately, and not a code issue: ~20% of this code was written by contractors
for a client under a consulting engagement. **A solicitor should confirm
copyright actually vests in you before sale.** A licence header cannot create
ownership the contracts did not transfer.

### 5.2 Recorded fixture keys are stringified parameter names

`WRITE_PARAMETER(P)` expands to `file.write("P", P)`. **Renaming a validation
parameter silently breaks the tests with a completely green build.** This has
already bitten once (16 failures from `lwoCurveGeneratorName`).
`rebrand/tools/fixture_key_check.py` guards it — run it after any parameter
rename.

### 5.3 Text vs binary file reads

Reading a file in text mode collapses `\r\n` to `\n`. Any offset computed that
way is wrong against a binary read, and rewrites land progressively earlier in
the file, eating live code. **This caused two separate code corruptions.** Any
tool that analyses and then rewrites must use one binary read for both.

### 5.4 The `me` / `MA` substring hazard

Never run an unanchored, case-insensitive replace of `me`, `MA`, `MB`, `LA`,
`LB`. Anchored, case-sensitive, map-driven replacement only, from a reviewed
list. (Phase 3 is done, but this applies to any future sweep.)

### 5.5 Other standing rules

- **Exact-match auditing under-reports.** Five times a "clean" result was
  defeated by a spelling variant. Re-scan loosely after any sweep reports zero.
- **`git add -A` stages the generated `swig_*_wrap.*` files.** Restore with
  `git checkout HEAD -- 'src/AQ_API/source/swig_*_wrap.*'`.
- **SEH tests fail by crashing, not by failing.** If
  `TestStructuredExceptionHandler` ever stops working, the test runner
  disappears rather than reporting red. A vanished run is the signal.

---

## 6. Open question — blocking

**Do xlOil worksheet functions have to take `ExcelObj` for every argument?**

The uncommitted edits to `aqDates.cpp` and `aqMath.cpp` change arguments to
native `const std::string&` and `double`. Against xlOil 0.19.0 this fails:

```
StaticRegister.h(357,82): error C2039: 'value': is not a member of
    'xloil::detail::ArgType<const std::string &>'
StaticRegister.h(357,82): error C2039: 'value': is not a member of
    'xloil::detail::ArgType<double>'
```

This needs resolving from the xlOil headers — whether native types are
supported on a different registration path, need a different include, or are
genuinely unsupported in this version — because it determines the signature
style for all 653 functions still to port. **Not yet investigated.**

---

## 7. Build

- VS2022, toolset v143, **x64 only**, `/std:c++17`, static runtime `/MT`,
  `/MP` on every project.
- Run `SetEnvironmentVariables.bat` once, then **restart Visual Studio**.
- Headless build (for scripted checks) needs `SolutionDir` passed explicitly,
  because the `.vcxproj` include paths are written in terms of `$(SolutionDir)`:

```bash
MSBuild.exe AlgoQuantLib-VS22.sln /p:Configuration=Release /p:Platform=x64 /m
```

Known landmines and their fixes are in `CLAUDE.md` §3.2 — MAX_PATH under Boost
`b2`, the xlOil static-build defines, and the local `AppObjects.cpp` patch that
**must survive any xlOil upgrade**.
