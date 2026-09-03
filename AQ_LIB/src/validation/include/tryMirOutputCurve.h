#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
{


    /* @brief			validation interface for the tryMirOutputCurve method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		curveName		Name of the curve set
    *  @return			Array of terms and corresponding discount factors from the yield curve
    */
    DoubleArray tryMirOutputCurve( AQLDataInstance* dataInstance,
                                   const AQLString& curveId,
                                   const AQLString& curveName );

}
