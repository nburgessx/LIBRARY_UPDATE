#pragma once

#include <string>
#include <vector>
#include "SwigTypes.h"	// For SWIG_STRINGMATRIX which manages the AQLStringMatrix idiosyncrasies type for R API

/* @brief			swig interface for aqCurveDisplay
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		curveIndex			Index of the curve set
*  @return			StandardStringMatrix of paymentDates and corresponding discount factors from the yield curve
*/
SWIG_STRINGMATRIX aqCurvesResultsDisplayDiscountFactors (const std::string& curveCollection, const std::string& curveIndex );
