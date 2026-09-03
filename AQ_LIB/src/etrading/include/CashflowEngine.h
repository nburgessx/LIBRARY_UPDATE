#pragma once

#include "SyntheticExcessSpread.h"
#include "Tranche.h"
#include "Trigger.h"

#include "LoanCalculations.h"
#include "LoanPortfolio.h"
#include "CashflowModel.h"

#include "DataFrame.h"

#include "CoreEnumerations.h"

#include "AQLDate.h"

#include <string>
#include <vector>


namespace etrading
{

	class CashflowEngine
	{
	public:
		/* @brief	Simple constructor
		*  @param[in]	name	The name of this cashflow engine
		*/
		CashflowEngine( const std::string& name );

		/* @brief	Configures the CashflowEngine from separate parameters
		*  @param[in]	loanPortfolio		The portfolio of loan cashflows to process
		*  @param[in]	trancheDefinitions	Specifies the tranches within the capital structure
		*  @param[in]	kirb				Regulatory capital amount
		*  @param[in]	elgd				Expected loss given default
		*  @param[in]	regulatoryCapitalMaturity	
		*  @param[in]	nDiversityMetric	
		*  @param[in]	supervisoryType		Which regulatory / supervisory framework to use. e.g. Legacy or IRBA
		*  @param[in]	poolType			Wholesale or retail
		*  @param[in]	effectiveDate		The effective date of the portfolio
		*  @param[in]	maturityDate		The maturity date of the portfolio
		*  @param[in]	triggers			Specifies triggers used to determine if reinvestments can be made
		*  @param[in]	reinvestmentEndDate	Specifies a reinvestment cut-off date 
		*  @param[in]	reinvestmentEndPeriod	
		*/
		void configure( const std::shared_ptr<LoanPortfolio>& loanPortfolio,
						const std::vector<std::shared_ptr<TrancheDefinition> >& trancheDefinitions,
						const double kirb,
						const double elgd,
						const int regulatoryCapitalMaturity,	// In MGEN this is 'mt'
						const size_t nDiversityMetric,			// In MGEN this is 'n'
						const SupervisoryTypeEnum supervisoryType,
						const PoolTypeEnum poolType,
						const AQLDate& effectiveDate,
						const AQLDate& maturityDate,
						const std::vector<std::shared_ptr<Trigger> >& triggers,
						const AQLDate& reinvestmentEndDate,
						const size_t reinvestmentEndPeriod );	

		/* @brief	Configures the CashflowEngine using a label value block
		*  @param[in]	configurationLVB		The portfolio of loan cashflows to process
		*  @param[in]	trancheDefinitions		Specifies the tranches within the capital structure
		*  @param[in]	triggers				Specifies triggers used to determine if reinvestments can be made
		*/
		void configure( const std::shared_ptr<LoanPortfolio>& loanPortfolio, 
						const LabelValueBlock& configurationLVB,
						const std::vector<std::shared_ptr<TrancheDefinition> >& trancheDefinitions,
						const std::vector<std::shared_ptr<Trigger> >& triggers,
						const bool validateKeys );


		static std::vector<std::string> lvbKeys()
		{
			const std::string arr[] =
			{
				CASHFLOW_ENGINE_KEYS::KIRB
				, CASHFLOW_ENGINE_KEYS::ELGD
				, CASHFLOW_ENGINE_KEYS::REGULATORY_CAPITAL_MATURITY
				, CASHFLOW_ENGINE_KEYS::DIVERSITY_METRIC
				, CASHFLOW_ENGINE_KEYS::SUPERVISORY_TYPE
				, CASHFLOW_ENGINE_KEYS::POOL_TYPE
				, CASHFLOW_ENGINE_KEYS::EFFECTIVE_DATE
				, CASHFLOW_ENGINE_KEYS::MATURITY_DATE
				, CASHFLOW_ENGINE_KEYS::REINVESTMENT_END_DATE
				, CASHFLOW_ENGINE_KEYS::REINVESTMENT_END_PERIOD
			};
			std::vector<std::string> expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));

			return expectedKeys;
		}

		/* @brief	sets the number of time periods to evaluate the capital structure over
		*  @param[in]	periods	The number of periods
		*/
		void setPeriods( const size_t periods );

		/* @brief
		*  @param[in]	nDiversityMetric
		*/
		void setDiversityMetric( const size_t nDiversityMetric );

		/* @brief Main calculation entry point
		*/
		void evaluate();

		//void resetExcessSpread( const size_t period, const double& excessSpreadAmount );

		// void applySyntheticExcessSpread( const size_t period, const double& periodLoss ) const;

		/* @brief	Perform discount factor calculculations using the yield curve data stored in the capital structure
		*/
		void populateYieldCurveData();

		/* @brief		Perform interest calculation for the specified time period
		*  @param[in]	period	The time period for which to calculate the interest amount due
		*/
		void payInterest( const size_t period );

		/* @brief		Allocated losses in the specified time period
		*  @param[in]	period		The time period at which to assign the loss
		*  @param[in]	periodLoss	The loss amount to allocate to tranches
		*/
		void payLosses( const size_t period, const double& periodLoss );

		/* @brief		Reinvest an amount in the specified time period
		*  @param[in]	period	The time period at which the reinvestment should take place
		*  @param[in]	amount	The amount to reinvest
		*/
		void reinvest( const size_t period, const double& amount );

		/* @brief	Perform principal allocation calculation
		*  @param[in]	period			The time period at which the principal amount should be paid
		*  @param[in]	principalAmount	The amount of principal to assign at the specified period
		*/
		void payPrincipal( const size_t period, const double& principalAmount );

		/* @brief	Recalculate the tranche attachment and detachment points at the specified time period
		*  @param[in]	period	The time period at which to perform the calculation
		*/
		void calculateAttachDetach( const size_t period );

		/* @brief	Calculates the regulatory capital requirement at the specified time period
		*  @param[in]	period	The period at which to perform the calculation
		*/
		void calculateRegulatoryCapital( const size_t period );

		 /* @brief	Check if PoolBalanceEnd minus Balance Ends of all tranches equals 0
		 *			i.e. verify at each timestep that ASSETS (Portfolio Loans) matches LIABILITIES (tranches).
		 *  @param[in]	liabilityFlows		A dataFrame containing the liabilityFlows calculated by evaluate() and returned by getLiabilityFlows()
		 *  @param[out]	validRows			A vector bool indicating whether equilibruim is maintained at each timestep	
		 *  @param[out]	residuals			A vector of double indicating the residual at each timestep (ASSETS - LIABILITIES)
		*/
		static void validateEquilibrium( const DataFrame& liabilityFlows, std::vector<bool>& validRows, std::vector<double>& residuals );

		/* @brief	Attaches an optional Synthetic Excess Spread tranche to the capital structure
		*  @param[in]	syntheticExcessSpread	The SyntheticExcessSpread object to use
		*/
		void setSyntheticExcessSpread( const std::shared_ptr<SyntheticExcessSpread>& syntheticExcessSpread );

		/* @brief Main output function to retrieve the results of the waterfall calculation
		*  @returns	A dataframe containing the cashflow breakdown and tranche allocation for the lifetime of the portfolio
		*/
		DataFrame getLiabilityFlows() const;

		/* @brief	Helper function: For a given TrancheDefinition, get the corresponding MonthlySchedule data calculated by evaluate()
		*  @param[in]	trancheDefinition	Specify which tranche we are interested in
		*  @returns		The monthly schedule data for that tranche.
		*/
		TrancheScheduleData getTrancheMonthlyScheduleDataForTranche( const std::shared_ptr<TrancheDefinition>& trancheDefinition ) const;

	private:

		/* @brief	A helper function which converts portfolio flows into a dataframe, suitable for output
		*/
		DataFrame outputPortfolioFlowsToDataFrame() const;


		std::string name_;

		std::shared_ptr<LoanPortfolio> loanPortfolio_;

		// One TrancheDefinition per tranche ( equity, mezz, senior )
		std::vector<std::shared_ptr<TrancheDefinition> > trancheDefinitions_;

		// One trancheScheduleData per tranche ( equity, mezz, senior )
		// The TrancheAllocation stores the investment balances per period
		std::vector<TrancheScheduleData> trancheMonthlyScheduleData_;	// This holds the tranche position on a monthly schedule
		std::vector<TrancheScheduleData> trancheCouponScheduleData_;	// This holds the tranche coupon schedule ( Monthly, Quarterly, Semi-Annual etc )


		double kirb_;
		double elgd_;

		int regulatoryCapitalMaturity_;		// mt
		size_t nDiversityMetric_;			// n

		SupervisoryTypeEnum supervisoryType_;
		PoolTypeEnum poolType_;

		AQLDate effectiveDate_;
		AQLDate maturityDate_;

		std::vector<AQLDate> resetDates_;
		std::vector<double> resetRates_;

		std::vector<AQLDate> portfolioFixingDates_;
		std::vector<AQLDate> portfolioAccrualStartDates_;
		std::vector<AQLDate> portfolioAccrualEndDates_;
		std::vector<AQLDate> portfolioPaymentDates_;

		std::vector<std::shared_ptr<Trigger> > triggers_;
		AQLDate reinvestmentEndDate_;
		size_t reinvestmentEndPeriod_;
		bool paymentAveraging_;

		std::shared_ptr<SyntheticExcessSpread> syntheticExcessSpread_;	// "sxs" in MGEN R code
		std::vector<double> excessSpread_;

		double portfolioBalanceSize_;
		size_t nPeriods_;

		PortfolioFlows portfolioFlows_;

		DataFrame liabilityFlows_;

	};

	
	


}

