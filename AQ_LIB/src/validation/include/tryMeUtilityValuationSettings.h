#pragma once

#include "LACoreTemplateType.h"
#include "Variant.h"

namespace validation
{
	/* @brief Method to convert raw input into a valuation settings data block
	*
	* @param[in]	rawInput			Valuation Settings Data
	* @returns	The modified input used for pricing
	*/
	StandardStringMatrix tryMeUtilityValuationSettingsDisplay( const StandardStringMatrix & rawInput );

}
    