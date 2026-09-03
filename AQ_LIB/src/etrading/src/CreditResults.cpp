// CreditResults.cpp

/*
 * @brief			Credit Results Class
 * @Created:		9th Jan 2020
 * @Author:			Ian Castleton
 * @Department:		Quant Research and Analytics
 *
 */
#include "CreditResults.h"
#include "ExceptionMacros.h"

namespace etrading
{
    
    // ================================= CREDIT RESULTS CLASS ================================================================

    // Copy Constructor
    CreditResults::CreditResults( const CreditResults& rhs ) 
        :	dataFrame_( rhs.dataFrame_ ),
			loanPortfolio_( rhs.loanPortfolio_ ),
			trancheDefinition_( rhs.trancheDefinition_ ),
			syntheticExcessSpread_( rhs.syntheticExcessSpread_ ),
			trigger_( rhs.trigger_ ),
			cashflowEngine_( rhs.cashflowEngine_ ),
			loanAmortizationCashflows_( rhs.loanAmortizationCashflows_ )
	{
	}


    // Assignment Operator
    CreditResults & CreditResults::operator=( const CreditResults & rhs )
    {
		// For Performance
		if ( & rhs == this )
		{
			return *this;
		}

		dataFrame_			= rhs.dataFrame_;
		loanPortfolio_		= rhs.loanPortfolio_;
		trancheDefinition_	= rhs.trancheDefinition_;
		syntheticExcessSpread_ = rhs.syntheticExcessSpread_;
		trigger_			= rhs.trigger_;
		cashflowEngine_		= rhs.cashflowEngine_;
		loanAmortizationCashflows_ = rhs.loanAmortizationCashflows_;

        return *this;
    }

	// Clone
	std::shared_ptr<CreditResults> CreditResults::clone() const
	{
		return std::make_shared<CreditResults>( CreditResults( *this ) );
	}

    // Main Constructor
	CreditResults::CreditResults( const std::shared_ptr<DataFrame> & dataFrame )
		: dataFrame_( dataFrame )
	{
	}

	CreditResults::CreditResults( const std::shared_ptr<LoanPortfolio> & loanPortfolio )
		: loanPortfolio_( loanPortfolio )
	{
	}

	CreditResults::CreditResults( const std::shared_ptr<TrancheDefinition> & trancheDefinition )
		: trancheDefinition_( trancheDefinition )
	{
	}

	CreditResults::CreditResults(const std::shared_ptr<SyntheticExcessSpread> & syntheticExcessSpread )
		: syntheticExcessSpread_( syntheticExcessSpread )
	{
	}

	CreditResults::CreditResults( const std::shared_ptr<Trigger> & trigger )
		: trigger_( trigger )
	{
	}

	CreditResults::CreditResults( const std::shared_ptr<CashflowEngine>& cashflowEngine )
		: cashflowEngine_( cashflowEngine )
	{
	}

	CreditResults::CreditResults(const std::shared_ptr<LoanAmortizationCashflows>& loanAmortizationCashflows )
		: loanAmortizationCashflows_(loanAmortizationCashflows)
	{
	}

	// Accessors
	std::shared_ptr<DataFrame> CreditResults::getDataFrame() const
	{
		AQ_REQUIRE( dataFrame_ != nullptr, "Invalid Credit Results: DataFrame Results do not exist")
		return dataFrame_;
	}

	std::shared_ptr<LoanPortfolio> CreditResults::getLoanPortfolio() const
	{
		AQ_REQUIRE( loanPortfolio_ != nullptr, "Invalid Credit Results: LoanPortfolio Results do not exist" )
		return loanPortfolio_;
	}

	std::shared_ptr<TrancheDefinition> CreditResults::getTrancheDefinition() const
	{
		AQ_REQUIRE( trancheDefinition_ != nullptr, "Invalid Credit Results: TrancheDefinition Results do not exist" )
		return trancheDefinition_;
	}

	std::shared_ptr<SyntheticExcessSpread> CreditResults::getSyntheticExcessSpread() const
	{
		AQ_REQUIRE( syntheticExcessSpread_ != nullptr, "Invalid Credit Results: SyntheticExcessSpread Results do not exist")
		return syntheticExcessSpread_;
	}

	std::shared_ptr<Trigger> CreditResults::getTrigger() const
	{
		AQ_REQUIRE( trigger_ != nullptr, "Invalid Credit Results: Trigger Results do not exist" )
		return trigger_;
	}
    
	std::shared_ptr<CashflowEngine> CreditResults::getCashflowEngine() const
	{
		AQ_REQUIRE( cashflowEngine_ != nullptr, "Invalid Credit Results: CashflowEngine Results do not exist")
			return cashflowEngine_;
	}

	std::shared_ptr<LoanAmortizationCashflows> CreditResults::getLoanAmortizationCashflows() const
	{
		AQ_REQUIRE(loanAmortizationCashflows_ != nullptr, "Invalid Credit Results: LoanAmortizationCashflows Results do not exist")
		return loanAmortizationCashflows_;
	}
}
