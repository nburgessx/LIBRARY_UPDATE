#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqCurvesDisplay
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		curveIndex			Index of the curve set
*  @return			Matrix of terms and corresponding discount factors from the yield curve
*/
std::vector<std::vector<double> > aqCurvesDisplay(const std::string& curveCollection, const std::string& curveIndex);