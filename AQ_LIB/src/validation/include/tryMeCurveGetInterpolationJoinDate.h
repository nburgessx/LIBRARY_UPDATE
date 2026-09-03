#pragma once

#include "LACoreTemplateType.h"

namespace validation
{
    /* @brief			validation interface for the tryMeCurveGetInterpolationJoinDate function, to clear the object pool
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		curveIndex			Index of the curve set
	*  @param [in]		interpolation		Interpolation type
	*  @return			Interpolation join date
    */
    LADate tryMeCurveGetInterpolationJoinDate(const LAString& curveCollection, const LAString& curveIndex, const LAString& interpolation = "");

}