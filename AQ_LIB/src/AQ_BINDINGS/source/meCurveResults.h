/* 
 * @brief			Swig interface for function meCurveResults...
 * @Created:		20th June 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <string>
#include <vector>
#include "SwigTypes.h"	// For SWIG_STRINGMATRIX which manages the LAStringMatrix idiosyncrasies type for R API

/* @brief			swig interface for meCurveDisplay
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		curveIndex			Index of the curve set
*  @return			StandardStringMatrix of paymentDates and corresponding discount factors from the yield curve
*/
SWIG_STRINGMATRIX meCurveResultsDisplayDiscountFactors (const std::string& curveCollection, const std::string& curveIndex );
