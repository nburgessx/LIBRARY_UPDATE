// aqFutureTicker.h

/*
 * @brief			Swig interface for aqFutureTicker... functions
 */

#pragma once

#include <string>

/* @brief			swig interface for aqFutureTickerCurrent
*  @param [in]		valuationDate			The valuation date
*  @param [in]		includeToday			Optional. Default FALSE. TRUE lets a contract dated on valuationDate count as current
*  @param [in]		showYearWithTwoDigits	Optional. Default FALSE. TRUE shows the year as 2 digits, e.g. Z25 not Z2025
*  @return			The current IMM futures ticker, which may start in the past yet end in the future
*/
std::string aqFutureTickerCurrent( const std::string& valuationDate,
                                    const bool includeToday = false,
                                    const bool showYearWithTwoDigits = false );

/* @brief			swig interface for aqFutureTickerNext
*  @param [in]		referenceDate			The IMM reference date
*  @param [in]		showYearWithTwoDigits	Optional. Default FALSE. TRUE shows the year as 2 digits, e.g. Z25 not Z2025
*  @return			The next IMM futures ticker relative to the reference date
*/
std::string aqFutureTickerNext( const std::string& referenceDate,
                                 const bool showYearWithTwoDigits = false );

/* @brief			swig interface for aqFutureTickerPrevious
*  @param [in]		referenceDate			The IMM reference date
*  @param [in]		showYearWithTwoDigits	Optional. Default FALSE. TRUE shows the year as 2 digits, e.g. Z25 not Z2025
*  @return			The previous IMM futures ticker relative to the reference date
*/
std::string aqFutureTickerPrevious( const std::string& referenceDate,
                                     const bool showYearWithTwoDigits = false );

/* @brief			swig interface for aqFutureTickerNth
*  @param [in]		valuationDate			The valuation date
*  @param [in]		nthIMM					The nth IMM futures ticker to generate
*  @param [in]		includeToday			Optional. Default FALSE. TRUE lets a contract dated on valuationDate count as current
*  @param [in]		showYearWithTwoDigits	Optional. Default FALSE. TRUE shows the year as 2 digits, e.g. Z25 not Z2025
*  @return			The nth IMM futures ticker relative to the valuation date
*/
std::string aqFutureTickerNth( const std::string& valuationDate,
                                const int nthIMM,
                                const bool includeToday = false,
                                const bool showYearWithTwoDigits = false );
