#include "LoanValidation.h"
#include "LoanCalculations.h"

#include "ExceptionMacros.h"
#include "CommonConstants.h"
#include "CoreEnumerations.h"

#include "Variant.h"

#include <algorithm>


// Private helper functions
namespace
{
	std::vector<double> scaleDoubleVector(const std::vector<double>& inputVector, const double& scaleFactor)
	{
		std::vector<double> scaledVector;
		scaledVector.reserve( inputVector.size() );
		for ( auto value : inputVector )
		{
			const double scaledValue = value * scaleFactor;
			scaledVector.push_back( scaledValue );
		}
		return scaledVector;
	}

}

namespace etrading
{
	// Helper methods which validate a portfolio of loans, and calculates the loan amortization all the way to maturity

	LoanAmortizationCashflows loansAmortization( const DataFrame& loanPortfolio,
												 const double& defaultStress,
												 const double& severityStress,
												 const double& prepayment,
												 const bool cumulativeDefault,
												 const int recoveryLagInMonths,
												 const bool keepPostTermRecoveries,
												 const bool aggregateLoans,
												 const double& defaultStressAddition )
	{
		// Make these column names into const variables
		AQ_REQUIRE( loanPortfolio.hasColumn( STRUCTURED_CREDIT_CSV_KEYS::MATURITY_MONTH ), "Missing column MATURITYMONTH in Loan Portfolio" );
		AQ_REQUIRE( loanPortfolio.hasColumn( STRUCTURED_CREDIT_CSV_KEYS::PEAD ), "Missing column PEAD in Loan Portfolio" );
		AQ_REQUIRE( loanPortfolio.hasColumn( STRUCTURED_CREDIT_CSV_KEYS::LOAN_TYPE ), "Missing column LOAN_TYPE in Loan Portfolio" );
		AQ_REQUIRE( loanPortfolio.hasColumn( STRUCTURED_CREDIT_CSV_KEYS::RISK_WEIGHT ), "Missing column RW in Loan Portfolio" );
		AQ_REQUIRE( loanPortfolio.hasColumn( STRUCTURED_CREDIT_CSV_KEYS::KIRB ), "Missing column KIRB in Loan Portfolio" );
		AQ_REQUIRE( loanPortfolio.hasColumn( STRUCTURED_CREDIT_CSV_KEYS::RATE ), "Missing column Rate in Loan Portfolio" );
		AQ_REQUIRE( loanPortfolio.hasColumn( STRUCTURED_CREDIT_CSV_KEYS::FREQ ), "Missing column Freq in Loan Portfolio" );
		AQ_REQUIRE( loanPortfolio.hasColumn( STRUCTURED_CREDIT_CSV_KEYS::PDP ), "Missing column PDP in Loan Portfolio" );
		AQ_REQUIRE( loanPortfolio.hasColumn( STRUCTURED_CREDIT_CSV_KEYS::LGD ), "Missing column LGD in Loan Portfolio" );

		// The Proportion Probability of Default ( vs Regulatory Probability of Default )
		const auto pdpColumn = loanPortfolio.getColumnAsDoubleVector( STRUCTURED_CREDIT_CSV_KEYS::PDP );
		std::vector<double> defaultAmountColumn = scaleDoubleVector( pdpColumn, defaultStress );

		// Include the defaultStressAddition
		const double scaledStressAddition = defaultStressAddition / 10000;
		for ( size_t i=0; i<defaultAmountColumn.size(); i++ )
		{
			defaultAmountColumn[ i ] += scaledStressAddition;
		}

		// The Loss Given Default
		const auto lgdColumn = loanPortfolio.getColumnAsDoubleVector( STRUCTURED_CREDIT_CSV_KEYS::LGD );
		std::vector<double> severityColumn = scaleDoubleVector( lgdColumn, severityStress );

		// "Prepayment" is not a property of data-tape, but an assumption
		const size_t loanCount = loanPortfolio.numRows();
		std::vector<double> prepaymentColumn( loanCount, prepayment );

		const auto termColumn = loanPortfolio.getColumnAsDoubleVector( STRUCTURED_CREDIT_CSV_KEYS::MATURITY_MONTH );
		
		// Find the term of the loan with the longest remaining maturity
		const double maxTerm = *( std::max_element(termColumn.begin(), termColumn.end() ) );

		const auto loanRateColumn        = loanPortfolio.getColumnAsDoubleVector( STRUCTURED_CREDIT_CSV_KEYS::RATE );
		const auto loanTypeColumn        = loanPortfolio.getColumnAsIntVector( STRUCTURED_CREDIT_CSV_KEYS::LOAN_TYPE );
		const auto riskWeightColumn      = loanPortfolio.getColumnAsDoubleVector( STRUCTURED_CREDIT_CSV_KEYS::RISK_WEIGHT );
		const auto kirbColumn            = loanPortfolio.getColumnAsDoubleVector( STRUCTURED_CREDIT_CSV_KEYS::KIRB );
		const auto frequencyColumn       = loanPortfolio.getColumnAsIntVector( STRUCTURED_CREDIT_CSV_KEYS::FREQ );
		const auto originalBalanceColumn = loanPortfolio.getColumnAsDoubleVector( STRUCTURED_CREDIT_CSV_KEYS::PEAD );

		// TODO: Conversion from double to size_t
		const size_t resultRows = ( (recoveryLagInMonths > 0) && keepPostTermRecoveries ) ? maxTerm + recoveryLagInMonths	: maxTerm;

		// TODO: Andy: Consider storing all the results, not just the cumulative.
		LoanAmortizationCashflows cumulativeLoanResults( resultRows);
		for (size_t monthIdx = 0; monthIdx < resultRows; monthIdx++)
		{
			cumulativeLoanResults[monthIdx] = {};					// Initialize all elements to zero.
			cumulativeLoanResults[monthIdx].period = monthIdx + 1;	// Initialize the period field
		}

		for (size_t loanIndex = 0; loanIndex < loanCount; loanIndex++)
		{
			// TODO: Take care with rounding: Loan months double to size_t
			const size_t loanTermInMonths   = termColumn[loanIndex];

			const double loanRate           = loanRateColumn[loanIndex];
			const double loanDefaultRate    = defaultAmountColumn[loanIndex];
			const double loanSeverityRate   = severityColumn[loanIndex];
			const double loanPrepaymentRate = prepaymentColumn[loanIndex];

			const int loanTypeAsInt = loanTypeColumn[loanIndex];
			AQ_REQUIRE(loanTypeAsInt == 0 || loanTypeAsInt == 1, "LOAN_TYPE column must contain 0 or 1 only, representing Amortizing or Bullet loans");

			const LoanTypeEnum loanType  = loanTypeAsInt == 0 ? AMORTIZING_LOAN_TYPE : BULLET_LOAN_TYPE;
			const double loanRiskWeight  = riskWeightColumn[loanIndex];
			const double loanKirb        = kirbColumn[loanIndex];
			const int loanPaymentsPerYear = frequencyColumn[loanIndex];
			const double originalBalance = originalBalanceColumn[loanIndex];

			const std::vector<double> loanRateInEachMonth(loanTermInMonths, loanRate);
			const std::vector<double> defaultRateInEachMonth(loanTermInMonths, loanDefaultRate);
			const std::vector<double> severityRateInEachMonth(loanTermInMonths, loanSeverityRate);
			const std::vector<double> prepaymentInEachMonth(loanTermInMonths, loanPrepaymentRate);

			LoanAmortizationCashflows loanCashflows = loanProjection(   resultRows,
																		originalBalance,
																		loanTermInMonths,
																		loanRateInEachMonth,
																		defaultRateInEachMonth,
																		severityRateInEachMonth,
																		prepaymentInEachMonth,
																		loanType,
																		loanRiskWeight,
																		loanKirb,
																		loanPaymentsPerYear,
																		cumulativeDefault,
																		recoveryLagInMonths,
																		keepPostTermRecoveries );

			// Accumulate each column of loanCashflows into total
			for (size_t period = 0; period < loanCashflows.size(); period++)
			{
				cumulativeLoanResults[period].accumulate(loanCashflows[period]);
			}

		}

		// Final normalization of the KIRB and RiskWeight
		const size_t nRows = cumulativeLoanResults.size();
		for ( size_t i = 0; i < nRows; i++ )
		{
			const double balanceAfterDefaultAndPrepayment	= cumulativeLoanResults[i].balanceAfterDefaultAndPrepayment;
			cumulativeLoanResults[i].kirbAmount				/= balanceAfterDefaultAndPrepayment;
			cumulativeLoanResults[i].riskWeight				/= balanceAfterDefaultAndPrepayment;
		}

		return cumulativeLoanResults;
	}

}

