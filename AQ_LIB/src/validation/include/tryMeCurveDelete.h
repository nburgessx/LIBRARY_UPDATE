#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
	
	/* @brief			validation interface for meUtilityRemoveCurve
    *  @param [in]		curveCollection		Name of curve collection
    *  @param [in]		curveIndex			Name of the curve to be removed from object pool	
    *  @return	A message showing the output of the action
    */
	LAString tryMeCurveDelete(const LAString& curveCollection, const LAString& curveIndex);

}
