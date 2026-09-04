#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{

    /* @brief			return a set of expected keys for ois swap par rate label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryAqOISParRateLVBKeys();

    /* @brief			validation interface for the aqOISParRate method
    *  @param [in]		swapLVB		A label value block defining the swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			OIS Par Swap Rate
    */
    double tryAqOISParRate( const LabelValueBlock& oisLVB, bool validateKeys = true );

}
