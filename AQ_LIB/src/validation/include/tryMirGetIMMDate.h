#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation
{


    /* @brief			validation interface for mirGetIMMDate1.
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		month			The month of the IMM date
    *  @param [in]		calendar		Calendar
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The IMM date on the given month
    */
    LAString tryMirGetIMMDate1( int year,
                                int month,
                                const LAString& calendar,
                                const LAString& slidingRule );


    /* @brief			validation interface for mirGetIMMDate2
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		number			The number'th IMM date of the year
    *  @param [in]		calendar		Calendar
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The nth IMM date of the year
    */
    LAString tryMirGetIMMDate2( int year,
                                int number,
                                const LAString& calendar,
                                const LAString& slidingRule );


    /* @brief			validation interface for mirGetIMMDate3
    *  @param [in]		baseDate		The base (reference) date
    *  @param [in]		number			The (number-1)th IMM date from the base date, starting from the one closest to the base date
    *  @param [in]		calendar		Calendar
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The nth IMM date from the base date, starting from the one closest to the base date
    */
    LAString tryMirGetIMMDate3( const LADate& baseDate,
                                int number,
                                const LAString& calendar,
                                const LAString& slidingRule );


}
