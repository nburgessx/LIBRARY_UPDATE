#pragma once


#include "LoanCalculations.h"


namespace validation_api
{
	/* @brief	Calculates the monthly payment for a loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	loanType		Whether  the loan is amortizing or has bullet repayment.
	* @returns	The monthly loan payment
	*/
	double tryMeSRTLoanPayment( const int& term, const double& loanRate, const double& originalBalance, const std::string& loanType );

	/* @brief	Calculates the amount of loan principal which is repaid in a specified period of a loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	period			The period (month) of the loan under consideratrion
	* @param[in]	loanType		Whether  the loan is amortizing or has bullet repayment.
	* @returns	The principal repayment in the specified period of the loan
	*/
	double tryMeSRTLoanPrincipal( const int& term, const double& loanRate, const double& originalBalance, const int& period, const std::string& loanType );

	/* @brief	Calculates the amount of loan principal remaining at a specified period of a loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	period			The period (month) of the loan under consideratrion
	* @param[in]	loanType		Whether  the loan is amortizing or has bullet repayment.
	* @returns	The principal remaining in the specified period of the loan
	*/
	double tryMeSRTLoanPrincipalRemaining( const int& term, const double& loanRate, const double& originalBalance, const int& period, const std::string& loanType );


	std::vector<etrading::LoanCashflowBreakdown> tryMeSRTLoanAmortizationDefaultPrepayment( const int& term,
																							const double& loanRate,
																							const double& originalBalance,
																							const std::vector<double>& defaultVector,
																							const std::vector<double>& severityVector,
																							const std::vector<double>& prepaymentVector,
																							const std::vector<double>& riskWeightVector,
																							const std::vector<double>& kirbVector,
																							const std::string& loanType );
}

