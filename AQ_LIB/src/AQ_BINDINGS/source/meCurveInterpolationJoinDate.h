/* 
 * @brief			Swig interface for function meCurveInterpolationJoinDate
 * @Created:		18 Nov 2017
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for meCurveInterpolationJoinDate
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		curveIndex			Index of the curve set
*  @param [in]		interpolation		Interpolation type
*  @return			Interpolation join date
*/
std::string meCurveInterpolationJoinDate(const std::string& curveCollection, const std::string& curveIndex, const std::string& interpolation) ;
