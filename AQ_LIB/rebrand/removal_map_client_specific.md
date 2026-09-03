# 0.6 — client-specific removal map

Decision (Nicholas): drop `msc*`, `LoanCalculations`, `SupervisoryRules`,
`CashflowClient` and the securitisation / CLO cluster they sit on. **Keep** the
CDS/credit infrastructure and CMS (rates) infrastructure — those feed the
`Credit` and `Rates` categories.

Execute in **Phase 1.4a** (after `mir` removal, before the identifier rebrand).

## DROP — confirmed client-specific

### XLL add-in (`.APPLES\...\MLIBQ_ADDIN\src`) — just don't port
| File | Fns |
|---|---|
| `mscCashflowEngine.cpp` | 6 |
| `mscDataFrame.cpp` | 4 |
| `mscLoanPortfolio.cpp` | 5 |
| `mscSyntheticExcessSpread.cpp` | 2 |
| `mscTranche.cpp` | 4 |
| `mscTrigger.cpp` | 4 |
| `meLoanCalculations.cpp` | 3 |
| `meSupervisoryRules.cpp` | 2 |
| `meCashflowClient.cpp` | 2 |

### `validation` wrappers — delete file + `GOOGLE_TEST` cases
`tryMeCashflowClient.{h,cpp}`, `tryMeLoanCalculations.{h,cpp}`,
`tryMeSupervisoryRules.{h,cpp}`.

### `etrading` — securitisation / CLO cluster — delete
`CashflowEngine.{h,cpp}`, `CashflowModel.{h,cpp}`, `LoanCalculations.cpp`,
`LoanPortfolio.{h,cpp}`, `LoanValidation.{h,cpp}`, `SyntheticExcessSpread.{h,cpp}`,
`Tranche.cpp`, `Trigger.{h,cpp}`.
No non-etrading includer except the `validation` wrappers above (themselves
dropped) and `GOOGLE_TEST` has no references — clean.

## KEEP — do NOT confuse with the above

| Kept | Why |
|---|---|
| `PremiumLeg`, `PremiumSchedule`, `PremiumCashflow`, `ProtectionLeg`, `ProtectionSchedule`, `CDSLeg`, `CreditDefaultSwap`, `CreditModel`, `CreditResults`, `CreditResultsContainer` | **CDS infrastructure** → `Credit` category |
| `CMSCashflow`, `CMSLeg`, `CMSSchedule`, `CMSStaticData` | **CMS (constant-maturity swap)** → `Rates` / `Swaps` |

## VERIFY before deleting — flagged  ⚠ KEEP FOR NOW

**Nicholas's call (this session): keep these for now; revisit at Phase 1.4a.**
Do not delete any row below without an explicit go-ahead — the Phase 1.4a
checklist has a STOP gate for it.


| Item | Question | Suggested check |
|---|---|---|
| `SchemaObject.cpp`, `SerializeContainedData.cpp`, `SerializationResult.cpp`, `SerializationUtilities.cpp` | Generic AQO/JSON serialisation (used by **generators** — keep) or securitisation-only (drop)? | `grep -rl "SchemaObject\|SerializeContainedData" src` minus the cluster; if generators / `ObjectUtilities` use it → **keep**. Likely keep. |
| `CreditResultsContainer.h` included by `tryMeUtilitySetup.cpp` | Real dependency or incidental include? | open the file; if it only registers a type for the pool, keep the container (it is CDS-side anyway). |
| `LADataFrame` / `mscDataFrame` overlap | Is `DataFrame` a generic tabular type used elsewhere, or CLO-only? | `grep -rl "DataFrame" src` — keep the generic type if shared. |
| `LARiskConfiguration*` hits from the first broad grep | false positives (matched `-i msc`) — `calibration` risk config is **not** in scope here | ignore |

## Order

1. Delete the `validation` wrappers (leaf).
2. Delete the `etrading` cluster; build.
3. Resolve the four VERIFY items; delete or keep accordingly; build.
4. Mark the corresponding rows `drop` in `xll_function_inventory.csv` so they are
   never ported.
