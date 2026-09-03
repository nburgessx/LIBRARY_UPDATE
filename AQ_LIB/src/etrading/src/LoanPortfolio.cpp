/*
 * @brief			Structured Credit Loan Portfolio
 *					Class to represent a portfolio of loans.
 *
 *					This code was ported from MGEN MHI R analytics library
 *					Author Andrew Friend
 *
 * @Created:		19th Nov 2019
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 */

#include "LoanPortfolio.h"
#include "LoanValidation.h"

#include "ExceptionMacros.h"
#include "DataUtilities.h"	// For MLIB_TO_STRING macros
#include <cmath>


namespace etrading
{

	/* LoanPortfolio is a class to represent a portfolio of loans
	*/

	// Main constructor
	LoanPortfolio::LoanPortfolio( const std::string& name, const size_t periods )
	{
		// Stress coefficients
		defaultStress_    = 1.0;
		severityStress_   = 1.0;
		prepaymentStress_ = 0.0;

		// default assumptions for missing parameterisations
		// TODO: Do you mind if I throw if these market data items are missing>?

		loanRateAssumption_  = 0.05;
		loanFreqAssumption_  = 1.0;
		cumulativeDefault_   = false;
		recoveryLagInMonths_ = 0;
		keepPostTermRecoveries_ = false;
		persistLoans_ = false;

		portfolioName_ = name;
		nPeriods_ = periods;
	}

	// Initializes the loan portfolio from a CSV file (known as a "tape")
	void LoanPortfolio::setPortfolioTape( const DataFrame& loanDataFromTape )
	{
		tapeData_ = loanDataFromTape;
	}

	// Returns the loan portfolio as a matrix, where each row of the matrix represents a separate loan definition.
	DataFrame LoanPortfolio::getPortfolioTape() const
	{
		return tapeData_;
	}

	// Rolls out the loans all the way to maturity, calculating the monthly interest and capital payments
	void LoanPortfolio::projectLoans()
	{
		
		preProcess( tapeData_ );

		auto loanAmortizaionCashflows = loansAmortization( tapeData_,
														   defaultStress_,
														   severityStress_,
														   prepaymentStress_,
														   cumulativeDefault_,
														   recoveryLagInMonths_,
														   keepPostTermRecoveries_,
														   defaultStressAddition_ );

		// Store the results of the projection in position 0 always
		// Do not simply call push_back() since projectedLoans_ is also used to store loan reinvestments
		if ( projectedLoans_.size() == 0 )
		{
			projectedLoans_.resize(1);
		}
		projectedLoans_[0] = loanAmortizaionCashflows;
	}

	// Helper method to apply initial processing to the set of loans
	void LoanPortfolio::preProcess( DataFrame& dataToCheck )
	{
		const std::string frequencyColumnName( STRUCTURED_CREDIT_CSV_KEYS::FREQ );
		if ( ! dataToCheck.hasColumn( frequencyColumnName ) )
		{
			// Loan frequency is unspecified; use tje assumed default value
			dataToCheck.addColumn( frequencyColumnName, loanFreqAssumption_ );
		}

		const std::string rateColumnName( STRUCTURED_CREDIT_CSV_KEYS::RATE );
		if ( ! dataToCheck.hasColumn( rateColumnName ) )
		{
			// Loan rate is unspecified; use the assumed default value
			dataToCheck.addColumn( rateColumnName, loanRateAssumption_ );
		}
	}

	// Specifies how reinvestment amounts should be allocated to new loans. Each new loan is represented by a RepLine or "Representative Line".
	// Each repline specifies which proportion of the capital should be invested in it.
	void LoanPortfolio::seedReinvestmentRepLines( const std::vector<RepLine>& repLines )
	{
		reinvestmentRepLines_ = repLines;
	}

	// Reinvests the specified amount into new loans at the specified time period.
	void LoanPortfolio::reinvestBalance( const size_t period, const double amount )
	{
		/* First calculate the total amount which is available to reinvest in this period.
		*  Note that the totalAmount quantity is not currently used;
		*  It is assumed that the input "amount" can be fully invested.
		*/
		const double totalToReinvest = calculatePrincipalCollection( period );

		// Where a stratified loan reinvestment portfolio has been defined use this as first choice
		if ( ! reinvestmentPortfolioTemplate_.empty() )
		{
			reinvestBalancePortfolio( period, amount );
		}
		else
		{
			// If there are no defined reinvestment rep lines we are in test mode and so we make a simple arbritary
			// loan asset definition and notify in the log
			if ( reinvestmentRepLines_.empty() )
			{
				// ... new_loan = loan_amortization_default_prepayment()
				MLIB_THROW( "No reinvestment portfolio or reinvestment RepLines available." );
			}
			else
			{
				reinvestBalanceRepLines( period, amount );
			}
		}
	}

	// Reinvests the specified amount into new loans at month "period".
	// Note you must first call method seedReinvestmentPortfolio to initialize a set of loans which will be used for reinvestment.
	void LoanPortfolio::reinvestBalancePortfolio( const size_t period, const double amount )
	{
		DataFrame reinvestmentPortfolio = reinvestmentPortfolioTemplate_;

		// Read the exposure at default column, and scale by the amount to invest
		const DoubleVector peadColumn = reinvestmentPortfolio.getColumnAsDoubleVector( STRUCTURED_CREDIT_CSV_KEYS::PEAD );

		VariantVector scaledPeadColumn;
		scaledPeadColumn.reserve( peadColumn.size() );
		for ( auto pead : peadColumn )
		{
			double scaledPead = pead * amount;
			scaledPeadColumn.push_back( scaledPead );
		}
		// Update column values
		reinvestmentPortfolio.addColumn( STRUCTURED_CREDIT_CSV_KEYS::PEAD, scaledPeadColumn );

		const int recoveryLag = 0;
		const bool cumulativeDefault = false;
		const bool keepPostTermRecoveries = false;
		auto loanAmortizationCashflows = loansAmortization( reinvestmentPortfolio,
															defaultStress_,
															severityStress_,
															prepaymentStress_,
															cumulativeDefault,
															recoveryLag,
															keepPostTermRecoveries,
															defaultStressAddition_ );

		storeReinvestmentResults( period, loanAmortizationCashflows );

		setReinvestmentBalance( period, amount );
	}

	// Reinvests the specified amount into new loans represented as RepLines (Representative Lines) at month "period".
	// Each RepLine specifies a loan definition, and a proportion of capital to be invested in that loan.
	// Note that you must first call method seedReinvestmentRepLines() to iniitalize the RepLines in the portfolio.
	void LoanPortfolio::reinvestBalanceRepLines( const size_t period, const double amount )
	{
		for (auto repLine : reinvestmentRepLines_)
		{
			repLine.setBalance( amount );
			const double allocatedBalance = repLine.getBalance() * repLine.getProportion();

			const double monthsToMaturity = repLine.getTerm();

			const std::vector<double> defaultRateVector( monthsToMaturity, repLine.getDefaultRate() );
			const std::vector<double> severityVector( monthsToMaturity, repLine.getSeverity() );
			const std::vector<double> prepaymentVector( monthsToMaturity, repLine.getPrepayment() );
			const std::vector<double> riskWeightVector( monthsToMaturity, repLine.getRiskWeight() );
			const std::vector<double> kirbVector( monthsToMaturity, repLine.getKirb() );

			auto loanAmortizationCashflows = loanAmortizationDefaultPrepayment( monthsToMaturity,
																			    repLine.getLoanRate(),
																			    allocatedBalance,
																			    defaultRateVector,
																			    severityVector,
																			    prepaymentVector,
																			    riskWeightVector,
																			    kirbVector,
																			    repLine.getLoanType() );

			storeReinvestmentResults( period, loanAmortizationCashflows );

			setReinvestmentBalance( period, amount );
		}
	}

	// --- Various Setter methods ---

	// Allocates storage for reinvestment balances up to the specified maximum time period
	void LoanPortfolio::initializeReinvestmentBalance( const size_t periods )
	{
		nPeriods_ = periods;
		reinvestmentBalance_.clear();
		reinvestmentBalance_.resize( periods, 0.0 );
		principalCollection_.clear();
		principalCollection_.resize( periods, 0.0 );
	}

	// Soecifies how reinvestment amounts should be allocated to new loans.
	// The loans are specified as a matrix (DataFrame) where each row of the matrix represents a loan definition.
	void LoanPortfolio::seedReinvestmentPortfolio( const DataFrame& reinvestmentPortfolio )
	{
		reinvestmentPortfolioTemplate_ = reinvestmentPortfolio;
	}

	// Initializes the portfolio with a set of loan cashflows in which the amortization payments have already been computed.
	void LoanPortfolio::seedPortfolio( const LoanAmortizationCashflows& loanAmortizaionCashflows )
	{
		if ( projectedLoans_.size() == 0 )
		{
			// Reserve space at the front of the vector for the regular loan projection results
			// Do not simply call push_back() since projectedLoans_ is also used to store loan reinvestments
			projectedLoans_.resize(1);
		}
		projectedLoans_[0] = loanAmortizaionCashflows;
	}

	// Helper method which stores reinvestment cashflows for reinvestment at the specified time period
	void LoanPortfolio::storeReinvestmentResults( const size_t period, const LoanAmortizationCashflows& loanAmortizationCashflows )
	{
		// The reinvestment in the loan starts after "period" months, and continues until maturity of the loan.

		const size_t numCashflows = loanAmortizationCashflows.size();

		const size_t rowsPriorToLoanStart = (period + 1);

		const size_t totalMonths = rowsPriorToLoanStart + numCashflows;
		std::vector<LoanCashflowBreakdown> loanReinvestmentCashflows;
		loanReinvestmentCashflows.reserve( totalMonths );
		
		// 1. Insert blank rows prior to loan start
		for (size_t monthIdx = 0; monthIdx < rowsPriorToLoanStart; monthIdx++)
		{
			LoanCashflowBreakdown cashflowBreakdownPriorToLoanStart;
			cashflowBreakdownPriorToLoanStart.initializeToNaN();
			cashflowBreakdownPriorToLoanStart.period = monthIdx + 1;
			loanReinvestmentCashflows.push_back( cashflowBreakdownPriorToLoanStart );
		}
		
		// 2. Copy loan cashflows
		for (size_t monthIdx = 0; monthIdx < numCashflows; monthIdx++)
		{
			LoanCashflowBreakdown cashflowBreakdown = loanAmortizationCashflows[monthIdx];
			cashflowBreakdown.period = ( monthIdx + 1 ) + rowsPriorToLoanStart;
			loanReinvestmentCashflows.push_back(cashflowBreakdown);
		}

		// Store the result
		if (projectedLoans_.size() == 0)
		{
			// Reserve space at the front of the vector for the regular loan projection results
			projectedLoans_.resize(1);
		}
		// Push reinvestment results /after/ the regular loan projections.
		projectedLoans_.push_back(loanReinvestmentCashflows);
	}

	void LoanPortfolio::clearAllReinvestmentResults()
	{
		if ( projectedLoans_.size() > 0 )
		{
			/* The loan projection is stored at position 0.
			 * The reinvestments are stored *after* position 0.
			 *
			 * So make a copy of the loan projection, then clear the projectedLoans_ container,
			 * and store the loan projection back into the empty container.
			 */

			LoanAmortizationCashflows projectedLoans = projectedLoans_[0];
			projectedLoans_.clear();
			projectedLoans_.push_back( projectedLoans ) ;
		}
	}

	// --- Various Getter methods ---

	// Get the series of loan cashflows all the way to maturity of the portfolio.
	// If loanNumber is zero (or omitted), returns the portfolio cashflows.
	// Specifying a loanNumber > 0 returns the amortization cashflows corresponding to loan reinvestment.
	LoanAmortizationCashflows LoanPortfolio::getLoanProjection( const size_t resultNumber ) const
	{
		MLIB_REQUIRE( resultNumber < projectedLoans_.size(), "No loan projection data at position " << resultNumber );

		return projectedLoans_[ resultNumber ];
	}

	// Get the series of loan cashflows all the way to maturity of the portfolio,
	// as an AnyTypeMatrix, suitable for display
	AnyTypeMatrix LoanPortfolio::getLoanProjectionAsAnyMatrix( const size_t resultNumber ) const
	{
		auto loanProjection = getLoanProjection( resultNumber );

		AnyTypeMatrix loanProjectionAsAnyMatrix  = displayLoanAmortizationCashflows( loanProjection );

		return loanProjectionAsAnyMatrix;
	}


	// Calculate the total amount of capital / principal available for reinvestment at the specified time period.
	double LoanPortfolio::calculatePrincipalCollection( const size_t period )
	{
		double totalAmount = 0.0;

		for ( auto loan : projectedLoans_ )
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if ( period < loan.size())
			{
				auto cashflowBreakdownForPeriod = loan[ period ];
				const double principalComponent = cashflowBreakdownForPeriod.principalComponent;
				if (!std::isnan( principalComponent ))
				{
					const double severityRecovered = cashflowBreakdownForPeriod.severityRecovered;
					const double prepayment = cashflowBreakdownForPeriod.prepayment;

					totalAmount += principalComponent + severityRecovered + prepayment;
				}
			}
		}

		return totalAmount;
	}

	// Get the portfolio balance outstanding at start of the specified time period
	double LoanPortfolio::getBalanceStart( const size_t period )
	{
		double totalAmount = 0.0;

		for ( auto loan : projectedLoans_ )
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if ( period < loan.size() )
			{
				auto cashflowBreakdownForPeriod = loan[ period ];
				const double balanceStart = cashflowBreakdownForPeriod.balanceStart;
				if ( ! std::isnan( balanceStart ))
				{
					totalAmount += balanceStart;
				}
			}
		}
		return totalAmount;
	}

	// Get the portfolio balance outstanding at the end of the specified time period.
	double LoanPortfolio::getBalanceEnd( const size_t period )
	{
		double totalAmount = 0.0;

		for ( auto loan : projectedLoans_ )
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if ( period < loan.size() )
			{

				auto cashflowBreakdownForPeriod = loan[ period ];
				const double balanceEnd = cashflowBreakdownForPeriod.balanceEnd;
				if ( ! std::isnan( balanceEnd ))
				{
					totalAmount += balanceEnd;
				}
			}
		}
		return totalAmount;
	}

	// Get the amount of defaulted balance at the specified time period.
	double LoanPortfolio::getDefaultBalance( const size_t period )
	{
		double totalAmount = 0.0;

		for ( auto loan : projectedLoans_ )
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if ( period < loan.size() )
			{
				auto cashflowBreakdownForPeriod = loan[ period ];
				const double defaultBalance = cashflowBreakdownForPeriod.defaultBalance;
				if ( ! std::isnan( defaultBalance ))
				{
					totalAmount += defaultBalance;
				}
			}
		}
		return totalAmount;
	}

	// The Loss Given Loss is a risk parameter which captures the uncertainty about the loss that will be realized
	double LoanPortfolio::getLossGivenLoss( const size_t period )
	{
		double totalAmount = 0.0;

		for (auto loan : projectedLoans_)
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if ( period < loan.size() )
			{
				auto cashflowBreakdownForPeriod = loan[ period ];
				const double lossGivenLoss = cashflowBreakdownForPeriod.lossGivenLoss;
				if ( ! std::isnan(lossGivenLoss ))
				{
					totalAmount += lossGivenLoss;
				}
			}
		}
		return totalAmount;
	}

	// Get the amount of loan recovered at the specified time period.
	double LoanPortfolio::getSeverityRecovered( const size_t period )
	{
		double totalAmount = 0.0;

		for (auto loan : projectedLoans_)
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if (period < loan.size() )
			{
				auto cashflowBreakdownForPeriod = loan[ period ];
				const double severityRecovered = cashflowBreakdownForPeriod.severityRecovered;
				if ( ! std::isnan(severityRecovered ))
				{
					totalAmount += severityRecovered;
				}
			}
		}
		return totalAmount;
	}

	// Get the amount of loan prepayment at the specified time period
	double LoanPortfolio::getPrepayment( const size_t period )
	{
		double totalAmount = 0.0;

		for (auto loan : projectedLoans_)
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if (period < loan.size() )
			{
				auto cashflowBreakdownForPeriod = loan[ period ];
				const double prepayment = cashflowBreakdownForPeriod.prepayment;
				if ( ! std::isnan( prepayment ))
				{
					totalAmount += prepayment;
				}
			}
		}
		return totalAmount;
	}

	// Get the remaining balance after loan default and prepayment are taken into account, at the specified time period.
	double LoanPortfolio::getBalanceAfterDefaultAndPrepayment( const size_t period )
	{
		double totalAmount = 0.0;

		for ( auto loan : projectedLoans_ )
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if ( period < loan.size() )
			{
				auto cashflowBreakdownForPeriod = loan[ period ];
				const double balanceAfterDefaultAndPrepayment = cashflowBreakdownForPeriod.balanceAfterDefaultAndPrepayment;
				if ( ! std::isnan( balanceAfterDefaultAndPrepayment ))
				{
					totalAmount += balanceAfterDefaultAndPrepayment;
				}
			}
		}
		return totalAmount;
	}

	// Get the amount of loan balance which is prepaid at the specified time period.
	double LoanPortfolio::getLoanPayment( const size_t period )
	{
		double totalAmount = 0.0;

		for (auto loan : projectedLoans_)
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if (period < loan.size() )
			{
				auto cashflowBreakdownForPeriod = loan[ period ];
				const double loanPayment = cashflowBreakdownForPeriod.loanPayment;
				if ( ! std::isnan( loanPayment ))
				{
					totalAmount += loanPayment;
				}
			}
		}
		return totalAmount;
	}

	// Get the amount loan interest due at the specified time period
	double LoanPortfolio::getInterestComponent( const size_t period )
	{
		double totalAmount = 0.0;

		for (auto loan : projectedLoans_)
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if (period < loan.size() )
			{
				auto cashflowBreakdownForPeriod = loan[ period ];
				const double interestComponent = cashflowBreakdownForPeriod.interestComponent;
				if ( ! std::isnan(interestComponent ))
				{
					totalAmount += interestComponent;
				}
			}
		}
		return totalAmount;
	}

	// Get the amount of loan due for repayment at the specified time period
	double LoanPortfolio::getPrincipalComponent( const size_t period )
	{
		double totalAmount = 0.0;

		for ( auto loan : projectedLoans_ )
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if (period < loan.size() )
			{
				auto cashflowBreakdownForPeriod = loan[ period ];
				const double principalComponent = cashflowBreakdownForPeriod.principalComponent;
				if ( ! std::isnan(principalComponent ))
				{
					totalAmount += principalComponent;
				}
			}
		}
		return totalAmount;

	}

	// Get the weighted exposure at default risk weight.
	// The risk weight is weighted by the balance at the end of the specified period.
	double LoanPortfolio::getWeadRiskWeight( const size_t period )
	{
		double totalAmount = 0.0;

		for (auto loan : projectedLoans_)
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if (period < loan.size() )
			{
				auto cashflowBreakdownForPeriod = loan[ period ];
				const double balanceEnd = cashflowBreakdownForPeriod.balanceEnd;
				const double riskWeight = cashflowBreakdownForPeriod.riskWeight;
				if ( ! std::isnan( riskWeight ) && ! std::isnan( balanceEnd ) )
				{
					totalAmount += riskWeight * balanceEnd;
				}
			}
		}
		return totalAmount;

	}

	// Get the weighted exposure at default capital requirement (KIRB).
	// The KIRB is weighted by the balance at the end of the specified period.
	double LoanPortfolio::getWeadKirb( const size_t period )
	{
		double totalAmount = 0.0;

		for ( auto loan : projectedLoans_ )
		{
			// Check if the period requested is off the end of the loan projection period
			// Our projectedLoans contains the accumulated quantities from all projected loans
			if (period < loan.size() )
			{
				auto cashflowBreakdownForPeriod = loan[ period - 1 ];
				const double balanceEnd = cashflowBreakdownForPeriod.balanceEnd;
				const double kirbAmount = cashflowBreakdownForPeriod.kirbAmount;
				if ( ! std::isnan(kirbAmount ) && ! std::isnan( balanceEnd ) )
				{
					totalAmount += kirbAmount * balanceEnd;
				}
			}
		}
		return totalAmount;
	}

	// Get the amount which has been reinvested into new loans, at the specified time period
	double LoanPortfolio::getReinvestmentBalance( const size_t period ) const
	{
		MLIB_REQUIRE( period < reinvestmentBalance_.size(), "Specified reinvestment period '" << period << "' is outside range of known reinvestment balances" );
		return reinvestmentBalance_[ period ];
	}

	// Records the amount of reinvestment made into new loans at the specified time period.
	void LoanPortfolio::setReinvestmentBalance( const size_t period, const double amount )
	{
		if (period >= reinvestmentBalance_.size() )
		{
			reinvestmentBalance_.resize( period + 1 );
		}
		reinvestmentBalance_[ period ] = amount;
	}

	// Get the amount of capital / principal available for reinvestment at the specified time period.
	double LoanPortfolio::getPrincipalCollection( const size_t period ) const
	{
		MLIB_REQUIRE(period < principalCollection_.size(), "Specified reinvestment period '" << period << "' is outside range of known principal collection");
		return principalCollection_[ period ];
	}

	// Set the amount of capital / principal available for reinvestment at the specified time period.
	void LoanPortfolio::setPrincipalCollection( const size_t period, const double amount )
	{
		if (period >= principalCollection_.size())
		{
			principalCollection_.resize( period + 1 );
		}
		principalCollection_[period ] = amount;
	}


	size_t LoanPortfolio::calculateMaximumLoanPeriod() const
	{
		size_t maxPeriod = 0;
		for ( auto loan : projectedLoans_ )
		{
			size_t loanMonths = loan.size();
			maxPeriod = std::max( maxPeriod, loanMonths );
		}
		return maxPeriod;
	}

	void PortfolioFlows::initialize( size_t nPeriods )
	{
		period.clear();									period.resize( nPeriods );
		periodDate.clear();								periodDate.resize( nPeriods );
		balanceStart.clear();							balanceStart.resize( nPeriods );
		balanceEnd.clear();								balanceEnd.resize( nPeriods );
		principalComponent.clear();						principalComponent.resize( nPeriods );
		severityRecovered.clear();						severityRecovered.resize( nPeriods );
		prepayment.clear();								prepayment.resize( nPeriods );
		lossGivenLoss.clear();							lossGivenLoss.resize( nPeriods );
		interestComponent.clear();						interestComponent.resize( nPeriods );
		reinvestmentBalance.clear();					reinvestmentBalance.resize( nPeriods );
		
		cumulativeDefaultBalance.clear();				cumulativeDefaultBalance.resize( nPeriods );
		cumulativeLossGivenLoss.clear();				cumulativeLossGivenLoss.resize( nPeriods );
		cumulativeSeverityRecovered.clear();			cumulativeSeverityRecovered.resize( nPeriods );
		
		cumulativeProportionDefaultBalance.clear();		cumulativeProportionDefaultBalance.resize( nPeriods );
		cumulativeProportionLossGivenLoss.clear();		cumulativeProportionLossGivenLoss.resize( nPeriods );
		cumulativeProportionSeverityRecovered.clear();	cumulativeProportionSeverityRecovered.resize( nPeriods );

	}


}

