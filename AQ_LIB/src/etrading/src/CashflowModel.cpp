#include "CashflowModel.h"
#include "LoanCalculations.h"
#include "SupervisoryRules.h"

#include "ExceptionMacros.h"

namespace
{
	double capTrancheAssignmentLevelIfSlightlyGreaterThanOne( const double& value, const std::string& valueName )
	{
		double cappedValue = value;

		if ( value > 1.0 )
		{
			const double noiseTolerance = 1.0e-10;
			if ( ( cappedValue - 1.0 ) > noiseTolerance )
			{
				// Significantly greater than 1.0. There must be a problem in the calculation
				AQ_THROW( "Calculated value for " + valueName + " point is greater than 1.0" );
			}
			else
			{
				// Within numericla noise level of 1.0. OK to cap.
				cappedValue = 1.0;
			}	
		}
		return cappedValue;
	}
}

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
												const PoolTypeEnum& poolTypeEnum )
	{

		AQ_REQUIRE( loanCashflows.size() > 0, "loanCashflows should contain at least 1 period.");
		
		const size_t nPeriods = loanCashflows.size();
		const size_t nTranches = trancheStructure.size();

		std::vector<TrancheCashflow> liabilityFlows;
		liabilityFlows.reserve( nPeriods );
		
		/* Process losses
		*
		*/
		for ( size_t period = 0; period < nPeriods; period++ )
		{
			TrancheCashflow liabilityFlow;
			liabilityFlow.trancheAllocations.resize( nTranches );

			if ( period == 0 )
			{
				for ( size_t j=0; j < nTranches; j++ )
				{
					liabilityFlow.trancheAllocations[j].balanceStart = trancheStructure[j].trancheSize_;
				}
			}

			liabilityFlow.period = ( period + 1 ); // Record the period starting at 1

			const LoanCashflowBreakdown& currentPeriodLoanCashflow = loanCashflows[ period ];
			
			/*
				# interest waterfall, calculated on previous closing balances

				# index + spr
				# int due, int paid
				# int shortfall
				# intcashflow
			*/

			const double periodInterest = currentPeriodLoanCashflow.interestComponent;
			liabilityFlow.coupon = periodInterest;

			/*
			    # for (i in count_tranches:1)
				# {
				#
				#   #        liabilityflows[period, 6+count_tranche_fields*j]<-
				#
				# }

				# ===========================================================================================
				# losses allocate bottom up ================================================================
				# loss waterfall, start with most junior tranche
				# period loss taken from portfolio
			*/

			// periodLoss is updated as the loss is assigned to the various tranches
			double periodLoss = currentPeriodLoanCashflow.lossGivenLoss;
			liabilityFlow.lossGivenLoss = periodLoss;

			for ( size_t j=0; j<nTranches; j++ )
			{
				// # is period loss
				if ( period > 0 )
				{
					const TrancheCashflow& prevLiabilityFlow = liabilityFlows.back();
					liabilityFlow.trancheAllocations[ j ].balanceStart = prevLiabilityFlow.trancheAllocations[ j ].balanceEnd;
				}
				
				const double lossAllocated = std::max( 0.0, std::min( liabilityFlow.trancheAllocations[ j ].balanceStart, periodLoss ) );
				
				// # assign the loss allocated to the tranche and calculate tranche_balance_end
				liabilityFlow.trancheAllocations[ j ].balanceEnd = liabilityFlow.trancheAllocations[ j ].balanceStart - lossAllocated;

				// # Record the loss allocated
				liabilityFlow.trancheAllocations[ j ].lossAllocated = lossAllocated;

				// # reduce the allocation from the period loss
				periodLoss = periodLoss - lossAllocated;
			}

			// # ===========================================================================================
			// # principal allocate top down ===============================================================
			const double periodPrincipalComponent = currentPeriodLoanCashflow.principalComponent;
			liabilityFlow.principalComponent = periodPrincipalComponent;

			const double periodRecovery = currentPeriodLoanCashflow.severityRecovered;
			liabilityFlow.severityRecovered =  periodRecovery;

			const double periodPrepayment = currentPeriodLoanCashflow.prepayment;
			liabilityFlow.prepayment = periodPrepayment;

			// # total principal collections
			double principalCollection = periodPrincipalComponent + periodRecovery + periodPrepayment;

			double notionalAllProRataTranches = 0.0;
			double trancheFactorSum = 0.0;
			double proRataBalance = 0.0;

			// # calculate notional_all_prorata_tranches
			for ( size_t k = 0; k < nTranches; k++ )
			{
				if ( trancheStructure[k].payType_ == PRO_RATA_TRANCHE_TYPE )
				{
					notionalAllProRataTranches += liabilityFlow.trancheAllocations[ k ].balanceEnd;
				}
			}

			// Assign principal, starting from the most senior tranche
			for ( int k = nTranches-1; k >= 0; k-- )
			{
				switch ( trancheStructure[k].payType_ )
				{
					case PRO_RATA_TRANCHE_TYPE:
					{
						// if hits first
						if ( proRataBalance == 0.0 )
						{
							proRataBalance = principalCollection;
						}
				
						double trancheFactor = 0.0;
						if ( notionalAllProRataTranches > 0.0 )
						{
							trancheFactor = liabilityFlow.trancheAllocations[ k ].balanceEnd / notionalAllProRataTranches;
						}

						/*
						# tranche_factor_sum <-tranche_factor + tranche_factor_sum

						# let us check that we cap the notional that can be applied to the residual balance of the tranche
						# MONDAY!!!!!!!!!!!!!!!!!

						# lf <- liabilityflows[period, 1+(count_tranche_fields*k)]
						# if (is.na(lf)) {
						#   cat('period NA', period)
						# }

						# if(!is.na(tranche_factor)) {
						*/

						const double balanceEnd = liabilityFlow.trancheAllocations[ k ].balanceEnd;
						const double principalAllocated = std::min( proRataBalance * trancheFactor,  balanceEnd );

						// # assign the principal allocated to the tranche and recalculate tranche_balance_end
						liabilityFlow.trancheAllocations[ k ].balanceEnd = balanceEnd - principalAllocated;
					
						// # record principal allocated
						liabilityFlow.trancheAllocations[ k ].principalAllocated = principalAllocated;

						// # reduce the allocation from the period loss balance
						principalCollection = principalCollection - principalAllocated;
						break;
					}
					case SEQUENTIAL_TRANCHE_TYPE:
					{
						const double principalAllocated = std::max( 0.0, std::min( liabilityFlow.trancheAllocations[ k ].balanceStart, principalCollection ) );

						// # assign the principal allocated to the tranche and recalculate tranche_balance_end
						const double balanceEnd = liabilityFlow.trancheAllocations[ k ].balanceEnd;
						liabilityFlow.trancheAllocations[ k ].balanceEnd = balanceEnd - principalAllocated;

						// # record principal allocated
						liabilityFlow.trancheAllocations[ k ].principalAllocated = principalAllocated;

						// # reduce the allocation from the period loss balance
						principalCollection = principalCollection - principalAllocated;
						break;
					}
					default:
						AQ_THROW( "Tranche  type must be either  'SEQUENTIAL' or 'PRORATA'" );
						break;
				}
			}

			// # ===========================================================================================

			// ## tranche % attach/detatch done bottom up

			double trancheSubordination = 0.0;

			//# evaluation from equity bottom up

			for ( size_t l=0; l < nTranches; l++ )
			{
				// # express tranche as % attach, detatch
				const double poolBalance = currentPeriodLoanCashflow.balanceEnd;

				// # write pool balance to liability flow struct
				liabilityFlow.poolBalance = poolBalance;

				// # attachment
				double attachment = trancheSubordination / poolBalance;

				/* The attachment point cannot be larger than 1.0
				*  If through numerical noise the attachment is only very slightly above 1.0, cap to 1.0.
				*/
				attachment = capTrancheAssignmentLevelIfSlightlyGreaterThanOne( attachment, "attachment");
				liabilityFlow.trancheAllocations[ l ].attachmentPoint = attachment;

				// # detachment
				const double trancheBalanceEnd = liabilityFlow.trancheAllocations[ l ].balanceEnd;

				double detachment =  ( trancheBalanceEnd + trancheSubordination ) / poolBalance;

				/* The detachment point cannot be larger than 1.0
				*  If through numerical noise the detachment is only very slightly above 1.0, cap to 1.0.
				*/
				detachment = capTrancheAssignmentLevelIfSlightlyGreaterThanOne( detachment, "detachment");
				liabilityFlow.trancheAllocations[ l ].detachmentPoint = detachment;

				// # aggregate cumulative subordination
				trancheSubordination = trancheSubordination + trancheBalanceEnd;

				if ( period < 10  )
				{
					// *** TODO What does paste() function do in R?
					// paste ("subord id",tranche_subordination)
				}

				// # Reg capital calculations ===========================================================
				// # Do not calculate RW SF on last day, as paid off
				if ( period < nPeriods - 1 )
				{
					if ( detachment == 0.0 )
					{
						liabilityFlow.trancheAllocations[ l ].riskWeight = 0.0;
					}
					else
					{
						switch( supervisoryTypeEnum )
						{
							case LEGACY_SUPERVISORY_TYPE:
							{
								const double riskWeight = legacyRules::supervisoryFormula( attachment, ( detachment - attachment), kirb, elgd, nEffectiveExposures );
								liabilityFlow.trancheAllocations[ l ].riskWeight = riskWeight;
								break;
							}
							case IRBA_SUPERVISORY_TYPE:
							{
								RankTypeEnum rankType = NON_SENIOR_RANK_TYPE;
								// # make rank senior when in the loop
								if ( l == 2 )
								{
									rankType = SENIOR_RANK_TYPE;
								}
								// # TODO check attach detach
								const double riskWeight = secIrbaRules::secIrbaRiskWeight( poolTypeEnum, nEffectiveExposures, rankType, kirb, elgd, trancheMaturity, attachment, detachment );
								liabilityFlow.trancheAllocations[ l ].riskWeight = riskWeight;
								break;
							}
							default:
								AQ_THROW( "supervisory_type must be either 'NEW' or 'OLD'" );
								break;
						}
					}
				
				}
				else
				{
					// Final period
					liabilityFlow.trancheAllocations[ l ].riskWeight = 0.0;
				}
			}
			liabilityFlows.push_back( liabilityFlow );
		}

		return liabilityFlows;
	}


}

