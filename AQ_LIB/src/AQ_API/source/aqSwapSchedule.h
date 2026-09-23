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

/* @brief			swig interface for the aqSwapObjectScheduleCreate function
*  @param [in]		scheduleName		Schedule name
*  @param [in]		swapScheduleLVB		Schedule label value block
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
*  @return			The scheduleName showing the schedule has been cached
*/
std::string aqSwapObjectScheduleCreate( const std::string& scheduleName,
                                     const SWIG_STRINGMATRIX & swapScheduleLVB,
                                     bool validateKeys );

/* @brief			swig interface for the aqSwapObjectScheduleDisplay function
*  @param [in]		scheduleName			Schedule name
*  @param [in]		showBespokeProperties	True to show bespoke schedule properties, default to false
*  @param [in]		showColumnHeaders		True to show column headers, default to true
*  @param [in]		columnList				Column header names to show specified columns. Default to empty list showing all columns
*  @return			Schedule display
*/
SWIG_STRINGMATRIX aqSwapObjectScheduleDisplay( const std::string& scheduleName,
                                            bool showBespokeProperties,
                                            bool showColumnHeaders,
                                            const std::vector<std::string>& columnList );

/* @brief			swig interface for the aqSwapObjectScheduleCreateBespoke function
*  @param [in]		scheduleName				Bespoke schedule name
*  @param [in]		bespokeScheduleProperties	Bespoke schedule properties label value block
*  @param [in]		bespokeScheduleLVB			Bespoke schedule cashflow label value block
*  @param [in]		validateKeys				True to validate the all keys provided are valid for the bespokeScheduleProperties. Default to True
*  @return			The schedule name
*/
std::string aqSwapObjectScheduleCreateBespoke( const std::string& scheduleName,
                                            const SWIG_STRINGMATRIX & bespokeScheduleProperties,
                                            const SWIG_STRINGMATRIX & bespokeScheduleLVB,
                                            bool validateKeys );

/* @brief			swig interface for the aqSwapObjectScheduleCreateBespokeFromCashflows function
*  @param [in]		scheduleObjectName			Bespoke schedule name
*  @param [in]		bespokeScheduleProperties	Bespoke schedule properties label value block
*  @param [in]		bespokeCashflowsLVB			Bespoke schedule cashflow label value block matrix
*  @param [in]		validateKeys				True to validate the all keys provided are valid for the bespokeScheduleProperties. Default to True
*  @return			The schedule name
*/
std::string aqSwapObjectScheduleCreateBespokeFromCashflows( const std::string& scheduleObjectName,
                                                          const SWIG_STRINGMATRIX & bespokeScheduleProperties,
                                                          const SWIG_STRINGMATRIX & bespokeCashflowsLVB,
                                                          bool validateKeys );