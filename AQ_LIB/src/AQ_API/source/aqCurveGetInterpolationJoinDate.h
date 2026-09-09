#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqCurveGetInterpolationJoinDate
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		curveIndex			Index of the curve set
*  @param [in]		interpolation		Interpolation type
*  @return			Interpolation join date
*/
std::string aqCurveGetInterpolationJoinDate(const std::string& curveCollection, const std::string& curveIndex, const std::string& interpolation);
