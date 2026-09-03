#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{

    /* @brief			return a set of expected keys for asset swap spread label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductAssetSwapSpreadLVBKeys();

    /* @brief			validation interface for the meProductAssetSwapSpread method
    *  @param [in]		bondPrice	        Bond Price, can be clean or dirty, but must specify in the assetSwapLVB
    *  @param [in]		assetSwapLVB		A label value block defining an asset swap
    *  @param [in]		validateKeys	    True to validate the all keys provided are valid. Default to True
    *  @return			Asset Swap Spread
    */
    double tryMeProductAssetSwapSpread( double bondPrice, const LabelValueBlock& assetSwapLVB, bool validateKeys = true );

}
