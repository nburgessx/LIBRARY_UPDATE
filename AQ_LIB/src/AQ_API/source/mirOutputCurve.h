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