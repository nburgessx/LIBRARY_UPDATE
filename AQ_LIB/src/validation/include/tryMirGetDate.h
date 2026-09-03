/*
 * @brief			validation interface for mirGetDate method(s)
 * @Created:		17 March 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
{

    /* @brief			validation method for mirGetDate
    *  @param [in]		baseDate			The from date
    *  @param [in]		term				Term added to the from date
    *  @param [in]		slidingRule			Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			The end date
    */
    LAString tryMirGetDate( const LADate& baseDate,
                            const LAString& term,
                            const LAString& slidingRule,
                            const LAString& calendar,
                            const LAString& rollConvention );


    /* @brief			validation method for mirGetDate
    *  @param [in]		baseDate			A vector of from dates
    *  @param [in]		term				Term added to the from date
    *  @param [in]		slidingRule			Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			A vector of end dates
    */
    DateVector tryMirGetDate( const DateVector& baseDate,
                              const LAString& term,
                              const LAString& slidingRule,
                              const LAString& calendar,
                              const LAString& rollConvention );
}
