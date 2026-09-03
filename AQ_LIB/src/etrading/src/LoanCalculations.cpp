#include "LoanCalculations.h"
#include "ExceptionMacros.h"
#include "DataUtilities.h"	// For AQ_TO_STRING macros
#include <cmath>


// Private helper functions
namespace
{
	/* @brief	Calculates the monthly payment for an amortising loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @returns	The monthly loan payment
	*/
	double amortizingLoanPayment( const int& term, const double& loanRate, const double& originalBalance )
	{
		const double periodicRate = loanRate / 12.0;
		const double paymentFactor = std::pow( 1.0 + periodicRate, term );
		const double payment = ( originalBalance * paymentFactor ) * ( periodicRate / ( paymentFactor - 1.0 ));

		return payment;
	}

	/* @brief	Calculates the amount of loan principal which is repaid in a specified period of an amortizing loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	period			The period (month) of the loan under consideratrion
	* @returns	The principal repayment in the specified period of the loan
	*/
	double amortizingLoanPrincipal( const int& term, const double& loanRate, const double& originalBalance, const int& period )
	{
		const double periodicRate = loanRate / 12.0;
		const double discountedPayment   = periodicRate * std::pow( ( 1.0 + periodicRate ), ( period - 1 ) );
		const double discountedPrincipal = std::pow( ( 1.0 + periodicRate ), term ) - 1.0;

		const double periodicPrincipal = originalBalance * ( discountedPayment / discountedPrincipal );
		return periodicPrincipal;
	}

	double amortizingLoanPrincipalRemaining( const int& term, const double& loanRate, const double& originalBalance, const int& period )
	{
		const double periodicRate = loanRate / 12.0;

		double loanPrincipalRemaining = originalBalance * ( ( std::pow( ( 1.0 + periodicRate ), term )
			- std::pow( ( 1.0 + periodicRate ), period ) ) / ( std::pow( ( 1.0 + periodicRate ), term ) - 1 ) );

		return loanPrincipalRemaining;
	}

}

namespace etrading
{
	// Initializes all double fields to NaN
	void LoanCashflowBreakdown::initializeToNaN()
	{
		period = 0;
		balanceStart					= std::numeric_limits<double>::quiet_NaN();
		defaultBalance					= std::numeric_limits<double>::quiet_NaN();
		lossGivenLoss					= std::numeric_limits<double>::quiet_NaN();
		severityRecovered				= std::numeric_limits<double>::quiet_NaN();
		prepayment						= std::numeric_limits<double>::quiet_NaN();
		balanceAfterDefaultAndPrepayment = std::numeric_limits<double>::quiet_NaN();
		loanPayment						= std::numeric_limits<double>::quiet_NaN();
		interestComponent				= std::numeric_limits<double>::quiet_NaN();
		principalComponent				= std::numeric_limits<double>::quiet_NaN();
		balanceEnd						= std::numeric_limits<double>::quiet_NaN();
		severityRecoveredNoLag			= std::numeric_limits<double>::quiet_NaN();
		riskWeight						= std::numeric_limits<double>::quiet_NaN();
		kirbAmount						= std::numeric_limits<double>::quiet_NaN();
	}

	// The list of column names in the LoanCashflowBreakdown structure
	std::vector<std::string> loanCashflowBreakdownColumns = 
		{	"Period",
			"BalanceStart",
			"DefaultBalance",
			"LossGivenLoss",
			"SeverityRecovered",
			"Prepayment",
			"BalanceAfterDefaultAndPrepayment",
			"LoanPayment",
			"InterestComponent",
			"PrincipalComponent",
			"BalanceEnd",
			"SeverityRecoveredNoLag",
			"RiskWeight",
			"KirbAmount" };
					

		// Converts a LoanAmortizationCashflows structure into an AnyTypeMatrix with headers, suitable for display
		AnyTypeMatrix displayLoanAmortizationCashflows( const LoanAmortizationCashflows& loanAmortisationCashflows )
		{
			AnyTypeVector headers;
			const size_t nColumns = loanCashflowBreakdownColumns.size();

			headers.reserve( nColumns );

			for ( size_t i=0; i<nColumns; i++ )
			{
				headers.push_back( loanCashflowBreakdownColumns[i] );
			}

			const size_t nPeriods = loanAmortisationCashflows.size();

			AnyTypeMatrix anyMatrix;
			anyMatrix.reserve( nPeriods + 1 );
			anyMatrix.push_back( headers );

			for ( size_t iPeriod=0; iPeriod<nPeriods; iPeriod++ )
			{
				AnyTypeVector body;
				body.reserve( nColumns );
			
				const auto&  loanCashflowBreakdown = loanAmortisationCashflows[ iPeriod ];

				body.push_back( (double) loanCashflowBreakdown.period );
				body.push_back( loanCashflowBreakdown.balanceStart );
				body.push_back( loanCashflowBreakdown.defaultBalance );
				body.push_back( loanCashflowBreakdown.lossGivenLoss );
				body.push_back( loanCashflowBreakdown.severityRecovered );
				body.push_back( loanCashflowBreakdown.prepayment );
				body.push_back( loanCashflowBreakdown.balanceAfterDefaultAndPrepayment );
				body.push_back( loanCashflowBreakdown.loanPayment );
				body.push_back( loanCashflowBreakdown.interestComponent );
				body.push_back( loanCashflowBreakdown.principalComponent );
				body.push_back( loanCashflowBreakdown.balanceEnd );
				body.push_back( loanCashflowBreakdown.severityRecoveredNoLag );
				body.push_back( loanCashflowBreakdown.riskWeight );
				body.push_back( loanCashflowBreakdown.kirbAmount );
				anyMatrix.push_back( body );
			}

			return anyMatrix;
		}

		// Utility function to populate a DataFrame with LoanAmortization results
		DataFrame loanAmortizationCashflowsToDataFrame( const LoanAmortizationCashflows& loanAmortisationCashflows )
		{
			const size_t nPeriods = loanAmortisationCashflows.size();
			const size_t nColumns = loanCashflowBreakdownColumns.size();

			VariantMatrix variantMatrix;
			variantMatrix.reserve( nPeriods );

			// At each timestep, create a row of data from the loanCashflowBreakdown structure,
			// then build up a variant matrix out of these rows.
			for (size_t iPeriod = 0; iPeriod < nPeriods; iPeriod++)
			{
				VariantVector row;
				row.reserve( nColumns );

				const auto&  loanCashflowBreakdown = loanAmortisationCashflows[iPeriod];

				row.push_back( (double)loanCashflowBreakdown.period );
				row.push_back( loanCashflowBreakdown.balanceStart );
				row.push_back( loanCashflowBreakdown.defaultBalance );
				row.push_back( loanCashflowBreakdown.lossGivenLoss );
				row.push_back( loanCashflowBreakdown.severityRecovered );
				row.push_back( loanCashflowBreakdown.prepayment );
				row.push_back( loanCashflowBreakdown.balanceAfterDefaultAndPrepayment );
				row.push_back( loanCashflowBreakdown.loanPayment );
				row.push_back( loanCashflowBreakdown.interestComponent );
				row.push_back( loanCashflowBreakdown.principalComponent );
				row.push_back( loanCashflowBreakdown.balanceEnd );
				row.push_back( loanCashflowBreakdown.severityRecoveredNoLag );
				row.push_back( loanCashflowBreakdown.riskWeight );
				row.push_back( loanCashflowBreakdown.kirbAmount );
				variantMatrix.push_back( row );
			}

			DataFrame dataFrame( loanCashflowBreakdownColumns, variantMatrix );
			return dataFrame;
		}

	// This method takes in another LoanCashflowBreakdown as "rhs", and adds all the field values into our own
	// Used to accumulate the cashflows from multiple loans into a single amortization table.
	void LoanCashflowBreakdown::accumulate( const LoanCashflowBreakdown& rhs )
	{
		balanceStart			+= rhs.balanceStart;
		defaultBalance			+= rhs.defaultBalance;
		lossGivenLoss			+= rhs.lossGivenLoss;
		severityRecovered		+= rhs.severityRecovered;
		prepayment				+= rhs.prepayment;
		balanceAfterDefaultAndPrepayment += rhs.balanceAfterDefaultAndPrepayment;
		loanPayment				+= rhs.loanPayment;
		interestComponent		+= rhs.interestComponent;
		principalComponent		+= rhs.principalComponent;
		balanceEnd				+= rhs.balanceEnd;
		severityRecoveredNoLag  += rhs.severityRecoveredNoLag;
		riskWeight				+= rhs.riskWeight;
		kirbAmount				+= rhs.kirbAmount;
	}

	/* @brief	Calculates the monthly payment for a loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	loanTypeEnum	Whether  the loan is amortizing or has bullet repayment.
	* @returns	The monthly loan payment
	*/
	double loanPayment( const int& term, const double& loanRate, const double& originalBalance, const LoanTypeEnum& loanTypeEnum )
	{
		double payment = 0.0;

		switch ( loanTypeEnum )
		{
		case AMORTIZING_LOAN_TYPE:
			payment = amortizingLoanPayment( term, loanRate, originalBalance );
			break;
		case BULLET_LOAN_TYPE:
			payment = 0.0;
			break;
		default:
			AQ_THROW( "Unsupported loanType: " + toString( loanTypeEnum ));
			break;
		}
		
		return payment;
	}

	/* @brief	Calculates the amount of loan principal which is repaid in a specified period of a loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	period			The period (month) of the loan under consideratrion
	* @param[in]	loanTypeEnum	Whether  the loan is amortizing or has bullet repayment.
	* @returns	The principal repayment in the specified period of the loan
	*/
	double loanPrincipal( const int& term, const double& loanRate, const double& originalBalance, const int& period, const LoanTypeEnum& loanTypeEnum )
	{
		
		double periodicPrincipal = 0.0;

		switch ( loanTypeEnum )
		{
		case AMORTIZING_LOAN_TYPE:
			periodicPrincipal = amortizingLoanPrincipal( term, loanRate, originalBalance, period );
			break;
		case BULLET_LOAN_TYPE:
			if ( period == term )
			{
				periodicPrincipal = originalBalance;
			}
			else
			{
				periodicPrincipal = 0.0;
			}
			break;
			
		default:
			AQ_THROW( "Unsupported loanType: " + toString( loanTypeEnum ));
			break;
		}
		
		return periodicPrincipal;
	}

	/* @brief	Calculates the amount of loan principal remaining at a specified period of a loan.
	* @param[in]	term			The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate		The annualized loan rate
	* @param[in]	originalBalance	The loan amount
	* @param[in]	period			The period (month) of the loan under consideratrion
	* @param[in]	loanTypeEnum	Whether  the loan is amortizing or has bullet repayment.
	* @returns	The principal remaining in the specified period of the loan
	*/
	double loanPrincipalRemaining( const int& term, const double& loanRate, const double& originalBalance, const int& period, const LoanTypeEnum& loanTypeEnum )
	{
		double loanPrincipalRemaining = 0.0;

		switch ( loanTypeEnum )
		{
		case AMORTIZING_LOAN_TYPE:
			loanPrincipalRemaining = amortizingLoanPrincipalRemaining( term, loanRate, originalBalance, period );
			break;
		case BULLET_LOAN_TYPE:
			if ( period < term )
			{
				loanPrincipalRemaining = originalBalance;
			}
			else
			{
				loanPrincipalRemaining = 0.0;
			}
			break;
		default:
			AQ_THROW( "Unsupported loanType: " + toString( loanTypeEnum ));
			break;
		}
		
		return loanPrincipalRemaining;
	
	}

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
	* @returns	A structure  containing the monthly loan cashflows and default amounts
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
																 const int loanPaymentsPerYear,
																 const bool cumulativeDefault,
																 const int recoveryLagInMonths,
																 const bool keepPostTermRecoveries )
	{
		
		const size_t resultRows = ((recoveryLagInMonths > 0) && keepPostTermRecoveries) ? term + recoveryLagInMonths : term;
		std::vector<LoanCashflowBreakdown> loanCashflows( resultRows );

		const double monthsInPeriod = 1.;
		const double periodsInYear = 12.;

		AQ_REQUIRE( term == defaultVector.size(), "Size of defaultVector (" + AQ_TO_STRING_FROM_SIZE_T(defaultVector.size()) + ") is inconsistent with the specified loan term (" + AQ_TO_STRING_FROM_INT(term) + ").");
		AQ_REQUIRE( term == severityVector.size(), "Size of severityVector (" + AQ_TO_STRING_FROM_SIZE_T(severityVector.size()) + ") is inconsistent with the specified loan term (" + AQ_TO_STRING_FROM_INT(term) + ").");
		AQ_REQUIRE( term == prepaymentVector.size(), "Size of prepaymentVector (" + AQ_TO_STRING_FROM_SIZE_T(prepaymentVector.size()) + ") is inconsistent with the specified loan term (" + AQ_TO_STRING_FROM_INT(term) + ").");
		AQ_REQUIRE( term == riskWeightVector.size(), "Size of riskWeightVector (" + AQ_TO_STRING_FROM_SIZE_T(riskWeightVector.size()) + ") is inconsistent with the specified loan term (" + AQ_TO_STRING_FROM_INT(term) + ").");
		AQ_REQUIRE( term == kirbVector.size(), "Size of kirbVector (" + AQ_TO_STRING_FROM_SIZE_T(kirbVector.size()) + ") is inconsistent with the specified loan term (" + AQ_TO_STRING_FROM_INT(term) + ").");

		const int stubMonths = term % loanPaymentsPerYear;

		for ( int iterPeriod = 0; iterPeriod < term; iterPeriod++ )
		{
			// Note: Here we obtain a non-const reference and update the vector in-place,
			// instead of pushing new items onto the vector. We need to do this because
			// cashflows are not updated sequentially when there is recovery-lag.
			LoanCashflowBreakdown& loanCashflow = loanCashflows[iterPeriod];
			loanCashflow.period = ( iterPeriod + 1 ); // Record the period starting at 1

			// Starting balance
			if ( iterPeriod == 0 )
			{
				loanCashflow.balanceStart = originalBalance;
			}
			else
			{
				const LoanCashflowBreakdown& prevCashflow = loanCashflows[iterPeriod - 1];
				loanCashflow.balanceStart = prevCashflow.balanceEnd;
			}
			const double balanceBegin = loanCashflow.balanceStart;

			// Defaults
			const double constantDefaultRate = defaultVector[ iterPeriod ];

			double defaultedBalance = 0.0;
			if ( cumulativeDefault )
			{
				defaultedBalance = originalBalance * (1. - std::pow((1. - constantDefaultRate), (monthsInPeriod / periodsInYear)));
			}
			else
			{
				defaultedBalance = balanceBegin * (1. - std::pow((1. - constantDefaultRate), (monthsInPeriod / periodsInYear)));
			}
			loanCashflow.defaultBalance = defaultedBalance;

			const double balanceAfterDefault = balanceBegin - defaultedBalance;
		
			// Loss Given Loss
			const double severity = severityVector[ iterPeriod ];
			const double lossGivenLoss = defaultedBalance * severity;
			loanCashflow.lossGivenLoss = lossGivenLoss;

			// Recovery
			loanCashflow.severityRecovered = 0.0;
			const double severityRecovered = defaultedBalance * ( 1 - severity );

			if ( iterPeriod < recoveryLagInMonths )
			{
				loanCashflow.severityRecovered = 0.0;

				if ( keepPostTermRecoveries )
				{
					loanCashflows[iterPeriod + term].period = iterPeriod + 1 + term;
				}
			}

			if ( iterPeriod + recoveryLagInMonths < term) 
			{
				loanCashflows[iterPeriod + recoveryLagInMonths].severityRecovered = severityRecovered;
			}
			else
			{
				if ( keepPostTermRecoveries )
				{
					loanCashflows[iterPeriod + recoveryLagInMonths].severityRecovered = severityRecovered;
				}
			}
			loanCashflow.severityRecoveredNoLag = severityRecovered;

			// Prepayments
			const double constantPrepaymentRate = prepaymentVector[ iterPeriod ];
			const double prepayment = balanceAfterDefault * (1. - std::pow( (1. - constantPrepaymentRate ), ( monthsInPeriod / periodsInYear ) ) );
			loanCashflow.prepayment = prepayment;

			// Balance after default and prepayment
			const double balanceAfterDefaultAndPrepayment = balanceAfterDefault - prepayment;
			loanCashflow.balanceAfterDefaultAndPrepayment = balanceAfterDefaultAndPrepayment;

			// Process Prepayments

			// Monthly payment
			// Term Loan B

			const bool accrualPaymentMonth = ((( iterPeriod + 1 ) - stubMonths) % loanPaymentsPerYear == 0 );

			if ( accrualPaymentMonth )
			{

				const double termRemaining = term - iterPeriod;
				const double loanPaymentAmount = loanPayment( termRemaining, loanRate, balanceAfterDefaultAndPrepayment, loanTypeEnum );
				loanCashflow.loanPayment = loanPaymentAmount;

				// Scheduled interest
				const double interestComponent = loanRate / 12.0 * balanceBegin;
				loanCashflow.interestComponent = interestComponent;

				// Scheduled principal
				const double initialLoanPeriod = 1.0;
				const double principalComponent = loanPrincipal( termRemaining, loanRate, balanceAfterDefaultAndPrepayment, initialLoanPeriod, loanTypeEnum );
				loanCashflow.principalComponent = principalComponent;
			}
			else
			{
				loanCashflow.loanPayment = 0.0;
				loanCashflow.interestComponent = 0.0;
				loanCashflow.principalComponent = 0.0;
			}


			// Ending balance
			const double balanceEnd = balanceAfterDefaultAndPrepayment - loanCashflow.principalComponent;
			loanCashflow.balanceEnd = balanceEnd;

			// Risk weight
			const double riskWeight = riskWeightVector[ iterPeriod ];
			loanCashflow.riskWeight = riskWeight;

			// KIRB
			const double kirbAmount = kirbVector[ iterPeriod ];
			loanCashflow.kirbAmount = kirbAmount;

		}
		
		return loanCashflows;
	}

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
	LoanAmortizationCashflows loanProjection(   const size_t resultRows,
												const double& originalBalance,
												const int loanTerm,
												const std::vector<double>& loanRateInEachMonth,
												const std::vector<double>& defaultRateInEachMonth,
												const std::vector<double>& severityRateInEachMonth,
												const std::vector<double>& prepaymentInEachMonth,
												const LoanTypeEnum loanType,
												const double& loanRiskWeight,
												const double& loanKirb,
												const int loanPaymentsPerYear,
												const bool cumulativeDefault,
												const int recoveryLagInMonths,
												const bool keepPostTermRecoveries)
	{
		AQ_REQUIRE( loanTerm >= 0, "Loan term must be zero or positive." );
		AQ_REQUIRE( resultRows >= (size_t) (loanTerm + recoveryLagInMonths ), "Results vector is not large enough to store projected loan. LoanTerm is: " << loanTerm << ", recoveryLag = " << recoveryLagInMonths );

		std::vector<LoanCashflowBreakdown> loanCashflows( resultRows );

		const double monthsInPeriod = 1.;
		const double periodsInYear = 12.;

		AQ_REQUIRE( loanTerm == loanRateInEachMonth.size(), "Size of loanRate vector (" + AQ_TO_STRING_FROM_SIZE_T(loanRateInEachMonth.size()) + ") is inconsistent with the specified loan term (" + AQ_TO_STRING_FROM_INT(loanTerm) + ")." );
		AQ_REQUIRE( loanTerm == defaultRateInEachMonth.size(), "Size of defaultRate vector (" + AQ_TO_STRING_FROM_SIZE_T(defaultRateInEachMonth.size()) + ") is inconsistent with the specified loan term (" + AQ_TO_STRING_FROM_INT(loanTerm) + ").");
		AQ_REQUIRE( loanTerm == severityRateInEachMonth.size(), "Size of severity vector (" + AQ_TO_STRING_FROM_SIZE_T(severityRateInEachMonth.size()) + ") is inconsistent with the specified loan term (" + AQ_TO_STRING_FROM_INT(loanTerm) + ").");
		AQ_REQUIRE( loanTerm == prepaymentInEachMonth.size(), "Size of prepayment vector (" + AQ_TO_STRING_FROM_SIZE_T(prepaymentInEachMonth.size()) + ") is inconsistent with the specified loan term (" + AQ_TO_STRING_FROM_INT(loanTerm) + ").");

		for (int monthIdx = 0; monthIdx < loanTerm; monthIdx++)
		{
			// Note: Here we obtain a non-const reference and update the vector in-place,
			// instead of pushing new items onto the vector. We need to do this because
			// cashflows are not updated sequentially when there is recovery-lag.
			LoanCashflowBreakdown& loanCashflow  = loanCashflows[ monthIdx ];

			// Period number
			loanCashflow.period = (monthIdx + 1); // Record the period starting at 1

			const double fractionalLoanRate = loanRateInEachMonth[monthIdx] / (12 / loanPaymentsPerYear);

			// used when the term it's not divisible by freq let's say a 10 months with apa
			const int stubMonths = loanTerm % loanPaymentsPerYear;

			// starting balance
			if (monthIdx == 0)
			{
				loanCashflow.balanceStart = originalBalance;
			}
			else
			{
				const LoanCashflowBreakdown& prevCashflow = loanCashflows[ monthIdx - 1 ];
				loanCashflow.balanceStart = prevCashflow.balanceEnd;
			}
			const double balanceBegin = loanCashflow.balanceStart;

			// process defaults
			const double constantDefaultRate = defaultRateInEachMonth[monthIdx];

			double defaultedBalance = 0.0;
			if ( cumulativeDefault )
			{
				defaultedBalance = originalBalance * (1. - std::pow((1. - constantDefaultRate), (monthsInPeriod / periodsInYear)));
			}
			else
			{
				defaultedBalance = balanceBegin * (1. - std::pow((1. - constantDefaultRate), (monthsInPeriod / periodsInYear)));
			}
			loanCashflow.defaultBalance = defaultedBalance;

			const double balanceAfterDefault = balanceBegin - defaultedBalance;

			// loss given loss
			const double severity = severityRateInEachMonth[monthIdx];
			const double lossGivenLoss = defaultedBalance * severity;
			loanCashflow.lossGivenLoss = lossGivenLoss;

			// recovery
			const double severityRecovered = defaultedBalance * (1 - severity);

			// diff <= to < |  cpp vectors start from 0, in R from 1
			if ( monthIdx < recoveryLagInMonths )
			{
				loanCashflow.severityRecovered = 0.0;

				if ( keepPostTermRecoveries )
				{
					loanCashflows[monthIdx + loanTerm].period = monthIdx + 1 + loanTerm;
				}
			}

			if ( monthIdx + recoveryLagInMonths < loanTerm )
			{
				loanCashflows[monthIdx + recoveryLagInMonths].severityRecovered = severityRecovered;
			}
			else
			{
				if ( keepPostTermRecoveries )
				{
					loanCashflows[monthIdx + recoveryLagInMonths].severityRecovered = severityRecovered;
				}
			}

			// severity recovered no lag
			loanCashflow.severityRecoveredNoLag = severityRecovered;

			// Process Prepayments
			const double constantPrepaymentRate = prepaymentInEachMonth[monthIdx];
			const double prepayment = balanceAfterDefault * (1. - std::pow((1. - constantPrepaymentRate), (monthsInPeriod / periodsInYear)));
			loanCashflow.prepayment = prepayment;

			// balance after default and prepayment
			const double balanceAfterDefaultAndPrepayment = balanceAfterDefault - prepayment;
			loanCashflow.balanceAfterDefaultAndPrepayment = balanceAfterDefaultAndPrepayment;

			const bool accrualPaymentMonth = (((monthIdx + 1) - stubMonths) % loanPaymentsPerYear == 0);

			switch (loanType)
			{
				case AMORTIZING_LOAN_TYPE:
				{
					if ( accrualPaymentMonth )
					{
						const double termRemaining = ( ( loanTerm - monthIdx + ( loanPaymentsPerYear - 1 ) ) / loanPaymentsPerYear );

						const double paymentFactor = std::pow((1 + fractionalLoanRate), termRemaining);

						loanCashflow.loanPayment = (balanceAfterDefaultAndPrepayment * paymentFactor ) * (fractionalLoanRate / (paymentFactor - 1) );

						// scheduled principal
						loanCashflow.principalComponent = balanceAfterDefaultAndPrepayment * (fractionalLoanRate * pow((1 + fractionalLoanRate), 0)) / (pow((1 + fractionalLoanRate), termRemaining) - 1);
					}
					else
					{
						loanCashflow.loanPayment = 0.0;
						loanCashflow.principalComponent = 0.0;
					}
					break;
				}
				
				case BULLET_LOAN_TYPE:
				{
					if ( accrualPaymentMonth )
					{
						loanCashflow.loanPayment = 0.0;

						// Repayment of principal at the end of the bullet loan
						if ( monthIdx + 1 == loanTerm )
						{
							loanCashflow.principalComponent = balanceAfterDefaultAndPrepayment;
						}
						else
						{
							loanCashflow.principalComponent = 0.0;
						}
					}
					else
					{
						loanCashflow.loanPayment = 0.0;
						loanCashflow.principalComponent = 0.0;
					}
					break;
				}
				default:
					AQ_THROW("Unsupported LoanType: " + toString(loanType));
			}

			// Scheduled interest
			if ( accrualPaymentMonth )
			{
				const double interestComponent = fractionalLoanRate * balanceBegin;
				loanCashflow.interestComponent = interestComponent;
			}
			else
			{
				loanCashflow.interestComponent = 0.0;
			}

			// Ending balance				
			const double balanceEnd = balanceAfterDefaultAndPrepayment - loanCashflow.principalComponent;
			loanCashflow.balanceEnd = balanceEnd;

			// Risk weight
			loanCashflow.riskWeight = balanceAfterDefaultAndPrepayment * loanRiskWeight;

			// KIRB
			loanCashflow.kirbAmount = balanceAfterDefaultAndPrepayment * loanKirb;

		} // Loop over months

		return loanCashflows;
	}
	
}

