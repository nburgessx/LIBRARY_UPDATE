/* 
 * @brief			Swig interface to Java for function meCurveDisplay
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for meCurveDisplay
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		curveIndex			Index of the curve set
*  @return			Matrix of terms and corresponding discount factors from the yield curve
*/
std::vector<std::vector<double> > meCurveDisplay(const std::string& curveCollection, const std::string& curveIndex);