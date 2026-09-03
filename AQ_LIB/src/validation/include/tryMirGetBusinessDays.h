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
