#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{


    /* @brief			validation interface for aqDatesIMMFromMonth.
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		month			The month of the IMM date
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The IMM date on the given month
    */
    AQLDate tryAqDatesIMMFromMonth( int year,
                                  int month,
                                  const AQLString& calendar,
                                  const AQLString& businessDayAdj );

    /* @brief			validation interface for aqDatesNthIMM
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		nth				The n'th IMM date of the year
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The nth IMM date of the year
    */
    AQLDate tryAqDatesNthIMM( int year,
                            int nth,
                            const AQLString& calendar,
                            const AQLString& businessDayAdj );

    /* @brief			validation interface for aqDatesNthIMMFromStartDate
    *  @param [in]		startDate		The start (reference) date
    *  @param [in]		nth				The nth IMM date from the start date, starting from the one closest to the start date
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The nth IMM date from the start date, starting from the one closest to the start date
    */
    AQLDate tryAqDatesNthIMMFromStartDate( const AQLDate& startDate,
                                         int nth,
                                         const AQLString& calendar,
                                         const AQLString& businessDayAdj );

    /* @brief			validation interface for tryAqDatesIMMCurrent
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @return			The current IMM start date, which may be in the past
    */
    AQLDate tryAqDatesIMMCurrent( const AQLDate & valuationDate, const bool includeToday = false );
    
    /* @brief			validation interface for tryAqDatesIMMNext
    *  @param [in]		referenceDate	The IMM reference date
    *  @return			The next IMM start date relative to the reference date
    */
    AQLDate tryAqDatesIMMNext( const AQLDate & referenceDate );
    
    /* @brief			validation interface for tryAqDatesIMMPrevious
    *  @param [in]		referenceDate	The IMM reference date
    *  @return			The previous IMM start date relative to the reference date
    */
    AQLDate tryAqDatesIMMPrevious( const AQLDate & referenceDate );

    /* @brief			validation interface for tryAqDatesIMMNth
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		nthIMM	        The nth IMM start date to generate
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @return			The nth IMM date relative to the valuation date
    */
    AQLDate tryAqDatesIMMNth( const AQLDate & valuationDate, const int nthIMM, const bool includeToday = false );

}
