#pragma once

#include "Variant.h"
#include "LACoreTemplateType.h"

namespace validation
{


    /* @brief			validation interface for the meCurveDisplay method
    *  @param [in]		curveCollection	ID of the curve set
    *  @param [in]		curveIndex		Index of the curve set
    *  @return			Array of terms and corresponding discount factors from the yield curve
    */
    DoubleArray tryMeCurveDisplay( const LAString& curveCollection, const LAString& curveIndex );

}
