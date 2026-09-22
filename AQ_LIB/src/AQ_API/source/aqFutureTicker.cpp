// aqFutureTicker.cpp

/*
 * @brief			Swig interface for aqFutureTicker... functions
 */

#include "aqFutureTicker.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqFutureTickersIMM.h"
#include "APISetUp.h"					// AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"		// etrading::stringToDate

/* @brief			Function to get the current (most recent) IMM futures ticker relative to a valuation date
*  @param [in]		valuationDate			The valuation date
*  @param [in]		includeToday			Optional. Default FALSE. TRUE lets a contract dated on valuationDate count as current
*  @param [in]		showYearWithTwoDigits	Optional. Default FALSE. TRUE shows the year as 2 digits, e.g. Z25 not Z2025
*  @return			The current IMM futures ticker, which may start in the past yet end in the future
*/
std::string aqFutureTickerCurrent( const std::string& valuationDate,
                                    const bool includeToday,
                                    const bool showYearWithTwoDigits )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate valuationDate_( etrading::stringToDate( valuationDate ) );

    // Call Function and Return Result
    std::string result = validation::tryAqFutureTickerCurrent( valuationDate_, includeToday, showYearWithTwoDigits );
    return result;

    AQ_API_END
}

/* @brief			Function to get the IMM futures ticker immediately after a reference date
*  @param [in]		referenceDate			The IMM reference date
*  @param [in]		showYearWithTwoDigits	Optional. Default FALSE. TRUE shows the year as 2 digits, e.g. Z25 not Z2025
*  @return			The next IMM futures ticker relative to the reference date
*/
std::string aqFutureTickerNext( const std::string& referenceDate,
                                 const bool showYearWithTwoDigits )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate referenceDate_( etrading::stringToDate( referenceDate ) );

    // Call Function and Return Result
    std::string result = validation::tryAqFutureTickerNext( referenceDate_, showYearWithTwoDigits );
    return result;

    AQ_API_END
}

/* @brief			Function to get the IMM futures ticker immediately before a reference date
*  @param [in]		referenceDate			The IMM reference date
*  @param [in]		showYearWithTwoDigits	Optional. Default FALSE. TRUE shows the year as 2 digits, e.g. Z25 not Z2025
*  @return			The previous IMM futures ticker relative to the reference date
*/
std::string aqFutureTickerPrevious( const std::string& referenceDate,
                                     const bool showYearWithTwoDigits )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate referenceDate_( etrading::stringToDate( referenceDate ) );

    // Call Function and Return Result
    std::string result = validation::tryAqFutureTickerPrevious( referenceDate_, showYearWithTwoDigits );
    return result;

    AQ_API_END
}

/* @brief			Function to get the nth IMM futures ticker relative to a valuation date
*  @param [in]		valuationDate			The valuation date
*  @param [in]		nthIMM					The nth IMM futures ticker to generate
*  @param [in]		includeToday			Optional. Default FALSE. TRUE lets a contract dated on valuationDate count as current
*  @param [in]		showYearWithTwoDigits	Optional. Default FALSE. TRUE shows the year as 2 digits, e.g. Z25 not Z2025
*  @return			The nth IMM futures ticker relative to the valuation date
*/
std::string aqFutureTickerNth( const std::string& valuationDate,
                                const int nthIMM,
                                const bool includeToday,
                                const bool showYearWithTwoDigits )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate valuationDate_( etrading::stringToDate( valuationDate ) );

    // Call Function and Return Result
    std::string result = validation::tryAqFutureTickerNth( valuationDate_, nthIMM, includeToday, showYearWithTwoDigits );
    return result;

    AQ_API_END
}
