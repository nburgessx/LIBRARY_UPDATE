// tryAqFutureTickersIMM.h

/*
 * @brief			Validation interface for the future tickers methods
 */

#pragma once
#include "AQLCoreTemplateType.h"

namespace validation
{
    /* @brief			validation interface for tryAqFutureTickerCurrent
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @param [in]		showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			The current IMM futures ticker, which may start in the past yet end in the future
    */
    std::string tryAqFutureTickerCurrent( const AQLDate & valuationDate, const bool includeToday = false, const bool showYearWithTwoDigits = false );
    
    /* @brief			validation interface for tryAqDateIMMNext
    *  @param [in]		referenceDate	The IMM reference date
    *  @param [in]		showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			The next IMM futures ticker relative to the reference date
    */
    std::string tryAqFutureTickerNext( const AQLDate & referenceDate, const bool showYearWithTwoDigits = false );
    
    /* @brief			validation interface for tryAqDateIMMPrevious
    *  @param [in]		referenceDate	The IMM reference date
    *  @param [in]		showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			The previous IMM futures ticker relative to the reference date
    */
    std::string tryAqFutureTickerPrevious( const AQLDate & referenceDate, const bool showYearWithTwoDigits = false );

    /* @brief			validation interface for tryAqDateIMMNth
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		nthIMM	        The nth IMM futures ticker to generate
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @param [in]		showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			The nth IMM futures ticker relative to the valuation date
    */
    std::string tryAqFutureTickerNth( const AQLDate & valuationDate, const int nthIMM, const bool includeToday = false, const bool showYearWithTwoDigits = false );
}