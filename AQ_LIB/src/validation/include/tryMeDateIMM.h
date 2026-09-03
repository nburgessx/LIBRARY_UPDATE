/*
 * @brief			validation interface for mirGetIMMDate method(s)
 * @Created:		18 March 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{


    /* @brief			validation interface for meDateIMMFromMonth.
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		month			The month of the IMM date
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The IMM date on the given month
    */
    LADate tryMeDateIMMFromMonth( int year,
                                  int month,
                                  const LAString& calendar,
                                  const LAString& businessDayAdj );

    /* @brief			validation interface for meDateNthIMM
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		nth				The n'th IMM date of the year
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The nth IMM date of the year
    */
    LADate tryMeDateNthIMM( int year,
                            int nth,
                            const LAString& calendar,
                            const LAString& businessDayAdj );

    /* @brief			validation interface for meDateNthIMMFromStartDate
    *  @param [in]		startDate		The start (reference) date
    *  @param [in]		nth				The nth IMM date from the start date, starting from the one closest to the start date
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The nth IMM date from the start date, starting from the one closest to the start date
    */
    LADate tryMeDateNthIMMFromStartDate( const LADate& startDate,
                                         int nth,
                                         const LAString& calendar,
                                         const LAString& businessDayAdj );

    /* @brief			validation interface for tryMeDateIMMCurrent
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @return			The current IMM start date, which may be in the past
    */
    LADate tryMeDateIMMCurrent( const LADate & valuationDate, const bool includeToday = false );
    
    /* @brief			validation interface for tryMeDateIMMNext
    *  @param [in]		referenceDate	The IMM reference date
    *  @return			The next IMM start date relative to the reference date
    */
    LADate tryMeDateIMMNext( const LADate & referenceDate );
    
    /* @brief			validation interface for tryMeDateIMMPrevious
    *  @param [in]		referenceDate	The IMM reference date
    *  @return			The previous IMM start date relative to the reference date
    */
    LADate tryMeDateIMMPrevious( const LADate & referenceDate );

    /* @brief			validation interface for tryMeDateIMMNth
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		nthIMM	        The nth IMM start date to generate
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @return			The nth IMM date relative to the valuation date
    */
    LADate tryMeDateIMMNth( const LADate & valuationDate, const int nthIMM, const bool includeToday = false );

}
