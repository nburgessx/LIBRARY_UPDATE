/*
 * @brief			Utility methods addressing issues to do with object pool
 * @Created:		09 August 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"

namespace etrading
{
	/* @brief			Method that removes a specified curve from object pool
    *  @param [in]		curveCollection		Name of curve collection
    *  @param [in]		curveIndex			Name of the curve to be removed from object pool
    *  @return	A message showing the output of the action
    */
	LAString removeCurveFromEntityPool(const LAString& curveCollection, const LAString& curveIndex);

	/* @brief			Method that returns a boolean indicator, telling us whether the specified curve is registerd in the object pool or not
    *  @param [in]		curveCollection		Name of curve collection
    *  @param [in]		curveIndex			Name of the curve to be removed from object pool
    *  @return	A message showing the output of the action
    */
    const bool isCurveRegistered(const LAString& curveCollection);
}