#pragma once

#include "AQLCoreTemplateType.h"

namespace etrading
{
	/* @brief			Method that removes a specified curve from object pool
    *  @param [in]		curveCollection		Name of curve collection
    *  @param [in]		curveIndex			Name of the curve to be removed from object pool
    *  @return	A message showing the output of the action
    */
	AQLString removeCurveFromEntityPool(const AQLString& curveCollection, const AQLString& curveIndex);

	/* @brief			Method that returns a boolean indicator, telling us whether the specified curve is registerd in the object pool or not
    *  @param [in]		curveCollection		Name of curve collection
    *  @param [in]		curveIndex			Name of the curve to be removed from object pool
    *  @return	A message showing the output of the action
    */
    const bool isCurveRegistered(const AQLString& curveCollection);
}