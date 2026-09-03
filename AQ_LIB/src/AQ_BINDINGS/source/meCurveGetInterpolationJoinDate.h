/* 
 * @brief			Swig interface for function meCurveGetInterpolationJoinDate
 * @Created:		18 Nov 2017
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for meCurveGetInterpolationJoinDate
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		curveIndex			Index of the curve set
*  @param [in]		interpolation		Interpolation type
*  @return			Interpolation join date
*/
std::string meCurveGetInterpolationJoinDate(const std::string& curveCollection, const std::string& curveIndex, const std::string& interpolation);
