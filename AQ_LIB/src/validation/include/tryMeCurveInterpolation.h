/*
 * @brief			validation interface for the tryMeCurveInterpolationJoinDate function
 * @Created:		18 Nov 2017
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
    /* @brief			validation interface for the tryMeCurveInterpolationJoinDate function, to clear the object pool
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		curveIndex			Index of the curve set
	*  @param [in]		interpolation		Interpolation type
	*  @return			Interpolation join date
    */
    LADate tryMeCurveInterpolationJoinDate(const LAString& curveCollection, const LAString& curveIndex, const LAString& interpolation = "");

}