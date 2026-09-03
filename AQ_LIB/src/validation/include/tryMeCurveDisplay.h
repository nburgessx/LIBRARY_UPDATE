/*
 * @brief			validation interface for meCurveDisplay method(s)
 * @Created:		19 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "Variant.h"
#include "LACoreTemplateType.h"

namespace validation_api
{


    /* @brief			validation interface for the meCurveDisplay method
    *  @param [in]		curveCollection	ID of the curve set
    *  @param [in]		curveIndex		Index of the curve set
    *  @return			Array of terms and corresponding discount factors from the yield curve
    */
    DoubleArray tryMeCurveDisplay( const LAString& curveCollection, const LAString& curveIndex );

}
