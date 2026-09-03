#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{

    /* @brief			return a set of expected keys for swap pv01 label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductSwapPV01LVBKeys();

    /* @brief			validation interface for the meProductSwapPV01 method
    *  @param [in]		swapLVB		A label value block defining the swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			Swap PV01
    */
    double tryMeProductSwapPV01( const LabelValueBlock& swapLVB, bool validateKeys = true );

}

