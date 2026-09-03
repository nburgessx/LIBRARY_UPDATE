/* 
 * @brief			Swig interface to Java for function mirOutputCurve
 * @Created:		31 March 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for mirOutputCurve
*  @param [in]		curveId			ID of the curve set
*  @param [in]		curveName		Name of the curve set
*  @return			Matrix of terms and corresponding discount factors from the yield curve
*/
std::vector<std::vector<double> > mirOutputCurve(const std::string& curveId,
								const std::string& curveName);