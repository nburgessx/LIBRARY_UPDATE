#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqSwapSchedule
*  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
*  @return			A matrix containing the swap schedule
*/
SWIG_STRINGMATRIX aqSwapSchedule( bool showColumnHeaders, 
										 const SWIG_STRINGMATRIX& swapScheduleLVB, 
										 bool validateKeys=true);

/* @brief			swig interface for aqSwapScheduleFixed
*  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
*  @return			A matrix containing the swap schedule
*/
SWIG_STRINGMATRIX aqSwapScheduleFixed( bool showColumnHeaders, 
											  const SWIG_STRINGMATRIX& swapScheduleLVB );

/* @brief			swig interface for aqSwapScheduleFloat
*  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
*  @return			A matrix containing the swap schedule
*/
SWIG_STRINGMATRIX aqSwapScheduleFloat( bool showColumnHeaders, 
											  const SWIG_STRINGMATRIX& swapScheduleLVB);

/* @brief			return a set of expected keys for swap leg schedule label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapScheduleLVBKeys();