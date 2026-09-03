/*
 * @brief			validation interface for the meUtilityReplay function
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#pragma once

#include "LAString.h"

namespace validation_api
{
    /* @brief			Function to load and replay a test file
    *  @param [in]		filepath		Full file path to the test csv file
    */
    const LAString tryMeUtilityReplay( const LAString& filepath );
}