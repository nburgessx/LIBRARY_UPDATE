
#include "LoanCalculations.h"
#include "tryMeLoanCalculations.h"

#include "RecordMacros.h"
#include "StructuredExceptionHandler.h"
#include "ExceptionMacros.h"


namespace validation_api
{
	/* @brief	Calculates the monthly payment for a loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	loanType		Whether  the loan is amortizing or has bullet repayment.
	* @returns	The monthly loan payment
	*/
	double tryMeSRTLoanPayment( const int& term, const double& loanRate, const double& originalBalance, const std::string& loanType )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( term, loanRate, originalBalance, loanType );

		const etrading::LoanTypeEnum loanTypeEnum = etrading::toLoanTypeEnum( loanType );

		const double paymentAmount = etrading::loanPayment( term, loanRate, originalBalance, loanTypeEnum );

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT( paymentAmount );

		VALID_EXCEPTION_END

	}

	/* @brief	Calculates the amount of loan principal which is repaid in a specified period of a loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	period			The period (month) of the loan under consideratrion
	* @param[in]	loanType		Whether  the loan is amortizing or has bullet repayment.
	* @returns	The principal repayment in the specified period of the loan
	*/
	double tryMeSRTLoanPrincipal( const int& term, const double& loanRate, const double& originalBalance, const int& period, const std::string& loanType )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( term, loanRate, originalBalance, loanType );

		const etrading::LoanTypeEnum loanTypeEnum = etrading::toLoanTypeEnum( loanType );

		const double principal = etrading::loanPrincipal( term, loanRate, originalBalance, period, loanTypeEnum );
		
		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT( principal );

		VALID_EXCEPTION_END
	}

	/* @brief	Calculates the amount of loan principal remaining at a specified period of a loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	period			The period (month) of the loan under consideratrion
	* @param[in]	loanType		Whether  the loan is amortizing or has bullet repayment.
	* @returns	The principal remaining in the specified period of the loan
	*/
	double tryMeSRTLoanPrincipalRemaining( const int& term, const double& loanRate, const double& originalBalance, const int& period, const std::string& loanType )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( term, loanRate, originalBalance, period, loanType );

		const etrading::LoanTypeEnum loanTypeEnum = etrading::toLoanTypeEnum( loanType );

		const double principalRemaining  = etrading::loanPrincipalRemaining( term, loanRate, originalBalance, period, loanTypeEnum );

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT( principalRemaining );

		VALID_EXCEPTION_END

	}
	
	std::vector<etrading::LoanCashflowBreakdown> tryMeSRTLoanAmortizationDefaultPrepayment( const int& term,
																							const double& loanRate,
																							const double& originalBalance,
																							const std::vector<double>& defaultVector,
																							const std::vector<double>& severityVector,
																							const std::vector<double>& prepaymentVector,
																							const std::vector<double>& riskWeightVector,
																							const std::vector<double>& kirbVector,
																							const std::string& loanType )
	{

		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( term, loanRate, originalBalance, defaultVector, prepaymentVector, riskWeightVector, kirbVector, loanType );

		const etrading::LoanTypeEnum loanTypeEnum = etrading::toLoanTypeEnum( loanType );

		const std::vector<etrading::LoanCashflowBreakdown> result = etrading::loanAmortizationDefaultPrepayment( term,
																												 loanRate,
																												 originalBalance,
																												 defaultVector,
																												 severityVector,
																												 prepaymentVector,
																												 riskWeightVector,
																												 kirbVector,
																												 loanTypeEnum );
																												 
		// Record Outputs AND Return the Result for logs, tests and playback
		//RECORD_OUTPUTS_AND_RETURN_RESULT( principalRemaining );
		return result;

		VALID_EXCEPTION_END	
	
	}

}