#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{

   /* @brief			return a set of expected keys for swap pv label value block
	*  @return			expected keys
	*/
	std::vector<std::string> tryAqSwapsLegLVBKeys(const AQLString& legName);

	/* @brief			validation interface for the aqSwapLegDisplay method
	*  @param [in]		legLVB			A label value block defining the swap leg
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @param [in] 	    columnList          Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Matrix of leg schedule and cashflows
	*/
	AnyTypeMatrix tryAqSwapLegDisplay(const LabelValueBlock& legLVB, bool validateKeys=true, bool showColumnHeaders=true, const std::vector<std::string>& columnList=std::vector<std::string>());

	/* @brief			validation interface for the aqSwapLegPV method
	*  @param [in]		legLVB		A label value block defining the swap leg
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			Swap Leg PV
	*/
	double tryAqSwapLegPV(const LabelValueBlock& legLVB, bool validateKeys=true);

	/* @brief			validation interface for the aqSwapLegAnnuity method
	*  @param [in]		legLVB		A label value block defining the swap leg
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			Swap Leg Annuity
	*/
	double tryAqSwapLegAnnuity(const LabelValueBlock& legLVB, bool validateKeys=true);

}

