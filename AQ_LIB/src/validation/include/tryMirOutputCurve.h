/*
 * @brief			validation interface for output curve method(s)
 * @Created:		31 March 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
{


    /* @brief			validation interface for the tryMirOutputCurve method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		curveName		Name of the curve set
    *  @return			Array of terms and corresponding discount factors from the yield curve
    */
    DoubleArray tryMirOutputCurve( LADataInstance* dataInstance,
                                   const LAString& curveId,
                                   const LAString& curveName );

}
