#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{

    /* @brief			return a set of expected keys for swap par rate label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryAqSwapParRateLVBKeys();

    /* @brief			validation interface for the aqSwapParRate method
    *  @param [in]		parSwapLVB		A label value block defining an par swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			Par Swap Rate
    */
    double tryAqSwapParRate( const LabelValueBlock& parSwapLVB, bool validateKeys = true );


}

