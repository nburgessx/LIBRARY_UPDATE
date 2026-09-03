#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
	
	/* @brief			validation interface for meUtilityRemoveCurve
    *  @param [in]		curveCollection		Name of curve collection
    *  @param [in]		curveIndex			Name of the curve to be removed from object pool	
    *  @return	A message showing the output of the action
    */
	AQLString tryMeCurveDelete(const AQLString& curveCollection, const AQLString& curveIndex);

}
