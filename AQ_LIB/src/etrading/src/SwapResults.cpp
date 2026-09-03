// SwapResults.cpp

/*
 * @brief			Swap Results Class
 * @Created:		31st December 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#include "SwapResults.h"
#include "ExceptionMacros.h"

namespace etrading
{
    
    // ================================= SWAP RESULTS CLASS ================================================================

    // Copy Constructor
    SwapResults::SwapResults( const SwapResults& rhs ) 
        : swapRiskResults_(rhs.swapRiskResults_)
	{
	}


    // Assignment Operator
    SwapResults & SwapResults::operator=( const SwapResults & rhs )
    {
		// For Performance
		if ( & rhs == this )
		{
			return *this;
		}

        // For Exception Safety
            
        // 1. Make a temp copy
        SwapResults temp( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( swapRiskResults_,	temp.swapRiskResults_	);

        return *this;
    }

	// Clone
	std::shared_ptr<SwapResults> SwapResults::clone() const
	{
		return std::make_shared<SwapResults>( SwapResults( *this ) );
	}

    // Main Constructor
    SwapResults::SwapResults( const std::shared_ptr<SwapRiskResults> & swapRiskResults )
        : swapRiskResults_(swapRiskResults)
    {
    }

    // Accessors
    std::shared_ptr<SwapRiskResults> SwapResults::swapRiskResults() const
    { 
		MLIB_REQUIRE( swapRiskResults_ != nullptr, "Invalid Swap Results: Swap Risk Results do not exist" )
        return swapRiskResults_;
    }

}
