/*
 * @brief			validation interface for the meUtilityValuationSettings
 * @Created:		5th February 2020
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "Variant.h"

namespace validation_api
{
	/* @brief Method to convert raw input into a valuation settings data block
	*
	* @param[in]	rawInput			Valuation Settings Data
	* @returns	The modified input used for pricing
	*/
	StandardStringMatrix tryMeUtilityValuationSettingsDisplay( const StandardStringMatrix & rawInput );

}
    