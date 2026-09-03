#pragma once

#include "DataFrame.h"
#include "LoanCalculations.h"
#include "RepLine.h"

#include "CoreEnumerations.h"
#include "Variant.h"

#include <string>
#include <vector>


namespace etrading
{
	/* Class to represent a portfolio of loans
	*/
	class LoanPortfolio
	{
	public:

		// Main constructor
		LoanPortfolio( const std::string& name, const size_t periods = 0 );

		// Initializes the loan portfolio from a CSV file (known as a "tape")
		void setPortfolioTape( const DataFrame& loanDataFromTape );

		// Returns the loan portfolio as a matrix, where each row of the matrix represents a separate loan definition.
		DataFrame getPortfolioTape() const;

		// Rolls out the loans all the way to maturity, calculating the monthly interest and capital payments
		void projectLoans();
		
		// Specifies how reinvestment amounts should be allocated to new loans. Each new loan is represented by a RepLine or "Representative Line".
		// Each repline specifies which proportion of the capital should be invested in it.
		void seedReinvestmentRepLines( const std::vector<RepLine>& repLines );

		// Soecifies how reinvestment amounts should be allocated to new loans.
		// The loans are specified as a matrix (DataFrame) where each row of the matrix represents a loan definition.
		void seedReinvestmentPortfolio( const DataFrame& reinvestmentPortfolio );

		// Initializes the portfolio with a set of loan cashflows in which the amortization payments have already been computed.
		void seedPortfolio( const LoanAmortizationCashflows& loanAmortizaionCashflows );

		// Reinvests the specified amount into new loans at the specified time period.
		void reinvestBalance( const size_t period, const double amount );

		// Reinvests the specified amount into new loans represented as RepLines (Representative Lines) at month "period".
		// Each RepLine specifies a loan definition, and a proportion of capital to be invested in that loan.
		// Note that you must first call method seedReinvestmentRepLines() to iniitalize the RepLines in the portfolio.
		void reinvestBalanceRepLines( const size_t period, const double amount );

		// Reinvests the specified amount into new loans at month "period".
		// Note you must first call method seedReinvestmentPortfolio to initialize a set of loans which will be used for reinvestment.
		void reinvestBalancePortfolio( const size_t period, const double amount );

		// Get the portfolio balance outstanding at start of the specified time period
		double getBalanceStart( const size_t period );

		// Get the portfolio balance outstanding at the end of the specified time period.
		double getBalanceEnd( const size_t period );

		// Get the amount of defaulted balance at the specified time period.
		double getDefaultBalance( const size_t period );

		// The Loss Given Loss is a risk parameter which captures the uncertainty about the loss that will be realized
		double getLossGivenLoss( const size_t period );

		// Get the amount of loan recovered at the specified time period.
		double getSeverityRecovered( const size_t period );

		// Get the amount of loan prepayment at the specified time period
		double getPrepayment( const size_t period );

		// Get the remaining balance after loan default and prepayment are taken into account, at the specified time period.
		double getBalanceAfterDefaultAndPrepayment( const size_t period );

		// Get the amount of loan balance which is prepaid at the specified time period.
		double getLoanPayment( const size_t period );

		// Get the amount loan interest due at the specified time period
		double getInterestComponent( const size_t period );

		// Get the amount of loan due for repayment at the specified time period
		double getPrincipalComponent( const size_t period );

		// Get the weighted exposure at default risk weight.
		// The risk weight is weighted by the balance at the end of the specified period.
		double getWeadRiskWeight( const size_t period );

		// Get the weighted exposure at default capital requirement (KIRB).
		// The KIRB is weighted by the balance at the end of the specified period.
		double getWeadKirb( const size_t period );

		// Calculate the total amount of capital / principal available for reinvestment at the specified time period.
		double calculatePrincipalCollection( const size_t period );

		// Get the series of loan cashflows all the way to maturity of the portfolio.
		// If loanNumber is zero (or omitted), returns the portfolio cashflows.
		// Specifying a loanNumber > 0 returns the amortization cashflows corresponding to loan reinvestment.
		LoanAmortizationCashflows getLoanProjection( const size_t resultNumber = 0 ) const;

		// Get the series of loan cashflows all the way to maturity of the portfolio,
		// as an AnyTypeMatrix, suitable for display
		AnyTypeMatrix getLoanProjectionAsAnyMatrix( const size_t resultNumber = 0 ) const;

		// Get the amount which has been reinvested into new loans, at the specified time period
		double getReinvestmentBalance( const size_t period ) const;

		// Records the amount of reinvestment made into new loands at the specified time period.
		void setReinvestmentBalance( const size_t period, const double amount );

		// Get the amount of capital / principal available for reinvestment at the specified time period.
		double getPrincipalCollection( const size_t period ) const;

		// Set the amount of capital / principal available for reinvestment at the specified time period.
		void setPrincipalCollection( const size_t period, const double amount );

		// Allocates storage for reinvestment balances up to the specified maximum time period
		void initializeReinvestmentBalance( const size_t periods );

		// Examine all the attached loan amortizations and return the maximum
		size_t calculateMaximumLoanPeriod() const;

		void clearAllReinvestmentResults();

	private:

		// Helper method to apply initial processing to the set of loans
		void preProcess( DataFrame& dataToCheck );

		// Helper method which stores reinvestment cashflows for reinvestment at the specified time period
		void storeReinvestmentResults( const size_t period, const LoanAmortizationCashflows& loanAmortizationCashflows );

		std::string portfolioName_;

		DataFrame tapeData_;

		DataFrame reinvestmentPortfolioTemplate_; // reinvestment_ptf in Portfolio.R.

		std::vector<RepLine> reinvestmentRepLines_;

		// Indexed by monthly period in which the reinvestment is made
		std::vector<double> reinvestmentBalance_;

		std::vector<double> principalCollection_;

		VariantMatrix migrationMatrix_;

		std::vector< LoanAmortizationCashflows > projectedLoans_;

		// Note: What datatype should we use for "periods" and what is this attribute used for?
		// In R:  numeric()
		size_t nPeriods_;

		// Stress coefficients
		double defaultStress_;
		double severityStress_;
		double prepaymentStress_;

		// Spread on top of the defaultSress multiplier
		double defaultStressAddition_;

		// default assumptions for missing parameterisations
		double loanRateAssumption_;
		double loanFreqAssumption_;
		bool cumulativeDefault_;
		int recoveryLagInMonths_;
		bool keepPostTermRecoveries_;
		bool persistLoans_;

	};

	// Class which stores the portfolio cashflow payments as a function of time. Used by the Capital Structure.
	class PortfolioFlows
	{
	public:
		void initialize( size_t nPeriods );

		std::vector<int> period;
		std::vector<AQLDate> periodDate;
		std::vector<double> balanceStart;
		std::vector<double> balanceEnd;
		std::vector<double> principalComponent;
		std::vector<double> severityRecovered;
		std::vector<double> prepayment;
		std::vector<double> lossGivenLoss;
		std::vector<double> interestComponent;
		std::vector<double> reinvestmentBalance;

		std::vector<double> cumulativeDefaultBalance;
		std::vector<double> cumulativeLossGivenLoss;
		std::vector<double> cumulativeSeverityRecovered;
		
		std::vector<double> cumulativeProportionDefaultBalance;
		std::vector<double> cumulativeProportionLossGivenLoss;
		std::vector<double> cumulativeProportionSeverityRecovered;
	};

}

