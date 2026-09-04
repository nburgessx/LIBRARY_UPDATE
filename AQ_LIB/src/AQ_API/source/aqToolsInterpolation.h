#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqToolsInterpolation
*  @param [in]		xValues			X-axis values of the data
*  @param [in]		yValues			Y-axis values of the data
*  @param [in]		xPoint			X-axis point
*  @param [in]		interpolation	Interpolation method
*  @return			the Y-axis interpolated value
*/
double aqToolsInterpolation(const std::vector<double>& array1, 
						const std::vector<double>& array2,
						double point,
						const std::string& interpolation);




