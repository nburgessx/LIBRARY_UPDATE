#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
{

    /* @brief			validation method for mirGetDate
    *  @param [in]		baseDate			The from date
    *  @param [in]		term				Term added to the from date
    *  @param [in]		slidingRule			Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			The end date
    */
    AQLString tryMirGetDate( const AQLDate& baseDate,
                            const AQLString& term,
                            const AQLString& slidingRule,
                            const AQLString& calendar,
                            const AQLString& rollConvention );


    /* @brief			validation method for mirGetDate
    *  @param [in]		baseDate			A vector of from dates
    *  @param [in]		term				Term added to the from date
    *  @param [in]		slidingRule			Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			A vector of end dates
    */
    DateVector tryMirGetDate( const DateVector& baseDate,
                              const AQLString& term,
                              const AQLString& slidingRule,
                              const AQLString& calendar,
                              const AQLString& rollConvention );
}
