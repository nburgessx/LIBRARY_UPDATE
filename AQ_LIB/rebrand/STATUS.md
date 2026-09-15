# Rebrand status — 2026-09-15

## Naming-convention fix: three `AQ_XLL` function families corrected to `<library><category>` form (2026-09-15)

Nicholas caught three `AQ_XLL` families that didn't fit the
`aq<Category><Function>` / `aq<Category>Object<Function>` naming convention
(`CLAUDE.md` §5.1). Renamed across all four surfaces per §5.1a's order
(`validation` → `GTEST` → `AQ_API` → `AQ_XLL`), plus the recorded fixture
CSVs (`git mv`'d to match) and `docs\api_map.csv`:

| Old | New |
|---|---|
| `aqCreditObjectBasketModelCreate` | `aqCreditBasketModelCreate` |
| `aqCreditObjectBasketModelSurvivalProbability` | `aqCreditBasketModelSurvivalProbability` |
| `aqCreditObjectDefaultSwap<X>` (11 functions: PV, PVByIntegration, PVByMonteCarlo, PVFromHazardRate, RiskyAnnuity, RiskyAnnuityFromHazardRate, AccruedYearFraction, CS01, ParSpread, ParSpreadFromHazardRate, HazardRateFromParSpread) | `aqCDSObject<X>` |
| `aqToolObjectGrid<X>` (Create, Save, Load, Display, ClearOne, ClearAll) | `aqGridObject<X>` |
| `aqToolObjectGridObjectNames` | `aqGridObjectNames` (collapsed — the literal `aqGridObject`+`ObjectNames` transform would have doubled "Object") |

Each with its `try`-prefixed `validation` counterpart renamed identically
(`tryAqCreditObjectBasketModelCreate` → `tryAqCreditBasketModelCreate`, etc.).
The Basket functions have no `AQ_API` binding today, so only `validation`/
`GTEST`/`AQ_XLL` were touched for that family. Of the Grid functions, only
`Display` is bound in `AQ_API` (`aqToolGrids.h`/`.cpp`).

**Deliberately left untouched (Nicholas's call):** the generated
`src\AQ_XLL\include\generated\aqManifestList.h` and the four generated
`swig_{Python,R,JAVA,CSharp}_wrap.{cxx,cpp}` files — both regenerate on their
own (`aqManifestList.h` via `Release_XL_Manifest`'s `PreBuildEvent` calling
`generateManifestList.bat`; the SWIG wrap files via a full SWIG regen in
Phase 5) and don't need hand-editing. Confirmed: building
`Release_XL_Manifest|x64` regenerates `aqManifestList.h` with the new names
with no further action needed. Historical entries elsewhere in this file
that predate this rename (e.g. the original Credit/Grid port write-ups) are
left as-authored — they describe what was true at the time, not current
state.

**Verified (Nicholas):** `validation`, `GTEST`, `AQ_XLL` (Debug|x64 and
`Release_XL_Manifest|x64`) and `AQ_API` (Python) all build green; GoogleTest
passes.

---

## Phase 4a (editions) closed: AQ_XLL-only, AQ_API runtime gate dropped (2026-09-15)

Nicholas confirmed `AQ_XLL` now has four per-edition build configurations —
`Release_XL_Bond`, `Release_XL_Swap`, `Release_XL_Credit`, `Release_XL_Curve`
— alongside the existing `Release` (Full), each built from the
`src\Core`/`src\Optional` filter split (done 2026-09-12). **Decision: this
is the entire Phase 4a deliverable — editions are an `AQ_XLL`-only, compile-
time concept. The `AQ_API` runtime edition-gate half of the original Phase
4a plan (`config\editions.json`, `config\licence.json`, a module-import
registration gate, `aqToolEdition()`) is dropped, not deferred.** `AQ_API`
(Python/C#/Java/R) ships one full binary per language with every category
always registered — no edition concept on that surface, now or later.

**Docs updated to match** (this entry is the changelog; the docs themselves
carry the current-state description, not a repeat of it here): `CLAUDE.md`
§2.1/§4.5/§5.2 (root) and `AQ_LIB\CLAUDE.md` §2.1/§4.4/§5.2/§6.3 rewritten
in place (`Editions` sections marked done, dual-mechanism language replaced
with `AQ_XLL`-only); `MIGRATION_PLAN.md` Phase 4a marked ☑ done with the
`AQ_API` half struck through as dropped (not deferred), the 2.4 edition→
category straw-man table marked ☑ and given a `Curve` row (missing from the
original four-edition straw man), the Phase 5.2 task that assumed an
`AQ_API` edition gate to wire into struck out, and decision `D20` added.
`STATUS.md` (this library's own top-level summary, not this file) and
`readme.md`'s end-user-facing Editions section rewritten to match — the
`readme.md` text previously described a **runtime**, entitlement-file-gated
edition switch, which was never built and now never will be.

**Not done, flagged for later, not blocking this decision:** `AQ_XLL`'s
edition→category mapping (which `Optional` files land in which
`Release_XL_*` config) has not been independently audited function-by-
function against the `MIGRATION_PLAN.md` §2.4 table in this session — only
that the four configurations exist and build green (per Nicholas). Worth a
pass before shipping: confirm e.g. `Release_XL_Curve` doesn't accidentally
carry Swap-only files, and that each edition's registered function set
matches what a customer buying that edition should get.

---

## AQ_XLL add-in loaded empty in every configuration — fixed, one flagged (2026-09-14)

Nicholas reported `AlgoQuantLib.xll` loading with zero `aq*` functions in
every configuration (Debug and all Release/edition variants), with a
`xlOil Load Failure` popup reading `#Error: #Error: Unable to read JSON data -
File not found ...\resources\config\CURVE_GENERATOR\JPY_OIS_LOB_2Y.JSON`.

**Two separate things, only one fixed here:**

1. **Missing resource file — fixed (2026-09-14, follow-up).** Nicholas chose
   to drop the missing entries rather than author the missing JSON. Audited
   all three startup object lists (`resources\config\{Curve,Bond,Swap}Generator.conf`,
   each cross-referenced against the JSON files actually present in the
   matching `resources\config\{CURVE,BOND,SWAP}_GENERATOR\` folder) rather
   than just the one reported file, since the same failure mode could exist
   elsewhere in either list:
   - `CurveGenerator.conf`: removed `JPY_OIS_LOB_2Y.JSON` and
     `JPY_OIS_LOB_3Y.JSON` — genuinely absent; the bare `JPY_OIS_LOB_*` family
     only ever had `1Y`/`NONE`, unlike the JSCC/LCH/TIBOR variants which have
     the full `1Y`/`2Y`/`3Y`/`NONE` set.
   - `SwapGenerator.conf`: **not a missing file** — line 24 read
     `EUR_ARR_BASIS_LIBOROIS.JSONEUR_BASIS_1X3.JSON`, two real, existing
     generator filenames concatenated onto one line with no line break
     (a pre-existing data-entry corruption, unrelated to the rebrand). Split
     back into two lines rather than removed, since both
     `EUR_ARR_BASIS_LIBOROIS.JSON` and `EUR_BASIS_1X3.JSON` exist and would
     otherwise have been dropped for no reason.
   - `BondGenerator.conf`: clean, no missing entries.
   All three lists now cross-check clean (0 missing) against their folders.

2. **Regression from the AQ_THROW/boost::format cleanup (fixed, this
   entry) — this is what actually took the whole add-in down**, not just one
   curve generator: `etrading\src\FolderConfig.cpp:455` had
   `catch (ETradingException e)` around `deSerializeFromJSON(...)`,
   specifically so `FolderConfig::deserializeObjectsForOptionalStartup` can
   quietly skip a bad/missing optional-config entry when `reportErrors` is
   off (see the comment on `AlgoQuantLib()`'s constructor in `aqMain.cpp`:
   "a config-load failure does not abort the add-in load"). The prior
   session's error-messaging pass converted `SerializeContainedData.cpp`'s
   `throw ETradingException(...)` (the thing this catch depends on) to
   `AQ_THROW` — which always throws `AQLCoreInvalidData`, a different
   hierarchy. The catch stopped matching, so the missing-file exception now
   propagated uncaught through `InitializeETrading`'s constructor into
   `xlAutoOpen`, aborting the whole add-in before any function registered.
   That prior session's catch-site audit checked every `catch(ETradingException&)`
   in the tree but wrongly assumed **both** of `FolderConfig.cpp`'s two
   `ETradingException` catches depended only on `math`'s untouched
   `toCachedObjectEnum`/`toScheduleTypeEnum` — missed that the second one
   (this one) depended on `etrading`'s own (converted) JSON deserializer.
   **Fix:** retyped the catch to `AQLCoreInvalidData&`. While in there, also
   fixed the doubled `#Error: #Error:` prefix visible in the popup — several
   `AQ_THROW` call sites carried a hardcoded `"#Error: "` in the message
   literal left over from before the conversion (the macro always prepends
   `#Error: ` itself); stripped the redundant one from every such site found
   tree-wide (`SerializeContainedData.cpp`, `SwapUtilities.cpp`,
   `FXCurveUtilities.cpp`, `CreateDataFile.cpp`, and three `validation`
   files) — cosmetic, not the cause, but worth cleaning up while diagnosing
   the same symptom.

**Verified:** full solution rebuilt clean in both Debug|x64 and Release|x64
after the fix (one follow-up needed: the retyped catch's now-unused `e`
tripped `TreatWarningAsError` on an unreferenced-variable warning — dropped
the name, matching the unnamed-catch style already used elsewhere in this
file). **Not yet verified:** an actual Excel load with the rebuilt `.xll` —
that still depends on Nicholas rebuilding and reopening Excel, and on item 1
above (the missing JSON) being resolved one way or another, since until it
is, the add-in will still hit the same `deserializeObjectsForOptionalStartup`
path — the difference is it will now be **silently skipped** (`reportErrors`
is off) rather than aborting the whole add-in load.

---

## Error messaging unified on AQ_THROW/AQ_REQUIRE; boost::format removed (2026-09-14)

Nicholas asked for all error throwing across the library to route through the
`AQ_REQUIRE` / `AQ_THROW` / `AQ_THROW_IF` macros in `ExceptionMacros.h` instead
of raw `throw`, and for `boost::format` to be replaced with `std::ostringstream`
(or plain string concatenation for simple one-substitution cases) everywhere.
Done across `validation`, `etrading`, `AQ_XLL`, `AQ_API` and `GTEST` — **~200
files changed**, full solution builds green (Debug|x64) after each batch.
`math`/`models`/`calibration` were explicitly left alone (legacy, headed for
deprecation/extraction per §8 — not worth the effort on code that may be
deleted).

**Batched and built between each, per the working agreement:**
`validation` (44 files) → `etrading` (~140 files) → `AQ_XLL`/`AQ_API`/`GTEST`
(10 files). Every raw `throw AQLCoreInvalidData` / `AQLCoreError` /
`AQLCoreSystemError` / `AQLCoreNumericalError` / `AQLCoreAppError` /
`ETradingException` / `Exception(...)` converted; `AQLCoreNumericalError` and
`AQLCoreSystemError` throws collapsed into `AQ_THROW`/`AQ_REQUIRE` (which only
ever throw `AQLCoreInvalidData`) — a decided simplification, confirmed safe
because nothing outside `math`/`models` catches those two subtypes by name.

**Real correctness issue found and fixed, not just cosmetic:** `ETradingException`
(derives `std::runtime_error`) is specifically caught by type in several places
(`CurveUtilities.cpp` x2, plus two GTEST fixtures exercising `FixingTable`/
`TableDateDouble`). Collapsing those throw sites into `AQ_THROW` changes the
dynamic type to `AQLCoreInvalidData` (different hierarchy — `AQLCoreError :
virtual std::exception`), which would have silently broken those `catch`
blocks. Retyped every affected `catch` to `AQLCoreInvalidData&` and updated the
two GTEST assertions that checked the exact thrown type and message text
(`TestFixingTable.cpp`, `TryAqTestCurveTenorBasisJPY3M6MConvergence.cpp`) —
confirmed via a full search of every `catch(ETradingException&)`/
`catch(AQLCore*&)` in the tree before and after, so nothing else depends on a
type that changed.

**A bug in the automation itself, caught by the compiler and fixed:** an
early regex pass collapsing `if(cond){throw X;}` into `AQ_THROW_IF(cond,"msg")`
silently ate the enclosing `if` whenever a trailing `else`/`else if` followed,
producing "illegal else without matching if" (`AQLUpdateCurveObject.cpp`,
`AQLUpdateObjectPoolForCurves.cpp`, `AQLCurveCalibrationHelpers.cpp`,
`CurveCalibrationData.cpp`, `BasisCurveCalibration.cpp`, and others — ~15
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
`ETradingException` / `Exception(...)` or live `boost::format` — none found;
the only remaining textual hits are confirmed dead code inside `/* */` or `//`
comments, left untouched per "never delete code or comments." SWIG-generated
`.cxx` files were out of scope and not touched. GoogleTest re-run not yet done
in this session — **deferred**: confirm numerically identical output against
the pre-batch baseline before this is considered fully closed out.

---

## Batch script hardened against a file-lock race; renamed per Nicholas (2026-09-13)

A real VS rebuild of `Release_XL_Manifest` (the first one exercising the new
`.bat`, see the entry below) still worked — build succeeded, correct 4/467
result — but printed three `The process cannot access the file because it is
being used by another process.` lines. Investigated rather than dismissed as
noise, since it was reproducible (also saw 1-2 occurrences of the same thing
directly testing the `.bat` via `cmd.exe` outside VS, not just inside a VS
build).

**Root cause: file-handle churn, not a logic bug.** The original `.bat`
opened the output header and the temp known/requested-name files via a
separate `>>` append **per function** — up to 467 opens-and-closes of the
same file in a tight loop. That is a well-known collision point with
antivirus real-time scanning or an IDE file-change watcher transiently
locking a file the instant it changes; `cmd.exe`'s `>>` doesn't retry on
failure, so an unlucky collision silently drops that one line. It happened
not to corrupt anything in the runs so far (467/467 and 4/467 both came out
correct), but that was luck, not a guarantee.

**A red herring chased down first, worth recording so it doesn't get
re-investigated:** the build reported `edition 'Active'` instead of the
expected `edition 'DemoSmall'`, which looked like a parsing bug in the
`edition:<name>` line handling. It wasn't — `active.txt`'s line 24 literally
reads `edition:Active` now (the label was changed at some point after the
JSON->text conversion, function list unchanged), confirmed by reading the
file directly before touching any parsing code. The `edition:` extraction
logic itself is correct.

**Fix:** rewrote the three multi-append loops (the `XLO_FUNC_START` source
scan, the manifest-line classification, and the header-writing loop) to each
use one grouped `( ... ) > "file"` redirection instead of many small `>>`
appends — the file is opened once for the whole operation instead of
hundreds of times. Needed one escaping fix as a result:
`(edition: %EDITION%)` written from *inside* a `(...)` block needs its
closing paren escaped (`%EDITION%^)`) so it isn't parsed as the block's own
terminator. Re-ran all the same test cases (wildcard, the real restricted
`active.txt`, an unknown-function-name manifest, a missing manifest, a bad
root) directly via `cmd.exe` — same correct results as before, but zero
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
project paths — writes `src\AQ_XLL\include\generated\aqManifestList.h`
correctly, no lock errors. Nicholas then rebuilt through Visual Studio itself
and confirmed it too is clean — no "process cannot access" messages, no
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

**1. Stop inferring the root from `__file__` at all — pass it in explicitly.**
Even with the immediate bug fixed, path-inferred-from-script-location is
inherently fragile against exactly this kind of reorganisation. The generator
(then still Python, now the `.bat` below) takes `AQ_LIB_root` as its first
argument; the `.vcxproj` passes `$(SolutionDir)`, which always knows the
right answer regardless of where the script itself lives.

**2. Eliminate the Python dependency entirely — rewrite as a pure `cmd.exe`
batch file**, after being offered three options (keep Python + a friendly
missing-Python build error; PowerShell, which parses JSON natively; pure
batch) and picking pure batch despite the trade-off flagged: `cmd.exe` has no
JSON parser, so this necessarily means dropping JSON as the manifest format
too.

**What changed:**
- `generate_xll_manifest_header.py` (deleted) -> `generate_xll_manifest_header.bat`,
  same folder. No interpreter dependency beyond `cmd.exe` itself — nothing to
  install on any Windows dev machine. Scans the same `XLO_FUNC_START` sites
  via `findstr`/`for /f` (using a temp known-names file, not a single big
  environment variable, to stay well clear of `cmd.exe`'s ~8191-char
  per-variable limit — 467 function names would blow past that). Same
  contract: `AQ_LIB_root manifest.txt output_header.h`, same validation
  (fails loudly, exit 1, if a manifest name doesn't exist or `AQ_LIB_root`
  looks wrong), same `#define AQ_XLL_ENABLE_<name> 0/1` output.
- `active.json`/`demo.json` -> `active.txt`/`demo.txt`. New format: `#`-prefixed
  comment lines (ignored), optional `edition:<name>` line, either a lone `*`
  (every function — resolved fresh at generation time, same as before, still
  nothing hand-maintained) or one function name per line. `active.txt` carries
  a full explanatory comment block at the top (Nicholas asked for this
  explicitly, since JSON couldn't carry inline documentation the way a plain
  text file can) — `demo.txt` stays a copy-from example, unchanged in
  content (still the same 4-function `DemoSmall` list), just reformatted.
  **`active.txt`'s live content was preserved as `DemoSmall` (not reset to
  `Full`)** when converting from `active.json`, since that was the manifest
  actually in use at the time of the move.
- `AQ_XLL.vcxproj`'s `PreBuildEvent` now calls the `.bat` directly (`call
  "...\generate_xll_manifest_header.bat" "$(SolutionDir)" ...`) — no `where
  python` check needed any more, since there's nothing left to be missing.
  `AQ_XLL_MANIFEST_FILE`'s default updated to `active.txt`. The stray `<None>`
  item Visual Studio had added for the script also had a stale path (missing
  the `\manifest\` segment — pointed at a location the file was never
  actually at); corrected while touching this.

**Verified:** ran the `.bat` directly via `cmd.exe` (not just eyeballed) —
wildcard (`467/467`), the real restricted `active.txt` (`4/467`, exact same
4 names as the Python version produced), an unknown-function-name manifest
(fails, exit 1, same error format), a missing manifest file, and a bad
`AQ_LIB_root` (all fail cleanly, exit 1). Then ran the **exact**
`PreBuildEvent` command line against the real project paths and confirmed it
writes the real `aqManifestFunctions.h` correctly. `AQ_XLL.vcxproj`/
`.vcxproj.filters` re-verified as well-formed XML (including the specific
`--`-in-comment mistake from two entries ago — checked again, none present).
**Not yet re-verified through an actual VS build** — recommend a full rebuild
of `Release_XL_Manifest` to confirm MSBuild's own invocation (through
`Microsoft.CppCommon.targets`, not a direct `cmd.exe` call) behaves
identically to the direct test above.

---

## `Release_XL_Manifest` confirmed working end-to-end (2026-09-13)

Nicholas built and tested the full `Release_XL_Manifest` feature (all entries
below, most recent first) and confirmed **everything works**: build green,
`aqManifestFunctions.h` regenerates correctly from `active.json` at its fixed
path, shows up under `AQ_XLL`'s `include` filter in Solution Explorer, the
generator script runs fine from its new `resources\scripts\` home, and —
the one item every prior entry flagged as unverified — **Excel's function
wizard under `AlgoQuantLib` genuinely shows only the functions listed in
`active.json`, not the full 467.** This closes out every "not yet
verified"/"not yet re-verified" caveat left by the entries below.

One incidental fix along the way, worth calling out since it's unrelated to
any of this feature's code: `demo_small.json` was renamed to `demo.json` from
within Visual Studio's Solution Explorer, which correctly updated the
`<None>` item paths in both `AQ_XLL.vcxproj` and `.vcxproj.filters`
automatically — no manual follow-up needed.

Also confirmed elsewhere in this session but worth restating here since it
caused two of the debugging detours above: `AQ_XLL` must be the solution's
**Startup Project** (Solution Explorer → right-click → Set as Startup
Project) for Run/F5 to launch Excel at all — if a different project (e.g.
`models`, a static library) is startup, VS tries to `CreateProcess` a `.lib`
directly and fails with the same "not a valid Win32 application" message,
which looks identical to the real debug-settings bug but has nothing to do
with `AQ_XLL.vcxproj.user`.

**Fully verified, nothing deferred, for this feature.**

---

## Manifest generator script moved out of `rebrand\tools\` (2026-09-13)

Nicholas: `rebrand\` gets deleted wholesale once the rebrand is complete
(CLAUDE.md/MIGRATION_PLAN.md §2.2 - it is the temporary stash tooling, not a
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
`aqMain.h` — so it now shows up in the "include" node of Solution Explorer
like any other header (absent/greyed out until the first `Release_XL_Manifest`
build creates it). `.gitignore` (at the `LIBRARY_UPDATE` repo root) got a new
`AQ_LIB/src/AQ_XLL/include/generated/` entry so it's never accidentally
staged — it is still fully regenerated by the `PreBuildEvent` every build, not
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
VS build** — the `ClInclude` item should appear under `AQ_XLL`'s `include`
filter in Solution Explorer, but this hasn't been confirmed in a real VS
session.

---

## `AQ_XLL_MANIFEST_FILE` fixed to one file (`active.json`); User Macros page abandoned (2026-09-13)

Two problems from the previous entry, both from Nicholas:

**1. VS's "User Macros" property page doesn't appear for `AQ_XLL`.** It's a
standard `DynamicLibrary` C++ project, and that page is reliably shown only
for NMake-style projects in modern Visual Studio — the underlying
`Label="UserMacros"` PropertyGroup mechanism still works at the MSBuild level,
but there was no GUI to edit it, defeating the point.

**2. A plain rebuild from the VS Build menu still showed every function.**
Root cause carried over from the prior entry: nothing was overriding
`AQ_XLL_MANIFEST_FILE`, so it fell back to the all-functions default — not a
bug in the gating mechanism itself, just nothing pointing it at a restricted
manifest yet.

**Fix — stop trying to make the *path* switchable from the IDE; make the
*file* fixed and switch its *content* instead.** `AQ_XLL_MANIFEST_FILE`
defaults to one single, permanent file: `src\AQ_XLL\resources\manifest\active.json`
(renamed from `default.json`). To change which functions a local
`Release_XL_Manifest` build exposes, open `active.json` in Solution Explorer
(under the `resources\manifest` filter, already visible there) and edit its
`"functions"` array directly, then rebuild — no property pages, no macros, no
VS restart, nothing to reload. `demo_small.json` stays as a copy-from
template. The `msbuild /p:AQ_XLL_MANIFEST_FILE=...` override still works
unchanged, for scripted/CI generation of many customer editions without
touching `active.json` at all.

**Caught in passing:** the previous edit's comment in `AQ_XLL.vcxproj`
contained a literal `--` inside an XML comment (`default -- to switch`),
which is illegal in XML and made the whole project file fail to parse. Fixed
before it reached a real VS session — re-verified `AQ_XLL.vcxproj`,
`.vcxproj.filters` and `.vcxproj.user` all parse as well-formed XML.

**Not yet re-verified through an actual build with a restricted
`active.json`** — recommend setting `active.json`'s `"functions"` to a small
list (e.g. copy `demo_small.json`'s four), rebuilding
`Release_XL_Manifest`, and confirming Excel's function wizard under
`AlgoQuantLib` shows only those before relying on this further.

---

## Manifest location moved inside AQ_XLL; wildcard "*" replaces the hand-listed default (2026-09-13)

Two corrections to the `Release_XL_Manifest` work below, both from Nicholas:

**1. Manifest files live inside `AQ_XLL`, not the shared `resources\config\`
tree.** Moved `resources\config\XLL_MANIFEST\*.json` → `src\AQ_XLL\resources\manifest\*.json`
— alongside the existing `src\AQ_XLL\resources\SetUp.md` — and added them as
`<None>` items in `AQ_XLL.vcxproj` under a new `resources\manifest` filter in
`AQ_XLL.vcxproj.filters`, so they're visible and browsable in Solution
Explorer like any other project resource. `AQ_XLL_MANIFEST_FILE`'s default in
the `.vcxproj` now points at
`$(SolutionDir)src\$(ProjectName)\resources\manifest\default.json`.

**2. No more hand-maintained "list every function" default.** The original
`default.json` enumerated all 467 function names as the "include everything"
edition — exactly the kind of static list that silently goes stale the next
time a function is added or renamed. `generate_xll_manifest_header.py` now
accepts `"functions": "*"` as a wildcard meaning "every function known
today," resolved fresh from the source scan at generation time. `default.json`
is now just `{"edition": "Full", "functions": "*"}` — nothing to maintain.
Explicit lists (`demo_small.json`) are unaffected and still validate every
name against the source scan.

Verified: both `default.json` (467/467 via wildcard) and `demo_small.json`
(4/467) regenerate cleanly from the new location. Not yet re-verified through
an actual MSBuild run after the path change — do that before relying on it.

---

## New `Release_XL_Manifest` AQ_XLL configuration — function-level custom editions (2026-09-13)

Added a fifth `AQ_XLL` edition alongside the existing category-level
`Release_XL_Bond/Credit/Curve/Swap` (CLAUDE.md Sec 4.4/9.5): `Release_XL_Manifest`
builds `AlgoQuantLib.xll` with only the individual worksheet functions named in
an external manifest JSON, cutting across category files (e.g. a handful of
`Swap` and `Bond` functions together) rather than whole categories.

**Mechanism (compile-time, not runtime):** every one of the 467
`XLO_FUNC_START(...)...XLO_FUNC_END(...)` sites across all 18 `src\AQ_XLL\src\*.cpp`
files is now wrapped `#if AQ_XLL_ENABLED(name) ... #endif` (scripted sweep,
verified 1:1 against the original function count — no misses, no double-wraps).
`AQ_XLL_ENABLED(name)` is defined once in
`src\AQ_XLL\include\aqXllTools.h`:

    #define AQ_XLL_ENABLED(name)  (!AQ_XLL_MANIFEST_BUILD || AQ_XLL_ENABLE_##name)

`AQ_XLL_MANIFEST_BUILD` defaults to 0 there, so the guard is a no-op for every
other configuration — `Debug`, `Release`, and the existing `Release_XL_*`
configs are unaffected (an undefined `AQ_XLL_ENABLE_<name>` token safely
evaluates to 0 in `#if`, no warning). Only `Release_XL_Manifest` defines
`AQ_XLL_MANIFEST_BUILD=1` and force-includes (`/FI`) a generated header listing
one `AQ_XLL_ENABLE_<name> 0/1` per known function.

**New files:**
- `rebrand\tools\generate_xll_manifest_header.py` — the generator. Ground
  truth for "known function names" is the AQ_XLL source itself (scans every
  `XLO_FUNC_START`), not `docs\api_map.csv`, so it can't drift from the actual
  port. Fails loudly (exit 1) if the manifest names a function that doesn't
  exist.
- `resources\config\XLL_MANIFEST\default.json` — the default manifest
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
launch settings — `LocalDebuggerCommand`/`LocalDebuggerCommandArguments`
pointing Visual Studio's Run at Excel with `$(TargetPath)`) had a
`PropertyGroup` for every existing configuration but none for
`Release_XL_Manifest|x64`. Without it, VS's default debug command for a
`DynamicLibrary` project is to launch `$(TargetPath)` **directly** —
Windows refuses to `CreateProcess` a DLL/XLL (no direct-execution entry
point), surfacing as "not a valid Win32 application" even though the built
`.xll` itself is a perfectly valid x64 PE (verified by hand: same MZ/PE/machine
header as the working editions). Fixed by adding the missing
`PropertyGroup` (cloned from `Release_XL_Bond`'s). **Also needed a full
Visual Studio restart, not just a project reload/rebuild** — `.vcxproj.user`
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
unlisted functions from Excel's function wizard — recommend testing with a
small manifest before shipping a customer edition this way.

---

## Config-file preservation after a bulk delete; `aqToolInitialize` now reloads config too (2026-09-12)

Nicholas flagged that the previous turn's `aqObjectDeleteAll`-with-no-argument
path (see the entry below) deletes every cached object, **including** objects
that were only there because a config file loaded them at startup (generator
templates from `resources\config\{SWAP,BOND,CURVE}_GENERATOR`, etc.) — and
nothing put them back. Unlike `tryAqObjectClearCache` (which already reloaded
config at its end, see item 3 below), the new bulk-delete overloads didn't.
Request: *"The initialize method and the delete methods need to load the
configuration files at the end of their routines."*

**Delete methods (`validation/src/tryAqObject.cpp`):** both
`tryAqObjectDeleteAll` overloads (the single-type one and the new
all-types one) now call `validation::tryAqToolLoadConfigurationFiles()`
after deleting, before returning the count — same pattern
`tryAqObjectClearCache` already used. Added `#include "tryAqToolSetup.h"` for
the declaration. **`tryAqObjectDelete` (the single-object delete) was
deliberately left unchanged** — reloading config does real disk I/O
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
generator config at all — so a fresh add-in load depended entirely on
whatever `tryAqObjectClearCache`/`trySetupAQL` happened to have done, which
isn't guaranteed at `xlAutoOpen`. Now it is loaded unconditionally on every
initialize call, matching the delete methods and `tryAqObjectClearCache`.
`tryAqToolLoadConfigurationFiles` never throws (returns an error string
instead), so a missing/bad config file still doesn't stop the add-in loading.

**Verified (static checks only — not yet through a real build):**
`rebrand/tools/api_pair_check.py` HARD GATE still 0; whole-tree
`validation/src/*.cpp` wiring check still 101/101, 0 gaps. No `.arg()`/param
counts changed (no XLO_FUNC signature touched), so the runtime-registration
landmine in `CLAUDE.md` §3.2 doesn't apply here. **This batch — the IR rename,
`aqObjectDeleteAll`/`aqObjectClearCache` changes, and now this config-reload
addition — has still not been through a real compiler.** Recommend building
before relying on any of it further.

---

## `InterestRate`→`IR` rename + `aqObjectDeleteAll`/`aqObjectClearCache` behaviour changes (2026-09-12)

Nicholas confirmed the build works and tests pass, and **promoted
`aqCurve.cpp`/`aqInterestRate.cpp` into `src\Core`** (own commit) — resolving
the dependency gap flagged in the previous entry (a `ReleaseBonds`-style
edition needs curves/fixing tables to price against). Three requests landed
this turn:

**1. `InterestRate` category renamed to `IR`.** Full-stack, same discipline as
the earlier `Ois`/`CMS`/`TRS` renames:
- `validation`: `git mv` + content rename —
  `tryAqInterestRateFixingTable.{h,cpp}` → `tryAqIRFixingTable.{h,cpp}`,
  `tryAqInterestRateFutureFra.{h,cpp}` → `tryAqIRFutureFra.{h,cpp}`,
  `tryAqInterestRateObjectFra.{h,cpp}` → `tryAqIRObjectFra.{h,cpp}`.
- `AQ_XLL`: `git mv aqInterestRate.cpp → aqIR.cpp`; all 13 `XLO_FUNC` names
  renamed (`aqInterestRate*` → `aqIR*`); file header comment updated.
- `AQ_API`: the `InterestRate` bindings live inside `aqCurveObject.{h,cpp}`
  (a pre-existing filing quirk — confirmed via grep before touching anything,
  same pattern as Credit-in-Swap) — 4 functions renamed
  (`aqInterestRateFixingTable*` → `aqIRFixingTable*`); no SWIG `.i` file
  changes needed since they `%include` the whole header rather than naming
  functions individually. Generated `swig_*_wrap.*` files deliberately left
  untouched (will regenerate).
- `GTEST`: 11 files' `#include`/call-site renames
  (`tryAqInterestRateFixingTableCreate` → `tryAqIRFixingTableCreate`).
- **3 fixture CSVs, all git-tracked this time** (unlike the 100 untracked
  Ois ones) — `git mv`'d and their path-string literals inside the GTEST
  files updated to match in the same sed pass.
- `projects/validation.vcxproj`(`.filters`) and `projects/AQ_XLL.vcxproj`
  (`.filters`) updated; both re-verified well-formed XML.
- `rebrand/tools/api_pair_check.py` `CATEGORIES`: `InterestRate` → `IR`;
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

**2. `aqObjectDeleteAll` — `ObjectType` is now optional; omitting it deletes
every cached object of every type.** Added true no-arg overloads to
`validation` rather than looping category strings in the XLL layer:
`tryAqObjectList()` (every name, across `etrading::Environment::STORED_TYPES`)
and `tryAqObjectDeleteAll()` (delegates to the same
`etrading::deleteAllObjects(Environment&)` that `tryAqObjectClearCache`
already used internally). The XLL wrapper checks `objectType.isMissing() ||
!objectType.isNonEmpty()` and branches to the new no-arg pair instead of the
existing per-type pair; stops the instance counter for every deleted name
either way. `.help()`/`.arg()` updated to say `ObjectType` is now optional.

**3. `aqObjectClearCache` — confirmed it already deletes objects from every
category** (Nicholas asked for this explicitly). Traced
`validation::tryAqObjectClearCache()` → it already calls
`etrading::deleteAllObjects(etrading::Environment::defaultEnv())`, which
loops every `CachedObjectEnum` in `Environment::STORED_TYPES` and deletes
every object of every type — **this was already true before today**, found
by reading the implementation rather than assuming a change was needed. What
*was* missing: the `AQ_XLL`-side handle-name instance-counter map
(`namesToCounter_`) was never reset after a full clear, so it would keep
accumulating stale entries for names that no longer existed. Added
`aq_xll::clearAllInstanceCounters()` (mutex-guarded `namesToCounter_.clear()`)
and call it at the end of `aqObjectClearCache`. Help text updated to state
explicitly what gets cleared (every category's objects, curve/swap/credit
results, the entity pool) rather than the vague "the object cache" — the
behaviour was already correct, the documentation wasn't.

**Not asked for, flagged rather than done silently:** `aqObjectList` still
requires `ObjectType` (mandatory) — the same "blank means all types"
convenience just added to `aqObjectDeleteAll` could trivially be added here
too, reusing the same new `tryAqObjectList()` no-arg overload, since it
already exists. Left alone since it wasn't part of the request; flag to
Nicholas as an easy follow-up if wanted.

**Verified (static checks only, no compiler run yet on the DeleteAll/
ClearCache changes):** whole-tree audit 467/467 clean throughout; every
`validation/src/*.cpp` still wired into `validation.vcxproj` (101/101, 0
gaps) after the file renames. **This batch has not yet been through a real
build** — recommend building before relying on it further, per the pattern
of the last few turns where static checks alone didn't catch everything.

---

## Two fixes (`aqObjectDecorateNames`, `aqBondGeneratorDisplay` transpose) + `AQL Classic`/`AQLString`/`AQLDate` scoped for the plan, not actioned (2026-09-12)

**1. `meUtilityLWODecorateNames` was missing from the gap audit** because it
has **no `validation` wrapper at all** (0 hits in `src\validation`) — the
gap-audit script only compares against declared `validation` wrappers, so a
function that was never given one couldn't show up as a gap. Traced to
`.APPLES\...\meUtilities.cpp`: a session-wide control function that flips the
AQObj handle-naming switches (instance counter / Excel-address decoration /
address-vs-unique-ID). That state already lives entirely in `aq_xll`
(`aqXllTools.h`'s "Handle behaviour switches" — `setInstanceCountNames`,
`setDecorateNamesWithExcelAddress`), not in `etrading`, so there is nothing
for `validation` to validate — same disposition as `aqToolEcho`/
`aqToolBuildTime`/`aqToolSEH` (AQ_XLL-only, no `tryAq*`).

Added:
- **`etrading::HedgeCurveInfo`-style third switch, new this session:**
  `convertExcelAddressToUniqueID_` in `aqXllTools.cpp`, wired into
  `getExcelLocationAsString()` — when on, the location suffix is a short
  `std::hash`-based numeric ID instead of the literal cell address (stable
  only within the current session, which is fine — it exists to give a short
  handle, not a portable one). Declared in `aqXllTools.h` next to the other
  two switches.
- **`aqObjectDecorateNames(enableCounter, appendLocation,
  showExcelCellAddress)` → `aqObject.cpp`** (generic lifecycle, no category —
  matches `aqObject<Lifecycle>`). Calls all three setters and returns a
  status string in the same shape as the legacy function
  ("AQObj Names: Instance Counting is ON, Append Excel Cell Location is ON,
  Showing Excel Location as EXCEL ADDRESS"). Defaults match the legacy
  function's own defaults (`enableCounter`/`appendLocation` default TRUE,
  `showExcelCellAddress` defaults FALSE — i.e. unique-ID mode by default).

**2. `aqBondGeneratorDisplay` output was untransposed.** Its single call to
`etrading::toAQLStringMatrixFromVariantMatrix( result, false )` explicitly
passed `false` for the `transpose` parameter — but that parameter's own
default is `true`, and its doc comment says so directly: *"note transposes by
default to match the default JSON schema convention."* The explicit `false`
was silently overriding the sensible default, i.e. was almost certainly a
copy/paste slip from an earlier function in the same file, not a considered
choice. **Fixed: `false` → `true`.** Confirmed via `git grep` that this was
the *only* call site anywhere in `AQ_XLL` passing `false` here — an isolated
bug, not a systemic pattern across the other `*GeneratorDisplay`/
`*Display` functions.

**3. `AQL Classic` / `AQLString` / `AQLDate` — investigated, scoped into
`MIGRATION_PLAN.md`, NOT actioned per explicit instruction.** Findings:

- **The `AQL Classic` `.vcxproj.filters` filter is small and tractable.**
  Only `GTEST.vcxproj.filters` actually has files under it: **17 entries** —
  6 curve-fixture pairs (`Curve{Accessors,Ois,FwdFxConst,Std,TenorBasis,
  XccyBasis}.{cpp,h}` under `src/GTEST/{src,include}`), `TestDatesCentralBank
  .cpp`, `TestDatesSwapSchedule.cpp`, `TestRiskSwapDeltaLadder.cpp`,
  `TestRiskTenorBasisCurve.cpp`, and — worth flagging on its own —
  **`TestMirDateFunctions.cpp`**, a `mir`-named test that should probably
  already have gone with the "`mir*` stack deleted wholesale" work from an
  earlier phase; needs checking whether it still calls anything `mir*` before
  deleting. `AQ_API.vcxproj.filters` **declares** the same filter
  (`src\etrading\AQL Classic`, `include\etrading\AQL Classic`) but has **zero
  files assigned to it** — an empty, unused filter, nothing to delete there
  beyond the declaration itself. No other project (`math`, `etrading`,
  `validation`, `calibration`, `models`, `AQ_XLL`) has this filter at all.
- **`AQLString` and `AQLDate` are a much bigger undertaking than the filter
  cleanup — do not conflate the two.** Repo-wide grep counts:
  **`AQLString`: ~36,962 occurrences across 1,108 files. `AQLDate`: ~9,904
  occurrences across 585 files.** This dwarfs every rename done in this
  rebrand so far, including the `me*`→`aq*` sweep.
- **`AQLString` (`src\math\{include,src}\AQLString.{h,cpp}`, ~1,550 lines):**
  a hand-rolled, atomic-refcounted, copy-on-write string class from an era
  before C++11 gave `std::string` move semantics and small-string
  optimisation — the exact problem COW strings existed to solve. **No
  architectural justification found for keeping it** in a C++17 codebase; its
  extra convenience (numeric constructors, `getDoubleValue()`/
  `getIntValue()`) is trivially replaced by `std::to_string`/`std::stod`/
  `std::stoi` or small free functions. Recommendation: **`std::string`** as
  the replacement, no custom type needed.
- **`AQLDate` (`src\math\include\AQLDate.h`, 112 lines + 695-line `.cpp`):** a
  hand-rolled Julian-day calendar class, **virtual** (`virtual ~AQLDate()`,
  `virtual void setDate(...)`), storing `year/month/day` plus a cached
  Julian long. Recommendation: **`boost::gregorian::date`** — Boost is
  already a direct dependency (BSL-1.0, no encumbrance), already the
  canonical date type in the newer `Curve` validation headers
  (`tryAqCurveDiscountFactor.h` etc. take `std::vector<boost::gregorian::
  date>` directly), and the bridge functions
  `etrading::toGregorianDateFromAQLDate`/`toAQLDateFromGregorianDate` already
  exist and are proven correct — meaning the hard conversion-correctness work
  is already done, just not yet load-bearing everywhere.
- **Scale means this is NOT a rebrand-timeline task.** A 37k/9.9k-occurrence
  type swap needs an automated codemod (clang-tidy `readability-*` /
  libclang-based rewrite, not manual `sed`) and a full regression run per
  batch, run as its **own initiative after the rebrand ships**, not folded
  into Phase 6. Scoped into `MIGRATION_PLAN.md` as a new **Phase 8** for
  exactly this reason — see that file for the entry. The `AQL Classic` filter
  deletion (17 files, isolated GTEST fixtures) is small enough to fold into
  the existing Phase 6 resources/legacy-extraction pass instead.

**No code deleted, no `AQLString`/`AQLDate` usage touched — per explicit
instruction, this turn was investigate-and-plan only.**

---

## ✅ BUILD CONFIRMED GREEN, ALL CONFIGS, TESTS PASS — Phase 4 XLL port essentially complete (2026-09-11)

Nicholas: "All builds are working and tests pass." This confirms the
gap-closing batch below (23 functions: Object lifecycle, `aqCurveUSDSpotDate`,
Tool strays + the `aqToolEchoDouble` re-fix, 10 Math vector overloads, 4 Swap
risk-ladder functions) compiles and links clean, on top of the two build-fix
rounds already applied to the Curve/Credit/Swap batches before it.

**Answering "have we migrated all functions?"**: yes, modulo 9 deliberately
excluded wrappers and the 2 categories (`Model`, `Generator`) that have zero
`validation` wrappers to port in the first place. Full detail in the section
immediately below (unchanged from the last entry) and in the top-level
`STATUS.md` §2, which now carries the same audit as the canonical
"is everything ported" answer, updated in lockstep with this file.

**Updated for this milestone:** top-level `STATUS.md` rewritten in full (it
was several sessions stale — still describing ~35% Phase 4 progress and an
already-resolved "does xlOil support native-type arguments" question). New
version leads with the 98%-ported headline, the exact 9-wrapper exclusion
list, a chronological summary of everything ported this session, and a
re-runnable copy of the gap-audit script so "are we done yet" never again
requires re-deriving the answer from scratch.

**Not yet done / next up, unchanged from before:**
1. `Model`/`Generator` — net-new `validation` wrapper design and build, not a
   port. Not scoped.
2. Phase 4a: `AQ_XLL` per-edition build *configurations* (the `src\Core`/
   `src\Optional` file split is done and committed; the actual
   `ReleaseBonds`/`ReleaseSwaps`/etc. configurations that consume it are not
   yet added) and the `AQ_API` runtime manifest gate (not started).
3. Phase 5 (bindings/SWIG regen, C#/Java/R verification, coverage gaps),
   Phase 6 (legacy extraction, licensing, `RELEASE_NOTES`, clang-format),
   Phase 7 (Linux/CMake, clang-tidy CI, clean repo) — all still ahead per
   `MIGRATION_PLAN.md`, none started.

---

## Phase 4 XLL port essentially COMPLETE: full gap audit closed to 9 intentional non-gaps (2026-09-11, NOT YET RE-BUILT)

Nicholas confirmed the build works and tests pass (all configs). "I want to
migrate all functions, what is the best way to do that" — answered by running
an exact gap audit (every `tryAq*` validation wrapper vs every registered
`aqXLL` function by name, a script-driven comparison rather than manual
category review) instead of guessing what was left. Result: **445 of 467
wrappers already ported; 30 genuine gaps** (`Model`/`Generator` excluded —
both have 0 validation wrappers, that's net-new work not a port).

**Two decisions put to Nicholas, both answered:**
- Legacy procedural SABR family (7 functions,
  `tryAqVolatilitySABR{Calibrate,GetPrem,GetVol,OutputParameter,
  SetupConvention,SetupParameter,SetupSwaptionVol}`) — **decided: leave out**,
  superseded by the already-ported object-based `tryAqVolatilityObjectSabr*`.
  Not ported; this is a deliberate, confirmed decision, not an oversight.
- Swap risk ladders (4 functions needing a `vector<LabelValueBlock>` /
  multi-trade marshalling design with no prior pattern in the codebase) —
  **decided: build it now.** Done — see below.

**23 of the 30 gap functions ported this batch, closing every gap that was
actually a gap:**

1. **Quick strays (5 real + 1 false positive):**
   - `aqObjectTypeAsString`, `aqObjectLoadAndReturnTupleResults` → `aqObject.cpp`
     (generic Object lifecycle; the tuple one returns a 2-row
     `[handle, cached-type]` column via `etrading::toString(CachedObjectEnum)`).
   - `aqCurveUSDSpotDate` → `aqCurve.cpp` (filed in `tryAqDate.h`, golden-named
     Curve — another cross-file quirk like Credit-in-Swap).
   - `aqToolBondAverageYield`, `aqToolBondYieldFromFuturePrice` → `aqTool.cpp`
     (filed in `tryAqBondObject.h`, golden-named Tool).
   - `aqToolSwapScheduleTemplate` → `aqTool.cpp` (filed in
     `tryAqSwapObjectSchedule.h`, golden-named Tool; previously noted as
     "do with Swap" — that was a *location* note from an earlier session, the
     golden name puts it in Tool, corrected here).
   - `tryAqBondObjectZSpreads` — investigated, turned out to be a **false
     positive**: the plural validation wrapper's own doc comment says it's
     "Validation interface for the **aqBondObjectZSpread**" (singular) — i.e.
     it already IS the public function behind the existing (singular-named)
     `aqBondObjectZSpread` in `aqBond.cpp`. The truly-singular
     `tryAqBondObjectZSpread` (different signature, curveCollection/
     forecastCurve as single strings) is explicitly marked "Helper interface"
     in its own doc comment — an internal building block, not meant to be
     public. No fix needed; confirmed by reading both doc comments, not
     guessed.

2. **`aqToolEchoDouble` — the other known `validation.vcxproj` wiring gap,
   fixed properly this time.** Added `tryAqToolEchoDouble.h`/`.cpp` to
   `validation.vcxproj`/`.filters` (next to `tryAqToolDate`), then re-added
   `aqToolEchoDouble` to `aqTool.cpp` (removed earlier this session for the
   same reason). **Repo-wide check: every `validation/src/*.cpp` on disk
   (101) is now wired into `validation.vcxproj` (101) — 0 gaps.** This class
   of error should not recur.

3. **Math vector overloads (10 functions) → `aqMath.cpp`:**
   `BlackScholes{Prices,ImpliedVols}`, `CapletFloorlet{Prices,ImpliedVols}`,
   `EuropeanIRSwaption{Prices,ImpliedVols}` (straightforward column-of-scalar
   generalisations of the already-ported single forms), and the low-level
   `ForwardRate(s)`/`DiscountFactor(s)` primitives (raw `(dates,values)`
   curve-fit inputs, needing `etrading::to{Interpolation,StateVariable,
   DayCount,CompoundingFrequency}Enum` string→enum conversions — all four
   free functions already existed in `CoreEnumerations.h`, just needed
   calling). **Each of these 4 names also has a second, `"*** LEGACY
   METHOD ***"`-labelled overload** (curveCollection/curveIndex based) in the
   same header — ported only the non-legacy `(dates,values)` overload, per
   the header's own explicit labelling (not a guess: the source comment says
   so). New local helpers added to `aqMath.cpp`: `toStringVectorOr`,
   `toDoubleVectorOr`, `toCallOrPutEnumVector`, `toStandardStringMatrixOr`.

4. **Swap risk ladders (4 functions) → `aqSwap.cpp`:**
   - **New marshalling pattern, `toLabelValueBlockVector`:** a table
     convention (row 0 = shared key headers, each subsequent row = one
     trade's values against those keys — an ordinary Excel table with a
     header row) used to build a `std::vector<LabelValueBlock>` from a single
     range. No such pattern existed anywhere in the codebase before this;
     used by `aqSwapObjectPVs` (batch PV) and `aqSwapDelta` (stateless
     multi-trade flat-shift delta, which also needed `etrading::
     getDataInstance()` for its `AQLDataInstance*` parameter).
   - **New output pattern, `toExcelLabeledMatrix`:** row/column labels plus a
     `DoubleMatrix` → one sheet with a blank-corner header row, reused by
     `aqSwapDelta` and `aqSwapObjectDeltaLadder`.
   - `aqSwapObjectDeltaLadderHorizontally` needed its own ragged
     multi-column layout (`vector<AQLStringVector>` / `vector<DoubleVector>`,
     one pair of columns per curve, padded to the longest column) — built
     inline, no reusable helper extracted since nothing else needs this exact
     shape.
   - `aqSwapObjectPVs` has a real signature quirk: `legNames` and
     `fixingTableNames` are non-`const` reference parameters despite being
     used as inputs per their doc comments (a pre-existing quirk in the
     validation header, not something to silently "fix") — handled by
     passing local mutable variables.

**Verified (static checks only, no compiler run yet on this batch):**
- Whole-tree: **466 functions across all of `AQ_XLL/src/*.cpp`, 0
  mismatches, 0 duplicate names, 0 unresolved `validation::tryAq*` symbols.**
- Re-ran the exact same gap-audit script after this batch: **9 gap entries
  remain, and every one is an intentional non-gap** — `tryAqBondObjectZSpreads`
  (false positive, explained above), `tryAqToolLVBAdd` (in-place mutator,
  doesn't fit a worksheet-function shape, correctly left unported), and the 7
  SABR legacy functions (explicitly decided not to port). **There is no
  longer any unaccounted-for gap between `validation` and `AQ_XLL`** for
  every category except `Model`/`Generator`, which remain genuinely empty
  (0 wrappers, net-new work).
- `api_pair_check.py` HARD GATE still 0 (this script scans `AQ_API`, not
  `AQ_XLL` — a hygiene check here, not a real gate on this work).
- No new files added this batch (`aqCurve.cpp`, `aqObject.cpp`, `aqTool.cpp`,
  `aqMath.cpp`, `aqSwap.cpp` all already existed and were already wired into
  `AQ_XLL.vcxproj`) — so no `.vcxproj`/`.filters` changes needed for `AQ_XLL`
  this round; only the two `validation.vcxproj` wiring-gap fixes.

**Not yet done:** a real compiler build of this batch. Given the pattern from
the last two build attempts (type-marshalling mismatches, then a linker gap),
**expect at least one more round of build fixes** — the enum-conversion calls
in the new Math functions and the two new marshalling helpers in `aqSwap.cpp`
are the least-previously-exercised code in this batch and the most likely
sources of a first-build surprise.

**What's left after this batch, in priority order:**
1. Build this batch, fix whatever surfaces.
2. `Model` and `Generator` categories — genuinely empty, need `validation`
   wrappers written from scratch (`tryAqGeneratorList`/`Describe`/`Validate`
   for Generator; Model may stay sparse per CLAUDE.md Sec5.1). This is no
   longer "migrate a function", it's "design and build a new one" — a
   different, larger kind of task than everything done today.
3. `Model`/`Generator` aside, the rebrand's Phase 4 (xlOil XLL port) is
   essentially feature-complete pending that build.
4. Phase 5 (bindings/SWIG regeneration, verify C#/Java/R), Phase 6 (legacy
   extraction, resources audit, licence headers), Phase 7 (Linux/CMake,
   clang-format, clean repo) — all still ahead, per `MIGRATION_PLAN.md`.
5. Top-level `STATUS.md` (the short pointer file) is now significantly out of
   date — still describes ~35% Phase 4 progress from several sessions ago.
   Worth a full rewrite once this batch is build-confirmed, not another
   incremental patch.

---

## Second build attempt: 1 linker error, a pre-existing `validation.vcxproj` wiring gap (2026-09-11, NOT YET RE-BUILT)

Compile succeeded this time (the 4 `aqCurve.cpp` fixes held); the only
failure was `LNK2001: unresolved external symbol ... tryAqCurveGetInterpolation
JoinDate`. **Root cause: `src/validation/src/tryAqCurveGetInterpolationJoinDate.cpp`
existed on disk, fully implemented and matching its header exactly, but was
never added to `projects/validation.vcxproj`** — so it never compiled into
the `validation` static lib, and `aqCurve.cpp`'s call to it had nothing to
link against. This is the exact same class of gap as the
`tryAqToolEchoDouble.cpp` one found earlier this session (a file that exists
correctly but was never wired into its project) — except this time, since the
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
`validation.vcxproj`'s `ClCompile` entries (100) — **exactly one gap, and
it's the already-known, already-triaged one**: `tryAqToolEchoDouble.cpp`
(found earlier this session, deliberately left unwired since nothing in this
rebrand calls it — `aqToolEchoDouble` was removed from `aqTool.cpp` for that
exact reason). No other silent gaps exist, so this class of error should not
recur on the next build.

---

## First real build attempt: 4 compile errors in `aqCurve.cpp`, all fixed (2026-09-11, NOT YET RE-BUILT)

Nicholas ran the actual Release x64 build. As expected, static/textual checks
alone didn't catch everything — 4 genuine compile errors, all in `aqCurve.cpp`
batch 2, all now fixed:

1. **`etrading::DiscountFactorTable` is not a member of `etrading`**
   (`aqCurveDiscountFactorsDisplay`). `DiscountFactorTable` is forward-declared
   in the **global namespace** in `tryAqCurveDiscountFactor.h` (`struct
   DiscountFactorTable;` sits *before* `namespace validation` opens), defined
   in `AQLCurveCalibrationHelpers.h`, also global. Fixed: `etrading::
   DiscountFactorTable` → `::DiscountFactorTable`.
2. **`toExcelColumn` type mismatch** (`aqCurveObjectEngineCalibrate`):
   `tryAqCurveObjectEngineCalibrate` returns `AQLStringVector`
   (`vector<AQLString>`), but `aq_xll::toExcelColumn` only has an overload for
   `std::vector<std::string>` — no implicit conversion exists between
   `AQLString` and `std::string`. Fixed with an explicit per-element
   `getCString()` copy into a `std::vector<std::string>` before calling
   `toExcelColumn`.
3. & 4. **`toExcelMatrix` has no `DoubleMatrix` overload**
   (`aqCurveObjectEngineJacobianDisplay`'s value branch and
   `aqCurveObjectJacobianDisplay`) — `aq_xll::toExcelMatrix` only overloads on
   `VariantMatrix` / `AQLStringMatrix` / `AnyTypeMatrix`, and a plain
   `DoubleMatrix` (`vector<vector<double>>`) matches none of them. Added a new
   file-local helper `toExcelDoubleMatrix( const DoubleMatrix& )` (wraps each
   `double` into an `etrading::Variant` and delegates to the existing
   `toExcelMatrix( VariantMatrix )` overload) and pointed both call sites at
   it instead.

**Root cause pattern worth naming:** all 4 errors are marshalling-layer type
mismatches between `AQL*`/raw-C++ container types and the `aq_xll::toExcel*`
helper's actual overload set — exactly the class of bug the static
signature-vs-header cross-checks done while writing this file could not
catch, since they verify the *validation* call compiles conceptually but not
that the *return marshalling* type-checks against the narrower `aq_xll`
overload surface. Worth remembering for the remaining unbuilt files
(`aqSwap.cpp`, `aqCredit.cpp`) — same class of error is plausible there too,
just not yet triggered because the build failed on `aqCurve.cpp` first (MSVC
stops enumerating a project's remaining files after an error battery from one
translation unit in this log, but doesn't skip other .cpp files entirely —
this build log only shows `aqCurve.cpp` errors because `AQ_XLL.vcxproj` likely
compiles files in filesystem/vcxproj order and `aqCurve.cpp` sorts before
`aqCredit.cpp`/`aqSwap.cpp`; expect more of this same error class on the next
build attempt once `aqCurve.cpp` is clean).

**Re-verified after the fix:** whole-tree param-vs-`.arg()` audit still
445/445 clean, 0 duplicates (these were pure type-marshalling fixes, no
signature or `.arg()` count changed). **Grepped `aqSwap.cpp`/`aqCredit.cpp`
for the same two failure patterns** (`toExcelMatrix( <DoubleMatrix var> )`,
`toExcelColumn` on a non-`vector<string>` return) — none found, but this
grep is necessarily incomplete (it can't run a real overload-resolution
check), so **the next build is very likely to surface more of this same
error class in the two still-untouched files** — expect it, don't be
surprised by it.

---

## ALL REMAINING CATEGORIES MIGRATED — Credit done (34 functions), Phase 4 XLL port essentially complete pending a build (2026-09-11, NOT YET BUILT)

Nicholas: "Let's migrate all remaining categories... do them all now." Last of
the three big deferred categories, `Credit`, is now written as `aqCredit.cpp`.

**Discovery: no dedicated `tryAqCredit*.h` files exist.** Every Credit
function lives inside `tryAqSwapObjectPricing.h`/`.cpp` (alongside the
already-ported CMS/TRS pricing — a pre-existing filing quirk from before the
category scheme existed), plus two more: `tryAqCreditObjectFeeLegCreate` in
`tryAqSwapObjectLeg.h` and `tryAqCreditObjectFeeScheduleCreate` in
`tryAqSwapObjectSchedule.h`. Per CLAUDE.md Sec5.1a, the golden **name**
decides the category, not the file — so all of these were pulled out into
their own `aqCredit.cpp`, not left in `aqSwap.cpp`.

**`aqCredit.cpp` written — 34 functions.** CDS pricing driven directly by a
hazard rate (`...FromHazardRate` forms: PV, RiskyAnnuity, ParSpread — 3), CDS
pricing driven by a cached credit model (PV, PVByIntegration, PVByMonteCarlo,
RiskyAnnuity, AccruedYearFraction, CS01, ParSpread — 7), credit model
lifecycle (Create using the `JSONInfoBlockTuples` two-block pattern already
established for Bond/Inflation curves, AsOfDate, CalibrationParameters,
HazardRate, SurvivalProbability, DefaultProbability, ImpliedSurvivalDate,
RiskyDiscountFactors — 8), credit spread (Spread, IndexSpread — 2), credit
(index) options (OptionPV(+FromForward), OptionImpliedVol(+FromForward),
IndexOptionPV/ImpliedVol/Vega/CS01/Theta — 9), credit basket model (Create,
SurvivalProbability — 2), and the two fee leg/schedule builders (2) —
3+7+8+2+9+2+2 = 33, plus `HazardRateFromParSpread` = 34.

**Flagged, not guessed: one genuine overload collision.**
`tryAqCreditObjectDefaultSwapHazardRateFromParSpread` has two C++ overloads
sharing the *exact same* validation name — one takes a `valuationSettingsLVB`
+ `recoveryRate` + `includeAccruedInterest` (hazard-rate-calculation style),
the other a `creditModelName` (credit-model style). Its three siblings
(PV/RiskyAnnuity/ParSpread) each give the hazard-rate variant a distinct
`...FromHazardRate` suffix and leave the model variant with the plain name —
this function alone breaks that pattern and keeps one shared name for both,
which Excel cannot register twice. This reads as a naming gap in the golden
source itself (someone forgot the analogous suffix), not a case where
guessing a fix is appropriate. **Ported the `valuationSettingsLVB`-driven
overload only**, as `aqCreditObjectDefaultSwapHazardRateFromParSpread`; the
`creditModelName`-driven overload is NOT exposed — documented in the file
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

**Status after this batch — every LOCKED category has at least a first pass
in `AQ_XLL`:** `Date, Curve, FX, Inflation, Volatility, InterestRate, Future,
Swap, AssetSwap, CMS, TRS, CapFloor, Swaption, BondOption, BondFutureOption,
Bond, Credit, Math, Model, Generator, Tool` — `Model` and `Generator` remain
genuinely empty (0 validation wrappers exist for either, confirmed in an
earlier session; `Generator` needs a `tryAqGeneratorList` wrapper built from
scratch before any XLL surface is possible — not yet done, unrelated to this
migration pass).

**Explicitly deferred across this whole "migrate everything" push — not
silently dropped, each documented in its file's header comment and here:**
- Curve: the curve Results/Jacobian risk family — **done this session**,
  scratch that, already ported; nothing outstanding in Curve.
- Swap: `tryAqSwapDelta` (stateless multi-trade), `tryAqSwapObjectDeltaLadder`/
  `DeltaLadderHorizontally` (multi-curve risk ladders — all three take a
  `std::vector<LabelValueBlock>` shape with no marshalling helper yet), and
  `tryAqSwapObjectPVs` (vector-of-swaps batch PV, same disposition as the
  Math `*Prices` vector overloads deferred earlier this session).
- Credit: one `tryAqCreditObjectDefaultSwapHazardRateFromParSpread` overload
  (see above).
- Volatility: the legacy procedural SABR family (`tryAqVolatilitySABR*`) vs
  the already-ported object-based `tryAqVolatilityObjectSabr*` — still an
  open decision from an earlier session, not re-raised this pass.

**Not yet done:** a real compiler build of `aqSwap.cpp`, `aqCredit.cpp`, and
Curve's batch 2 — none of the three have been through anything but static
checks yet. **This is the single most important next step** before trusting
any of this further; expect the usual first-build friction (a missed
include, a signature that drifted between when it was read and when it was
used) despite the careful checks already run.

**Also still pending, unrelated to categories:** `Swap` category work
technically also needed the Ois-fold-in, which is now done (see the section
below); the `Model`/`Generator` categories remain genuinely empty and need
their own from-scratch work, not a port; top-level `STATUS.md` (the short
pointer file) has not been refreshed since the `InterestRate` rename and now
significantly understates progress — worth a full rewrite once a build
confirms this batch, rather than another incremental patch.

---

## Swap category done (62 functions) + `Ois` rename completed (2026-09-11, NOT YET BUILT)

Nicholas confirmed the Curve batch-2 build works. "Let's migrate all remaining
categories... do them all now" continued into `Swap`.

**Prerequisite: the long-pending `Ois`→`Swap` rename, done first.** The
pre-existing `tryAqOisPV`/`tryAqOisParRate` wrappers (+ `*LVBKeys`
companions) predated the category scheme and needed renaming before Swap
could be written cleanly (flagged as a to-do since the `Ois`-fold decision
several turns ago). Renamed in full, files included, across every surface:
- `validation`: `tryAqOisPV.{h,cpp}` → `tryAqSwapOisPV.{h,cpp}`,
  `tryAqOisParRate.{h,cpp}` → `tryAqSwapOisParRate.{h,cpp}` (git mv + content
  rename); `projects/validation.vcxproj` + `.filters` updated.
- `AQ_API`: `aqOisPV.{h,cpp}` → `aqSwapOisPV.{h,cpp}`,
  `aqOisParRate.{h,cpp}` → `aqSwapOisParRate.{h,cpp}`; all 4 SWIG `.i` files
  (CSharp/JAVA/Python/R) updated; `projects/AQ_API.vcxproj` + `.filters`
  updated. (The generated `swig_Python_wrap.cxx` still says the old names —
  expected, untouched, regenerates on next SWIG build.)
- `GTEST`: `TryAqTestTradeEUROISParRate.cpp` — includes, the two `TEST_DIR`
  fixture-path string constants, and both call sites renamed.
- **100 fixture CSV files** renamed (`EURYC_tryAqOis{PV,ParRate}_{inputs,outputs}N.csv`
  → `EURYC_tryAqSwapOis*`) — discovered these are **not git-tracked**
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

**`aqSwap.cpp` written — 62 functions.** Covers: stateless swap pricing
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
return ragged per-curve vector-of-vectors — no marshalling helper for either
shape exists yet, and building one well is a bigger design task than this
batch's pace justifies; `tryAqSwapObjectPVs` (the vector-of-swaps batch PV
overload) is deferred for the same reason the Math `*Prices` vector overloads
were deferred earlier this session. `tryAqCreditObjectFeeLegCreate`/
`tryAqCreditObjectFeeScheduleCreate` (filed in the Swap leg/schedule headers
but golden-named Credit) and `tryAqToolSwapScheduleTemplate` (golden-named
Tool) were correctly left out of this file — they belong to their own
category files.

**New shared local helpers, `aqSwap.cpp`'s anonymous namespace (all
file-local, matching the established per-file pattern):**
`toLabelValueBlockOr`, `toStringVectorOr`, `toAQLStringVectorOr`,
`toDoubleVectorOr`, `toStrOr`, `toAQLStringVectorWithoutCounter` (converts
`getNamesWithoutCounter`'s `std::vector<std::string>` to the `AQLStringVector`
`tryAqSwapObjectDelta` actually takes — a real type mismatch caught and fixed
before it could hit a build), and `toExcelStackedLegMatrices` (concatenates
the `std::vector<AnyTypeMatrix>` that `tryAqSwapObjectDisplay`/
`DisplayCashflows` return — one block per leg — into a single sheet with a
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
- `api_pair_check.py` HARD GATE still 0 (unaffected either way — that script
  only scans `AQ_API`, not `AQ_XLL`).

**Not yet done:** a real compiler build (first time this file has been
through anything but static/textual checks).

**Next: Credit (~34 wrappers, minus whatever CMS/TRS pricing already covered
under `aqCreditObject.cpp` in `AQ_API` — needs a fresh enumeration pass, not
yet done this turn) — the last of the three big deferred categories.**

---

## Curve category COMPLETE: batch 2 added, 93 functions total (2026-09-11, NOT YET BUILT)

Nicholas confirmed the build works after `aqCurve.cpp` was wired in (batch 1,
34 functions — see the section below). "Let's migrate all remaining
categories... do them all now" then triggered batch 2, finishing Curve.

**Decision needed and resolved first** (was flagged as open in the prior
session): Curve has two overlapping function families — the new AQObj
curve-handle API (batch 1) and an older `curveCollection`+`curveIndex`
stateless family that predates it (CompoundRate, DiscountFactor, ForwardRate,
the four heavy one-shot `Calibrate{Basis,CTD,FXForwards,OIS,Swap}` functions,
their `ObjectCreate{Basis,FXForwards,OIS,Swap}` handle-returning twins, etc).
Nicholas's answer: **port both — stateless functions as `aqCurve<Function>`
(no `Object`), object/handle functions as `aqCurveObject<Function>`**, exactly
per the golden `tryAq*` name (no renaming; these are pre-existing wrappers).

**Batch 2 added to `aqCurve.cpp`** (59 functions, bringing the file to 93):
CompoundRate ×2, Delete, Display, Frequency, GetInterpolationJoinDate /
InterpolationJoinDate (two distinct wrappers, same job), EuroDollar
ConvexityAdjustment, the legacy DiscountFactor family ×12 (incl.
DiscountFactorsDisplay returning a Term/PaymentDate/DiscountFactor table
marshalled from `etrading::DiscountFactorTable`, and TermsToDates/
DatesToTerms), the legacy ForwardRate family ×5 (incl. the
curveCollection-or-handle-taking `...FromForwardDatesFromObject` variant, and
the table-building `aqCurveObjectForwardRatesTable`), HullWhite/Vasicek
forward-rate model checks ×3, the `ObjectData` LVB convenience builder ×2
(`aqCurveObjectDataCreate` takes 8 optional `AQLStringMatrix` blocks — swap/
FRA/futures/central-bank/FX), dual bootstrap (stateless ×1, 26 params; object
×1 returning a curve-index map marshalled as a key/value block), engine
calibrate + its Jacobian display ×3, the curve-results/Jacobian-risk family
×11 (`RiskType`/`BusinessDayAdjustmentEnum` string args converted via
`etrading::toRiskTypeEnum`/`toBusinessDayAdjustmentEnum` from
`CoreEnumerations.h`), curve groups ×2, and finally the 5 heavy
`Calibrate*` + 4 `ObjectCreate*` one-shot functions (10-17 raw
`AQLStringMatrix` params each).

**New shared local helpers added to `aqCurve.cpp`'s anonymous namespace**
(each is file-local per the established pattern, not shared via
`aqXllTools.h`): `toDoubleOr`, `toAQLDateOr` (missing → default-constructed
`AQLDate()`, the "use the curve's own as-of date" sentinel several
model-check functions rely on), `toBoolVector` (a column of TRUE/FALSE cells
→ `std::vector<bool>`, via `etrading::Variant::getValue<bool>()`), and
`toAQLStringMatrixOr` (missing/blank → a genuinely empty `AQLStringMatrix`,
**not** the 1x1-empty-string block `toAQLStringMatrix` would otherwise
produce for a missing cell — several calibration/data-block functions test
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
`aqTool.cpp`/`aqDate.cpp`) — every one of these was wrapped with
`.getCString()` rather than passed to `returnValue` as-is.

**Verified (static checks only, no compiler run yet):**
- Whole-tree param-vs-`.arg()` audit: **349 functions across all of
  `AQ_XLL/src/*.cpp`, 0 mismatches.** `aqCurve.cpp` alone: **93/93 clean.**
- Every one of the 93 `validation::tryAq*` calls in `aqCurve.cpp` confirmed to
  match an actually-declared name across `src/validation/include/*.h` (a
  Python cross-check against the header text, not just a signature glance).
- No duplicate `XLO_FUNC` names anywhere in the tree.

**Not yet done:** a real compiler build (first time `aqCurve.cpp` batch 2 has
been through anything but static/textual checks — batch 1 alone was already
build-verified in the prior turn, batch 2 has not).

**Curve category is now considered complete** bar three deliberately-excluded
internal helpers (`getDayCount`, `isBasisFlag`,
`populateDiscountFactorConventions` in `tryAqCurveDiscountFactor.h` — legacy
default-population plumbing, no `aqCurve` naming, nothing for a user to call).

**Next: Swap (~67 wrappers, including the pre-existing `Ois`→`aqSwapOis*`
rename work) and Credit (~34 wrappers).**

---

## `aqCurve.cpp` wired into `AQ_XLL.vcxproj` (2026-09-11, NOT YET BUILT)

Added the two remaining entries for `aqCurve.cpp` (written earlier this
session, batch-1, 34 functions — see the "Still pending" section below for
what it covers):

- `projects\AQ_XLL.vcxproj` — `<ClCompile Include="..\src\AQ_XLL\src\aqCurve.cpp" />`,
  alphabetically between `aqCMS.cpp` and `aqDate.cpp`.
- `projects\AQ_XLL.vcxproj.filters` — same entry, filed under **`src\Optional`**
  (per the new edition-filter convention — see the section above), between
  `aqCMS.cpp` and `aqFuture.cpp`.

**Verified (static checks only, no compiler run yet):**
- Both files re-parse as well-formed XML (`xml.etree.ElementTree`).
- `<ClCompile Include=` file lists identical between `.vcxproj` and
  `.vcxproj.filters` — 18/18, no orphans either direction (the
  orphaned-`</ClCompile>`-tag mistake from earlier this session did not
  recur).
- `<ClCompile Include=` count (18) matches `</ClCompile>` count (18) in
  `.filters`.
- Ran `rebrand\tools\api_pair_check.py`: HARD GATE still **0**. **Caveat worth
  recording:** this script only scans `src\AQ_API\source\*.{h,cpp}` — it does
  **not** see `src\AQ_XLL\src\*.cpp` at all, so it neither validates nor is
  affected by `aqCurve.cpp`; it was already 0 before this change and stays 0
  after. The actual XLL-specific check (param count vs `.arg()` count) was
  already run and passed in the prior session (34/34 clean, part of the
  290-function whole-tree audit) — not re-run here since no `aqCurve.cpp`
  content changed, only its project wiring.

**Next: a real build.** This is the first thing that will actually compile
`aqCurve.cpp` — it has never been through a compiler. Expect possible
first-build issues (missing includes, minor signature drift) despite the
careful signature-vs-header cross-check done when the file was written.

---

## Resumed (2026-09-11): build green, tests pass; `Core`/`Optional` edition filters added; docs updated

Nicholas confirmed **the build works and tests pass** with everything that was
pending at the pause point below (the `StructuredExceptionHandler`
plain-English + crash-location rework, the CMS/TRS renames, `aqBond.cpp`
merge, `aqToolSEH()`) — first real compiler+GTEST confirmation of this
session's work, superseding the "static checks only, not yet built" caveats
further down for everything up to and including that point.

**Nicholas separately added and committed to git** (outside this session, own
work): two new Solution Explorer filters in
`projects\AQ_XLL.vcxproj.filters` — **`src\Core`** (always-built:
`aqXllTools.cpp`, `aqMain.cpp`, `aqDate.cpp`, `aqObject.cpp`, `aqMath.cpp`,
`aqTool.cpp`) and **`src\Optional`** (every product-category file —
`aqAssetSwap.cpp`, `aqBond.cpp`, `aqCapFloor.cpp`, `aqCMS.cpp`,
`aqFuture.cpp`, `aqFX.cpp`, `aqInflation.cpp`, `aqInterestRate.cpp`,
`aqSwaption.cpp`, `aqTRS.cpp`, `aqVolatility.cpp` — `aqCurve.cpp` not yet
added, see below). Intent: when per-edition build configurations
(`ReleaseBonds`, `ReleaseSwaps`, `ReleaseCurves`, …) are added alongside the
existing `Debug`/`DebugEditAndContinue`/`ReleaseProfiler`/`Release`, each
compiles `Core` plus only the `Optional` file(s) its edition needs (excluded
elsewhere); `Release` (Full) excludes nothing. **Documented this session** in
`CLAUDE.md` (both, §4.4/§4.5 + a new §6.3/§9.5) and `MIGRATION_PLAN.md`
(Phase 4a split into an `AQ_API` runtime-gate half, unchanged, and a new
`AQ_XLL` compile-time-gate half with a new checklist item 4a.5) — **the
"no per-edition builds" line in the old Phase 4a text was specific to
`AQ_API`'s language-binding matrix and did not anticipate this; both plans now
coexist explicitly, one per surface.** No code changed for this — pure
documentation to match what Nicholas already committed.

**Standing reminder for whoever writes the next `AQ_XLL` category file:** new
category files go in `src\Optional`, never `src\Core` — the always-needed set
is already complete (`aqCurve.cpp`, once wired into the vcxproj — see the
paused-work section immediately below — goes to `Optional`).

---

## ⏸ Still pending from the pause point below (2026-09-11)

**Next action for the next session:** finish wiring `aqCurve.cpp` into the
build, then continue the Curve migration. In order:

1. **`aqCurve.cpp` is written (batch 1, 34 functions) but NOT YET added to
   `projects/AQ_XLL.vcxproj` / `.vcxproj.filters`.** Confirmed by grep — no
   `aqCurve.cpp` entry exists in the vcxproj yet. Add it the same way every
   other new category file was added this session (`<ClCompile
   Include="..\src\AQ_XLL\src\aqCurve.cpp" />` in both files, filter
   `Source Files`), then re-verify both files are well-formed XML
   (`python -c "import xml.etree.ElementTree as ET; ET.parse(path)"`) and that
   the `<ClCompile Include=` count matches the `</ClCompile>` count (the
   orphaned-tag bug from earlier this session).
2. **Batch-1 verification already done, before adding to the project:**
   whole-tree param-vs-`.arg()` audit run against every `src/AQ_XLL/src/*.cpp`
   — **290 functions total, 0 real mismatches** (`aqToolEcho` flagged by the
   quick regex is a false positive — it takes `const ExcelObj*`, not `&`, and
   is genuinely 1 param / 1 `.arg()`, pre-existing and fine). `aqCurve.cpp`
   itself: **34/34 clean**. No duplicate `XLO_FUNC` names anywhere in the tree.
   Signatures for all 34 were cross-checked line-by-line against the actual
   validation headers (`tryAqCurveObjectUtilities.h`, `...Display.h`,
   `...Calibrate.h`, `...CalibrateHedge.h`, `...DiscountFactor.h`,
   `...ForwardRate.h`, `tryAqCurveZeroRate.h`, `tryAqCurveGenerator.h`,
   `tryAqCurveMarketData.h`) — all match exactly. Caught and fixed two issues
   while cross-checking: (a) `toIntOr` is a **file-local** helper (lives in an
   anonymous namespace in `aqMath.cpp`, not shared via `aqXllTools.h`) — added
   a local copy to `aqCurve.cpp`'s anonymous namespace, same as the existing
   local `toStrOr`; (b) `etrading::trim_to_upper` (used in
   `aqCurveGeneratorCreate`/`aqCurveMarketDataCreate`) needs
   `#include <CoreEnumerations.h>`, which was missing — added. **Still not
   run through an actual compiler** — these are static/textual checks only;
   the real build is the next hard gate.
3. **`aqCurve.cpp` batch 1 covers:** object lifecycle
   (List/Delete/DeleteAll/Save/Load, 5), Display (1), Calibrate +
   CalibrateHedge (2 — the hedge one returns `etrading::HedgeCurveInfo`,
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
   (`validation → AQ_API → AQ_XLL → GTEST`), then continue with the rest of
   Curve — see the enumerated ~97-function list captured earlier this session
   (not reproduced here in full; re-derive from `src/validation/include/
   tryAqCurve*.h` if needed, there are 32 headers). **Not yet started:** the
   curve Results/Jacobian risk family (`tryAqCurveResults.h`,
   `tryAqCurveObjectJacobianDisplay.h`), `tryAqCurveObjectDualBootstrap.h`,
   `tryAqCurveHullWhite.h`/`tryAqCurveVasicek.h`, `tryAqCurveCalibrateCTD.h`,
   the misc utilities (`tryAqCurveCompoundRate.h`,
   `tryAqCurveEuroDollarConvexityAdjustment.h`, `tryAqCurveFrequency.h`,
   `tryAqCurveGetInterpolationJoinDate.h`, `tryAqCurveInterpolation.h`),
   `tryAqBondCurves.h` (check whether this belongs to `Bond` not `Curve` —
   name suggests bond-curve, likely already covered by `aqBond.cpp`, verify
   before touching), and `tryAqCurveObjectData.h`/`tryAqCurveObjectEngineCalibrate.h`
   (not yet read).
5. **Still an OPEN DECISION, not yet asked of Nicholas:** the four heavy
   one-shot creation functions `tryAqCurveObjectCreateBasis/FXForwards/OIS/Swap`
   (`tryAqCurveObjectCreateBasis.h`, `...CreateFXForwards.h`, `...CreateOIS.h`,
   `...CreateSwap.h` — 10-17 raw `AQLStringMatrix` params each). These look
   like a legacy one-shot path that may be superseded by the
   `CurveGenerator`+`CurveMarketData`+`tryAqCurveObjectCalibrate` two-step
   architecture already ported in batch 1 (which `CLAUDE.md` §4.4 explicitly
   endorses as the intended design). **Ask Nicholas before porting these** —
   don't guess.

**Also still open from earlier in this session (unrelated to Curve):**
- The legacy SABR calibration family decision
  (`tryAqVolatilitySABR{Calibrate,GetPrem,GetVol,...}` vs the already-ported
  object-based `tryAqVolatilityObjectSabr*`) — not yet asked.
- `Swap` category (~67 wrappers, including the `Ois`→`aqSwapOis*` rename work
  documented in `CLAUDE.md` §5.1) and `Credit` category (~34 wrappers) — not
  started, come after Curve.
- `src/validation/src/tryAqToolEchoDouble.cpp` needs adding to
  `projects/validation.vcxproj` (pre-existing gap, unrelated to this session).
- Confirm with Nicholas whether the improved `aqToolSEH()` message (see next
  section) actually resolves a `file(line)` in Excel once rebuilt — the
  DbgHelp path is untested end-to-end.
- Top-level `STATUS.md` (the short pointer file, not this one) has not been
  refreshed since the `InterestRate` rename — still says ~35%/Phase 4 without
  mentioning CMS/TRS/aqToolSEH/Curve-start. Worth a refresh next session.

Nothing in this session has been committed (still uncommitted, matching every
other entry in this file) and nothing has been built — every check above is
static (grep/regex/XML-parse), not a compiler run. **First thing next
session: do a real build** before trusting any of this further.

---

### `aqToolSEH()` feedback: structured-exception messages made plain-English + crash-located (2026-09-11, uncommitted, NOT BUILT)

Nicholas tried `aqToolSEH()` in Excel: **it works** — `AQ_XLL_GUARD` /
`etrading::StructuredExceptionHandler` catches the access violation cleanly,
no crash. Follow-up ask: make the returned error message concise plain English
(`#Structured Exception: <Category> - <plain english>`, not the old verbose
MSDN-quoted paragraph), and report the crash site (`__FILE__`/`__LINE__`) to
speed up debugging.

Changed `src/etrading/include/StructuredExceptionHandler.h` +
`src/etrading/src/StructuredExceptionHandler.cpp` (the shared SEH translator
behind every `AQ_XLL_GUARD`/`VALID_EXCEPTION_START`, not just `aqToolSEH` —
this improves every structured-exception message library-wide):

- `StructuredExceptionCodeMap`'s map value changed from a single blended
  `const char*` to a new `StructuredExceptionInfo{ category_, plainEnglish_ }`
  struct. All 20 Win32 exception codes re-worded: short category
  ("Access Violation", "Integer Divide By Zero", ...) + one concise
  plain-English sentence each (old text was a verbatim MSDN paragraph).
- `SEHandler` now formats `"#Structured Exception: <Category> - <plain
  english> at <file>(<line>)."` — e.g. `#Structured Exception: Access
  Violation - attempted to read from or write to a memory address it does not
  have access to. at aqTool.cpp(412).`
- **Crash-site location, not handler location:** added a file-local
  `resolveCrashLocation()` using DbgHelp (`SymInitialize` +
  `SymGetLineFromAddr64`) against `pExcept->ExceptionRecord->ExceptionAddress`
  — the actual faulting instruction — to resolve `file(line)` when a PDB is
  available (debug builds). Falls back to the raw hex address when no PDB /
  symbol is found (e.g. Release), and to nothing if resolution fails outright
  — never throws from inside the SE translator itself. `__FILE__`/`__LINE__`
  passed to the final `AQLCoreError` are still this handler's own location
  (unavoidable — that's what the macro expands to at the `throw` site); the
  crash site is carried in the message text instead, which is what actually
  answers "where did it crash".
- New includes: `<DbgHelp.h>` + `#pragma comment(lib, "Dbghelp.lib")` (Windows
  only, inside the existing `_WIN32`/`_WIN64` guard) — no `.vcxproj` change
  needed, the pragma embeds the linker directive in the `.obj` and every
  consumer (`AQ_XLL`, `AQ_API`, `GTEST`) picks it up at final link.

**Verification (no build yet):** grepped the tree for other callers of
`StructuredExceptionCodeMap`/`getExceptionCodes` — none found outside this
pair of files, so the map-value-type change is self-contained. Needs a real
build + a re-run of `aqToolSEH()` in Excel to confirm the DbgHelp path
resolves a file/line in the Debug config (PDB for `etrading`/`AQ_XLL` must be
present at runtime, same folder as the `.xll`/`.pdb`) — **not yet verified
end-to-end**, flagged for the next Excel round-trip with Nicholas.

---

### `TotalReturnSwap` category renamed to `TRS` (2026-09-11, uncommitted, NOT BUILT)

Same pattern as the `ConstantMaturitySwap` → `CMS` rename, one layer further:
**`TotalReturnSwap` already had a real `AQ_API` surface** (unlike CMS) — 4
functions in `aqCreditObject.cpp/.h` (filed under the Credit API source, not a
dedicated TRS file — same pre-existing filing quirk as the Ois/FixingTable
functions living in `aqCurveObject.h`), already generated into all 4 SWIG
`_wrap` files.

- `validation`: `tryAqSwapObjectPricing.{h,cpp}` — substring rename
  `TotalReturnSwapObject → TRSObject` (6 hits each file). Confirmed
  `etrading::TotalReturnSwap` (the internal pricer class) and
  `#include "TotalReturnSwap.h"` untouched — same anchor discipline as CMS.
- `AQ_API`: `aqCreditObject.h` (9 hits) / `.cpp` (16 hits) renamed
  (`aqTotalReturnSwapObject* → aqTRSObject*`, both the function defs and their
  `validation::tryAq...` call sites). The 4 generated `swig_*_wrap.*` files
  still say the old name — expected, untouched, will regenerate on next SWIG
  build.
- `GTEST`: `TestTotalReturnSwap.cpp` — 29 call-site substitutions. (Its own
  `TRS_*` constant names were already TRS-branded; only the wrapper-name calls
  needed changing.)
- `AQ_XLL`: `aqTotalReturnSwap.cpp` → `aqTRS.cpp`, 4 functions renamed,
  `AQ_XLL.vcxproj`/`.filters` updated (clean single-line edit this time — no
  repeat of the CMS batch's orphaned-tag mistake).
- 16 fixture files renamed (`tryAqTRSObject{PV,ParRate,ParSpread,Annuity}*_{inputs,outputs}.csv`,
  including variant suffixes like `_ALL`, `_Float`, `_Premium`, `_PayOnSurvival`).
- Docs: `CLAUDE.md` (both), `MIGRATION_PLAN.md`, `rebrand/tools/api_pair_check.py`
  `CATEGORIES` updated `TotalReturnSwap` → `TRS`; **`docs/api_map.csv`
  regenerated** this time (unlike CMS) since the AQ_API rows actually changed —
  `aqTRSObjectPV/ParRate/ParSpread/Annuity` now show category `TRS`.

**Verification:** HARD GATE = 0; `aqTRS.cpp` 4/4 param-vs-`.arg()`; zero
residual `TotalReturnSwapObject` outside the generated SWIG files; both
project files re-confirmed well-formed XML; fixture rename count matches (16
renamed, 0 old names left).

### Nicholas's follow-up on batch 1: CMS rename, BondOption/BondFutureOption merge, aqToolSEH diagnostic (2026-09-11, uncommitted, NOT BUILT)

Three requests after reviewing batch 1:

**1. `ConstantMaturitySwap` category renamed to `CMS`.** Golden-source rename,
validation first: `tryAqConstantMaturitySwapObject{PVUsingConvexityAdjustment,
ParRateUsingConvexityAdjustment}` → `tryAqCMSObject*` in the shared
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
matches the bare internal class name (per CLAUDE.md §5.1a: category names
don't propagate into `etrading` class/file names).
- `AQ_XLL`: `aqConstantMaturitySwap.cpp` → `aqCMS.cpp`, 2 functions renamed,
  `AQ_XLL.vcxproj`/`.filters` updated.
- `GTEST`: `TestConstantMaturitySwap.cpp` call sites updated (test class name,
  helper function name and `TEST_DIR` folder segment deliberately left as-is -
  internal test naming, not governed by the golden-source rule). 4 fixture
  files renamed (`tryAqCMSObject{PV,ParRate}UsingConvexityAdjustment_{inputs,outputs}.csv`).
- Docs: `CLAUDE.md` (both), `MIGRATION_PLAN.md`, `rebrand/tools/api_pair_check.py`
  `CATEGORIES` updated `ConstantMaturitySwap` → `CMS`.

**2. `aqBondOption.cpp` / `aqBondFutureOption.cpp` deleted; their 6 functions
moved into `aqBond.cpp` verbatim (signatures unchanged).** `AQ_XLL.vcxproj` +
`.filters` updated - **caught and fixed a self-inflicted XML corruption here**:
a scripted removal of the two `<ClCompile>` entries mishandled the 3-line
`.filters` block (`<ClCompile>` / `<Filter>` / `</ClCompile>`) and left two
orphaned `</ClCompile>` tags with no matching open tag. Caught by counting
`<ClCompile Include=` vs `</ClCompile>` (17 vs 19) before moving on, rather than
trusting the "no residual filename string" grep alone; fixed and reverified
both `.vcxproj` and `.filters` as well-formed XML via `xml.etree.ElementTree`.
Documented the resulting **file-per-category exception** in `CLAUDE.md` §5.1a
and `MIGRATION_PLAN.md` D18: consolidate a category's file into a sibling's
only when they share the same underlying cached object (see point 3), not
merely a name prefix.

**3. Nicholas asked where `aqBondFutureOptionObjectCreate` is - there isn't
one, and that's correct, not a gap.** Traced `tryAqBondOptionObjectCreate` →
`etrading::createOption()` → `OptionFactory.cpp`: the factory's
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
header and `CLAUDE.md` §5.1a so the next person doesn't go looking for it either.

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

### "Migrate all remaining categories" — batch 1 of N: 11 small/medium categories DONE (2026-09-11, uncommitted, NOT BUILT)

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

**Verification:** whole-tree param-vs-`.arg()` audit — **255/255 functions
across every `AQ_XLL/src/*.cpp`, 0 mismatches**; no duplicate `XLO_FUNC` names;
every `validation::tryAq*` symbol called resolves in `src/validation/include`;
`vcxproj`/`.filters` file lists identical; `api_pair_check` HARD GATE = 0.

**Bug caught and fixed before it reached a build:** `aqInflationCurveCreate`
and the two `aqVolatilityObjectSabr{MarketDataCreate,ModelCalibrate}` functions
were first written using `std::vector<validation::TableInfo>` for the
multi-block-create pattern (copying the `aqBondCurveCreate` /
`aqBondGeneratorCreate` idiom) — but unlike `tryAqBondObject.h` /
`tryAqToolGrid.h`, neither `tryAqInflationObjectPricing.h` nor
`tryAqVolatilityObject.h` declares a `validation::TableInfo` typedef; both take
`etrading::JSONInfoBlockTuples` directly. `std::vector<validation::TableInfo>`
would have failed to compile ("no member named 'TableInfo' in namespace
'validation'"). Fixed by using `etrading::JSONInfoBlockTuples` directly (the
underlying tuple type is identical, so `toTableInfo(...)` still binds) —
caught by re-reading each header rather than assuming the pattern held.

**Decision needed — the legacy SABR calibration family, deferred:**
`tryAqVolatilitySABR{Calibrate,GetPrem,GetVol,OutputParameter,SetupConvention,
SetupParameter,SetupSwaptionVol}` (`tryAqVolatilitySABRCalibrate.h` and
siblings) is a **procedural, ID-string-driven** calibration workflow
(`alphaID`, `betaID`, `nuID`, `rhoID`, `convID`, `capConvID`, `curveMat`, …)
that looks superseded by the cleaner object-based `tryAqVolatilityObjectSabr*`
API already ported above. **Not ported — needs Nicholas's call:** port it
as-is (needs real domain understanding of what those ID strings reference to
marshal correctly — high risk of a plausible-but-wrong port), or drop it as
legacy since the object-based Sabr* API appears to be the intended replacement.

**Not attempted yet:** `Swap` (67 wrappers, incl. folding `Ois` in as
`aqSwapOis*` — see the section below), `Credit` (34), `Curve` (~100 — the
biggest, needs new enum-string marshalling helpers for `InterpolationEnum` /
`StateVariableEnum` / `DayCountEnum` / etc. and unblocks the deferred Math
`ForwardRate`/`DiscountFactor` functions too), `Model` (**empty — 0 validation
wrappers exist**, nothing to migrate), `Generator` (**0 wrappers — needs
`tryAqGeneratorList` written first**, unchanged from the earlier note).

**== PAUSE POINT (2026-09-10) ==**

**Committed HEAD: `18328864` ("Natvis Debug Vizulaization Update").** Since the
old `3809f148` baseline, Nicholas has committed:

- **`22212bc5`** ("AQ_XLL, validation and GTEST function name synchronization",
  7,905 files) — **task 2.6** (the full `validation` / `GTEST` / `AQ_API` /
  fixture rename to the 22-category singular scheme: `Vol`→`Volatility`,
  `Future` + `Ois` added, `tryAqObj<Cat>* → tryAq<Cat>Object*`), the fixture-
  rename recovery, the **AQ_XLL Tool + Object-lifecycle port** from
  `.APPLES\...\meUtilities.cpp`, and **Interpolation/PCA re-homed `Tool`→`Math`**
  (`tryAqMath{Interpolation,PCA}`). Built green, GTest passing.
- **`18328864`** — `Visualizer.natvis` refresh + `AQ_XLL.vcxproj`. ⚠ this commit
  also staged `src/AQ_API/source/swig_Python_wrap.cxx` — a **generated** file
  that the standing rule says never to commit (§"Known noise"). Flag to Nicholas:
  `git rm --cached` it and re-ignore, or accept it and note the exception.

**Uncommitted working-tree delta right now:**

| Path | State |
|---|---|
| `src/AQ_XLL/src/aqBond.cpp` `aqDate.cpp` `aqMath.cpp` `aqTool.cpp` | **BUILT GREEN, GTest passing** (Nicholas) — the "remaining Bond / Tool / Date / Math functions" fill-in |
| `src/AQ_XLL/src/aqInterestRate.cpp` (renamed from `aqRate.cpp`) + `AQ_XLL.vcxproj` / `.filters` | **NOT BUILT** — new category file, added since the last green build |
| `src/validation/{include,src}/tryAqInterestRate{FixingTable,FutureFra,ObjectFra}.{h,cpp}` (renamed from `tryAqRate*`) + `validation.vcxproj` / `.filters` | **NOT BUILT** — see the category rename below |
| `src/AQ_API/source/aqCurveObject.{h,cpp}` | `aqRateFixingTable* → aqInterestRateFixingTable*` (the Python/SWIG binding for this — it lives in the Curve API file, a pre-existing filing quirk, not touched) |
| `src/GTEST/src/Test{AQObjCurveDiscountFactorsWithSpread,AQObjHedgeCurveDelta,AQObjSwapDelta,AQObjSwapDeltaDualBootstrap,AQObjSwapDeltaWithCurveEngine,AQObjSwapDeltaXCCY,AQObjSwapDeltaXCCY_CSA,AQObjSwapDeltaXCCY_ZAR,AQObjSwapDelta_ARR,AQObjSwapDelta_JPY,AQObjSwapFloatRate}.cpp` | call-site rename, same reason |
| 3 fixtures (`FIXING@1`, `EUR6M_FIXINGS@191`, `EUR3M_FIXINGS@74` `_tryAqRateFixingTableCreate_inputs.csv`) | renamed to `tryAqInterestRateFixingTableCreate_inputs.csv` |
| `CLAUDE.md` (both), `MIGRATION_PLAN.md`, `rebrand/tools/api_pair_check.py`, `docs/api_map.csv` (regenerated) | category list `Rate → InterestRate` |
| `STATUS.md`, `rebrand/STATUS.md` | doc updates (this pause) |
| `rebrand/phase2_validation_rename_MAP.csv` | untracked — the task-2.6 rename map; sibling of the committed `phase3_*_MAP.csv` files, `git add` it with the next commit |

Also carried forward: `src/validation/src/tryAqToolEchoDouble.cpp` was committed
in `22212bc5` but is **not in `projects/validation.vcxproj`**, so it never
compiles — that is why `aqToolEchoDouble` was dropped from `aqTool.cpp` (link
error). Add the `.cpp` to `validation.vcxproj` + `.filters` on the next
`validation` rebuild, then re-add the XLL wrapper.

### `Rate` category renamed → `InterestRate` (2026-09-11, decided + applied, NOT BUILT)

Nicholas: `aqRate*` read oddly; chose **`InterestRate`** over `IR` (keeps the
spelled-out, IntelliSense-groupable style of the other categories) and over
leaving it as `Rate` (too vague — reads as FX/hazard rate). Applied as a full
golden-source category rename, anchored to the 3 known function families only
(`FixingTable`, `FutureFra`, `ObjectFra` — never a blind `Rate*` sweep, which
would have hit unrelated tokens like local variables `aqRatesFixingDates` and
the pre-existing `tryAqCurveCompoundRate` / `…EuroDollarConvexityAdjustment`
`@brief` comments):

- `validation`: 3 files renamed (`tryAqRateFixingTable.{h,cpp}` →
  `tryAqInterestRateFixingTable.{h,cpp}`, similarly `FutureFra` / `ObjectFra`);
  13 wrapper functions renamed; `validation.vcxproj` + `.filters` updated.
- `AQ_XLL`: `aqRate.cpp → aqInterestRate.cpp`, 13 functions renamed to match;
  `AQ_XLL.vcxproj` + `.filters` updated. `.arg()` count re-verified: 13/13.
- `AQ_API`: `aqCurveObject.cpp/.h` — the 4 `aqRateFixingTable*` Python/SWIG
  bindings renamed (this is the one place `Rate` had already reached a
  consumer; filed under the Curve API source, not moved).
- `GTEST`: 11 files' `tryAqRateFixingTable*` call sites updated.
- 3 fixture files renamed (`resources/test/inputs/...`).
- Docs: both `CLAUDE.md`, `MIGRATION_PLAN.md` §2.2/porting-order prose,
  `rebrand/tools/api_pair_check.py` `CATEGORIES`, `docs/api_map.csv`
  regenerated (118 rows, `InterestRate: 4` in the public-surface breakdown).
- `src/AQ_API/source/swig_Python_wrap.cxx` still has the old `aqRateFixingTable*`
  names — expected, it is generated and was reverted to `HEAD`, not hand-edited;
  it picks up the new names on the next SWIG regen.
- **Verification (no build):** HARD GATE = 0; tree-wide grep for
  `tryAqRate(FixingTable|FutureFra|ObjectFra)` / `aqRate(FixingTable|FutureFra|ObjectFra)`
  across `src/` + `projects/` = 0 (outside the generated SWIG file).
- **Caught and fixed mid-rename:** the first sweep scoped its file list with
  `git grep -l`, which only searches **tracked** files — it silently skipped
  the then-untracked `aqInterestRate.cpp` (a `mv`, not `git mv`, since it had
  never been committed) and missed the two `FutureFra` functions entirely,
  because their real names (`tryAqRateFuturePriceToFraRate[FromConvAdj]`) don't
  contain the anchor substring "FutureFra" — only their *file* does. Both fixed
  with a second, targeted pass once found; the residual-grep check above is now
  post-fix and clean. Lesson for future category renames: run the identifier
  scan against the full symbol list (or `git grep --untracked`), not just
  filename-shaped anchors.

**Needs a full rebuild** (`validation` → `AQ_API` → `AQ_XLL` → `GTEST`) since it
touches `validation` — larger than the AQ_XLL-only rebuilds this pause point
otherwise needed.

### `Ois` folded into `Swap` as a product variant (2026-09-11, DOCS ONLY — no code changed yet)

Nicholas: `Ois` should not be its own category — it is a vanilla swap whose
floating leg compounds an overnight index instead of a term rate, not a
structurally distinct product the way `AssetSwap`/`CapFloor` are. Decision:
category count drops **22 → 21**; the golden-source form is
**`aqSwapOis<Function>`** (stateless; `aqSwapOisObject<Function>` if a stateful
form is ever needed), living in `aqSwap.cpp` — **not** a separate `aqOis.cpp` —
and in `src\validation\Swap\` alongside the vanilla-swap wrappers.

**Docs updated now** (`CLAUDE.md` both copies §5.1/§5.1a, `MIGRATION_PLAN.md`
§2.2/§2.7/D14, `rebrand/tools/api_pair_check.py` `CATEGORIES` — `Ois` removed
from the list). Also added a standing **"category-migration sequence"** note
(`AQ_LIB\CLAUDE.md` new §5.1a): validation (golden source, rename legacy
wrappers here first) → `GTEST` → `AQ_API` → `AQ_XLL`, in that order, every time
— codifying what this session has been doing ad hoc, per Nicholas's request to
make sure all four surfaces stay synchronized to the `validation` name and any
drift gets cleaned up as part of the migration, not deferred.

**No code renamed yet — this is preparation for when `Swap` (and Ois within
it) is next migrated.** What that migration will need to do, found while
scoping this:

- `validation` — **pre-existing** `tryAqOisPV` / `tryAqOisParRate` +
  `*LVBKeys` companions (4 wrappers, `tryAqOisPV.{h,cpp}` /
  `tryAqOisParRate.{h,cpp}`) predate the category scheme and must rename to
  `tryAqSwapOisPV` / `tryAqSwapOisParRate` (+ files).
- `AQ_API` — **already shipped** `aqOisPV` / `aqOisParRate` (Python/C#/Java/R
  via SWIG, `aqOisPV.{h,cpp}` / `aqOisParRate.{h,cpp}`) rename to
  `aqSwapOisPV` / `aqSwapOisParRate` (+ files + the 4 SWIG `.i`).
- `GTEST` — `TryAqTestTradeEUROISParRate.cpp` call sites + likely the file name.
- `AQ_XLL` — new functions `aqSwapOisPV`, `aqSwapOisParRate` (+ `LVBKeys`) in
  `aqSwap.cpp`, alongside `aqSwapPV`/`aqSwapParRate` when that category is built.
- `docs/api_map.csv` — **not regenerated this session on purpose.** Its 4
  `aqOisPV*`/`aqOisParRate*` rows still say category `Ois`, which is accurate
  until the rename above lands; regenerating now (after removing `Ois` from
  `CATEGORIES`) would mislabel them via the script's no-match fallback
  (observed: they fall into the `(lifecycle)` bucket in the console report —
  cosmetic, `api_pair_check` HARD GATE is unaffected at 0). Regenerate
  `api_map.csv` only once the rename lands.

### Bond / Tool / Date / Math XLL fill-in (2026-09-10) — BUILT GREEN, tests passing (Nicholas)

Nicholas: "migrate the remaining Bond, Tool, Date and Math functions." **92 new
`XLO_FUNC` worksheet functions** added to the four existing category files (no
new files, no vcxproj change). Whole-file param-vs-`.arg()` audit: **165/165
across all AQ_XLL `.cpp`, 0 mismatch.** `api_pair_check` HARD GATE = 0. Every
function routes through its identically-named `validation::tryAq*` wrapper.
(One more, `aqToolEchoDouble`, was written and then dropped after a link
failure — see the `aqTool.cpp` note below.)

- **`aqDate.cpp` +26** (now 30) — `YearFraction`, `BusinessDays`,
  `ShiftedSpotDate`, `IsWorkingDay`, `IsHoliday`, `IsWeekend`, `IsWeekday`,
  `FuturesContract`, `CentralBank`, `ECB`, `ECBSwapStart/End`,
  `NthECBMeetingDate`, `NthECBSwapStart/EndDate`, `NextECBMeetingDate`,
  `NextECBSwapStart/EndDate`, `IMMFromMonth`, `NthIMM`, `NthIMMFromStartDate`,
  `IMMCurrent`, `IMMNext`, `IMMPrevious`, `IMMNth`, `IsRegularSwapSchedule`.
  Includes `<tryAqDateCentralBank.h>` `<tryAqDateIMM.h>`
  `<tryAqDateIsRegularSwapSchedule.h>`; file-local `toInt`.
- **`aqBond.cpp` +17** (now 59) — bond curve: `Create`, `Display`, `Yield`,
  `NelsonSiegelCalibrate`, `NelsonSiegelYield`, `PolynomialCalibrate`,
  `PolynomialYield`, `SvenssonCalibrate`, `SvenssonYield`; bond object:
  `PriceFromBondCurve`, `YieldFromBondCurve`, `ForwardReinvestedCoupon`,
  `Quote`, `YieldFromObject`, `YieldOptimized`; stateless: `BondSchedule`,
  `BondScheduleLVBKeys`. Includes `<tryAqBondCurves.h>` `<tryAqBondSchedule.h>`
  `<AQObjUtilities.h>` (`etrading::getBond` for the two `BondPtr`-arg wrappers)
  `<BondCurves.h>` `<NelsonSiegelFitting.h>` `<PolynomialFitting.h>`; file-local
  `nssParamsFromRange` / `nssResultBlock`. Calibrate results are rendered as a
  (label, value) block. `NelsonSiegel` reads a 4-cell InitialGuess, `Svensson`
  a 6-cell one — **verify the Excel layout of InitialGuess / bounds in use.**
  `aqBondCurveCreate` passes `std::vector<validation::TableInfo>` to the
  wrapper's `JSONInfoBlockTuples` param (the two tuple typedefs are identical).
- **`aqTool.cpp` +11** (now 35) — `LVBCreate` (1-2 value columns) and the
  object-grid family: `ObjectGridCreate/Save/Load/Display/ObjectNames/ClearOne/
  ClearAll`, `ObjectMultiGridCreate` (up to 3 named grids) `/Display/SubNames`.
  Includes `<tryAqToolGrid.h>` `<tryAqToolMultiGrid.h>`; file-local
  `flexibleDataToExcel` (prepends the column-name header row to the grid
  `Display` matrix).
  - **`aqToolEchoDouble` dropped** — link failed (`LNK2001` on
    `validation::tryAqToolEchoDouble`): `src/validation/src/tryAqToolEchoDouble.cpp`
    exists and defines it but is **not listed in `projects/validation.vcxproj`**,
    so it is never compiled into the `validation` lib. Pre-existing gap
    (`api_pair_check` counts it off the header). Fix when `validation` is next
    rebuilt: add the `.cpp` to `validation.vcxproj` + `.filters`, then re-add
    `aqToolEchoDouble`. It is only a round-trip diagnostic — `aqToolEcho`
    already echoes any value — so nothing is lost meanwhile.
- **`aqMath.cpp` +38** (now 41) — Black-Scholes `ImpliedVol` + 6 Greeks
  (`DeltaForward`, `DeltaSpot`, `Gamma`, `Vega`, `Theta`, `Rho`);
  `CapletFloorletPrice` / `ImpliedVol`; 6 volatility conversions
  (`VolatilityTo{Normal,Lognormal,ShiftedLognormal}From{...}`);
  `LiborConvexityAdjustmentInArrears` / `LiborRateInArrears` +
  `...ForArbitraryFixingDate` pair; European IR swaption `Price` / `ImpliedVol`
  / `Delta` / `Gamma` / `Vega` / `Theta` / `CashAnnuity`; normal distribution
  (`Standard`, `StandardPDF`, `StandardInverse`, and the mean/variance
  `NormalDistribution` / `PDF` / `Inverse`); polynomial
  `Interpolation` / `Interpolations` / `PoynomialIntegration` /
  `PoynomialIntegrations` (**wrapper misspells "Poynomial" — matched verbatim
  per golden source**); `IntegrateUsingTerms` and `Integrate` (date form).
  Includes `<tryAqMathCapletFloorlet.h>` `<tryAqMathConvexity.h>`
  `<tryAqMathEuropeanIRSwaption.h>` `<tryAqMathIntegrate.h>`
  `<tryAqMathPolynomial.h>`; file-local `toStrOr` / `toUInt`. `CallOrPut` and
  `VolatilityType` marshalled via `etrading::toCallOrPutEnum` /
  `toVolatilityTypeEnum`.

### Rate category → new `aqInterestRate.cpp` (2026-09-10, uncommitted, NOT BUILT)

**13 `XLO_FUNC` worksheet functions** in a **new file** `src/AQ_XLL/src/aqInterestRate.cpp`,
added to `AQ_XLL.vcxproj` + `.filters`. Param-vs-`.arg()`: 13/13, 0 mismatch.

- Fixing table (a cached table of historical index fixings):
  `aqInterestRateFixingTableCreate` (currency / curve-tenor form), `…Display`,
  `…Value` (one date), `…Values` (a column of dates).
- Rate future ↔ FRA (stateless): `aqInterestRateFuturePriceToFraRate`
  (Hull-White convexity), `aqInterestRateFuturePriceToFraRateFromConvAdj`
  (explicit adjustment).
- FRA object: `aqInterestRateObjectFraCreate`, `…PV`, `…Display`, `…DisplayCashflows`,
  `…Rate`, `…ToFuturePrice`, `…ToFuturePriceFromConvAdj`.

File-local `toGregorian` / `toGregorianVector` (the fixing-table wrappers take
`boost::gregorian::date`, not `AQLDate` — converted via
`etrading::toGregorianDateFromAQLDate`). `aqInterestRateFixingTableDisplay` uses the
`toExcelMatrix(const etrading::VariantMatrix&)` overload added this session.
`aqInterestRateObjectFraCreate` follows the generator-create pattern (decorate + call +
`appendInstanceCounter`, no create-vs-modify guard — matches
`aqBondObjectCreateFromGenerator`). The three multi-overload wrappers
(`FixingTableCreate`, `FixingTableValues`) are each exposed once, via the
currency/tenor + plain-dates forms; the `LabelValueBlock` overloads are not
wired (add later if wanted).

### Generator category — NOT created (no wrappers exist)

Asked for alongside Rate. **`aqGenerator.cpp` was not created:** there are **zero
`tryAqGenerator*` validation wrappers** in `src/validation/` — the `Generator`
category (CLAUDE.md §5.1: introspection only — `aqGeneratorList` / `Describe` /
`Validate`) has never been built. Generator *construction* lives in the asset
categories (`aqBondGeneratorCreate`, and the pending `aqCurveGeneratorCreate` /
`aqSwapGeneratorCreate`), not here. Creating an empty file would add noise. To
populate this category, first add an `etrading` directory scan of
`resources/config/{BOND,CURVE,SWAP}_GENERATOR/` + a `tryAqGeneratorList` wrapper
(then `Describe` / `Validate`); the XLL/API surface follows. Flagged for
Nicholas's go-ahead (same item as under "Next steps on resume").

### Still deferred from Bond / Tool / Date / Math

Need enum-vector or curve-context marshalling not yet in `aqXllTools` — do with
the Curve category:
- Math `ForwardRate` / `ForwardRates` / `DiscountFactor` / `DiscountFactors`
  (both overload sets, 6 wrappers) — `InterpolationEnum`, `StateVariableEnum`,
  `DayCountEnum`, `BusinessDayAdjustmentEnum`, `CompoundingFrequencyEnum` +
  `forwardAdjustmentTable`.
- Math vector overloads: `BlackScholesPrices` / `ImpliedVols`,
  `CapletFloorletPrices` / `ImpliedVols`, `EuropeanIRSwaptionPrices` /
  `ImpliedVols` (`std::vector<CallOrPutEnum>` etc.) — the scalar forms cover the
  common use.
- Bond `tryAqBondObjectZSpread` (singular) — `aqBondObjectZSpread` already
  routes to the plural `tryAqBondObjectZSpreads`.
- Tool `tryAqToolLVBAdd` (in-place `STDStringMatrix&` mutator, not
  worksheet-shaped); `tryAqToolSwapScheduleTemplate` (Tool wrapper living in
  `tryAqSwapObjectSchedule.h` — do with Swap).
- `meUtilityLWODecorateNames`, `meUtilityMLIBSetUp` / `MLIBTearDown` — still no
  golden-source wrapper.

### Next steps on resume (priority order)

0. **Rebuild `validation` → `AQ_API` → `AQ_XLL` → `GTEST`** — the `Rate` →
   `InterestRate` rename touches `validation`, so this is a full rebuild, not
   just `AQ_XLL`. Nothing else in the current delta needs more than that.
1. ~~Decide the `Rate` category name~~ — **done**: `InterestRate` (Nicholas,
   2026-09-11), applied.
2. **Commit the small remaining delta** (see the PAUSE POINT table): the
   Bond/Tool/Date/Math XLL fill-in + `aqInterestRate.cpp` (+ vcxproj/filters) +
   `rebrand/phase2_validation_rename_MAP.csv` + the doc updates. Task 2.6, the
   Tool/Object port and Interpolation/PCA→Math are **already committed** in
   `22212bc5`. Before staging, check
   `git status --porcelain -- 'src/AQ_API/source/swig_*_wrap.*'` is empty — and
   note `swig_Python_wrap.cxx` slipped into `18328864`; decide whether to
   `git rm --cached` it.
2. **Confirm the 4 pre-existing test failures** (`TestDatesCentralBank`,
   `TestAQObjCurve`, `TestExampleObject`,
   `TestStructuredExceptionHandler.UNIT_IntegerDivideByZero`) also fail on a
   clean `3809f148` checkout — i.e. not caused by any of this work. If Nicholas's
   green run already showed them passing, this is moot.
3. **New `AQ_XLL` file `aqFuture.cpp`** — `Future` is a validation-only category
   (`tryAqFutureTicker*`); no XLL surface yet. Small, same pattern as the Tool
   port. (`Ois` no longer gets its own file — see the "`Ois` folded into `Swap`"
   section above; its 4 `tryAqOis*` wrappers rename to `tryAqSwapOis*` and land
   in `aqSwap.cpp` when `Swap` is migrated.)
4. **`meUtilityLWODecorateNames`** (Tool) + **`meUtilityMLIBSetUp` /
   `meUtilityMLIBTearDown`** — deferred from the Tool port because no
   golden-source wrapper exists. Decide: write conforming
   `tryAqTool{ObjectDecorateNames,Setup,TearDown}` wrappers, or drop these
   functions from the surface.
5. **`aqGeneratorList`** — the generator-name introspection function
   (CLAUDE.md §5.1) that does not exist in either library. New `etrading`
   directory scan of `resources/config/{BOND,CURVE,SWAP}_GENERATOR/` +
   `tryAqGeneratorList` wrapper + XLL/API. Nicholas said Batch-1 `aqObjectList`
   is "ok for now"; this is the fuller answer when wanted.
6. **Resume the main XLL port backlog** (the bigger Phase 4 work) — Curves →
   Swaps → products → Models, driven by
   `rebrand/xll_function_inventory.csv`. See "⇒ RESUME HERE — AQ_XLL port".
7. Lower priority, still open: the validation **recording rollout** (plan 4.9,
   see "⇒ RESUME HERE — recording rollout"); `AQ_API` SEH guard (plan 4.12);
   the 5 + 1 wrapper-name-drift advisories; the `tryAqObjSwapsDV01*` /
   credit-fixture golden-source cleanup (Phase 6 resources audit).

- Docs: `Vol` → `Volatility`; **Future** + **Ois** added as categories (now 22);
  D19 = the `validation` wrapper is the GOLDEN SOURCE for names. `api_pair_check.py`
  CATEGORIES updated.
- Rename map: `rebrand/phase2_validation_rename_MAP.csv` — 467 wrappers, 416
  renames, 0 collisions. Decisions baked in: `tryAqFuturesTicker*` →
  `tryAqFutureTicker*` (new `Future` cat), `tryAqOIS*` → `tryAqOis*` (new `Ois`
  cat), `tryAqObj{QuickLoad,QuickSave,Type,TypeAsString,CreateRange}` →
  `tryAqObject*`, `tryAqObjBondsCompoundYields` → `tryAqBondObjectCompoundYield`
  (wrapper singularised), `tryAqBondsYield` → `tryAqBondCurveYield`.
- **DONE this session (uncommitted, NOT BUILT):**
  - **Date batch** — `tryAqDates*` → `tryAqDate*` (29 fns) across validation +
    `AQ_API` (`aqDates*` → `aqDate*` method names + files) + `GTEST` (suites +
    `TestAqDates*.cpp` → `TestAqDate*.cpp`) + `AQ_XLL` call sites + SWIG `.i` +
    all 3 vcxproj/.filters. **No fixtures exist for Date.**
  - **Object lifecycle batch** — `tryAqObj{Load,Save,Exists,ClearCache,
    CreateRange,Delete,DeleteAll,List,LoadFromString,LoadAndReturnTupleResults,
    QuickLoad,QuickSave,Type,TypeAsString}` → `tryAqObject*` (14 fns). Container
    files `tryAqObjects.{h,cpp}` → `tryAqObject.{h,cpp}`, AQ_API `aqObjects.*` →
    `aqObject.*`, ~40 GTEST includers, SWIG `.i`, vcxproj. `tryAqObjToolsGrid*`
    deliberately left (that is the Tool batch). **No fixtures.**
  - Reverted twice: `swig_Python_wrap.cxx` (generated — regenerated in Phase 5)
    and `rebrand/phase3_Bsimple_MAP.csv` (historical) were caught by an
    unscoped sweep. Sweeps are now scoped to `src/` + `projects/`, excluding
    `swig_*_wrap.*`.
- **FULL MAP APPLIED (uncommitted, NOT BUILT).** All 416 renames from
  `phase2_validation_rename_MAP.csv` applied in one pass (the user said "go"):
  - `validation` — 467 wrappers renamed; ~240 `.h/.cpp` files `git mv`'d
    (exact-stem + container headers like `tryAqObjBonds.h → tryAqBondObject.h`,
    `tryAqCurvesDiscountFactor.h → tryAqCurveDiscountFactor.h`, …); all `#include`
    updated.
  - `AQ_API` — `aq*` binding method names + source files + SWIG `.i` `%include`.
  - `GTEST` — `tryAq*` calls + `_inputs.csv` path-string literals + `TestAq*.cpp`
    file names where the stem was a function name.
  - `AQ_XLL` — `validation::tryAq…` call sites.
  - `resources/test` — **~3,560 fixture files** `git mv`'d
    (`<oldfn>_{inputs,outputs}*.csv → <newfn>_…`). Content untouched.
  - 3 × `.vcxproj`/`.filters`. `docs/api_map.csv` regenerated.
  - `swig_*_wrap.*` reverted twice (generated — Phase 5).
- **Verification (what I could check without a build):**
  - `api_pair_check.py` **HARD GATE = 0** (every public fn routes through validation).
  - Old function names in code: **0**. Old validation header files: **0**.
  - 5 wrapper-name-drift advisories — all pre-existing (the deferred set), not new.
  - **24 fixtures keep old names** — `tryAqObjSwapsDV01*`, `tryAqObjSwaps_{IRS,XCCY}_DV01*`,
    `tryAqObjCreditDefaultSwap{AnnuityFromHazardRate,PVIntegration,PVMonteCarlo,PVMonteCarloSobol}*`.
    Their GTEST path-strings are ALSO still old (hardcoded literals that aren't a
    function name), so file+string are self-consistent → tests still read them.
    These are drift/orphan fixtures for the Phase 6 resources audit.
  - A handful of GTEST `.csv` string literals reference fixtures that exist under
    **neither** old nor new name (`tryAqSwapObjectPV_inputs.csv`,
    `tryAqSwapObjectDeltaLadder_spot_4Y_inputs.csv`, the VariableNotional set) —
    **pre-existing** (0 files under the old name too), not caused by this rename.
- **GATE — build `validation` + `AQ_API` + `GTEST` and run GTest.** Output must
  be numerically identical to the pre-rename baseline. Any fixture-name failure
  will name the exact file; report it and I fix that one. This is the
  build-between-batches checkpoint (done once here, since the map went in as one
  batch).
- Still to do after green: the two new-category AQ_XLL files `aqFuture.cpp` /
  `aqOIS.cpp` (Future/Ois currently validation-only); the `meUtility*` → `Tool`
  port from `.APPLES`.

### AQ_XLL Tool / Object port (2026-09-09, in progress, uncommitted, NOT BUILT)

Third explicit task — expose the `Tool` + `Object`-lifecycle surface in the Excel
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
| `meLWOExists` | `tryAqObjectExists` | `aqObjectExists` | ✅ already |
| `meLWOLoad` | `tryAqObjectLoad` | `aqObjectLoad` | ✅ already |
| `meLWOSave` | `tryAqObjectSave` | `aqObjectSave` | ✅ already |
| `meLWOLoadFromString` | `tryAqObjectLoadFromString` | `aqObjectLoadFromString` | aqObject.cpp |
| `meLWOQuickLoad` | `tryAqObjectQuickLoad` | `aqObjectQuickLoad` | aqObject.cpp |
| `meLWOQuickSave` | `tryAqObjectQuickSave` | `aqObjectQuickSave` | aqObject.cpp |
| `meUtilityClearLWOCache` | `tryAqObjectClearCache` | `aqObjectClearCache` | aqObject.cpp |
| `meUtilityResize` | (n/a) | `aqToolResize` | ✅ already |
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

No validation wrapper in AQ_LIB (defer — needs a wrapper first, or drop):
`meUtilityLWODecorateNames`, `meUtilityThreadsEnableLocks`, `meUtilityMLIBTearDown`.
`meUtilityMLIBSetUp/TearDown` — `MLIB` is a legacy prefix; the setup half maps to
`tryAqToolSetup`, the teardown half has no wrapper. Also unmapped in AQ_XLL so far:
the `tryAqToolObjectGrid*` / `tryAqToolObjectMultiGrid*` / `tryAqToolSwapScheduleTemplate`
wrappers (these come from `.APPLES` files other than `meUtilities.cpp`).

Batches (build between each, on Nicholas's side):

1. **Object lifecycle → `aqObject.cpp`** — `aqObjectType`, `aqObjectList`,
   `aqObjectDelete`, `aqObjectDeleteAll`, `aqObjectLoadFromString`,
   `aqObjectQuickLoad`, `aqObjectQuickSave`, `aqObjectClearCache` (8 fns).
   **DONE (uncommitted, NOT BUILT).** Added `#include <boost/format.hpp>` +
   `<tryAqToolSetup.h>`. Every fn has `.arg()` count == parameter count.
   **BUILT GREEN in Excel (Nicholas, 2026-09-09).**
2. **Tool setup/version/parallel + record/replay → `aqTool.cpp`** —
   `aqToolVersion`, `aqToolClearEntityPool`, `aqToolLoadStaticData`,
   `aqToolLoadCalendarFile`, `aqToolLoadConfigurationFiles`,
   `aqToolParallelModeEnable`, `aqToolParallelModeStatus`, `aqToolRecord`,
   `aqToolReplay` (9 fns). All string / bool / int in, string out.
   **DONE — BUILT GREEN with tests passing (Nicholas, 2026-09-09).** Added
   `#include <cstdio>` `<ctime>` + `<tryAqToolSetup.h>` `<tryAqToolRecord.h>`
   `<tryAqToolReplay.h>`; file-local `toIntOr()` for the optional
   `StartIndex`/`MaxIndex` args.
   - `meUtilityMLIBSetUp` / `meUtilityMLIBTearDown` **dropped** — no
     golden-source wrapper: the only setup helpers in `tryAqToolSetup.h` are
     `trySetupAQL(irProps, calendar, cbCalendar)` / `tryTearDownAQL()`, which
     do not conform to the `tryAq<Category>` scheme. Needs a conforming
     `tryAqToolSetup` / `tryAqToolTearDown` wrapper defined first.
3. **Tool LVB family + data reshaping + maths → `aqTool.cpp`** (13 fns).
   **DONE — BUILT GREEN with tests passing (Nicholas, 2026-09-09).**
   - LVB: `aqToolLVBFromKeysValues`, `aqToolLVB`, `aqToolLVBGroup` (10 optional
     block args), `aqToolLVBFromMultipleKeysValues` (5 keys/values/prefix
     triples = 15 args), `aqToolLVBFromKeysAndMultipleValues` (6 args).
   - Reshaping: `aqToolClean` (5), `aqToolAppend` (11), `aqToolDataFilter` (2),
     `aqToolValuationSettingsDisplay` (1).
   - Maths: `aqToolInterpolation` (5), `aqToolPCA` (4), `aqToolTermsToDates` (2),
     `aqToolDatesToTerms` (2).
   - **New `aqXllTools` helpers** (added to `.h` + `.cpp`): `toAQLStringVector`,
     `toDoubleMatrix`, `toStandardStringMatrix`, `toVariantMatrix` (row-major,
     type-preserving, error cell → its text, blank → EMPTY Variant),
     `toVariantVector`, and a `toExcelMatrix( const etrading::VariantMatrix& )`
     overload (native Excel types out). File-local `toDoubleOr` / `addLvbBlock`
     / `addStringColumn` in `aqTool.cpp`.
   - **ValuationSettings orientation:** input read as-is (rows of key/value
     pairs) — matches the current `TestValuationSettings` GTEST contract
     (`{{ "CURVECOLLECTION", curveUSD3M }}`). The old XLL+ add-in force-
     transposed here but its own comment flagged that as a bug ("needs
     fixing!!!"); not carried.
   - `.arg()` count == parameter count verified for all 26 functions in
     `aqTool.cpp`.
4. `meUtilityLWODecorateNames` still deferred — no golden-source wrapper.

### Interpolation + PCA moved Tool → Math (2026-09-09) — BUILT GREEN, tests passing (Nicholas)

Nicholas: interpolation and PCA are `Math` building blocks, not `Tool`. Renamed
the golden-source wrappers and every surface that follows them:

- `validation`: `tryAqToolInterpolation` → **`tryAqMathInterpolation`**,
  `tryAqToolPCA` → **`tryAqMathPCA`**; files `tryAqTool{Interpolation,PCA}.{h,cpp}`
  → `tryAqMath{Interpolation,PCA}.{h,cpp}`; `projects/validation.vcxproj{,.filters}`.
- `AQ_API`: `aqToolInterpolation` / `aqToolPCA` → `aqMathInterpolation` /
  `aqMathPCA`; source files renamed; 4 SWIG `.i` (`#include` + `%include`);
  `projects/AQ_API.vcxproj{,.filters}`.
- `GTEST`: `TestUtilitiesInterpolation.cpp` call sites (12) + include. (Bespoke
  unit test — suite/case names left as-is; not a recording-generated test, and
  there are **no fixtures** for either function, so nothing else to move.)
- `AQ_XLL`: the two `XLO_FUNC` blocks moved out of `aqTool.cpp` into
  `aqMath.cpp` as `aqMathInterpolation` / `aqMathPCA`; `toDoubleOr` helper moved
  with them; `aqMath.cpp` gained a file-local `toIntOr` and `using namespace
  aq_xll;`.
- `docs/api_map.csv` regenerated — 118 rows, `aqMath{Interpolation,PCA}` now
  under `Math`. HARD GATE = 0. No `swig_*_wrap` touched.

### Open question raised — a generator-name listing function

Nicholas asked whether the old `.APPLES` add-in has a function to list the
generator names from the config folders (so users don't memorise them).
**Answer: no.** `.APPLES\...\MLIBQ_ADDIN` only has, per asset class,
`*GeneratorCreate` / `*GeneratorDisplay` / `*GeneratorModify` and
`*CreateFromGenerator` — `*GeneratorDisplay` dumps one *named* generator; nothing
enumerates the `resources/config/{BOND,CURVE,SWAP}_GENERATOR/*.JSON` names.
CLAUDE.md §5.1 already earmarks this as intended-but-unbuilt: the `Generator`
category = introspection only (`aqGeneratorList` / `Describe` / `Validate`).
Building it = a new `etrading` directory-scan + a `tryAqGeneratorList` wrapper +
XLL/API surface. Not started; flagged for Nicholas's go-ahead.

### Fixture-rename recovery + final cleanup (2026-09-09, uncommitted, NOT BUILT)

After Nicholas's build+GTest run flagged ~20 failures, root cause was a
fixture/​code-string boundary mismatch: the fixture sweep matched only names
**starting** with a function name (`b.startswith(old)`) while the code-string
rewrite used a **bounded-token** regex that also hit infixes like
`"..._tryAqObjSwaps..._inputs.csv"` — so prefixed fixture FILES kept old names
while their string LITERALS moved to new → "error opening file".

- **Reverted all `resources/test/` fixture moves to HEAD** (needed all three:
  `git checkout HEAD -- resources/test/`, `git reset HEAD -- resources/test/`,
  `git clean -fdq resources/test/inputs/` — `git add -A` had staged the renames
  as Add/Delete pairs).
- **Fixture rename take-3** — boundary `(?<![A-Za-z0-9])<old>(?![A-Za-z0-9])`
  (allows a leading `_`), driven by `pairs + apair` (both `tryAq*` and `aq*`
  names), skipping any result still containing a stale fragment:
  **7,551 renamed, 0 skipped, 0 `tryAqObj…` left**.
- **Relaxed code pass** — 69 GTEST/validation files: rewrote `_`-prefixed
  regex/scan/recording string fragments the first `(?<![A-Za-z0-9_])` pass had
  missed (e.g. `"_tryAqBondObjectPriceFromDiscountMargin_inputs.csv"` suffix
  constants).
- Verified `BOND1_tryAqBondObjectPrice_inputs.csv` and
  `EUR_FIXEDSCHEDULE_tryAqSwapObjectScheduleCreateBespoke_inputs.csv` now exist,
  matching the rewritten code strings.

**Residual function fixed (was the only real code-level miss):**
`tryAqObjCurvesDiscountFactorsForwardStartingFromTenor` (function *definition* at
`src/validation/src/tryAqCurveDiscountFactor.cpp:624`, missing from the map —
the map had the stateless `…Tenor` and the object-form plural `…Tenors` but not
the object-form singular) → **`tryAqCurveObjectDiscountFactorsForwardStartingFromTenor`**,
plus its `CreateDataFile`/`generatorFunction` labels. 0 fixtures / 0 GTEST readers
depended on the old name.

**Dead recording-label cleanup** (WRITE-side `CreateDataFile` / `decorateCurvename`
labels only — 0 committed fixtures, 0 GTEST readers, provenance not on the
test-read path; changed to golden names for internal consistency):
`tryAqObjCurvesCalibrate{Basis,OIS,Swap,FXForwards}` →
`tryAqCurveObjectCreate{Basis,OIS,Swap,FXForwards}` in the four
`tryAqCurveObjectCreate*.cpp`; `…DiscountFactorsForwardStartingFromTenor` labels
in `tryAqCurveObjectDiscountFactor.cpp` → `…Tenors` (match enclosing fn);
`tryAqObjSwapsDeltaLadderHorizontal` label in `tryAqSwapObjectDelta.cpp:395` →
`tryAqSwapObjectDeltaLadderHorizontal` (real fixtures already use the new name).

**Left as-is on purpose — `tryAqObjSwapsDV01*`** (18 fixtures on disk +
`TestAQObjSwapDelta{,XCCY_CSA,_JPY}.cpp` hardcoded path literals). File name and
GTEST read-string are mutually self-consistent, so the tests pass. Renaming would
mean an 18-fixture + 4-file lockstep move for zero functional gain — this is a
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
  headers — advisory comment cleanup only, no code effect.
- No `swig_*_wrap.*` staged. `docs/api_map.csv` regenerated (118 public / 467
  wrappers).

**4 failures assessed as pre-existing (NOT caused by the rename) — Nicholas to
confirm they also fail on clean HEAD:**
- `TestDatesCentralBank.UNIT_AreThereEnoughDates` — `totalDaysToLast: -1704`;
  stale ECB meeting-date data in `AQLMathCentralBank::meetingSchedule("ECB")`,
  no `tryAq*` involved.
- `TestAQObjCurve` + `TestExampleObject` — write to `.../resource/test/...`
  (singular); pre-existing FolderConfig path bug.
- `TestStructuredExceptionHandler.UNIT_IntegerDivideByZero` — SEH "throws
  nothing"; build/optimizer issue, no `tryAq*`.

**GATE — Nicholas: rebuild `validation` + `AQ_API` + `GTEST`, run GTest.**
Expect the ~15 fixture-mismatch failures gone; the 4 above expected to remain
(please confirm on clean HEAD). Then commit this batch.

## ⇒ RESUME HERE — meLWOBond port (2026-09-08)

Porting `meLWOBond*` from `.APPLES\...\MLIBQ_ADDIN\src\meLWO.cpp` into
`src/AQ_XLL/src/aqBonds.cpp`, renamed `meLWOBond* → aqObjBonds*` (plural
`Bonds`, the locked category — NOT the literal `aqObjBond*` first asked for;
plural is what pairs with the `tryAqObjBonds*` wrappers and the
`api_pair_check` gate). Two-part plan agreed with Nicholas.

**Part (a) — DONE (uncommitted, NOT BUILT):** 11 functions added to
`aqBonds.cpp` — `aqObjBondsDisplaySchedule`, `aqObjBondsDisplayCashflows`,
`aqObjBondsDirtyPrice`, `aqObjBondsAccruedInterestDays`,
`aqObjBondsCleanPrice`, `aqObjBondsAccruedInterest`, `aqObjBondsYield`,
`aqObjBondsCompoundYield`, `aqObjBondsPrice`, `aqObjBondsDV01`,
`aqObjBondsDV01Numerical`. (`meLWOBondDisplay`/`meLWOBondCreate` were already
ported in `e5c57046` as `aqObjBondsDisplay`/`aqObjBondsCreate`.)
New marshalling helpers in `aqXllTools.{h,cpp}`: `toDoubleVector`,
`toExcelDoubleColumn`, `toExcelIntColumn`.
`aqObjBondsCompoundYield` calls the plural-spelled wrapper
`tryAqObjBondsCompoundYields` (pre-existing wrapper quirk).
No vcxproj change — `aqBonds.cpp` / `aqXllTools.cpp` already listed.
**Next: build all configs; then GoogleTest / Excel-verify.**

**Part (b) — batch b1 DONE (uncommitted, NOT BUILT).** 24 functions added to
`aqBonds.cpp`: `aqObjBondsModifiedDuration`, `aqObjBondsOisSpread`,
`aqObjBondsLastCouponDate`, `aqObjBondsPriceFromDirtyToClean`,
`aqObjBondsPriceFromCleanToDirty`, `aqObjBondsZSpreadFromRates`,
`aqObjBondsZSpread` (→ plural wrapper `tryAqObjBondsZSpreads`),
`aqObjBondsForwardPrice`, `aqObjBondsRepoRate`, `aqObjBondsRepoRateFromFuture`,
`aqObjBondsFuturePrice`, `aqObjBondsConversionFactor`, `aqObjBondsGrossBasis`,
`aqObjBondsNetBasis`, `aqObjBondsCheapestToDeliver`,
`aqObjBondsCheapestToDeliverByNetBasis`, `aqObjBondsFRNPriceFromDiscountMargin`,
`aqObjBondsFRNPriceFromYield`, `aqObjBondsFRNYieldFromPrice`,
`aqObjBondsFRNDiscountMarginFromPrice`, `aqObjBondsPriceFromCreditModel`,
`aqObjBondsBPVPerTick`, plus the two stateless `aqBondsAverageYield` /
`aqBondsYieldFromFuturePrice` (B4 decision: `aqBonds*` in `aqBonds.cpp`; they
call `validation::tryAqToolsBond*`, so `api_pair_check` will flag an
`aqBonds*`↔`tryAqToolsBond*` drift — deferred wrapper rename, same class as the
existing 5 advisories). New helper `toStringVector` in `aqXllTools.{h,cpp}`.
Whole-file param-vs-`.arg()` audit: 37/37 match (0 mismatches).
**Next: build all configs; verify in Excel.**

**Part (b) — batch b2a DONE (uncommitted, NOT BUILT).** 3 create functions added
to `aqBonds.cpp`, each with the `aqObjBondsCreate` create-vs-modify guard
(`decorateWithExcelLocation` + `allowAQObjUpdates` + `appendInstanceCounter`):
- `aqObjBondsCreateFromLVB` — keeps the legacy `VerticalLVBKeys` arg; new
  `toLabelValueBlock(obj, keysAreVertical)` overload in `aqXllTools` transposes
  the string matrix when FALSE.
- `aqObjBondsCreateFromGenerator` (legacy `meLWOBondCreateFromGeneratorLVB` — the
  `LVB` suffix is dropped to match `tryAqObjBondsCreateFromGenerator`).
- `aqObjBondsCreateAUDNotionalBond` — 10 Excel args, wrapper takes 9 (the 10th,
  `AllowUpdates`, drives the create guard only).
Whole-file audit: 40/40 param-vs-`.arg()` match. **Next: build; verify.**

**Part (b) — batch b2b DONE (uncommitted, NOT BUILT).** The 2 bond-generator
functions added to `aqBonds.cpp`:
- `aqObjBondsGeneratorCreate` — new `aqXllTools::toTableInfo(obj)` reads a range
  column-major into `tuple<COL_1.. names, Variant::getContainedTypeInfo types,
  column-major VariantMatrix of string Variants>` (the `validation::TableInfo`
  shape); `key2`/`value2` optional. No create-vs-modify guard (matches the
  source), just decorate + call + `appendInstanceCounter`.
- `aqObjBondsGeneratorDisplay` — `validation::tryAqObjBondsGeneratorDisplay`
  returns `etrading::VariantMatrix`; rendered via
  `etrading::toAQLStringMatrixFromVariantMatrix(result, false)` into the new
  `toExcelMatrix(const AQLStringMatrix&)` overload (numeric-aware, same cell
  path as `toExcelMatrix(AnyTypeMatrix)`). **Display orientation (transpose
  flag) is a guess — verify visually in Excel.**

**meLWO.cpp bond port — COMPLETE** (parts a + b1 + b2a + b2b): 42 XLO_FUNC in
`aqBonds.cpp` (2 pre-existing + 40 ported this session), whole-file
param-vs-`.arg()` audit 42/42. Remaining `meLWO.cpp`
items are the 5 `meLWOFixingTable*` (Rates category, a later `aqRates.cpp`) and
the 2 in the separate `meLWOBonds.cpp`.

**AQ_XLL comment / include cleanup (uncommitted, NOT BUILT).** Per Nicholas:
all 8 `src/AQ_XLL/{src,include}` files scrubbed of `.APPLES` paths, legacy
function/type names (`meLWO*`, `meUtility*`, `CXlOper`, `XllPlus*`,
`getDataByColumnAndDescription`, `populateExcelArrayWith*`, `MLIB_START*`,
`checkIfStaticDataLoaded`, `appendExcelLocation`, ...) and the word "legacy" in
comments; `@brief` markers removed; every `#include "x"` → `#include <x>` (all
resolve through the project `/I` dirs, so the form change is safe). No code
behaviour change. Grep for the legacy tokens over `src/AQ_XLL` now returns clean.

**Also fixed this session:** `aqObjSave` (`aqObj.cpp`) had 3 params / 4 `.arg()`
from commit `a8b50b47` — that was the "corrupt XLL / unhandled xloil::Exception"
at load, NOT the LTCG theory and NOT the bond port. Added the 4th param
`showArrayOutputs` + implemented the TRUE → 3-row column behaviour.

**Out of scope for `aqBonds.cpp`:** the 5 `meLWOFixingTable*` in `meLWO.cpp` are
the `Rates` category (`tryAqObjRatesFixingTable*` wrappers) — a later `aqRates.cpp`.
`meLWOBondQuote` / `meLWOBondForwardReinvestedCoupon` live in `meLWOBonds.cpp`,
not `meLWO.cpp`.

Two workstreams are open in parallel:
- **A. AQ_XLL xlOil port (active this session)** — see "⇒ RESUME HERE — AQ_XLL
  port" immediately below.
- **B. validation recording rollout (plan 4.9)** — see "⇒ RESUME HERE —
  recording rollout" further down. Untouched this session.

---

## ⇒ RESUME HERE — AQ_XLL port (2026-09-07)

**What works in Excel now (Nicholas-verified):**
- `aqToolsInitialize` — loads config, prints the resolved `Calendar.csv` path.
  Library auto-inits in the `AlgoQuantLib` add-in constructor (xlAutoOpen);
  `AQ_INITIALIZE` macro is the per-function lazy guard.
- `aqDatesFromTenor` / `aqDatesFromYearFraction` — calendars usable.
- `aqObjBondsCreate` / `aqObjBondsDisplay` (new `aqBonds.cpp`).
- `aqObjExists` / `aqObjLoad` / `aqObjSave` (new `aqObj.cpp`).
- `aqToolsResize` — positional clip/pad reshape (respects source dims;
  pad blanks / truncate). Row-major reflow bug fixed.
- Numeric-string cells in `Display`/matrix output now returned as real numbers
  (formattable), mirroring the legacy `canStringConvertToNumber` path.

**OPEN — blocking the multi-row `aqObjSave` output:**
`AQ_IS_ARRAY_OUTPUT` (macro → `aq_xll::isArrayOutput()` → `callerRangeSize()`
via raw `xlfCaller`, handles SRef + Ref) returns false even under
Ctrl+Shift+Enter on Nicholas's Excel, so `aqObjSave` returns the scalar and
Excel repeats it across the selection. Likely cause: modern Excel intercepts
CSE and never reports the array range to the add-in, so Enter-vs-CSE cannot be
distinguished at all.
- **Mitigation already coded (uncommitted):** `aqObjSave` 4th optional arg
  `AsArray` — TRUE forces the 3-row column, FALSE forces the message, omitted =
  auto. Plus `aqToolsCallerInfo()` diagnostic returning `"<r> x <c> (array=<0|1>)"`.
- **Next step:** build; run `=aqToolsCallerInfo()` plain vs CSE-over-3-cells.
  - both `1 x 1` → auto-detect is impossible on this Excel; keep `AsArray`,
    strip the auto-detect from other multi-output funcs (don't fake it).
  - CSE shows `3 x 1` → detection works, dig into why `aqObjSave` still
    scalars (array build path itself is proven — bond display uses the same
    `ExcelArrayBuilder` + `returnValue` and works).

**Then continue the port in the agreed order** — Dates → Tools → Curves →
Swaps → Products → Models. `rebrand/xll_function_inventory.csv` (653 rows) is
the driver; decision columns (`proposed_new_category`, `keep_drop_merge`,
new name) still need filling per file, starting with `meDates` (36 fns).
`~460` me*/meLWO* functions in the etrading filter to port; `LAXL.cpp` (161
`mir*`) deprecate-or-`aql*`; `msc*`/client (~29) delete.

**Carry forward (AQ_XLL specifics):**
- `AQ_API` has NO auto-init — bindings callers must call `setUpAQL()` /
  `setupAQL()` / `initAQL()` (all → `validation::trySetupAQL`). Only Python
  verified. `AQ_API_START` is still a bare `try{}` (plan 4.12).
- `aqObjSave` deliberately no longer upper-cases the file path (legacy did;
  breaks case-sensitive FS / Linux). Name + type still upper-cased.
- Naming calls made this session, matching the `tryAq*` wrappers + pairing
  check, NOT the literal ask: `Bonds` (plural, not `Bond`), `aqObj*` (not
  `aqObject*`). Nicholas has accepted these so far.
- New marshalling helpers in `aqXllTools.{h,cpp}`: `toBool`,
  `toAQLStringMatrix`, `toLabelValueBlock`, `toExcelMatrix` (numeric-aware),
  `toExcelColumn` (no pad — an N×1 must stay N×1), `reshapeToSize`,
  `decorateWithExcelLocation`, `callerRangeSize`, `isArrayOutput`.
- Dead XLL+ include/lib paths still in `projects/AQ_XLL.vcxproj` — remove when
  next touching it (plan carry-over).

---

## ⇒ RESUME HERE — recording rollout

**1. Continue the validation recording rollout (plan 4.9).** Pass (a) is
done: 32 conversions in 6 files. What remains, in the order I would do it:

- **Fix the misleading comment in `RecordMacros.h`** (cheap, do it first). Its
  worked example shows a `decorateFilename(...)` block being replaced by plain
  `AQ_RECORD_INPUTS`. That is WRONG — `AQ_RECORD_INPUTS` does not decorate, it
  always yields `<function>_inputs`, so following the comment silently renames
  the fixture file and breaks the test at run time with a green build.
- **99 decorated blocks** → `AQ_RECORD_DECORATED_INPUTS`. First confirm how that
  macro maps prefix/suffix onto `decorateFilename(name, prefix)`.
- **81 variable-filename blocks** — need reading individually.
- **18 key≠parameter-name recordings** (pass b) — listed in plan 4.9. Converting
  these mechanically WOULD break fixtures; rename the parameter to match the key
  instead, then re-run `fixture_key_check.py`.
- **Functions with no recording at all** (pass c) — largest, needs new fixtures.

**⚠ Two hard rules for the converter tool, learned the expensive way:**
1. Verify the replaced span is exactly the `if (recordEnabled()) { ... }` block
   and nothing more. One bad boundary ate a live statement
   (`double result = ...` became `double rAQ_RECORD_OUTPUTS( ... );`).
2. Verify the file includes `RecordMacros.h` before rewriting it. Three files
   had only hand-rolled recording and so never included it.

**2. Then the other open Phase 4 items:** 4.12 (SEH for `AQ_API` —
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
`aqObjSave` (multi-row output still blocked — see RESUME HERE — AQ_XLL port).

Convention (plan 4.11): every file is `aq<Category>.{cpp,h}`; the single
exception is `aqXllTools.{h,cpp}`, the shared Excel-side helper library
(marshalling + AQObj handle decoration). `aqTools.cpp` is the Tools *category*;
`aqObj.cpp` holds the `aqObj<Lifecycle>` functions (no category word).

`XllPlusTips*.cpp` were NOT copied — 4,629 lines built on XLL+ types. Behaviour
was ported instead. The AQObj handle logic (instance counter, cell address,
create-vs-modify guard) is in `aqXllTools`; the core framework was already in
`src/etrading`.

SEH: `TestStructuredExceptionHandler` gives it automated coverage for the first
time. **If the translator ever stops working those tests do not fail, they
CRASH the runner** — a vanished test run is the signal.

## Phase 3 — complete

`src/` and `projects/` contain zero `Mizuho`, `MLIB`, `me*`, `mir*`,
`LA*`/`MA*`/`MB*`, `validation_api`, `XllPlus`, person names, or copyright
banners. `resources/` reduced to `config`, `test`, `setup` (3,522 files deleted);
legacy-named paths there fell from 1,310 to 126.

## ⚠ Carry these forward — they outrank routine cleanup

1. **Two undeclared third-party components in `src/math`**, both found by
   accident: `AQLSobol.cpp` (Peter Jaeckel — notice has a PRESERVATION CLAUSE)
   and `AQLNl2sol.cpp` (NL2SOL, 6,541 lines — Dennis/Welsch/Gay/Peters,
   NSF-funded, ACM TOMS). **Excluded by name from every sweep. Never strip their
   attribution.** Neither is in the CLAUDE.md §3.3 dependency table. The Phase 6
   provenance scan is REQUIRED BEFORE SALE.
2. **Recorded fixture keys are stringified C++ parameter names.** Renaming a
   validation parameter breaks tests silently, build still green.
   `fixture_key_check.py` guards it.
3. **Pre-existing bug:** `tryAqObjRatesFixingTable.cpp` writes
   `file.write("fixingValues", fixingDates)` and
   `file.write("fixingDates", fixingValues)` — the keys are swapped. Fix code
   and rebase those fixtures TOGETHER, never separately.
4. **`git add -A` stages the generated `swig_*_wrap.*` files.** Restore with
   `git checkout HEAD -- 'src/AQ_API/source/swig_*_wrap.*'`; never commit them.
5. **Exact-match auditing under-reports** — five times a "clean" result was
   defeated by a spelling variant. Re-scan loosely after any sweep reports zero.
6. **No test exercises `tryAqToolsReplay`** — untested public API, Phase 5 gap.

---

## Done this run (newest first)

| commit | what |
|---|---|
| _(uncommitted)_ | **Step 11 — category naming scheme + AQ_XLL rebrand.** Categories now SINGULAR; handle marker moved from `aqObj<Category>` prefix to `aq<Category>Object<Fn>`; named sub-objects skip `Object` (`aqBondCurveYield`); lifecycle `aqObject<Lifecycle>`; `Vols`→`Volatility`. Docs: `CLAUDE.md` (both), `MIGRATION_PLAN.md` §2.2/§2.5/§2.6/D14/D15/D16/D18/Phase 3.2/3.5/4.7/4.11, `rebrand/STATUS.md`, `rebrand/tools/api_pair_check.py` (CATEGORIES + `category()`). **AQ_XLL code:** `git mv` `aqBonds/aqDates/aqObj/aqTools.cpp` → `aqBond/aqDate/aqObject/aqTool.cpp`; 53 functions renamed (42 bond + 4 date + 3 object + 4 tool); comments + `AQ_XLL.vcxproj`/`.filters` updated. Param/`.arg()` audit `aqBond.cpp` 42/42. `aqMath.cpp` / `aqMain.cpp` / `aqXllTools.{h,cpp}` unchanged names (Math already singular; XllTools is a utility, not a category). **`validation::tryAqObj…` call sites in AQ_XLL still use the OLD wrapper names** — temporary desync until MIGRATION_PLAN 2.6 renames `validation` / `AQ_API` / `GTEST` test names / fixtures. **NOT BUILT.** |
| _(uncommitted)_ | **AQ_XLL fails to load in Excel — ROOT CAUSE FOUND + FIXED.** `xlAutoOpen` threw `xloil::Exception<std::runtime_error>` ("unhandled C++ exception in Excel.EXE"). Cause: `src/AQ_XLL/src/aqObj.cpp` `aqObjSave` had **3 parameters but 4 `.arg()` descriptions** (the `ShowArrayOutputs` arg text was added in commit `a8b50b47` "AqObjSave update" without the matching 4th parameter). xlOil's `.arg(i>=nParams)` throws "Too many args for function" at registration. Introduced by `a8b50b47`, which post-dates the last Excel-verified commit `e5c57046`, so it was never caught. Fix: added the 4th param `showArrayOutputs` and implemented the documented TRUE → 3-row `{result, filePath, name}` column / else → name behaviour (uses existing `toBool` + `toExcelColumn`). **Not related to LTCG or to the tranche-(a) bond port** (all 11 new funcs audited: param/arg counts match). The earlier `AQ_XLL.vcxproj` LTCG edit was reverted — project settings unchanged from HEAD. `targets/64/{Release,Debug,ReleaseProfiler}/AQ_XLL` were deleted (build output only). **Next: rebuild AQ_XLL, verify it loads, then verify tranche (a).** |
| _(uncommitted)_ | **meLWOBond port tranche (a)** — 11 `meLWOBond*` functions ported from `.APPLES\...\meLWO.cpp` into `src/AQ_XLL/src/aqBonds.cpp` as `aqObjBonds*` (DisplaySchedule, DisplayCashflows, DirtyPrice, AccruedInterestDays, CleanPrice, AccruedInterest, Yield, CompoundYield, Price, DV01, DV01Numerical). New helpers `toDoubleVector` / `toExcelDoubleColumn` / `toExcelIntColumn` in `aqXllTools.{h,cpp}`. CLAUDE.md §9 gains an "always update STATUS.md" rule. **Not built yet.** |
| _(uncommitted)_ | `aqObjSave` gains optional `AsArray` arg (force column / force message / auto); new `aqToolsCallerInfo()` diagnostic. Mitigation for the CSE-detection problem. **Not built yet.** |
| `e5c57046` | **AQ_XLL port tranche 1** — `aqMain.cpp` auto-inits the library in the add-in ctor; `AQ_INITIALIZE` + `AQ_IS_ARRAY_OUTPUT` macros; new `aqBonds.cpp` (`aqObjBondsCreate/Display`), `aqObj.cpp` (`aqObjExists/Load/Save`); `aqToolsInitialize`, `aqToolsResize` (positional clip/pad), path slash-normalisation; marshalling helpers in `aqXllTools` (`toBool`, `toAQLStringMatrix`, `toLabelValueBlock`, `toExcelMatrix` numeric-aware, `toExcelColumn`, `reshapeToSize`, `callerRangeSize`, `isArrayOutput`). vcxproj/.filters updated. Verified in Excel by Nicholas, not by GTest. |
| `68ca1ceb` / `bf2cb765` | earlier AQ_XLL updates + step-2 build fixes (RecordMacros `AQLString`). |
| `2d7eb95c` | **Phase 3 step 5** — calendar holiday-centre delimiter `:` → `+`. New `CALENDAR_CENTRE_DELIMITER`/`splitCalendarCentres()` in `math/AQLPriceDataCalendar.{h,cpp}`; routed ~18 parse/build sites (convertFrom/ToString, 16 curve-helper `toToken(':')`, NYB-strip). Input still accepts `:`; output always `+`. New test `Calendars.UNIT_CentreDelimiter_PlusAndColonEquivalent`. |
| `530e764e` | **B-fixtures** — 7,960 `resources/test/inputs` fixture paths + 152 src files (TEST_DIR strings, `generatorFunction` labels, `decorateCurvename/Filename` prefixes, `UNIT_TestMe*` labels, comments) renamed `me/tryMe → aq/tryAq`. `me/tryMeUtilityClearLWOCache → aq/tryAqObjClearCache`. |
| `756d9bb8` | **B-LWO** — `meLWO*/tryMeLWO* → aqObjects*/tryAqObjects*` (496 ids, 85 files). Map A "by product" rejected (21 collisions with the stateless names); Map B `LWO→Objects` adopted. Code only. |
| `c9bb9f49` | **B-simple** — 442 `me*/tryMe*` (non-LWO) → `aq*/tryAq*` code identifiers (247 files). Family map in `rebrand/phase3_Bsimple_MAP.csv`. Code only (strings/resources were deferred to B-fixtures). |
| `649d3aa5` | **B-delete** — removed 6 orphaned securitisation validation files (`tryMe{CashflowClient,LoanCalculations,SupervisoryRules}`, incl. `tryMeSRT*`). Were not in any vcxproj. |
| `700f5722` | `LAString`/`LADate` embedded in 64 identifiers (`getCompulsoryValueAsLAString`, `getLADate`, `TestLAString`, …) → `AQLString`/`AQLDate`. 943 refs. |
| `a8905ca5` | `boost::swap` → `std::swap` in `StatisticsUtilities` (kills the deprecated-header warning). |
| `0bab362e` | `InitializeAQETrading → InitializeETrading`, `InitializeAQGoogleTest → InitializeGoogleTest`. |
| `91a0ad38` / `13ad0c2e` | **Phase 3.1c (Classic)** — `MA/MM/MV/MD/MF/ME_ → AQL` in math/models/calibration (431 renames). `MLIB_2D_MATRIX_CHECK → AQ_2D_MATRIX_CHECK`. Census tool widened. |
| earlier | Phase 3.1 math, 3.2 models, 3.2 etrading (`LA→AQL`, `LWO→AQObj`), Initialize renames, `validation_api→validation`, `MLIB_*→AQ_*`, project renames `AQ_BINDINGS→AQ_API` / `GOOGLE_TEST→GTEST`, solution `AlgoQuantLib-VS22.sln`, solution folders `AQ - Core` / `AQL - Classic`. |

**Group B (`me→aq`) is COMPLETE** — no `me*/tryMe*/meLWO*` names anywhere in `src/` or `resources/test/`.

---

## Pending — Phase 3

### Step 7a — legacy include guards  ← DONE (`4fb2fdb6`)
15 `__LAMATH*_H__` guards in `src/models/include/` → `__AQLMATH*_H__`. Guard-token
only, no build impact.

### Step 7b/7c — LWO + MLIB sweep  ← DONE (`ab35bf0b`, `8e632c65`)
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
prose/error-string mentions (`"LWO Curve … does not exist"` → `AQObj`/`Object`? — a
naming call). `validation_api` / `MLIB_CLIENT_API` / `GOOGLE_TEST` residue is
**Linux-Makefile only → Phase 7**. `XllPlus` → Phase 4.

### Step 6 — mir removal  ← DONE (6a `97c8461e`, 6b `5a29a51a`)
mir stack deleted. Residual `mir` only in strings/comments (Replay.cpp dispatch
keys `functionList["tryMirSetUpOISCurve"]`, a few error messages) -> fold into 7b.
`TestMirDateFunctions.cpp` kept (no mir) -> rename to a non-legacy name in 7b.

### Step 6 (old plan text below, superseded)
**6a DONE (`97c8461e`)**: 5 GTEST curve-fixture builders (`CurveOis/Std/TenorBasis/
XccyBasis/FwdFxConst.cpp`) now call `etrading::AQLUpdateStaticDataManager::setUp*
Curve` directly. Needs build + GTest.

**6b TODO** (bigger than first scoped):
  1. Port `src/GTEST/src/InterestRateSwap.cpp` — shared helper used by ~8 keeper
     Trade tests; its `parRate()/pv()/pv01()` call `tryMirGetParRate4` (→ schedule
     gen + `etrading::AQLCurveForwardRateHelpers::getParRate`), `tryMirSwapPV`,
     `tryMirSwapPV01`. Not trivial passthroughs — read the 3 wrappers, reproduce inline.
  2. Gut `TestCurveEngineCalibrate.cpp` — drop the DEAD `testEngineCurveForwardRates`
     method (all call sites already commented out) + `#include "tryMirGetForwardRate.h"`
     + `#include "CurveOis.h"`.
  3. `git rm`: `src/AQ_API/source/mir*.{h,cpp}` (58, nothing else references), 
     `src/validation/{include,src}/tryMir*.{h,cpp}` (65), and 36 mir-only GTEST .cpp
     (`InterestRateSwap.cpp`? NO — that's the ported helper; its `.h` in GTEST/include
     is shared, keep. The 36 are `TestCurve{Ois,Std,TenorBasis,XccyBasis,FwdFxConst}*`
     (test suites, not the builders), `TestCurveReplay`, `TestMirDateFunctions`,
     `TestDatesECB`, `TestTradeEUR{AssetSwapSpread,OISParRate,SwapStubRate}`,
     `CurveConsistencies_{OIS,STD,TenorBasis}`, `LinearSplineTests`, `Test_IsFwdInter`,
     `InterestRateSwap.cpp`→NO). 
     Recount: exclude the 5 kept builders and InterestRateSwap → ~35 test .cpp.
  4. Edit `swig_{CSharp,JAVA,Python,R}.i` — drop `%include`/`#include "mir*.h"` lines
     (keep `EntityPoolUtilities.h`).
  5. Edit `apiTryAqToolsLVB.h` — drop dead `#include "MBTemplateType.h"`.
  6. Edit `BindFileToClassConstructor.h:20` — stale `_tryMirSetUpOISCurve_inputs`
     comment example.
  7. vcxproj/.filters: strip mir + deleted-test entries from AQ_API / validation / GTEST.
  8. **KEEP all `resources/test/inputs/` fixture files** — keeper tests read the
     `*_tryMirSetUp*_inputs.csv` fixtures via the ported builders; orphan CSVs
     (dirs of deleted tests) → Phase 6 resources audit.
  9. `EntityPoolUtilities` + `tryAqToolsClearEntityPool` — CORE, keep.

### OLD step-7 note (broad attempt, reverted)
A quick script anchored on `#ifndef`/`#define` lines mangled **non-guard conditional-define macros**
(`IR_CALIBRATION_DATA_MAXLOOP → …AQXLOOP`, `MARKETTYPE → AQRKETTYPE`, `LEVENBERG_MARQUARDT → …AQRQUARDT`,
`MAXTERM → AQXTERM`) — the classic `MA`-in-English hazard. Fully reverted; nothing committed.

**Redo approach:** only rewrite guards that are genuinely legacy-prefixed *header guards*, i.e.
`__?(LA|LB)[A-Z]` or `__?LAMATH…_H__` style, verified as the file's `#ifndef X … #define X … #endif`
triple (X identical on all three), with the English-word filter (`MAX/MARKET/MARQUARDT/MANAGER/…`).
Real targets found: **~17 files in `src/models/include/`** with `__LAMATH…_H__` guards
(`AQLMathSABR.h`, `AQLMathJumpDiffusion.h`, …). `AQLMathSwaptionFormulaSZ.h` has a UTF-8 BOM
before `#ifndef` — a naive `^#` regex skips its `#ifndef` line (must use `utf-8-sig` / match after BOM).
Also `MARKETTYPE_SWAP/BASIS/ZERORATE`, `MANAGER` in `calibration/include/AQLDefinitions.h` /
`models/include/ConstantDeclarations.h` are English string-constant macros — **leave**.

Comment sweep (`LWO`/`mir` prose, stale fixture-format comments) — lower priority, do after guards.

### Step 6 — delete the `mir` stack  ← DEFERRED (needs helper port first)
Not cleanly deletable: `CurveOis.{h,cpp}` / `CurveStd` / `CurveTenorBasis` / `CurveXccyBasis` are
**shared GTest curve-fixture builders** used by ~15 non-mir pricing/risk/credit tests
(`TestConstantMaturitySwap`, `TestCreditBasketModel`, `TestTotalReturnSwap`, `TestRiskSwapDeltaLadder`,
`TestTrade{GBP,JPY,USD}SwapParRate`, …) and they build curves via `tryMirSetUp*Curve`.
60 GTest files touch the mir surface.

**To do it:** (1) port those 4 fixture builders to `tryAqObjectsCurveCreate*` / `tryAqCurvesCalibrate*`;
(2) delete `mir` impl (58 `AQ_API/mir*` + 65 `validation/tryMir*`), ~40 mir-only GTest files
(`TestCurve{Ois,Std,TenorBasis,XccyBasis,FwdFxConst}*`, `TestCurveReplay`, `TestMirDateFunctions`,
`TestDatesECB`, `TestTradeEUR*`, `InterestRateSwap`, `CurveConsistencies_*`, `LinearSplineTests`,
`Test_IsFwdInter`), 695 `resources` mir fixtures, the 58 `%include "mir*.h"` lines in the 4 SWIG `.i`
files, and all vcxproj/.filters entries.
User decision recorded: **losing the mir-based regression tests is acceptable — the aq/me suite is the replacement.**
`EntityPoolUtilities.{h,cpp}` is CORE (widely used) — **keep**; `tryAqToolsClearEntityPool()` is its wrapper.
`tryMirClearEntityPool` (mir duplicate) goes with the rest.

### Step 8 — category folder layout + `docs/api_map.csv`  ← DEFERRED
Folder the `validation` `tryAq*` wrappers by category (`src/validation/Curves/`, …), generate
`docs/api_map.csv` (`public | category | validation wrapper | etrading entry | test`), add the
`aq* ↔ tryAq*` CI check. Cleaner to do after the mir delete shrinks `validation`.

### Also outstanding in Phase 3
- `MBRoot.h` / `MBTemplateType.h` — **dead `#include`s that reference non-existent files** in
  `validation/include/apiTryAqToolsLVB.h:14` and `validation/include/tryMirSwapDuration.h:14-15`.
  `tryMirSwapDuration.h` goes with the mir delete. `apiTryAqToolsLVB.h` needs its dead include removed
  (it currently only survives because nothing compiled includes it).
- Guard-underscore normalisation (step 7 targets above).

---

## Public API naming scheme — CURRENT (step 11, 2026-09-08)

Categories are **SINGULAR**. The handle API is the word `Object` after the
category; a named sub-object (Curve, Generator, MarketData, Model, FixingTable)
already denotes an object and does not repeat it.

| Form | Meaning | Example |
|---|---|---|
| `aq<Category><Function>` | stateless — data in, value out | `aqBondScheduleKeys` |
| `aq<Category>Object<Function>` | handle API on the category's product | `aqBondObjectDirtyPrice`, `aqSwapObjectParRate` |
| `aq<Category><SubObject><Function>` | handle API on a named sub-object | `aqBondCurveYield`, `aqBondGeneratorCreate` |
| `aqObject<Lifecycle>` | generic handle lifecycle | `aqObjectLoad`, `aqObjectSave`, `aqObjectClearCache` |
| `AQObj*` / `AQOBJ_*` | internal C++ classes / macros (unchanged) | `AQObjCurve`, `AQOBJ_KEY` |

`validation` (`try` + identical name), `AQ_API` and `GTEST` **test names** all
use these forms. Do not use the earlier `aqObjects*` or `aqObj<Category>*`
prefixes or plural categories anywhere.

**Applied so far:** `AQ_XLL` only (step 11, this session — file renames
`aqBonds/aqDates/aqObj/aqTools.cpp` → `aqBond/aqDate/aqObject/aqTool.cpp`, 53
functions renamed, `.vcxproj`/`.filters` updated). Its `validation::tryAqObj…`
call sites still use the OLD wrapper names — `validation` / `AQ_API` / `GTEST` /
`resources\test` fixtures are the pending sync (MIGRATION_PLAN task 2.6, its own
branch + green-diff gate).

### History (commits — descriptions only, do not follow the old scheme)
- **step 8** `7fd00c7c` / `30fc5505` — introduced `aqObjects*` → `aqObj<Category>*`
  (774 renames, 7,300 fixtures) + internal `AQO`→`AQObj`. Superseded by step 11.
- **step 9** `db980842` — Options umbrella removed, sub-types promoted. Still holds.
- **step 11** (this session) — plural → singular, `aqObj<Cat>` → `aq<Cat>Object`,
  `Vols`→`Volatility`. AQ_XLL done; rest pending.

### Two exclusions that must survive any future AQO/AQObj pass
`AQObjects` (fixture path segment), `IsAQObject`, `isAQObject` — these already
contain `AQObject`; renaming `AQO`→`AQObj` inside them yields `AQObjbject(s)`.

### Open, from the api_pair_check advisory
Five public functions whose wrapper name has drifted from `tryAq<same name>`.
All five *do* route through `validation` (the 4.1 contract holds) — this is
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

### Repo-wide CLAUDE.md — synced
`REPO\CLAUDE.md` (one level above `LIBRARY_UPDATE`, outside the git repo) was
updated by hand once Nicholas granted write access: the category list (`Objects`
dropped, `FX` added), the `aq`/`aqObj` table, the `LWO -> AQObj` rows, the
`AQObjbject` exclusion rule, the architecture diagram, and a rewritten section
2.1 that now points here as the live running record instead of claiming the
code-level rebrand has not started. It is **not** version-controlled -- a
pre-edit copy is in the session scratchpad only, so treat further edits to it
with care.

---

## Step 9 — Options umbrella removed, sub-types promoted (done, 2026-09-04)

`db980842`. From Nicholas's reviewed map (column "use" of the returned
`phase3_step8_MAP.csv`). Rule: **a product sub-type is its own top-level
category**, not a member of an umbrella.

| Umbrella dropped | Promoted to categories |
|---|---|
| `Options` (removed entirely) | `CapFloor` 20, `Swaption` 17, `BondOption` 9, `BondFutureOption` 4 |
| `Swaps` (kept, narrowed) | `AssetSwap` 12, `TotalReturnSwap` 24, `ConstantMaturitySwap` 8 |
| `Curves` (kept, narrowed) | `Inflation` 15 |

113 renames, 0 collisions, 10 src files + 69 fixtures moved in lockstep.
**Category list is now LOCKED (20)** — updated in `CLAUDE.md`, `REPO\CLAUDE.md`,
`MIGRATION_PLAN.md` §2.2 and `rebrand/tools/api_pair_check.py`. `Swaps` now means
the vanilla swap plus legs and schedules. Stateless surface untouched (it never
had an `Options` category and already used `aqAssetSwap*`).

### Note on the map Nicholas returned
Its column C matched the committed map exactly on all 499 rows, but the file was
**truncated mid-alphabet** at `tryAqObjectsFixingTableValues_inputs` — 275 of 774
rows absent, including every `tryAqObj` family from `Fra` onward and all 53
stateless rows. The rule behind the 63 overrides was inferred, checked against
those overrides (61/63 reproduced; the 2 misses were a copy slip and a typo fix,
both confirmed by Nicholas), and then applied to the rows the file never reached.

### Typo fixes — exact tokens only
`DDeltaLadder->DeltaLadder`, `Implived->Implied`, `Mutiple->Multiple`,
`Asof->AsOf` (2). Applied as **exact whole tokens**: a substring sweep of `Asof`
would have hit `AsofDate` / `getAsofDate` / `mAsofDate` (which collide with
existing `AsOfDate` / `getAsOfDate` spellings — a separate, pre-existing internal
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

## Step 10 — MLIB removal (10a done, 2026-09-04)

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
- **`src/AQ_API/source/swig_*_wrap.{cpp,cxx}`** — SWIG-generated. Still contain old `me*`/`mir*` names
  in HEAD (regenerated in Phase 5, not in the core build). They also keep re-appearing as phantom
  worktree modifications (autocrlf on a large generated file); `git checkout HEAD -- src/AQ_API/source/swig_*_wrap.*`
  clears them. Don't stage them.
- Ubiquitous `LF will be replaced by CRLF` git warnings — repo line-ending config, harmless.

## Later phases (unchanged)
- Phase 4: xlOil XLL port (blocked on Nicholas's worked examples), 4a editions manifest, 4b generators/config + calendar JSON `:`→`+` migration (103 files).
- Phase 5: verify C#/Java/R bindings, regenerate SWIG wrappers, expand tests (yield-curve/Jacobian, bond price/yield Bloomberg-parity).
- Phase 6: credit-cluster untangle (`CreditResults.h` etc.), legacy `calibration/math/models` extraction, `resources/{examples,api,documentation}` audit, per-file licence headers + `NOTICE`/`THIRD_PARTY_LICENSES.md`.
- Phase 7: Linux CMake rebrand + validate, clang-format bulk pass, clang-tidy CI gate, copy to clean repo.

## Tooling
- `rebrand/tools/prefix_census.py` — legacy-prefix census per project.
- `rebrand/tools/run_batch.py` — approved-list batch runner (git mv + \b-anchored replace + string-literal report + verify gate). `FILE_PREFIX` handles `LWO→AQObj`.
- Scratch scripts used this run (not committed) are in the session scratchpad:
  `map_bsimple.py`, `map_blwo.py`, `run_bsimple.py`, `run_blwo.py`, `bfixtures.py`,
  `step6b_delete.py`, `step6b_vcxproj.py`, `step7b_prose.py`, `step7c.py`,
  `step8_map.py`, `step8a.py`, `step8b.py`, `step9.py`.
- `rebrand/phase3_Bsimple_MAP.csv`, `phase3_BLWO_mapB.csv` — the approved `me→aq` maps (committed).
- `rebrand/phase3_step8_MAP.csv` — the approved step-8 map, 774 rows (committed).
- `rebrand/phase3_step9_MAP.csv` — the approved step-9 map, 113 rows (committed).
- `rebrand/tools/fixture_key_check.py` — fixture keys vs current parameter names.
- `rebrand/tools/api_pair_check.py` — `aq*` must route through `validation`; emits `docs/api_map.csv`.
  Run with `--write` to refresh the map; exit 1 makes it usable as a CI gate.
