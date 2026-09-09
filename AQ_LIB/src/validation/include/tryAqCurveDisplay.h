#pragma once

#include "Variant.h"
#include "AQLCoreTemplateType.h"

namespace validation
{


    /* @brief			validation interface for the aqCurveDisplay method
    *  @param [in]		curveCollection	ID of the curve set
    *  @param [in]		curveIndex		Index of the curve set
    *  @return			Array of terms and corresponding discount factors from the yield curve
    */
    DoubleArray tryAqCurveDisplay( const AQLString& curveCollection, const AQLString& curveIndex );

}
