# Phase 0 — status & hand-back

Working inventories for the rebrand. Reviewed items feed Phase 1–5. This folder
is transient — it does not ship and is not carried into the clean repo.

| # | Item | Status | Artefact |
|---|---|---|---|
| 0.1 | All 12 build configs green | **Nicholas — build needed now** (see below) | — |
| 0.2 | `AQ_EXTERNAL_LIB_PATH` resolves | drafted (see below) | this file |
| 0.3 | Pre-rebrand test baseline recording | **Nicholas — needs a build** | see below |
| 0.4 | Git `baseline` tag | ☑ done — tag `baseline` on `36bffcc` (pristine pre-rebrand code) | `git tag` |
| 0.5 | `mir` call-graph | ☑ | `callgraph_mir.md` |
| 0.6 | Client-specific removal map | ☑ (ambiguous rows: KEEP, review at 1.4a) | `removal_map_client_specific.md` |
| 0.7 | 653-function XLL inventory | ☑ | `xll_function_inventory.csv` |
| 0.8 | Calendar-centre delimiter sites | ☑ (D12 = `+` only) | `calendar_delimiter_sites.md` |
| 0.9 | `LICENSE` + `THIRD_PARTY_LICENSES.md` | ☑ | `../LICENSE`, `../THIRD_PARTY_LICENSES.md` |
| 0.10 | Config & generator inventory | ☑ | `inventory_config_generators.md` |

## Already applied (commits on top of `baseline`)

`git reset --hard baseline` reverts everything below.

| Commit | Change | Risk |
|---|---|---|
| `6c262aa` | planning docs, inventories, `LICENSE`; `Mizuho`/`MHI`/`MHSC` → `AlgoQuantHub` in `src` comments; `etrading.nuspec` rebranded | comments/docs only |
| `50cad24` | removed 733 contractor header blocks; 90 Shift-JIS files → UTF-8; ~4,800 Japanese chars stripped from comments; 129 full-width punct → ASCII; **2 string-literal fixes** (`LEVENBERG-MARQUARDT_METHOD` hyphen — behavioural; `Grid_T` error text) | comments/encoding + 2 flagged lines |
| `767aa4e` | client drive paths (`M:\…5858_mlibir`, `S:\mizuho\…`) → `.\config\…` | dead fallback strings |

No code identifiers renamed. **You need to build now** to confirm these
comment/encoding changes didn't disturb anything (esp. the 90 UTF-8 re-encodes
and the `LEVENBERG` branch), *before* the Phase 1 identifier rename starts.

---

## 0.2 — external libs path

On disk the dependency tree is at
`REPO\.ALGO_QUANT_LIB\AQ_EXTERNAL_LIBS\library` (Boost 1.91, QuantLib-1.43,
Eigen3, Adept203, googletest-1.17.0, swigwin-4.0.0, xlOil-0.19.0).

`SetEnvVars.bat` derives its default as a folder *beside* `AQ_LIB`,
which does not exist under `LIBRARY_UPDATE`. **Action for Nicholas:** run the
script once and, at the external-libs prompt, enter the path above (or wherever
you keep the real copy). Confirm `%AQ_EXTERNAL_LIB_PATH%` after, then restart VS.

`.ALGO_QUANT_LIB` is a read-only backup — fine to *reference* for builds, never
write to it. If you would rather not depend on a `.`-prefixed backup folder for
builds, relocate the external libs to a stable path and point the env var there.

---

## 0.3 — baseline recording (for Nicholas)

Before any rename lands, capture the behavioural baseline so every later stage
can be diffed against it:

1. Build `Debug|x64` and `Release|x64` (whole solution).
2. Run `GOOGLE_TEST` (Release) — save full console + any generated result files
   to `baseline\gtest\`.
3. Run the harnesses under `resources\test\batch_file` and
   `resources\test\excel_tests` — save outputs to `baseline\excel\`.
4. If the `validation` layer has an input/output recording switch, enable it for
   the above and copy the recordings to `baseline\validation_io\`.
5. Build each of `Release{Python,CSharp,Java,R}|x64`; run the matching
   `resources\api\*` test app; save outputs to `baseline\bindings\`.
6. Put `baseline\` **outside** `AQ_LIB` (it is a reference, not source).

After each rename stage, re-run 2–5 and `diff` against `baseline\`. Renames are
behaviour-preserving — any numeric delta is a bug from that stage. The only
sanctioned behaviour change in the whole rebrand is the calendar delimiter,
which gets its own new test.

---

## What is safe to start now (no build needed)

- Review the five drafted inventories in this folder.
- Sign off the locked category list (`MIGRATION_PLAN.md` §2.2) — done.
- Decide the edition → category map straw man (`MIGRATION_PLAN.md` §2.4).
- Decide the ambiguous rows in `removal_map_client_specific.md`.
