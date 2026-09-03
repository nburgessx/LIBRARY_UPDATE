// CreditResults.h

/*
 * @brief			Credit Results Class
 * @Created:		9th Jan 2020
 * @Author:			Ian Castleton
 * @Department:		Quant Research and Analytics
 *
 */
#pragma once
#include <vector>
#include <memory>

#include "AQLDate.h"
#include "CoreEnumerations.h"

#include "Variant.h"

// Credit Result Components
#include "DataFrame.h"
#include "LoanPortfolio.h"
#include "LoanCalculations.h"

#include "Trigger.h"
#include "Tranche.h"
#include "SyntheticExcessSpread.h"
#include "CashflowEngine.h"


namespace etrading
{

    // ================================= CREDIT RESULTS CLASS ================================================================

    // Class to Store Credit Results; Please ensure this class remains thread-safe
    class CreditResults
    {

    public:

        // Constructor
        CreditResults() {};
        
        // Destructor
        ~CreditResults() {};
        
        // Copy Constructor
        CreditResults( const CreditResults & rhs );

		// Clone
		std::shared_ptr<CreditResults> clone() const;

        // Assignment Operator
        CreditResults & operator=( const CreditResults & rhs );

		CreditResults( const std::shared_ptr<DataFrame> & dataframe );

        // LoanPortfolio constructor
        CreditResults( const std::shared_ptr<LoanPortfolio> & loanPortfolio );

		// Store TrancheDefinition / Description
		CreditResults( const std::shared_ptr<TrancheDefinition> & trancheDefinition );

		// Store SyntheticExcessSpread
		CreditResults( const std::shared_ptr<SyntheticExcessSpread> & syntheticExcessSpread );

		// Store a trigger object
		CreditResults( const std::shared_ptr<Trigger> & trigger );

		// Store an entire CashflowEngine
		CreditResults( const std::shared_ptr<CashflowEngine> & cashflowEngine );

		// Store LoanAmortization results
		CreditResults( const std::shared_ptr<LoanAmortizationCashflows>& loanAmortizationCashflows );

        // Accessors
		std::shared_ptr<DataFrame> getDataFrame() const;

        std::shared_ptr<LoanPortfolio> getLoanPortfolio() const;
		
		std::shared_ptr<TrancheDefinition> getTrancheDefinition() const;

		std::shared_ptr<SyntheticExcessSpread> getSyntheticExcessSpread() const;

		std::shared_ptr<Trigger> getTrigger() const;

		std::shared_ptr<CashflowEngine> getCashflowEngine() const;

		std::shared_ptr<LoanAmortizationCashflows> getLoanAmortizationCashflows() const;

        // Helper Methods
		bool doesExistDataFrame() const { return dataFrame_ != nullptr; }

        bool doesExistLoanPortfolio() const { return loanPortfolio_ != nullptr; }

		bool doesExistTrancheDefinition() const { return trancheDefinition_ != nullptr; }

		bool doesExistSyntheticExcessSpread() const { return syntheticExcessSpread_ != nullptr; }

		bool doesExistTrigger() const { return trigger_ != nullptr; }

		bool doesExistCashflowEngine() const { return cashflowEngine_ != nullptr; }

		bool doesExistLoanAmortizationCashflows() const { return loanAmortizationCashflows_ != nullptr; }

    private:

		std::shared_ptr<DataFrame> dataFrame_;
        std::shared_ptr<LoanPortfolio> loanPortfolio_;
		std::shared_ptr<TrancheDefinition> trancheDefinition_;
		std::shared_ptr<SyntheticExcessSpread> syntheticExcessSpread_;
		std::shared_ptr<Trigger> trigger_;
		std::shared_ptr<CashflowEngine> cashflowEngine_;

		// Store amortization results.
		std::shared_ptr<LoanAmortizationCashflows> loanAmortizationCashflows_;

    };

}

