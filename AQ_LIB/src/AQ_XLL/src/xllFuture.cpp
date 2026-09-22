/*
 * Future category - xlOil worksheet functions (stateless).
 *
 * Each function pairs with the identically named validation wrapper (plus the
 * `try` prefix). Marshalling to and from Excel is the aq_xll helpers in
 * xllSupport.h.
 */

#include <xllMain.h>

#include <string>

#include <xllSupport.h>
#include <tryAqFutureTickersIMM.h>   // validation::tryAqFutureTicker*

using namespace aq_xll;

namespace
{
    int toInt( const xloil::ExcelObj& obj )
    {
        return static_cast<int>( obj.get<double>() );
    }
}


// The current (most recent) futures ticker relative to a valuation date.
#if AQ_XLL_ENABLED(aqFutureTickerCurrent)
XLO_FUNC_START( aqFutureTickerCurrent(
    const ExcelObj& valuationDate,
    const ExcelObj& includeToday,
    const ExcelObj& showYearWithTwoDigits ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqFutureTickerCurrent(
        toAQLDate( valuationDate ), toBool( includeToday, false ), toBool( showYearWithTwoDigits, false ) ) );
}
XLO_FUNC_END( aqFutureTickerCurrent )
    .help( L"The current (most recent) futures ticker relative to a valuation date." )
    .arg( L"ValuationDate",         L"The valuation date" )
    .arg( L"IncludeToday",          L"Optional. Default FALSE. TRUE lets a contract dated on ValuationDate count as current" )
    .arg( L"ShowYearWithTwoDigits", L"Optional. Default FALSE. TRUE shows the year as 2 digits, e.g. Z25 not Z2025" );
#endif


// The futures ticker immediately after a reference date.
#if AQ_XLL_ENABLED(aqFutureTickerNext)
XLO_FUNC_START( aqFutureTickerNext(
    const ExcelObj& referenceDate,
    const ExcelObj& showYearWithTwoDigits ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqFutureTickerNext(
        toAQLDate( referenceDate ), toBool( showYearWithTwoDigits, false ) ) );
}
XLO_FUNC_END( aqFutureTickerNext )
    .help( L"The futures ticker immediately after a reference date." )
    .arg( L"ReferenceDate",         L"The reference date" )
    .arg( L"ShowYearWithTwoDigits", L"Optional. Default FALSE. TRUE shows the year as 2 digits, e.g. Z25 not Z2025" );
#endif


// The futures ticker immediately before a reference date.
#if AQ_XLL_ENABLED(aqFutureTickerPrevious)
XLO_FUNC_START( aqFutureTickerPrevious(
    const ExcelObj& referenceDate,
    const ExcelObj& showYearWithTwoDigits ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqFutureTickerPrevious(
        toAQLDate( referenceDate ), toBool( showYearWithTwoDigits, false ) ) );
}
XLO_FUNC_END( aqFutureTickerPrevious )
    .help( L"The futures ticker immediately before a reference date." )
    .arg( L"ReferenceDate",         L"The reference date" )
    .arg( L"ShowYearWithTwoDigits", L"Optional. Default FALSE. TRUE shows the year as 2 digits, e.g. Z25 not Z2025" );
#endif


// The Nth futures ticker relative to a valuation date.
#if AQ_XLL_ENABLED(aqFutureTickerNth)
XLO_FUNC_START( aqFutureTickerNth(
    const ExcelObj& valuationDate,
    const ExcelObj& nthIMM,
    const ExcelObj& includeToday,
    const ExcelObj& showYearWithTwoDigits ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqFutureTickerNth(
        toAQLDate( valuationDate ), toInt( nthIMM ), toBool( includeToday, false ), toBool( showYearWithTwoDigits, false ) ) );
}
XLO_FUNC_END( aqFutureTickerNth )
    .help( L"The Nth futures ticker relative to a valuation date." )
    .arg( L"ValuationDate",         L"The valuation date" )
    .arg( L"NthIMM",                L"Which contract; positive is forward, negative is backward" )
    .arg( L"IncludeToday",          L"Optional. Default FALSE. TRUE lets a contract dated on ValuationDate count" )
    .arg( L"ShowYearWithTwoDigits", L"Optional. Default FALSE. TRUE shows the year as 2 digits, e.g. Z25 not Z2025" );
#endif
