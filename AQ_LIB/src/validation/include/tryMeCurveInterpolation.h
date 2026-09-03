#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
    /* @brief			validation interface for the tryMeCurveInterpolationJoinDate function, to clear the object pool
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		curveIndex			Index of the curve set
	*  @param [in]		interpolation		Interpolation type
	*  @return			Interpolation join date
    */
    AQLDate tryMeCurveInterpolationJoinDate(const AQLString& curveCollection, const AQLString& curveIndex, const AQLString& interpolation = "");

}