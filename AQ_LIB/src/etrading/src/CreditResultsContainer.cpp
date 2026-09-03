// CreditResultsContainer.cpp

/*
 * @brief			Credit Results Container Class
 * @Created:		9th Jan 2020
 * @Author:			Ian Castleton
 * @Department:		Quant Research and Analytics
 *
 */
#include "CreditResultsContainer.h"
#include "ExceptionMacros.h"

namespace etrading
{
    // ----------------------------- HELPER METHODS -----------------------------------------

    // Helper Method to Check if Credit Results are enabled
    bool isEnabledCreditResults()
    {
        return etrading::CreditResultsContainer::getInstance().isEnabled();
    }
    
    // Helper Method to Check if Credit Results exist for the Credit Handle
    bool doesExistCreditResults( const std::string & creditHandle )
    {
		// Don't throw here if credit results are disabled, just return false
		if( !isEnabledCreditResults() )
		{
			return false;
		}

        return etrading::CreditResultsContainer::getInstance().doesExist( creditHandle );
    }


	// Helper Method to Check if LoanPortfolio Results exist for the given credit handle
	bool doesExistLoanPortfolioResults( const std::string & creditHandle )
	{
		// Don't throw here if credit results disabled
		if( !isEnabledCreditResults() )
		{
			return false;
		}

        bool doesExistLoanPortfolio = false;
        if ( doesExistCreditResults( creditHandle ) )
        {
			doesExistLoanPortfolio = CreditResultsContainer::getInstance().getCreditResults( creditHandle )->doesExistLoanPortfolio();
        }
        return doesExistLoanPortfolio;
    }

	bool doesExistTrancheDefinitionResults(const std::string & creditHandle)
	{
		// Don't throw here if credit results disabled
		if (!isEnabledCreditResults())
		{
			return false;
		}

		bool doesExistTrancheDefinition = false;
		if (doesExistCreditResults(creditHandle))
		{
			doesExistTrancheDefinition = CreditResultsContainer::getInstance().getCreditResults(creditHandle)->doesExistTrancheDefinition();
		}
		return doesExistTrancheDefinition;
	}

	bool doesExistTriggerResults( const std::string & creditHandle )
	{
		// Don't throw here if credit results disabled
		if ( ! isEnabledCreditResults() )
		{
			return false;
		}

		bool doesExistTrigger = false;
		if (doesExistCreditResults(creditHandle))
		{
			doesExistTrigger = CreditResultsContainer::getInstance().getCreditResults(creditHandle)->doesExistTrigger();
		}
		return doesExistTrigger;
	}




    // ----------------------------- CLASS METHODS -----------------------------------------

    // Method to get the Credit Results Shared Pointer
    std::shared_ptr<CreditResults> CreditResultsContainer::getCreditResults( const std::string & creditHandle ) const
    {
		MLIB_REQUIRE( isEnabledCreditResults(), "Credit Results have been Disabled" )

        // Mutex Required for Thread-Safety - Results can get deleted or overwritten
        boost::shared_lock<boost::shared_mutex> lock( resultsAccess_ );

        std::string searchKey( creditHandle );
        auto it = creditResultsContainer_.find( searchKey );
        MLIB_REQUIRE( it != creditResultsContainer_.end(), "Credit Handle '" + creditHandle + "' does not exist" )
        return it->second;
    }
    
    // Method to add a credit results object to the credit results container/cache
    void CreditResultsContainer::addCreditResults( const std::string & creditHandle, std::shared_ptr<CreditResults> creditResults )
    {
		MLIB_REQUIRE( isEnabledCreditResults(), "Credit Results have been Disabled" )

        // Mutex Required to Write - Exclusive Access via unique_lock
        boost::unique_lock<boost::shared_mutex> uniqueLock( resultsAccess_ );

        std::string searchKey( creditHandle );
        creditResultsContainer_[searchKey] = creditResults;
    }
    
    // Method to delete a credit results object from the credit results container/cache, will return true if successful and false otherwise if not found in cache or something similar
    bool CreditResultsContainer::deleteCreditResults( const std::string & creditHandle )
    {
        // Mutex Required to Delete - Exclusive Access via unique_lock
        boost::unique_lock<boost::shared_mutex> uniqueLock( resultsAccess_ );
        
		bool creditResultsDeleted = false;

		// Delete Credit Results
		std::string searchKey( creditHandle );
		auto it = creditResultsContainer_.find( searchKey );

		if (it != creditResultsContainer_.end())
		{
			creditResultsContainer_.erase(it);
			creditResultsDeleted = true;
		}

        return creditResultsDeleted;
    }

    // Method to delete all credit results objects from the credit results container/cache
    void CreditResultsContainer::deleteAllCreditResults()
    {
        // Mutex Required to Delete - Exclusive Access via unique_lock
        boost::unique_lock<boost::shared_mutex> uniqueLock( resultsAccess_ );
        
        creditResultsContainer_.clear();
    }

    // Method to check if credit results object exists
    bool CreditResultsContainer::doesExist( const std::string & creditHandle ) const
    {
        // Mutex Required for Thread-Safety - Results can get deleted or overwritten
        boost::shared_lock<boost::shared_mutex> lock( resultsAccess_ );

        const std::string searchKey( creditHandle );
        auto it = creditResultsContainer_.find( searchKey );
        bool doesCreditResultExist =  it!=creditResultsContainer_.end();
        return doesCreditResultExist;
    }

    // Enable/Disable Credit Results
    void CreditResultsContainer::enableCreditResults( const bool enable )
    {
        // Mutex Required to Write - Allow Shared Read Access but Prevent Read Mutex Upgrades to Write
        boost::upgrade_lock<boost::shared_mutex> lock( resultsAccess_ );

        isEnabled_ = enable;
    }

}
