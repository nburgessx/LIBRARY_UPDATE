// SwapResultsContainer.h

/*
 * @brief			Swap Results Container Class
 */

#pragma once
#include <memory>           // Shared Pointer
#include <boost/thread.hpp> // shared_mutex
#include <map>
#include "Singleton.h"      // Singleton template
#include "SwapResults.h"
#include "CoreEnumerations.h"

// Macro to Clear Swap Results Container
#define AQ_CLEAR_SWAP_RESULTS_CACHE etrading::SwapResultsContainer::getInstance().deleteAllSwapResults(); 

namespace etrading
{
    // ----------------------------- HELPER METHODS -----------------------------------------

    // Helper Method to Check if Swap Results are enabled
    bool isEnabledSwapResults();
    
    // Helper Method to Check if Swap Results exist for the swap handle
    bool doesExistSwapResults( const std::string & swapHandle );
	
	// Helper Method to Check if Swap Risk Results exist for the given swap handle
    bool doesExistSwapRiskResults(const std::string & swapHandle );

    // ----------------------------- CLASS METHODS -----------------------------------------

    // SwapResultsContainer, A singleton that holds all the swap results for each swap collection, which is accessible via the getInstance() method - Uses lazy instantiation
    class SwapResultsContainer : public Singleton<SwapResultsContainer>
    {

    public:

        // Method to get the Swap Results Shared Pointer
        std::shared_ptr<SwapResults> getSwapResults( const std::string & swapHandle ) const;

        // Method to add a swap results object to the swap results container/cache
        void addSwapResults( const std::string & swapHandle, std::shared_ptr<SwapResults> swapResults );
        
        // Method to delete a swap results object from the swap results container/cache, will return true if successful and false otherwise if not found in cache or something similar
        bool deleteSwapResults( const std::string & swapHandle );
        
        // Method to delete all swap results objects from the swap results container/cache
        void deleteAllSwapResults();

        // Method to check if swap results object exists
        bool doesExist( const std::string & swapHandle ) const;

        // Set Enable flag to Enable/Disable Swap Results
        void enableSwapResults( const bool enable );
        
        // Check if Swap Results are Enabled
        bool isEnabled() const { return isEnabled_; }

    private:

        // Private Constructur - Singleton
        // Enable the swap results by default
        SwapResultsContainer() : isEnabled_( true ) {};
        
        // Singleton friend class
        friend Singleton<SwapResultsContainer>;

        // Copy Constructor and Assignment Operators should never be called
        SwapResultsContainer( const SwapResultsContainer& ) = delete;
        SwapResultsContainer & operator=( const SwapResultsContainer& ) = delete;

        // Mutex for results access when adding and removing swap results
        mutable boost::shared_mutex resultsAccess_;
        
        // Swap Results Map: Key = Swap Handle, Value = SwapResults
        std::map<std::string, std::shared_ptr<SwapResults> > swapResultsContainer_;

        bool isEnabled_;
    };

	// ----------------------------- HELPER CLASS -----------------------------------------

	// Helper Class to Disable Swap Results
	// Constructor disables the swap results and once out of scope the destructor will re-enable the swap results object
	// Use as follows: "DisableSwapResults d;"
	class DisableSwapResults
	{
		public:
			DisableSwapResults()	{ SwapResultsContainer::getInstance().enableSwapResults( false );	}
			~DisableSwapResults()	{ SwapResultsContainer::getInstance().enableSwapResults( true );	}
	};
}
