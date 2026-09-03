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
						const std::string& interpolation);




