#pragma once

#include "LADate.h"

namespace validation_api
{

    /* @brief			validation interface for the mirGetTerm method
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		dayCount		Day count convention
    *  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day
    *  @return			Term between fromDate and toDate
    */
    double tryMirGetTerm( const LADate& fromDate,
                          const LADate& toDate,
                          const LAString& dayCount,
                          bool includeLast );



}
