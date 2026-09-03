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

### `etrading` — securitisation / CLO cluster — ⚠ NOT CLEAN

`CashflowEngine`, `CashflowModel`, `LoanCalculations`, `LoanPortfolio`,
`LoanValidation`, `SyntheticExcessSpread`, `Tranche`, `Trigger`.

**Entanglement found (was missed in the first pass):**
`etrading/include/CreditResults.h` — which is on the **KEEP** list (CDS
result-reporting) — `#include`s `LoanPortfolio.h`, `LoanCalculations.h`,
`SyntheticExcessSpread.h`, `CashflowEngine.h`, `Trigger.h`, `Tranche.h`, and
holds a `std::shared_ptr<Trigger>` member. `Trigger.h` in turn `#include`s
`LoanCalculations.h`. `validation/tryMeUtilitySetup.cpp` uses
`CreditResultsContainer`. So the cluster cannot be deleted without first
deciding what `CreditResults` is in the rebranded library:

- **(a)** `CreditResults` is genuinely securitisation code mis-labelled as CDS →
  the whole `CreditResults` + `CreditResultsContainer` + cluster goes, and
  `tryMeUtilitySetup.cpp` is adjusted.
- **(b)** `CreditResults` is real CDS result-reporting that happens to carry
  vestigial loan/trigger/tranche members → strip those members, keep
  `CreditResults`, then the cluster deletes cleanly.

**This is a domain call for Nicholas** and needs a proper "what is the `Credit`
category" design pass — moved to its own step (`MIGRATION_PLAN.md` "Phase 6.0 —
Credit untangle"), not a Phase 1 mechanical delete.

**Safe subset that CAN go in Phase 1** (no inbound deps from kept code, no test
deps): the `validation` wrappers `tryMeCashflowClient.*`, `tryMeLoanCalculations.*`,
`tryMeSupervisoryRules.*`, and `SupervisoryRules.cpp` — **but** `SupervisoryRules.h`
is `#include`d by `CashflowEngine.cpp` / `CashflowModel.cpp` (both being kept for
now due to the `CreditResults` entanglement), so even this subset has to wait for
the untangle. **Net: defer the whole client-credit removal to Phase 6.0.**

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
