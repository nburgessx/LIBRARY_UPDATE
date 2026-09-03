#pragma once

#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation_api
{

    /* @brief			return a set of expected keys for swap leg schedule label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductSwapScheduleLVBKeys();

    /* @brief			validation interface for the tryMeProductSwapSchedule method
    *  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
	*  @param [in]		swapScheduleLVB		A LabelValueBlock containing the schedule configuration parameters
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
	*  @param [in]	    columnList          Column header names to show specified columns. Default to empty list 
    *  @return			A matrix containing the swap schedule
    */
    LAStringMatrix tryMeProductSwapSchedule( bool showColumnHeaders, const LabelValueBlock& swapScheduleLVB, bool validateKeys = true, const std::vector<std::string>& columnList=std::vector<std::string>(), const bool& convertDatesToExcelFormat=true );

	/* @brief			validation interface for the tryMeProductSwapScheduleFixed method
    *  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
	*  @param [in]		swapScheduleLVB		A LabelValueBlock containing the schedule configuration parameters
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
	*  @param [in]	    columnList          Column header names to show specified columns. Default to empty list 
    *  @return			A matrix containing the swap schedule
    */
    LAStringMatrix tryMeProductSwapScheduleFixed( bool showColumnHeaders, const LabelValueBlock& swapScheduleLVB, bool validateKeys = true, const std::vector<std::string>& columnList=std::vector<std::string>(), const bool& convertDatesToExcelFormat=true );

	/* @brief			validation interface for the tryMeProductSwapScheduleFloat method
    *  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
	*  @param [in]		swapScheduleLVB		A LabelValueBlock containing the schedule configuration parameters
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
	*  @param [in]	    columnList          Column header names to show specified columns. Default to empty list 
    *  @return			A matrix containing the swap schedule
    */
    LAStringMatrix tryMeProductSwapScheduleFloat( bool showColumnHeaders, const LabelValueBlock& swapScheduleLVB, bool validateKeys = true, const std::vector<std::string>& columnList=std::vector<std::string>(), const bool& convertDatesToExcelFormat=true );

}
