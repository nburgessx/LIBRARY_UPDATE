/* 
 * @brief			Swig interface to Java for function meUtilityInterpolation
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for meUtilityInterpolation
*  @param [in]		xValues			X-axis values of the data
*  @param [in]		yValues			Y-axis values of the data
*  @param [in]		xPoint			X-axis point
*  @param [in]		interpolation	Interpolation method
*  @return			the Y-axis interpolated value
*/
double meUtilityInterpolation(const std::vector<double>& array1, 
						const std::vector<double>& array2,
						double point,
						const std::string& interpolation);




