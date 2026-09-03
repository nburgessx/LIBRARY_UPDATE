// CurveGroup.cpp

/*
 * @brief			Curve Group Class
 * @Created:		4th February 2020
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "CurveGroup.h"
#include "ExceptionMacros.h"
#include "CurveValidation.h"			        // Convert Dates to Terms and vice versa
#include "LADefinitions.h"                      // Needed for the delimiter curve index delimiter token "MULTI_STATIC_DATA_DELIMITER", which is typically a token
#include "LWOUtilities.h"						// Undecorate LWO Handles and getCurveCollection method
#include "CoreEnumerations.h"					// For trim_to_upper() method

namespace etrading
{
    
     // =============================== CURVE GROUP CLASS ==================================================================

    // Main Constructor - Curve Groups Hold Curve Handles for a Single Currency and for a Single Curve Collection
    CurveGroup::CurveGroup( const std::vector<std::string> & curveHandles )
    {
		// Reserve Dimensions for Performance
		curveHandles_.reserve( curveHandles.size() );
		
		for( size_t i = 0; i < curveHandles.size(); ++i )
		{
			// Undecorate Handle - Remove instance vounter and make uppercase
			const std::string undecoratedHandle = trim_to_upper( undecorateHandle( curveHandles[i] ) );
			
			// *** IMPORTANT Legacy Feature *** Curve Collection Names are Case Sensitive - no uppercasing please.
			const std::string curveCollection = getCurveCollectionFromHandle( undecoratedHandle );

			// Update Member Data
			curveHandles_.push_back( undecoratedHandle );
			
			if ( i == 0 )
			{
				curveCollection_ = curveCollection;
			}
			else
			{
				// Validation - Check all curve handles have the same curve currency and curve collection
				AQ_REQUIRE( curveCollection == curveCollection_, "Invalid Curve Group: Curve Handles within the group must have the same curve currency and curve collection" )
			}
		}
		
    }

    // Copy Constructor
    CurveGroup::CurveGroup( const CurveGroup& rhs ) 
        :   curveHandles_(rhs.curveHandles_),
            curveCollection_(rhs.curveCollection_)
	{
	}

	// Clone
	std::shared_ptr<CurveGroup> CurveGroup::clone() const
	{
		return std::make_shared<CurveGroup>( CurveGroup( *this ) );
	}

    // Assignment Operator
    CurveGroup & CurveGroup::operator=( const CurveGroup & rhs )
    {
        // For Exception Safety
            
        // 1. Make a temp copy
        CurveGroup temp( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( curveHandles_,           temp.curveHandles_ );
        std::swap( curveCollection_,       temp.curveCollection_ );
        
        return *this;
    }

}
