# AlgoQuantLib — project status

**As at 2026-09-06.** Overview of where the rebrand stands and what is left.

- The **phase-by-phase plan** is `MIGRATION_PLAN.md`.
- The **detailed running record** of the rebrand (per-commit, per-step) is
  `rebrand/STATUS.md`. This file is the summary above it; that file is the
  evidence beneath it.

---

## 1. Headline

**Roughly 55% through the plan by effort.** The hard, risky part — the mass
identifier rebrand — is done. The largest single piece of remaining work is the
XLL port, which is barely started.

| Phase | Scope | State |
|---|---|---|
| 0 | Baseline, inventories, safety net | **done** |
| 1 | Structural: projects, folders, macros, scripts | **done** |
| 2 | Category taxonomy (20 categories, locked) | **done** |
| 3 | Identifier rebrand + calendar delimiter | **done** |
| 3c | Retire `mir*` | **done** (deleted wholesale) |
| 4 | xlOil XLL port | **~10%** — see §3 |
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

**HEAD is `bf2cb765`. There are uncommitted changes, from two different
sources — read this before building.**

| File | Whose | State |
|---|---|---|
| `src/etrading/include/RecordMacros.h` | Claude | Fix for the `AQLString`/`std::string` build break. **`validation` verified compiling clean** (headless MSBuild, Release x64). Not yet committed. |
| `src/AQ_XLL/src/aqDates.cpp` | Nicholas | `ExcelObj` args changed to `const std::string&` / `double`. **Does not compile** against xlOil 0.19.0 — see §6. |
| `src/AQ_XLL/src/aqMath.cpp` | Nicholas | Same change, same failure. |
| `projects/AQ_XLL.vcxproj.user` | — | Local debug settings, not tracked content. |

Last **fully confirmed-green** state (build + GoogleTest, verified by Nicholas)
was `61795829`. Everything after that has been built but not yet had a full
GoogleTest run confirmed.

---

## 3. Outstanding work

### 3.1 Phase 4 — xlOil XLL port  ← the critical path

The port source is `.APPLES\APPLE\src\MLIBQ_ADDIN`: 40 files, ~82k lines,
**653 exported Excel functions**. Present state of `src/AQ_XLL`:

    include/  aqMain.h  aqXllTools.h
    src/      aqDates.cpp  aqMain.cpp  aqMath.cpp  aqTools.cpp  aqXllTools.cpp

**8 `XLO_FUNC_START` functions exist.** Of those, two are real ported library
functions (`aqDatesFromTenor`, `aqDatesFromYearFraction`); the rest are
`aqDatesToday`/`aqDatesNow` and proof-of-concept maths.

Outstanding:

- [ ] **Port the remaining function tranches**, in the agreed order:
      Dates → Tools → Curves → Swaps → Products → Models. Dates is the natural
      continuation and is partly done.
- [ ] **4.10 — `AQ_XLL_GUARD` on every new XLL function.** The macro exists and
      is proven by test; it just has to be applied as functions land.
- [ ] **4.11 — file naming.** Convention holds: every file is
      `aq<Category>.{cpp,h}`, sole exception `aqXllTools.{h,cpp}`.
- [ ] **4.12 — SEH for `AQ_API`.** `AQ_API_START` in
      `src/AQ_API/source/APISetUp.h` is a bare `try {`. The bindings' own
      marshalling can therefore still take the host process down, exactly the
      failure `AQ_XLL_GUARD` prevents on the Excel side. **Not yet fixed.**
- [ ] Remove the dead XLL+ include/library paths still listed in
      `projects/AQ_XLL.vcxproj`.
- [ ] Decide the xlOil argument-type question in §6 — it blocks further
      function signatures.

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
