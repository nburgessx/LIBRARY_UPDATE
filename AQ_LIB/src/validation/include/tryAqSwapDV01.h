#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{


    /* @brief			return a set of expected keys for swap DV01 label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryAqSwapDV01LVBKeys();

    /* @brief			validation interface for the aqSwapDV01 method
    *  @param [in]		swapLVB		A label value block defining the swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			Swap DV01
    */
    double tryAqSwapDV01( const LabelValueBlock& swapLVB, bool validateKeys );
}