#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{

    /* @brief			return a set of expected keys for swap par rate label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryAqSwapsParRateLVBKeys();

    /* @brief			validation interface for the aqSwapsParRate method
    *  @param [in]		parSwapLVB		A label value block defining an par swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			Par Swap Rate
    */
    double tryAqSwapsParRate( const LabelValueBlock& parSwapLVB, bool validateKeys = true );


}

