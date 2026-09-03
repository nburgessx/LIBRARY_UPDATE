#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{

    /* @brief			return a set of expected keys for swap pv label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductSwapPVLVBKeys();

    /* @brief			validation interface for the meProductSwapPV method
    *  @param [in]		swapLVB		A label value block defining the swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			Swap PV
    */
    double tryMeProductSwapPV( const LabelValueBlock& swapLVB, bool validateKeys = true );
}

