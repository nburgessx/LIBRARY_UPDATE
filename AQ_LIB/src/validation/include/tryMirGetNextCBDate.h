/*
 * @brief			validation interface for mirGetNextCBDate method(s)
 * @Created:		04 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LADate.h"

namespace validation_api
{

    /* @brief			validation interface for the mirGetNextCBDate method
    *  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
    *  @param [in]		baseDate		Base date
    *  @param [in]		strictlyAfter	True if strictly after the base date
    *  @return			Date of the next central bank meeting on/after the base date
    */
    LADate tryMirGetNextCBDate( const LAString& centralBankId,
                                const LADate& baseDate,
                                bool strictlyAfter );


}
