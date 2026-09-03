#pragma once

#include "DataFrame.h"
#include "CoreEnumerations.h"

#include "LACoreTemplateType.h"  // AnyTypeMatrix

#include <string>
#include <vector>

namespace etrading
{
	/* @brief	Calculates the monthly payment for a loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	loanTypeEnum	Whether  the loan is amortizing or has bullet repayment.
	* @returns	The monthly loan payment
	*/
	double loanPayment( const int& term, const double& loanRate, const double& originalBalance, const LoanTypeEnum& loanTypeEnum );

	/* @brief	Calculates the amount of loan principal which is repaid in a specified period of a loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	period			The period (month) of the loan under consideratrion
	* @param[in]	loanTypeEnum	Whether  the loan is amortizing or has bullet repayment.
	* @returns	The principal repayment in the specified period of the loan
	*/
	double loanPrincipal( const int& term, const double& loanRate, const double& originalBalance, const int& period, const LoanTypeEnum& loanTypeEnum );

	/* @brief	Calculates the amount of loan principal remaining at a specified period of a loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	period			The period (month) of the loan under consideratrion
	* @param[in]	loanTypeEnum	Whether  the loan is amortizing or has bullet repayment.
	* @returns	The principal remaining in the specified period of the loan
	*/
	double loanPrincipalRemaining( const int& term, const double& loanRate, const double& originalBalance, const int& period, const LoanTypeEnum& loanTypeEnum );

	/* A structure which stores the projected monthly cashflow breakdown of a loan, including default amounts
	*/
	struct LoanCashflowBreakdown
	{
	public:
		size_t period;
		double balanceStart;
		double defaultBalance;
		double lossGivenLoss;
		double severityRecovered;
		double prepayment;
		double balanceAfterDefaultAndPrepayment;
		double loanPayment;
		double interestComponent;
		double principalComponent;
		double balanceEnd;
		double severityRecoveredNoLag;
		double riskWeight;
		double kirbAmount;

		// Initializes all double fields to NaN
		void initializeToNaN();

		// This method takes in another LoanCashflowBreakdown as "rhs", and adds all the field values into our own
		// Used to accumulate the cashflows from multiple loans into a single amortization table.
		void accumulate( const LoanCashflowBreakdown& rhs );
	};

	// A vector of LoanCashflowBreakdown represents the amortization cashflows of a loan as a function of time.
	// Each subsequent item in the vector represents the cashflows at a subsequent month
	// Typically the vector will extend all the way out to maturity of the loan.
	typedef std::vector<LoanCashflowBreakdown> LoanAmortizationCashflows;

	// The list of column names in the LoanCashflowBreakdown structure
	extern std::vector<std::string> cashflowBreakdownColumns;

	// Converts a LoanAmortizationCashflows structure into an AnyTypeMatrix with headers, suitable for display
	AnyTypeMatrix displayLoanAmortizationCashflows( const LoanAmortizationCashflows& loanAmortisationCashflows );

	// Utility function to populate a DataFrame with LoanAmortization results
	DataFrame loanAmortizationCashflowsToDataFrame( const LoanAmortizationCashflows& loanAmortisationCashflows );


	/* @brief Projects the monthly cashflows of a loan
	* @param[in]	term				The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate			The annualized loan rate
	* @param[in]	originalBalance		The loan amount
	* @param[in]	defaultVector		Specifies the default rate in each period of the loan
	* @param[in]	severityVector		Specifies the percentage loss, if a default occurs
	* @param[in]	prepaymentVector	Specifies the prepayment rate in each period of the loan
	* @param[in]	riskWeightVector	Specifies the risk-weight in each period of the loan
	* @param[in]	kirbVector			Specifies the capital requirement in each period fo a loan 
	* @param[in]	loanTypeEnum		Whether  the loan is amortizing or has bullet repayment.
	* @param[in]	loanPaymentsPerYear An int specifying the loan payment frequency. 1=monthly, 3=quarterly etc
	* @param[in]	cumulativeDefault	A boolean flag specifying whether the defaultedBalance is based on the original loan amount, or on the balance at the beginning of the time period
	* @param[in]	recoveryLagInMonths	Whether the recovery is received immediately, or if it is paid with a lag
	* @param[in]	keepPostTermRecoveries A boolean flag specifying whether the recovered amount should be included, if the recovery occurs after the loan term
	*
	* @returns	A structure containing the monthly loan cashflows and default amounts
	*/
	LoanAmortizationCashflows loanAmortizationDefaultPrepayment( const int& term,
																 const double& loanRate,
																 const double& originalBalance,
																 const std::vector<double>& defaultVector,
																 const std::vector<double>& severityVector,
																 const std::vector<double>& prepaymentVector,
																 const std::vector<double>& riskWeightVector,
																 const std::vector<double>& kirbVector,
																 const LoanTypeEnum& loanTypeEnum,
																 const int loanPaymentsPerYear = 1,
																 const bool cumulativeDefault = false,
																 const int recoveryLagInMonths = 0,
																 const bool keepPostTermRecoveries = false );


	/* @brief Projects the monthly cashflows of a loan
	* @param[in]	resultRows				The maximum number of rows required for the result matrix, in order to store loan payments all the way to maturity
	* @param[in]	originalBalance			The initial loan amount
	* @param[in]	loanTerm				The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRateInEachMonth		The annualized loan rate, specified for each month during the life of the loan
	* @param[in]	defaultRateInEachMonth	The default rate in each period of the loan
	* @param[in]	severityRateInEachMonth	Specifies the percentage loss, if a default occurs
	* @param[in]	prepaymentInEachMonth	Specifies the prepayment rate in each period of the loan
	* @param[in]	loanType				An enum specifying whether  the loan is amortizing or has bullet repayment.
	* @param[in]	loanRiskWeight			Specifies the risk-weight of the loan as a single numnber
	* @param[in]	loanKirb				Specifies the capital requirement of the loan as a single number
	* @param[in]	loanPaymentsPerYear		Frequency of interest payments. Default is 1 (i.e. annual interest)
	* @param[in]	cumulativeDefault		A boolean
	* @param[in]	recoveryLagInMonths		Number of months lag. Default is 0
	* @param[in]	keepPostTermRecoveries	A boolean
	* @returns	A structure containing the monthly loan cashflows and default amounts. Each item in the vector represents 1 month in the life of the loan
	*/
	LoanAmortizationCashflows  loanProjection(  const size_t resultRows,
												const double& originalBalance,
												const int loanTerm,
												const std::vector<double>& loanRateInEachMonth,
												const std::vector<double>& defaultRateInEachMonth,
												const std::vector<double>& severityRateInEachMonth,
												const std::vector<double>& prepaymentInEachMonth,
												const LoanTypeEnum loanType,
												const double& loanRiskWeight,
												const double& loanKirb,
												const int loanPaymentsPerYear = 1,
												const bool cumulativeDefault = false,
												const int recoveryLagInMonths = 0,
												const bool keepPostTermRecoveries = false );

}

