// aqToolDiagnostics.h

/*
 * @brief			Swig interface for small Tool-category utility functions: aqToolEchoDouble,
 *					aqToolBondAverageYield, aqToolBondYieldFromFuturePrice and aqToolSwapScheduleTemplate.
 *					(The latter three are golden-named Tool but conceptually product-specific - filed
 *					here alongside the other stateless Tool utilities, matching where they are coded
 *					in AQ_XLL's xllTool.cpp.)
 */

#pragma once

#include <string>
#include <vector>

#include "SwigTypes.h"	// For SWIG_STRINGMATRIX which manages the AQLStringMatrix idiosyncrasies type for R API

/* @brief			swig interface for aqToolEchoDouble. Echo a double straight back through the validation layer.
*  @param [in]		value		Any number
*  @return			The same number
*/
double aqToolEchoDouble( const double& value );

/* @brief			swig interface for aqToolBondAverageYield. The average of several underlying bond yields.
*  @param [in]		underlyingBondYields	A vector of bond yields to average
*  @return			The average yield
*/
double aqToolBondAverageYield( const std::vector<double>& underlyingBondYields );

/* @brief			swig interface for aqToolBondYieldFromFuturePrice. A bond yield implied by a bond-future price.
*  @param [in]		futurePrice		The bond-future price
*  @return			The implied bond yield
*/
double aqToolBondYieldFromFuturePrice( const double& futurePrice );

/* @brief			swig interface for aqToolSwapScheduleTemplate. A template of a swap's floating/fixing leg
*                   schedules, from a label/value block of schedule properties.
*  @param [in]		showColumnHeaders	Include a header row
*  @param [in]		swapScheduleLVB		The schedule configuration as a label/value block
*  @param [in]		validateKeys		Optional. Default TRUE. Check the LVB keys
*  @param [in]		columnList			Optional. Column names to include; default all columns
*  @return			The swap schedule template
*/
SWIG_STRINGMATRIX aqToolSwapScheduleTemplate( const bool showColumnHeaders,
                                               const SWIG_STRINGMATRIX& swapScheduleLVB,
                                               const bool validateKeys = true,
                                               const std::vector<std::string>& columnList = std::vector<std::string>() );
