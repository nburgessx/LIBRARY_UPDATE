/*
 * @brief			validation interface for the meProductSwapPV method
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation_api
{

   /* @brief			return a set of expected keys for swap pv label value block
	*  @return			expected keys
	*/
	std::vector<std::string> tryMeSwapLegLVBKeys(const LAString& legName);

	/* @brief			validation interface for the meProductSwapLegDisplay method
	*  @param [in]		legLVB			A label value block defining the swap leg
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @param [in] 	    columnList          Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Matrix of leg schedule and cashflows
	*/
	AnyTypeMatrix tryMeProductSwapLegDisplay(const LabelValueBlock& legLVB, bool validateKeys=true, bool showColumnHeaders=true, const std::vector<std::string>& columnList=std::vector<std::string>());

	/* @brief			validation interface for the meProductSwapLegPV method
	*  @param [in]		legLVB		A label value block defining the swap leg
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			Swap Leg PV
	*/
	double tryMeProductSwapLegPV(const LabelValueBlock& legLVB, bool validateKeys=true);

	/* @brief			validation interface for the meProductSwapLegAnnuity method
	*  @param [in]		legLVB		A label value block defining the swap leg
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			Swap Leg Annuity
	*/
	double tryMeProductSwapLegAnnuity(const LabelValueBlock& legLVB, bool validateKeys=true);

}

