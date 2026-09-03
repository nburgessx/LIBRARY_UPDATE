#pragma once

#include "meCurveGetInterpolationJoinDate.h"
#include "AQLDate.h"
#include "AQLString.h"
#include "tryMeCurveGetInterpolationJoinDate.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for meCurveGetInterpolationJoinDate
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		curveIndex			Index of the curve set
*  @param [in]		interpolation		Interpolation type
*  @return			Interpolation join date
*/
std::string meCurveGetInterpolationJoinDate(const std::string& curveCollection, const std::string& curveIndex, const std::string& interpolation) throw(std::exception)
{
    AQ_API_START
	std::string ret;

	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());
	AQLString tmp_interpolation(interpolation.c_str());

	ret = validation::tryMeCurveGetInterpolationJoinDate(tmp_curveCollection, tmp_curveIndex, tmp_interpolation).stringWithFormat("YYYYMMDD").getCString();
	 
	return ret;
    AQ_API_END
}
