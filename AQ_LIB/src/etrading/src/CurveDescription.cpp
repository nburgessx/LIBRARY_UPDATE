// CurveDescription.cpp

/*
 * @brief			Curve Description Class
 * @Created:		15th July 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#include "CurveDescription.h"
#include "ExceptionMacros.h"
#include "CurveValidation.h"			        // Convert Dates to Terms and vice versa
#include "LAMathInterpolationUtilities.h"		// Get Interpolation Values
#include "CurveUtilities.h"				        // DateFromTenor methods
#include "LACurvePricingObject.h"	            // Methods to get the curve daycount conventions
#include "AQLEnumConversion.h"		            // Methods to convert enum values to legacy enums
#include "LADefinitions.h"                      // Needed for the delimiter curve index delimiter token "MULTI_STATIC_DATA_DELIMITER", which is typically a token

// Interpolation Methods
#include "AQLMonotoneConvexInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLParabolicInterpolation.h"
#include "AQLLinearSplineInterpolation.h"
#include "AQLLinearMonotoneSplineInterpolation.h"
#include "AQLConstrainedSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLLinearInterpolation.h"

namespace etrading
{
    
     // =============================== CURVE DESCRIPTION CLASS ==================================================================

    // Alternative Constructor
    CurveDescription::CurveDescription( const CurveTypeEnum & curveTypeEnum,
                                        const CurveTenorEnum & curveTenorEnum,
                                        const std::string & curveCollection,
                                        const std::string & indexName,
                                        const std::string & objectPoolLookupTable )
        :   curveTypeEnum_(curveTypeEnum),
            curveTenorEnum_(curveTenorEnum),
            curveCollection_(curveCollection),
            indexName_(indexName),
            objectPoolLookupTable_(objectPoolLookupTable)
    {
        curveTenor_ = toString( curveTenorEnum_ );
        curveType_ = toString( curveTypeEnum_ );
    }

    // Copy Constructor
    CurveDescription::CurveDescription( const CurveDescription& rhs ) 
        :   curveTypeEnum_(rhs.curveTypeEnum_),
            curveType_(rhs.curveType_),
            curveTenor_(rhs.curveTenor_),
            curveTenorEnum_(rhs.curveTenorEnum_),
            curveCollection_(rhs.curveCollection_),
            indexName_(rhs.indexName_),
            objectPoolLookupTable_(rhs.objectPoolLookupTable_)
	{
	}

	// Clone
	std::shared_ptr<CurveDescription> CurveDescription::clone() const
	{
		return std::make_shared<CurveDescription>( CurveDescription( *this ) );
	}

    // Assignment Operator
    CurveDescription & CurveDescription::operator=( const CurveDescription & rhs )
    {
		// For Performance 
		if ( &rhs == this )
		{
			return *this;
		}

        // For Exception Safety
            
        // 1. Make a temp copy
        CurveDescription temp( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( curveTypeEnum_,          temp.curveTypeEnum_ );
        std::swap( curveType_,              temp.curveType_ );
        std::swap( curveTenorEnum_,         temp.curveTenorEnum_ );
        std::swap( curveTenor_,             temp.curveTenor_ );
        std::swap( curveCollection_,        temp.curveCollection_ );
        std::swap( indexName_,              temp.indexName_ );
        std::swap( objectPoolLookupTable_,  temp.objectPoolLookupTable_ );
        
        return *this;
    }


    std::string CurveDescription::firstCurveIndex() const
    { 
        // Position of the first delimiter character
        std::size_t pos = indexName_.find( MULTI_STATIC_DATA_DELIMITER );
        
        if ( pos != std::string::npos )
        {
            // Delimiter Found

            // std::string::pos has base index position of zero
            size_t lengthOfFirstIndex = pos; 
            return indexName_.substr( 0, lengthOfFirstIndex ); // substring( start, length )
        }
        else
        {
            // Delimiter Not Found
            return indexName_;
        }

    }
}
