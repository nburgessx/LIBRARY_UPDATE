/*
 * @brief			validation interface for meProductBondSchedule method
 * @Created:		7 Dec 2016
 * @Author:			Ian Castleton
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation_api
{

    /* @brief			return a set of expected keys for bond leg schedule label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductBondScheduleLVBKeys();

    /* @brief			validation interface for the tryMeProductBondSchedule method
    *  @param [in]		bondScheduleLVB		A label value block defining the bond.
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
	*  @param [in]		showColumnHeaders	True to include the column headers in the output. Default to True
	*  @param [in] 	    columnList          Column header names to show specified columns. Default to empty list showing all columns.
    *  @return			A matrix of floating leg/fixing leg schedules
    */
    LAStringMatrix tryMeProductBondSchedule( const LabelValueBlock& bondScheduleLVB, bool validateKeys = true, bool showColumnHeaders = true, const std::vector<std::string>& columnList=std::vector<std::string>() );
}

