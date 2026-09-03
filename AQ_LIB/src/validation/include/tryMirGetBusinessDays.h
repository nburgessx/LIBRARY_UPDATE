/*
 * @brief			validation interface for mirGetBusinessDays method(s)
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

    /* @brief			validation interface for the mirGetBusinessDays method
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		calendar		Calendar
    *  @return			Number of business days between fromDate and toDate
    */
    int tryMirGetBusinessDays( const LADate& fromDate,
                               const LADate& toDate,
                               const LAString& calendar );


}
