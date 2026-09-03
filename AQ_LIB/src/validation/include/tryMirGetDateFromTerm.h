#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
{

    /* @brief			swig interface for mirGetDateFromTerm
    *  @param [in]		fromDate			The curve collection ID
    *  @param [in]		termY				Year fraction
    *  @param [in]		dayCount			Day count convention
    *  @param [in]		includeLast			True if include the last day, not used in the core function
    *  @return			The end date derived from the FromDate and given year fraction
    */
    AQLString tryMirGetDateFromTerm( const AQLDate& fromDate,
                                    double termY,
                                    const AQLString& dayCount,
                                    bool includeLast );


}

