#pragma once

#include "AQLDate.h"

namespace validation
{

    /* @brief			validation interface for the mirGetBusinessDays method
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		calendar		Calendar
    *  @return			Number of business days between fromDate and toDate
    */
    int tryMirGetBusinessDays( const AQLDate& fromDate,
                               const AQLDate& toDate,
                               const AQLString& calendar );


}
