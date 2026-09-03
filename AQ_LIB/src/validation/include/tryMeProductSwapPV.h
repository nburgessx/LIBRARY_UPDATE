/*
 * @brief			validation interface for the meProductSwapPV method
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation_api
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

