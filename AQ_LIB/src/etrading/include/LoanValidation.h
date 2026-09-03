/*
 * @brief			Structured Credit Loan validation and amortization calculations
 *					Contains functions to validate a portfolio of loans and perform various amortization calculations.
 *					Based on the MGEN file "loan_amortization.R"
 * 
 * @Created:		28th Nov 2019
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 */

#pragma once

#include "LoanCalculations.h"
#include "DataFrame.h"

#include <string>
#include <vector>


namespace etrading
{
	// Helper methods which validate a portfolio of loans, and calculates the loan amortization all the way to maturity

	LoanAmortizationCashflows loansAmortization( const DataFrame& loanPortfolio,
												 const double& defaultStress,
												 const double& severityStress,
												 const double& prepayment,
												 const bool cumulativeDefault = false,
												 const int recoveryLagInMonths = 0,
												 const bool keepPostTermRecoveries = false,
												 const bool aggregateLoans = true,
												 const double& defaultStressAddition = 0.0 );

}

