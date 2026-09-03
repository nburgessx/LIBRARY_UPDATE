/*
 * @brief			validation interface for mirGetECBStartDate method(s)
 * @Created:		04 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LADate.h"

namespace validation_api
{

    /* @brief			validation interface for the mirGetECBStartDate method
    *  @param [in]		ecbDate ECB(European Central Bank) date
    *  @return			The start date of an ECB Swap base on the ecb date
    */
    LADate tryMirGetECBStartDate( const LADate& ecbDate );

}
