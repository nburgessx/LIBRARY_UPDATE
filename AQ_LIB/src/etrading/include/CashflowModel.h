#pragma once

#include "Tranche.h"
#include "LoanCalculations.h"
#include "CoreEnumerations.h"


#include <vector>


namespace etrading
{


	/* @brief CashflowModel: This function projects the input loan-cashflows through a capital structure
	*  @param[in]	trancheStructure	This vector describes the tranches that make up the capital structure
	*  @param[in]	kirb				The capital requirement on the securitized exposures, in accortance with the  Internal Ratings-Based Approach
	*  @param[in]	elgd				The exposure-weighted average loss given default: The share of an asset that is lost if a borrower defaults
	*  @param[in]	trancheMaturity		The tranches' remaining effective maturity in years.
	*  @param[in]	nEffectiveExposures	A measure of the portfolio diversification, which in turn implies the granularity
	*  @param[in]	supervisoryTypeEnum	The regulatory rules to follow: LA or IRBA ( internal ratings-based approach )
	*  @param[in]	poolTypeEnum		The type of the pool ("Wholesale" or "Retail")
	*/
	std::vector<TrancheCashflow> cashflowModel( const std::vector<LoanCashflowBreakdown>& loanCashflows,
												const std::vector<TrancheDefinition>& trancheStructure,
												const double& kirb,
												const double& elgd,
												const double& trancheMaturity,
												const int& nEffectiveExposures,
												const SupervisoryTypeEnum& supervisoryTypeEnum,
												const PoolTypeEnum& poolTypeEnum );

}

