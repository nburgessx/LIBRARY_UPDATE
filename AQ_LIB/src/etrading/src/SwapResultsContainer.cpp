// SwapResultsContainer.cpp

/*
 * @brief			Swap Results Container Class
 */
#include "SwapResultsContainer.h"
#include "ExceptionMacros.h"

namespace etrading
{
    // ----------------------------- HELPER METHODS -----------------------------------------

    // Helper Method to Check if Swap Results are enabled
    bool isEnabledSwapResults()
    {
        return etrading::SwapResultsContainer::getInstance().isEnabled();
    }
    
    // Helper Method to Check if Swap Results exist for the Swap Handle
    bool doesExistSwapResults( const std::string & swapHandle )
    {
		// Don't throw here if swap results are disabled, just return false
		if( !isEnabledSwapResults() )
		{
			return false;
		}

        return etrading::SwapResultsContainer::getInstance().doesExist( swapHandle );
    }

	// Helper Method to Check if Swap Risk Results exist for the given swap handle
	bool doesExistSwapRiskResults( const std::string & swapHandle )
	{
		// Don't throw here if swap results disabled
		if( !isEnabledSwapResults() )
		{
			return false;
		}

        bool doesExistSwapRiskResults = false;
        if ( doesExistSwapResults( swapHandle ) )
        {
            doesExistSwapRiskResults = etrading::SwapResultsContainer::getInstance().getSwapResults( swapHandle )->doesExistSwapRiskResults();
        }
        return doesExistSwapRiskResults;
    }

    // ----------------------------- CLASS METHODS -----------------------------------------

    // Method to get the Swap Results Shared Pointer
    std::shared_ptr<SwapResults> SwapResultsContainer::getSwapResults( const std::string & swapHandle ) const
    {
		AQ_REQUIRE( isEnabledSwapResults(), "Swap Results have been Disabled" )

        // Mutex Required for Thread-Safety - Results can get deleted or overwritten
        boost::shared_lock<boost::shared_mutex> lock( resultsAccess_ );

        std::string searchKey( swapHandle );
        auto it = swapResultsContainer_.find(searchKey);
        AQ_REQUIRE( it != swapResultsContainer_.end(), "Swap Handle '" + swapHandle + "' does not exist" )
        return it->second;
    }
    
    // Method to add a swap results object to the swap results container/cache
    void SwapResultsContainer::addSwapResults( const std::string & swapHandle, std::shared_ptr<SwapResults> swapResults )
    {
		AQ_REQUIRE( isEnabledSwapResults(), "Swap Results have been Disabled" )

        // Mutex Required to Write - Exclusive Access via unique_lock
        boost::unique_lock<boost::shared_mutex> uniqueLock( resultsAccess_ );

        std::string searchKey( swapHandle );
        swapResultsContainer_[searchKey] = swapResults;
    }
    
    // Method to delete a swap results object from the swap results container/cache, will return true if successful and false otherwise if not found in cache or something similar
    bool SwapResultsContainer::deleteSwapResults( const std::string & swapHandle )
    {
        // Mutex Required to Delete - Exclusive Access via unique_lock
        boost::unique_lock<boost::shared_mutex> uniqueLock( resultsAccess_ );
        
		bool swapResultsDeleted = false;

		// Delete Swap Results
		std::string searchKey( swapHandle );
		auto it = swapResultsContainer_.find(searchKey);

		if (it != swapResultsContainer_.end())
		{
			swapResultsContainer_.erase(it);
			swapResultsDeleted = true;
		}

        return swapResultsDeleted;
    }

    // Method to delete all swap results objects from the swap results container/cache
    void SwapResultsContainer::deleteAllSwapResults()
    {
        // Mutex Required to Delete - Exclusive Access via unique_lock
        boost::unique_lock<boost::shared_mutex> uniqueLock( resultsAccess_ );
        
        swapResultsContainer_.clear();
    }

    // Method to check if swap results object exists
    bool SwapResultsContainer::doesExist( const std::string & swapHandle ) const
    {
        // Mutex Required for Thread-Safety - Results can get deleted or overwritten
        boost::shared_lock<boost::shared_mutex> lock( resultsAccess_ );

        const std::string searchKey( swapHandle );
        auto it = swapResultsContainer_.find(searchKey);
        bool doesSwapExist =  it!=swapResultsContainer_.end();
        return doesSwapExist;
    }

    // Enable/Disable Swap Results
    void SwapResultsContainer::enableSwapResults( const bool enable )
    {
        // Mutex Required to Write - Allow Shared Read Access but Prevent Read Mutex Upgrades to Write
        boost::upgrade_lock<boost::shared_mutex> lock( resultsAccess_ );

        isEnabled_ = enable;
    }

}