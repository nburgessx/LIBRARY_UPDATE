/*
 * @brief			Structured Credit API functions
 *					Contains functions to expose the structured credit loan cashflow breakdown calculations
 *
 *					This code was ported from MGEN AlgoQuantHub R analytics library
 *					Author Andrew Friend
 *
 * @Created:		20th March 2019
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 */

#pragma once




#include "CashflowModel.h"
#include "LoanCalculations.h"
#include "LabelValueBlock.h"
#include "LACoreTemplateType.h"
#include "CommonConstants.h"


namespace validation_api
{
	/* @brief Projects the monthly cashflows of an amortizing loan
	* @param[in]	term				The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate			The annualized loan rate
	* @param[in]	originalBalance		The loan amount
	* @param[in]	defaultVector		Specifies the default rate in each period of the loan
	* @param[in]	severityVector		Specifies the percentage loss, if a default occurs
	* @param[in]	prepaymentVector	Specifies the prepayment rate in each period of the loan
	* @param[in]	riskWeightVector	Specifies the risk-weight in each period of the loan
	* @param[in]	kirbVector			Specifies the capital requirement in each period fo a loan 
	* @param[in]	loanType			Whether  the loan is amortizing or has bullet repayment.
	*
	* @returns	A structure  containing the monthly loan cashflows and default amounts
	*/
	std::vector<etrading::LoanCashflowBreakdown> tryMeSRTAmortizeSingleLoan( const int& term,
																			 const double& loanRate,
																			 const double& originalBalance,
																			 const std::vector<double>& defaultVector,
																			 const std::vector<double>& severityVector,
																			 const std::vector<double>& prepaymentVector,
																			 const std::vector<double>& riskWeightVector,
																			 const std::vector<double>& kirbVector,
																			 const std::string& loanType );

	/* @brief	Projects the monthly cashflows of an amortizing loan. Similar to the function above
	*			except that the results are presented in an Excel-friendly matrix.
	* @param[in]	term				The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate			The annualized loan rate
	* @param[in]	originalBalance		The loan amount
	* @param[in]	defaultVector		Specifies the default rate in each period of the loan
	* @param[in]	severityVector		Specifies the percentage loss, if a default occurs
	* @param[in]	prepaymentVector	Specifies the prepayment rate in each period of the loan
	* @param[in]	riskWeightVector	Specifies the risk-weight in each period of the loan
	* @param[in]	kirbVector			Specifies the capital requirement in each period fo a loan 
	* @param[in]	loanType			Whether  the loan is amortizing or has bullet repayment.
	*  @param[in]	includeCashflowColumnHeaders	Whether to annotate the LAStringMatrix with column headers
	*
	* @returns	A matrix containing the cashflows and default amounts
	*/
	LAStringMatrix tryMeSRTAmortizeSingleLoanAsMatrix( const int& term,
													 const double& loanRate,
													 const double& originalBalance,
													 const std::vector<double>& defaultVector,
													 const std::vector<double>& severityVector,
													 const std::vector<double>& prepaymentVector,
													 const std::vector<double>& riskWeightVector,
													 const std::vector<double>& kirbVector,
													 const std::string& loanType,
													 const bool& includeCashflowColumnHeaders );

	/* @brief	Reads a CSV file containing a portfolio of loans and computes the cashflow amortization of the portfolio
	*  @param[in]	loanCsvFilename	The CSV file containing the loan portfolio details.
	*  @param[in]	csvColumnNames	The CSV file can contain many columns; this data lists the column names corresponding to "EAD", "RW", "KIRB"
	*
	*  @returns	A structure containing the monthly cashflows and default amounts of the loan portfolio
	*/
	std::vector<etrading::LoanCashflowBreakdown> tryMeSRTAmortizeLoanPortfolio( const std::string& loanCsvFilename, const etrading::LabelValueBlock& csvColumnNames );

	/* @brief	Reads a CSV file containing a portfolio of loans and computes the cashflow amortization of the portfolio. Similar to the function above
	*			except that the results are presented in an Excel-friendly matrix.
	*  @param[in]	loanCsvFilename	The CSV file containing the loan portfolio details.
	*  @param[in]	csvColumnNames	The CSV file can contain many columns; this data lists the column names corresponding to "EAD", "RW", "KIRB"
	*  @param[in]	includeCashflowColumnHeaders	Whether to annotate the LAStringMatrix with column headers
	*
	*  @returns	A structure containing the monthly cashflows and default amounts of the loan portfolio
	*/
	LAStringMatrix tryMeSRTAmortizeLoanPortfolioAsMatrix( const std::string& loanCsvFilename, const etrading::LabelValueBlock& csvColumnNames, const bool& includeCashflowColumnHeaders );


	/* @brief CashflowModel: This function projects the input loan-cashflows through a capital structure
	*		  Note: This is the R-library exposed function.
	*  @param[in]	trancheStructure	This vector describes the tranches that make up the capital structure
	*  @param[in]	kirb				The capital requirement on the securitized exposures, in accortance with the  Internal Ratings-Based Approach
	*  @param[in]	elgd				The exposure-weighted average loss given default: The share of an asset that is lost if a borrower defaults
	*  @param[in]	trancheMaturity		The tranches' remaining effective maturity in years.
	*  @param[in]	nEffectiveExposures	A measure of the portfolio diversification, which in turn implies the granularity
	*  @param[in]	supervisoryType		The regulatory rules to follow: LA or IRBA ( internal ratings-based approach )
	*  @param[in]	poolType			The type of the pool ("Wholesale" or "Retail")
	*/
	std::vector<etrading::TrancheCashflow> tryMeSRTCalculateCashflow( const std::vector<etrading::LoanCashflowBreakdown>& loanCashflows,
																	  const std::vector<etrading::TrancheDefinition>& trancheStructure,
																	  const double& kirb,
																	  const double& elgd,
																	  const double& trancheMaturity,
																	  const int& nEffectiveExposures,
																	  const std::string& supervisoryType,
																	  const std::string& poolType );

	namespace LoanPortfolio
	{
	
		static std::vector<std::string> lvbKeys()
		{
			const std::string arr[] =
			{
				etrading::STRUCTURED_CREDIT_CSV_KEYS::EXPOSURE_AT_DEFAULT
				,etrading::STRUCTURED_CREDIT_CSV_KEYS::RISK_WEIGHT 
				,etrading::STRUCTURED_CREDIT_CSV_KEYS::KIRB
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
			return expectedKeys;
		}
	}
}



