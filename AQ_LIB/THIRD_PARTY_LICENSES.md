# Third-Party Licences

AlgoQuantLib is distributed with the third-party components below. Each remains
under its own licence. AlgoQuantLib's own licence is in `LICENSE`.

> **Status: draft.** Versions and paths are current; the full licence texts and
> the xlOil modified-file list are completed in migration Phase 6.6.

| Component | Version | Licence | Shipped in | Notes |
|---|---|---|---|---|
| Boost | 1.91.0 | Boost Software License 1.0 | binaries (static) | Permissive. No binary attribution required, but the licence text is included here. |
| QuantLib | 1.43 | QuantLib licence (modified BSD, 3-clause style) | binaries (static) | Permissive. |
| xlOil | 0.19.0 | Apache License 2.0 | `AlgoQuantLib.xll` (static) | **Modified.** AlgoQuantLib ships a patched xlOil (see below). Apache-2.0 §4(b) requires modified files to carry prominent notices. |
| Eigen | 3.x | MPL 2.0 | headers only (linked) | File-level copyleft. Used unmodified and header-only — no source-publication obligation is triggered. **Do not modify Eigen sources**; wrap instead. |
| Adept | 2.0.3 | Apache License 2.0 | binaries (static) | Permissive. |
| GoogleTest | 1.17.0 | BSD 3-Clause | **not shipped** — test build only | Listed for completeness. |
| rapidjson | (vendored) | MIT | headers only | Permissive. Confirm still in use after the rebrand; remove from this list if dropped. |

## xlOil modifications

AlgoQuantLib distributes a modified copy of xlOil 0.19.0. Modified files:

- `src/xlOil-COM/AppObjects.cpp` — added an explicit template instantiation
  `template class Collection<ExcelWorkbook, Excel::Workbooks>;` inside
  `namespace xloil`, required for the static-XLL build (the upstream dllexport
  DLL build forces this instantiation implicitly; the static build does not).

A patch file capturing this diff is kept at
`resources/thirdparty/xlOil-0.19.0-aq.patch` (to be added in Phase 6.6).

## Full licence texts

Full texts to be vendored under `resources/thirdparty/licenses/` in Phase 6.6:
`BOOST-1.0.txt`, `QUANTLIB.txt`, `APACHE-2.0.txt` (xlOil, Adept), `MPL-2.0.txt`
(Eigen), `BSD-3-CLAUSE.txt` (GoogleTest), `MIT.txt` (rapidjson).
