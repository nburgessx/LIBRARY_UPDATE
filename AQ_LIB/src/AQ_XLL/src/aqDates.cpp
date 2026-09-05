/*
 * @brief   Dates category - Excel worksheet functions.
 *
 *          Ported from .APPLES\APPLE\src\MLIBQ_ADDIN\src\meDates.cpp. The XLL+
 *          registration blob and the _4/_12 export wrappers are gone: xlOil
 *          generates those from XLO_FUNC_START/END, and owns the structured
 *          exception handling that MLIB_START/END used to provide. Only the
 *          _Impl body carries across, and it routes through the validation
 *          layer exactly as the original did.
 *
 *          Renamed for the clean break (CLAUDE.md 5.6): meDateFromTenor and
 *          meDateFromYearFraction become aqDatesFromTenor and
 *          aqDatesFromYearFraction - the Dates category, plural, matching the
 *          names already published by AQ_API and validation.
 */

#include <aqMain.h>
#include "aqXllTools.h"

#include "tryAqDates.h"

using namespace aq_xll;


/* @brief   End date(s) from a start date (or range of start dates) plus a tenor.
*/
XLO_FUNC_START( aqDatesFromTenor(
    const ExcelObj& startDates,
    const ExcelObj& tenor,
    const ExcelObj& businessDayAdj,
    const ExcelObj& calendar,
    const ExcelObj& rollConvention ) )
{
    AQ_XLL_GUARD

    const DateVector starts = toDateVector( startDates, true, "StartDates" );

    const DateVector ends = validation::tryAqDatesFromTenor( starts,
                                                             toAQLString( tenor ),
                                                             toAQLString( businessDayAdj ),
                                                             toAQLString( calendar ),
                                                             toAQLString( rollConvention ) );

    return returnValue( toExcelDateColumn( ends ) );
}
XLO_FUNC_END( aqDatesFromTenor )
    .help( L"Return the end date(s) based on StartDate(s) + Tenor. Format the cells as dates." )
    .arg( L"StartDates",     L"A single start date or a range of start dates" )
    .arg( L"Tenor",          L"Tenor, e.g. 3M, 5Y, 1W" )
    .arg( L"BusinessDayAdj", L"Business day adjustment, e.g. Following, ModifiedFollowing, Preceding" )
    .arg( L"Calendar",       L"Holiday centre(s), e.g. LnB or LnB+NYB" )
    .arg( L"RollConvention", L"Normal, IMM, EOM, Lunar, etc" );


/* @brief   End date from a start date plus a year fraction, under a day count.
*/
XLO_FUNC_START( aqDatesFromYearFraction(
    const ExcelObj& startDate,
    const ExcelObj& yearFraction,
    const ExcelObj& dayCount ) )
{
    AQ_XLL_GUARD

    const AQLDate end = validation::tryAqDatesFromYearFraction( toAQLDate( startDate ),
                                                                yearFraction.get< double >(),
                                                                toAQLString( dayCount ) );

    return returnValue( toExcelDate( end ) );
}
XLO_FUNC_END( aqDatesFromYearFraction )
    .help( L"Return the end date implied by a start date plus a year fraction. Format the cell as a date." )
    .arg( L"StartDate",    L"The start date" )
    .arg( L"YearFraction", L"The year fraction to advance by" )
    .arg( L"DayCount",     L"Day count convention, e.g. ACT/360, ACT/365, 30/360" );


// ---------------------------------------------------------------------------
//  Previously in dates.cpp
// ---------------------------------------------------------------------------

#include <ctime>
#include <cmath>

// Date Time Helper Methods
namespace
{
    // Days since 1970-01-01 from a civil date (Howard Hinnant's algorithm)
    long long daysFromCivil(long long y, unsigned m, unsigned d)
    {
        y -= m <= 2;
        const long long era = (y >= 0 ? y : y - 399) / 400;
        const unsigned yoe = static_cast<unsigned>(y - era * 400);
        const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
        const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        return era * 146097 + static_cast<long long>(doe) - 719468;
    }

    // Local wall-clock time as an Excel date serial (1900 date system)
    double excelLocalSerial()
    {
        const std::time_t t = std::time(nullptr);
        std::tm lt{};
        localtime_s(&lt, &t);
        const auto days = daysFromCivil(lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
        const double frac = (lt.tm_hour * 3600.0 + lt.tm_min * 60.0 + lt.tm_sec) / 86400.0;
        return static_cast<double>(days + 25569) + frac;
    }
}


// Non-Volatile Today Method
XLO_FUNC_START(aqDatesToday())
{
    return returnValue(std::floor(excelLocalSerial()));
}
XLO_FUNC_END(aqDatesToday)
.help(L"Today's date as a non-volatile date serial. Format the cell as a date.");


// Non-Volatile Now Method
XLO_FUNC_START(aqDatesNow())
{
    return returnValue(excelLocalSerial());
}
XLO_FUNC_END(aqDatesNow)
.help(L"Current date and time, non-volatile. Format the cell as date/time.");
