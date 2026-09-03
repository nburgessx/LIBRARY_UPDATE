/*
 * @brief			validation interface for the meUtilityRecord
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
    /* @brief			validation interface for the meUtilityVersion function
    *  @param [in]		enable		True to enable the recording of tests
    *  @param [in]		folder		Output folder
    *  @param [in]		repeat		Allowed repeated tests
    *  @param [in]		startIndex	Start index for repeated tests
    *  @param [in]		maxIndex	Max index for repeated tests
    *  @return			A notification string
    */
    LAString tryMeUtilityRecord( bool enable, const LAString& folder, bool repeat = false, int startIndex = 0, int maxIndex = 0 );
}