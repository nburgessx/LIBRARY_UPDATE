#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{

    /* @brief			return a set of expected keys for ois swap par rate label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryAqSwapOisParRateLVBKeys();

    /* @brief			validation interface for the aqSwapOisParRate method
    *  @param [in]		swapLVB		A label value block defining the swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			OIS Par Swap Rate
    */
    double tryAqSwapOisParRate( const LabelValueBlock& oisLVB, bool validateKeys = true );

}
