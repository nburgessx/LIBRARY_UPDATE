// tryMeFuturesTickersIMM.h

/*
 * @brief			Validation interface for the future tickers methods
 * @Created:		7th February 20119
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include "LACoreTemplateType.h"

namespace validation
{
    /* @brief			validation interface for tryMeFuturesTickerCurrent
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @param [in]		showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			The current IMM futures ticker, which may start in the past yet end in the future
    */
    std::string tryMeFuturesTickerCurrent( const LADate & valuationDate, const bool includeToday = false, const bool showYearWithTwoDigits = false );
    
    /* @brief			validation interface for tryMeDateIMMNext
    *  @param [in]		referenceDate	The IMM reference date
    *  @param [in]		showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			The next IMM futures ticker relative to the reference date
    */
    std::string tryMeFuturesTickerNext( const LADate & referenceDate, const bool showYearWithTwoDigits = false );
    
    /* @brief			validation interface for tryMeDateIMMPrevious
    *  @param [in]		referenceDate	The IMM reference date
    *  @param [in]		showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			The previous IMM futures ticker relative to the reference date
    */
    std::string tryMeFuturesTickerPrevious( const LADate & referenceDate, const bool showYearWithTwoDigits = false );

    /* @brief			validation interface for tryMeDateIMMNth
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		nthIMM	        The nth IMM futures ticker to generate
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @param [in]		showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			The nth IMM futures ticker relative to the valuation date
    */
    std::string tryMeFuturesTickerNth( const LADate & valuationDate, const int nthIMM, const bool includeToday = false, const bool showYearWithTwoDigits = false );
}