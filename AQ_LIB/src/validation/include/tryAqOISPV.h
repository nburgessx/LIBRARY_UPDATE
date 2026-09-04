#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{

    /* @brief			return a set of expected keys for swap pv label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryAqOISPVLVBKeys();

    /* @brief			validation interface for the aqOISPV method
    *  @param [in]		swapLVB		A label value block defining the swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			OIS PV
    */
    double tryAqOISPV( const LabelValueBlock& oisLVB, bool validateKeys = true );
}
