#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation_api
{


    /* @brief			return a set of expected keys for swap DV01 label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductSwapDV01LVBKeys();

    /* @brief			validation interface for the meProductSwapDV01 method
    *  @param [in]		swapLVB		A label value block defining the swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			Swap DV01
    */
    double tryMeProductSwapDV01( const LabelValueBlock& swapLVB, bool validateKeys );
}