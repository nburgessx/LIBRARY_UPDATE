/*
 * @brief			validation interface for the meProductAssetSwapSpread method
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation_api
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
