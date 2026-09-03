/*
 * @brief			validation interface for tryMeProductOISParRate
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

    /* @brief			return a set of expected keys for ois swap par rate label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductOISParRateLVBKeys();

    /* @brief			validation interface for the meProductOISParRate method
    *  @param [in]		swapLVB		A label value block defining the swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			OIS Par Swap Rate
    */
    double tryMeProductOISParRate( const LabelValueBlock& oisLVB, bool validateKeys = true );

}
