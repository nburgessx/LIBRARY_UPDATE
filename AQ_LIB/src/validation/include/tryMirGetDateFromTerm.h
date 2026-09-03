#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
{

    /* @brief			swig interface for mirGetDateFromTerm
    *  @param [in]		fromDate			The curve collection ID
    *  @param [in]		termY				Year fraction
    *  @param [in]		dayCount			Day count convention
    *  @param [in]		includeLast			True if include the last day, not used in the core function
    *  @return			The end date derived from the FromDate and given year fraction
    */
    LAString tryMirGetDateFromTerm( const LADate& fromDate,
                                    double termY,
                                    const LAString& dayCount,
                                    bool includeLast );


}

