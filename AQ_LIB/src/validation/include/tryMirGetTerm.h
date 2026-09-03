#pragma once

#include "AQLDate.h"

namespace validation
{

    /* @brief			validation interface for the mirGetTerm method
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		dayCount		Day count convention
    *  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day
    *  @return			Term between fromDate and toDate
    */
    double tryMirGetTerm( const AQLDate& fromDate,
                          const AQLDate& toDate,
                          const AQLString& dayCount,
                          bool includeLast );



}
