/*
 * @brief			validation interface for meProductSwapParRate
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

    /* @brief			return a set of expected keys for swap par rate label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductSwapParRateLVBKeys();

    /* @brief			validation interface for the meProductSwapParRate method
    *  @param [in]		parSwapLVB		A label value block defining an par swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			Par Swap Rate
    */
    double tryMeProductSwapParRate( const LabelValueBlock& parSwapLVB, bool validateKeys = true );


}

