/* 
 * @brief			Swig interface to Java for function mirInterpolation
 * @Created:		24 March 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for mirInterpolation
*  @param [in]		array1			X-axis values of the data to be interpolated
*  @param [in]		array2			Y-axis values of the data to be interpolated
*  @param [in]		point			X-axis point to interpolate
*  @param [in]		interpolation	Interpolation method of the yield curve
*  @return			the Y-axis interpolated value
*/
double mirInterpolation(const std::vector<double>& array1, 
						const std::vector<double>& array2,
						double point, 
						const std::string& interpolation) throw(std::exception);




