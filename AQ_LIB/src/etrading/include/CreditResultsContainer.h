// CreditResultsContainer.h

/*
 * @brief			Credit Results Container Class
 * @Created:		9th Jan 2020
 * @Author:			Ian Castleton
 * @Department:		Quant Research and Analytics
 *
 */

#pragma once
#include <memory>           // Shared Pointer
#include <boost/thread.hpp> // shared_mutex
#include <map>
#include "Singleton.h"      // Singleton template
#include "CreditResults.h"
#include "CoreEnumerations.h"

// Macro to cler Credit Results Container
#define AQ_CLEAR_CREDIT_RESULTS_CACHE etrading::CreditResultsContainer::getInstance().deleteAllCreditResults(); 

namespace etrading
{
    // ----------------------------- HELPER METHODS -----------------------------------------


    // Helper Method to Check if Credit Results are enabled
    bool isEnabledCreditResults();
 
	// Helper Method to Check if LoanPortfolio results exist for the given credit handle
    bool doesExistLoanPortfolioResults( const std::string & creditHandle );

	// Helper Method to Check if TrancheDefinition results exist for the credit handle
	bool doesExistTrancheDefinitionResults( const std::string & creditHandle);

	// Helper Method to Check if Trigger results exist for the credit handle
	bool doesExistTriggerResults( const std::string & creditHandle);
	
    // ----------------------------- CLASS METHODS -----------------------------------------

    // CreditResultsContainer, A singleton that holds all the credit results for each credit collection, which is accessible via the getInstance() method - Uses lazy instantiation
    class CreditResultsContainer : public Singleton<CreditResultsContainer>
    {

    public:

        // Method to get the Credit Results Shared Pointer
        std::shared_ptr<CreditResults> getCreditResults( const std::string & creditResultsHandle ) const;

        // Method to add a credit results object to the credit results container/cache
        void addCreditResults( const std::string & creditResultsHandle, std::shared_ptr<CreditResults> creditResults );
        
        // Method to delete a credit results object from the credit results container/cache, will return true if successful and false otherwise if not found in cache or something similar
        bool deleteCreditResults( const std::string & creditHandle );
        
        // Method to delete all credit results objects from the credit results container/cache
        void deleteAllCreditResults();

        // Method to check if credit results object exists
        bool doesExist( const std::string & creditHandle ) const;

        // Set Enable flag to Enable/Disable Credit Results
        void enableCreditResults( const bool enable );
        
        // Check if Credit Results are Enabled
        bool isEnabled() const { return isEnabled_; }

    private:

        // Private Constructur - Singleton
        // Enable the credit results by default
        CreditResultsContainer() : isEnabled_( true ) {};
        
        // Singleton friend class
        friend Singleton<CreditResultsContainer>;

        // Copy Constructor and Assignment Operators should never be called
        CreditResultsContainer( const CreditResultsContainer& ) = delete;
        CreditResultsContainer & operator=( const CreditResultsContainer& ) = delete;

        // Mutex for results access when adding and removing credit results
        mutable boost::shared_mutex resultsAccess_;
        
        // Credit Results Map: Key = Credit Handle, Value = CreditResults
        std::map<std::string, std::shared_ptr<CreditResults> > creditResultsContainer_;

        bool isEnabled_;
    };

	// ----------------------------- HELPER CLASS -----------------------------------------

	// Helper Class to Disable Credit Results
	// Constructor disables the credit results and once out of scope the destructor will re-enable the credit results object
	// Use as follows: "DisableCreditResults d;"
	class DisableCreditResults
	{
		public:
			DisableCreditResults()	{ CreditResultsContainer::getInstance().enableCreditResults( false );	}
			~DisableCreditResults()	{ CreditResultsContainer::getInstance().enableCreditResults( true );	}
	};
}
