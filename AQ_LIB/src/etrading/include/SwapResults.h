// SwapResults.h

/*
 * @brief			Swap Results Class
 * @Created:		31st December 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#pragma once
#include <vector>
#include <memory>

#include "LADate.h"
#include "CoreEnumerations.h"
#include "LAInterpolationBase.h"
#include "Variant.h"

// Swap Result Components
#include "SwapRiskResults.h"


namespace etrading
{
    // Local anonymous namespace with typedefs for notational brevity
    namespace
    {
        // Typedefs
        typedef std::vector<double> VectorDouble;
        typedef std::vector<LADate> VectorDate;
        typedef std::vector<double> DoubleVector;   // For Backwards compatibility
        typedef std::vector<LADate> DateVector;     // For Backwards compatibility
    }

    // ================================= SWAP RESULTS CLASS ================================================================

    // Class to Store Swap Results; Please ensure this class remains thread-safe
    class SwapResults
    {

    public:

        // Constructor
        SwapResults() {};
        
        // Destructor
        ~SwapResults() {};
        
        // Copy Constructor
        SwapResults( const SwapResults & rhs );

		// Clone
		std::shared_ptr<SwapResults> clone() const;

        // Assignment Operator
        SwapResults & operator=( const SwapResults & rhs );

        // Swap Risk Constructor
        SwapResults( const std::shared_ptr<SwapRiskResults> & swapRiskResults );

        // Accessors
        std::shared_ptr<SwapRiskResults> swapRiskResults() const;
		
        // Helper Methods
        bool doesExistSwapRiskResults() const { return swapRiskResults_ != nullptr; }

    private:

        std::shared_ptr<SwapRiskResults> swapRiskResults_;
    };

}

