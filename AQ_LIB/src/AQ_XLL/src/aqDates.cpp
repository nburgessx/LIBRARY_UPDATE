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

#include <main.h>
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
