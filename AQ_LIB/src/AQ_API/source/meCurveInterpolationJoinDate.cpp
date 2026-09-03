#pragma once

#include "meCurveInterpolationJoinDate.h"
#include "LADate.h"
#include "LAString.h"
#include "tryMeCurveInterpolation.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for meCurveInterpolationJoinDate
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		curveIndex			Index of the curve set
*  @param [in]		interpolation		Interpolation type
*  @return			Interpolation join date
*/
std::string meCurveInterpolationJoinDate(const std::string& curveCollection, const std::string& curveIndex, const std::string& interpolation)
{
    AQ_API_START
	std::string ret;

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_interpolation(interpolation.c_str());

	ret = validation::tryMeCurveInterpolationJoinDate(tmp_curveCollection, tmp_curveIndex, tmp_interpolation).stringWithFormat("YYYYMMDD").getCString();
	 
	return ret;
    AQ_API_END
}
