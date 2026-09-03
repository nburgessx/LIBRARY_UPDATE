/*
 * @brief			validation interface for mirGetNextECBDate method(s)
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

    /* @brief			validation interface for the mirGetNextECBDate method
    *  @param [in]		baseDate		Base date
    *  @param [in]		strictlyAfter	True if strictly after the base date
    *  @return			Date of the next ECB (European Central Bank) meeting on/after the base date
    */
    LADate tryMirGetNextECBDate( const LADate& baseDate,
                                 bool strictlyAfter = true );


}
