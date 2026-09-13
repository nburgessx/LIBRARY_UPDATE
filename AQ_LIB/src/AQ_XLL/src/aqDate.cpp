/*
 * Date category - xlOil worksheet functions. Each routes through the
 * validation layer; marshalling is the aq_xll helpers in aqXllTools.h.
 */

#include <aqMain.h>
#include <ctime>
#include <cmath>

#include <aqXllTools.h>
#include <tryAqDate.h>
#include <tryAqDateCentralBank.h>          // validation::tryAqDate{CentralBank,ECB,ECBSwap*,NthECB*,NextECB*}
#include <tryAqDateIMM.h>                  // validation::tryAqDate{IMMFromMonth,NthIMM,NthIMMFromStartDate,IMMCurrent,IMMNext,IMMPrevious,IMMNth}
#include <tryAqDateIsRegularSwapSchedule.h>

using namespace aq_xll;

namespace
{
    // A required integer worksheet argument.
    int toInt( const xloil::ExcelObj& obj )
    {
        return static_cast< int >( obj.get<double>() );
    }
}

// Date Time Helper Methods
namespace
{
	// Days since 1970-01-01 from a civil date (Howard Hinnant's algorithm)
	long long daysFromCivil( long long y, unsigned m, unsigned d )
	{
		y -= m <= 2;
		const long long era = ( y >= 0 ? y : y - 399 ) / 400;
		const unsigned yoe = static_cast<unsigned>( y - era * 400 );
		const unsigned doy = ( 153 * ( m + ( m > 2 ? -3 : 9 ) ) + 2 ) / 5 + d - 1;
		const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
		return era * 146097 + static_cast<long long>( doe ) - 719468;
	}

	// Local wall-clock time as an Excel date serial (1900 date system)
	double excelLocalSerial()
	{
		const std::time_t t = std::time( nullptr );
		std::tm lt{};
		localtime_s( &lt, &t );
		const auto days = daysFromCivil( lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday );
		const double frac = ( lt.tm_hour * 3600.0 + lt.tm_min * 60.0 + lt.tm_sec ) / 86400.0;
		return static_cast<double>( days + 25569 ) + frac;
	}
}

// Non-Volatile Today Method
#if AQ_XLL_ENABLED(aqDateToday)
XLO_FUNC_START( aqDateToday() )
{
	return returnValue( std::floor( excelLocalSerial() ) );
}
XLO_FUNC_END( aqDateToday )
    .help( L"Today's date as a non-volatile date serial. Format the cell as a date." );
#endif


// Non-Volatile Now Method
#if AQ_XLL_ENABLED(aqDateNow)
XLO_FUNC_START( aqDateNow() )
{
	return returnValue( excelLocalSerial() );
}
XLO_FUNC_END( aqDateNow )
    .help( L"Current date and time, non-volatile. Format the cell as date/time." );
#endif


// End date(s) from a start date (or range of start dates) plus a tenor.
#if AQ_XLL_ENABLED(aqDateFromTenor)
XLO_FUNC_START( aqDateFromTenor(
    const ExcelObj& startDates,
    const ExcelObj& tenor,
    const ExcelObj& businessDayAdj,
    const ExcelObj& calendar,
    const ExcelObj& rollConvention ) )
{
	AQ_XLL_GUARD

	const DateVector starts =
	    toDateVector( startDates, true, "StartDates" );

	const DateVector ends =
	    validation::tryAqDateFromTenor(
	        starts,
	        toAQLString( tenor ),
	        toAQLString( businessDayAdj ),
	        toAQLString( calendar ),
	        toAQLString( rollConvention ) );

	return returnValue( toExcelDateColumn( ends ) );
}
XLO_FUNC_END( aqDateFromTenor )
    .help( L"Return the end date(s) based on StartDate(s) + Tenor. Format the cells as dates." )
    .arg( L"StartDates",     L"A single start date or a range of start dates" )
    .arg( L"Tenor",          L"Tenor, e.g. 3M, 5Y, 1W" )
    .arg( L"BusinessDayAdj", L"Business day adjustment, e.g. Following, ModifiedFollowing, Preceding" )
    .arg( L"Calendar",       L"Holiday centre(s), e.g. LnB or LnB+NYB" )
    .arg( L"RollConvention", L"Normal, IMM, EOM, Lunar, etc" );
#endif


// End date from a start date plus a year fraction, under a day count.
#if AQ_XLL_ENABLED(aqDateFromYearFraction)
XLO_FUNC_START( aqDateFromYearFraction(
    const ExcelObj& startDate,
    const ExcelObj& yearFraction,
    const ExcelObj& dayCount ) )
{
    AQ_XLL_GUARD

    const AQLDate end = validation::tryAqDateFromYearFraction( toAQLDate( startDate ),
	                                                            yearFraction.get<double>(),
	                                                            toAQLString( dayCount ) );

    return returnValue( toExcelDate( end ) );
}
XLO_FUNC_END( aqDateFromYearFraction )
    .help( L"Return the end date implied by a start date plus a year fraction. Format the cell as a date." )
    .arg( L"StartDate",    L"The start date" )
    .arg( L"YearFraction", L"The year fraction to advance by" )
    .arg( L"DayCount",     L"Day count convention, e.g. ACT/360, ACT/365, 30/360" );
#endif


// Year fraction between two dates under a day count.
#if AQ_XLL_ENABLED(aqDateYearFraction)
XLO_FUNC_START( aqDateYearFraction(
    const ExcelObj& fromDate,
    const ExcelObj& toDate,
    const ExcelObj& dayCount,
    const ExcelObj& includeLast ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqDateYearFraction( toAQLDate( fromDate ),
                                                           toAQLDate( toDate ),
                                                           toAQLString( dayCount ),
                                                           toBool( includeLast, true ) ) );
}
XLO_FUNC_END( aqDateYearFraction )
    .help( L"Year fraction between two dates under a day count." )
    .arg( L"FromDate",     L"The start date" )
    .arg( L"ToDate",       L"The end date" )
    .arg( L"DayCount",     L"Day count convention, e.g. ACT/360, ACT/365, 30/360" )
    .arg( L"IncludeLast",  L"Optional. Default TRUE. Include the last day in the count" );
#endif


// Business days between two dates for a calendar.
#if AQ_XLL_ENABLED(aqDateBusinessDays)
XLO_FUNC_START( aqDateBusinessDays(
    const ExcelObj& fromDate,
    const ExcelObj& toDate,
    const ExcelObj& calendar ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( static_cast<double>( validation::tryAqDateBusinessDays(
        toAQLDate( fromDate ), toAQLDate( toDate ), toAQLString( calendar ) ) ) );
}
XLO_FUNC_END( aqDateBusinessDays )
    .help( L"Number of business days between two dates for a holiday calendar." )
    .arg( L"FromDate", L"The start date" )
    .arg( L"ToDate",   L"The end date" )
    .arg( L"Calendar", L"Holiday centre(s), e.g. LnB or LnB+NYB" );
#endif


// Spot date implied by a fixing lag and a payment lag.
#if AQ_XLL_ENABLED(aqDateShiftedSpotDate)
XLO_FUNC_START( aqDateShiftedSpotDate(
    const ExcelObj& asOfDate,
    const ExcelObj& fixingLag,
    const ExcelObj& fixingCalendar,
    const ExcelObj& fixingBusDayAdj,
    const ExcelObj& paymentLag,
    const ExcelObj& paymentCalendar,
    const ExcelObj& paymentBusDayAdj ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const AQLDate result = validation::tryAqDateShiftedSpotDate(
        toAQLDate( asOfDate ),
        toAQLString( fixingLag ),
        toAQLString( fixingCalendar ),
        toAQLString( fixingBusDayAdj ),
        toAQLString( paymentLag ),
        toAQLString( paymentCalendar ),
        toAQLString( paymentBusDayAdj ) );

    return returnValue( toExcelDate( result ) );
}
XLO_FUNC_END( aqDateShiftedSpotDate )
    .help( L"Spot date implied by a fixing lag and a payment lag from AsOfDate. Format the cell as a date." )
    .arg( L"AsOfDate",         L"The as-of date" )
    .arg( L"FixingLag",        L"Fixing lag, e.g. 2D" )
    .arg( L"FixingCalendar",   L"Fixing holiday centre(s)" )
    .arg( L"FixingBusDayAdj",  L"Fixing business day adjustment" )
    .arg( L"PaymentLag",       L"Payment lag, e.g. 2D" )
    .arg( L"PaymentCalendar",  L"Payment holiday centre(s)" )
    .arg( L"PaymentBusDayAdj", L"Payment business day adjustment" );
#endif


// TRUE if the date is a working day for the given holiday centre.
#if AQ_XLL_ENABLED(aqDateIsWorkingDay)
XLO_FUNC_START( aqDateIsWorkingDay(
    const ExcelObj& date,
    const ExcelObj& holidayCentre ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqDateIsWorkingDay( toAQLDate( date ), toAQLString( holidayCentre ) ) );
}
XLO_FUNC_END( aqDateIsWorkingDay )
    .help( L"TRUE if the date is a working day for the given holiday centre." )
    .arg( L"Date",          L"The date to test" )
    .arg( L"HolidayCentre", L"Holiday centre(s), e.g. LnB or LnB+NYB" );
#endif


// TRUE if the date is a holiday for the given holiday centre.
#if AQ_XLL_ENABLED(aqDateIsHoliday)
XLO_FUNC_START( aqDateIsHoliday(
    const ExcelObj& date,
    const ExcelObj& holidayCentre ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqDateIsHoliday( toAQLDate( date ), toAQLString( holidayCentre ) ) );
}
XLO_FUNC_END( aqDateIsHoliday )
    .help( L"TRUE if the date is a holiday for the given holiday centre." )
    .arg( L"Date",          L"The date to test" )
    .arg( L"HolidayCentre", L"Holiday centre(s), e.g. LnB or LnB+NYB" );
#endif


// TRUE if the date falls on a weekend.
#if AQ_XLL_ENABLED(aqDateIsWeekend)
XLO_FUNC_START( aqDateIsWeekend(
    const ExcelObj& date ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqDateIsWeekend( toAQLDate( date ) ) );
}
XLO_FUNC_END( aqDateIsWeekend )
    .help( L"TRUE if the date falls on a Saturday or Sunday." )
    .arg( L"Date", L"The date to test" );
#endif


// TRUE if the date falls on a weekday.
#if AQ_XLL_ENABLED(aqDateIsWeekday)
XLO_FUNC_START( aqDateIsWeekday(
    const ExcelObj& date ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqDateIsWeekday( toAQLDate( date ) ) );
}
XLO_FUNC_END( aqDateIsWeekday )
    .help( L"TRUE if the date falls on a Monday to Friday." )
    .arg( L"Date", L"The date to test" );
#endif


// Contract expiry date from a futures ticker.
#if AQ_XLL_ENABLED(aqDateFuturesContract)
XLO_FUNC_START( aqDateFuturesContract(
    const ExcelObj& futuresTicker ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateFuturesContract( toAQLString( futuresTicker ) ) ) );
}
XLO_FUNC_END( aqDateFuturesContract )
    .help( L"Contract expiry date implied by a futures ticker. Format the cell as a date." )
    .arg( L"FuturesTicker", L"The futures ticker, e.g. EDZ25, FFF26" );
#endif


// Central-bank meeting date on or after (or strictly after) a date.
#if AQ_XLL_ENABLED(aqDateCentralBank)
XLO_FUNC_START( aqDateCentralBank(
    const ExcelObj& centralBankId,
    const ExcelObj& date,
    const ExcelObj& strictlyAfter ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateCentralBank(
        toAQLString( centralBankId ), toAQLDate( date ), toBool( strictlyAfter, true ) ) ) );
}
XLO_FUNC_END( aqDateCentralBank )
    .help( L"The central-bank meeting date on or after the given date. Format the cell as a date." )
    .arg( L"CentralBankId", L"Central bank identifier, e.g. ECB, FED, BOE" )
    .arg( L"Date",          L"The reference date" )
    .arg( L"StrictlyAfter", L"Optional. Default TRUE. TRUE skips a meeting that falls on Date itself" );
#endif


// Next ECB meeting date on (or strictly after) a date.
#if AQ_XLL_ENABLED(aqDateECB)
XLO_FUNC_START( aqDateECB(
    const ExcelObj& date,
    const ExcelObj& strictlyAfter ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateECB( toAQLDate( date ), toBool( strictlyAfter, true ) ) ) );
}
XLO_FUNC_END( aqDateECB )
    .help( L"The next ECB meeting date on or after the given date. Format the cell as a date." )
    .arg( L"Date",          L"The reference date" )
    .arg( L"StrictlyAfter", L"Optional. Default TRUE. TRUE skips a meeting that falls on Date itself" );
#endif


// ECB swap start date for a date.
#if AQ_XLL_ENABLED(aqDateECBSwapStart)
XLO_FUNC_START( aqDateECBSwapStart(
    const ExcelObj& date ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateECBSwapStart( toAQLDate( date ) ) ) );
}
XLO_FUNC_END( aqDateECBSwapStart )
    .help( L"The ECB-dated swap start date for the given date. Format the cell as a date." )
    .arg( L"Date", L"The reference date" );
#endif


// ECB swap end date for a date.
#if AQ_XLL_ENABLED(aqDateECBSwapEnd)
XLO_FUNC_START( aqDateECBSwapEnd(
    const ExcelObj& date ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateECBSwapEnd( toAQLDate( date ) ) ) );
}
XLO_FUNC_END( aqDateECBSwapEnd )
    .help( L"The ECB-dated swap end date for the given date. Format the cell as a date." )
    .arg( L"Date", L"The reference date" );
#endif


// The Nth ECB meeting date from an as-of date.
#if AQ_XLL_ENABLED(aqDateNthECBMeetingDate)
XLO_FUNC_START( aqDateNthECBMeetingDate(
    const ExcelObj& asOfDate,
    const ExcelObj& n ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateNthECBMeetingDate( toAQLDate( asOfDate ), toInt( n ) ) ) );
}
XLO_FUNC_END( aqDateNthECBMeetingDate )
    .help( L"The Nth ECB meeting date from an as-of date. Format the cell as a date." )
    .arg( L"AsOfDate", L"The as-of date" )
    .arg( L"N",        L"Which meeting (1 = the next one)" );
#endif


// The Nth ECB swap start date from an as-of date.
#if AQ_XLL_ENABLED(aqDateNthECBSwapStartDate)
XLO_FUNC_START( aqDateNthECBSwapStartDate(
    const ExcelObj& asOfDate,
    const ExcelObj& n ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateNthECBSwapStartDate( toAQLDate( asOfDate ), toInt( n ) ) ) );
}
XLO_FUNC_END( aqDateNthECBSwapStartDate )
    .help( L"The Nth ECB-dated swap start date from an as-of date. Format the cell as a date." )
    .arg( L"AsOfDate", L"The as-of date" )
    .arg( L"N",        L"Which one (1 = the next)" );
#endif


// The Nth ECB swap end date from an as-of date.
#if AQ_XLL_ENABLED(aqDateNthECBSwapEndDate)
XLO_FUNC_START( aqDateNthECBSwapEndDate(
    const ExcelObj& asOfDate,
    const ExcelObj& n ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateNthECBSwapEndDate( toAQLDate( asOfDate ), toInt( n ) ) ) );
}
XLO_FUNC_END( aqDateNthECBSwapEndDate )
    .help( L"The Nth ECB-dated swap end date from an as-of date. Format the cell as a date." )
    .arg( L"AsOfDate", L"The as-of date" )
    .arg( L"N",        L"Which one (1 = the next)" );
#endif


// The ECB meeting date after a given meeting date.
#if AQ_XLL_ENABLED(aqDateNextECBMeetingDate)
XLO_FUNC_START( aqDateNextECBMeetingDate(
    const ExcelObj& meetingDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateNextECBMeetingDate( toAQLDate( meetingDate ) ) ) );
}
XLO_FUNC_END( aqDateNextECBMeetingDate )
    .help( L"The ECB meeting date immediately after the given meeting date. Format the cell as a date." )
    .arg( L"MeetingDate", L"An ECB meeting date" );
#endif


// The ECB swap start date after a given one.
#if AQ_XLL_ENABLED(aqDateNextECBSwapStartDate)
XLO_FUNC_START( aqDateNextECBSwapStartDate(
    const ExcelObj& swapStartDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateNextECBSwapStartDate( toAQLDate( swapStartDate ) ) ) );
}
XLO_FUNC_END( aqDateNextECBSwapStartDate )
    .help( L"The ECB-dated swap start date immediately after the given one. Format the cell as a date." )
    .arg( L"SwapStartDate", L"An ECB-dated swap start date" );
#endif


// The ECB swap end date after a given one.
#if AQ_XLL_ENABLED(aqDateNextECBSwapEndDate)
XLO_FUNC_START( aqDateNextECBSwapEndDate(
    const ExcelObj& swapEndDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateNextECBSwapEndDate( toAQLDate( swapEndDate ) ) ) );
}
XLO_FUNC_END( aqDateNextECBSwapEndDate )
    .help( L"The ECB-dated swap end date immediately after the given one. Format the cell as a date." )
    .arg( L"SwapEndDate", L"An ECB-dated swap end date" );
#endif


// The IMM date for a given year and month.
#if AQ_XLL_ENABLED(aqDateIMMFromMonth)
XLO_FUNC_START( aqDateIMMFromMonth(
    const ExcelObj& year,
    const ExcelObj& month,
    const ExcelObj& calendar,
    const ExcelObj& businessDayAdj ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateIMMFromMonth(
        toInt( year ), toInt( month ), toAQLString( calendar ), toAQLString( businessDayAdj ) ) ) );
}
XLO_FUNC_END( aqDateIMMFromMonth )
    .help( L"The IMM date for a given year and month. Format the cell as a date." )
    .arg( L"Year",           L"Calendar year, e.g. 2026" )
    .arg( L"Month",          L"Calendar month, 1-12" )
    .arg( L"Calendar",       L"Holiday centre(s)" )
    .arg( L"BusinessDayAdj", L"Business day adjustment" );
#endif


// The Nth IMM date of a year.
#if AQ_XLL_ENABLED(aqDateNthIMM)
XLO_FUNC_START( aqDateNthIMM(
    const ExcelObj& year,
    const ExcelObj& nth,
    const ExcelObj& calendar,
    const ExcelObj& businessDayAdj ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateNthIMM(
        toInt( year ), toInt( nth ), toAQLString( calendar ), toAQLString( businessDayAdj ) ) ) );
}
XLO_FUNC_END( aqDateNthIMM )
    .help( L"The Nth IMM date of a calendar year. Format the cell as a date." )
    .arg( L"Year",           L"Calendar year, e.g. 2026" )
    .arg( L"Nth",            L"Which IMM date of the year, 1-4" )
    .arg( L"Calendar",       L"Holiday centre(s)" )
    .arg( L"BusinessDayAdj", L"Business day adjustment" );
#endif


// The Nth IMM date on or after a start date.
#if AQ_XLL_ENABLED(aqDateNthIMMFromStartDate)
XLO_FUNC_START( aqDateNthIMMFromStartDate(
    const ExcelObj& startDate,
    const ExcelObj& nth,
    const ExcelObj& calendar,
    const ExcelObj& businessDayAdj ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateNthIMMFromStartDate(
        toAQLDate( startDate ), toInt( nth ), toAQLString( calendar ), toAQLString( businessDayAdj ) ) ) );
}
XLO_FUNC_END( aqDateNthIMMFromStartDate )
    .help( L"The Nth IMM date on or after a start date. Format the cell as a date." )
    .arg( L"StartDate",      L"The start date" )
    .arg( L"Nth",            L"Which IMM date counting from StartDate, 1-based" )
    .arg( L"Calendar",       L"Holiday centre(s)" )
    .arg( L"BusinessDayAdj", L"Business day adjustment" );
#endif


// The current IMM date relative to a valuation date.
#if AQ_XLL_ENABLED(aqDateIMMCurrent)
XLO_FUNC_START( aqDateIMMCurrent(
    const ExcelObj& valuationDate,
    const ExcelObj& includeToday ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateIMMCurrent(
        toAQLDate( valuationDate ), toBool( includeToday, false ) ) ) );
}
XLO_FUNC_END( aqDateIMMCurrent )
    .help( L"The current (most recent) IMM date relative to a valuation date. Format the cell as a date." )
    .arg( L"ValuationDate", L"The valuation date" )
    .arg( L"IncludeToday",  L"Optional. Default FALSE. TRUE lets an IMM date on the valuation date count as current" );
#endif


// The IMM date after a reference date.
#if AQ_XLL_ENABLED(aqDateIMMNext)
XLO_FUNC_START( aqDateIMMNext(
    const ExcelObj& referenceDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateIMMNext( toAQLDate( referenceDate ) ) ) );
}
XLO_FUNC_END( aqDateIMMNext )
    .help( L"The IMM date immediately after a reference date. Format the cell as a date." )
    .arg( L"ReferenceDate", L"The reference date" );
#endif


// The IMM date before a reference date.
#if AQ_XLL_ENABLED(aqDateIMMPrevious)
XLO_FUNC_START( aqDateIMMPrevious(
    const ExcelObj& referenceDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateIMMPrevious( toAQLDate( referenceDate ) ) ) );
}
XLO_FUNC_END( aqDateIMMPrevious )
    .help( L"The IMM date immediately before a reference date. Format the cell as a date." )
    .arg( L"ReferenceDate", L"The reference date" );
#endif


// The Nth IMM date relative to a valuation date.
#if AQ_XLL_ENABLED(aqDateIMMNth)
XLO_FUNC_START( aqDateIMMNth(
    const ExcelObj& valuationDate,
    const ExcelObj& nthIMM,
    const ExcelObj& includeToday ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqDateIMMNth(
        toAQLDate( valuationDate ), toInt( nthIMM ), toBool( includeToday, false ) ) ) );
}
XLO_FUNC_END( aqDateIMMNth )
    .help( L"The Nth IMM date relative to a valuation date. Format the cell as a date." )
    .arg( L"ValuationDate", L"The valuation date" )
    .arg( L"NthIMM",        L"Which IMM date; positive is forward, negative is backward" )
    .arg( L"IncludeToday",  L"Optional. Default FALSE. TRUE lets an IMM date on the valuation date count" );
#endif


// TRUE if a start/maturity pair describes a regular (whole-period) swap schedule.
#if AQ_XLL_ENABLED(aqDateIsRegularSwapSchedule)
XLO_FUNC_START( aqDateIsRegularSwapSchedule(
    const ExcelObj& swapStart,
    const ExcelObj& swapMaturity,
    const ExcelObj& isMaturityAdjusted,
    const ExcelObj& frequency,
    const ExcelObj& busDayAdj,
    const ExcelObj& calendar,
    const ExcelObj& rollDay,
    const ExcelObj& rollConvention ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqDateIsRegularSwapSchedule(
        toAQLDate( swapStart ),
        toAQLDate( swapMaturity ),
        toBool( isMaturityAdjusted, false ),
        toAQLString( frequency ),
        toAQLString( busDayAdj ),
        toAQLString( calendar ),
        toInt( rollDay ),
        toAQLString( rollConvention ) ) );
}
XLO_FUNC_END( aqDateIsRegularSwapSchedule )
    .help( L"TRUE if the start/maturity pair describes a regular (whole-period) swap schedule." )
    .arg( L"SwapStart",          L"Swap start date" )
    .arg( L"SwapMaturity",       L"Swap maturity date" )
    .arg( L"IsMaturityAdjusted", L"TRUE if the maturity date has already been business-day adjusted" )
    .arg( L"Frequency",          L"Coupon frequency, e.g. 3M, 6M, 1Y" )
    .arg( L"BusDayAdj",          L"Business day adjustment" )
    .arg( L"Calendar",           L"Holiday centre(s)" )
    .arg( L"RollDay",            L"Roll day of month (0 to derive from the start date)" )
    .arg( L"RollConvention",     L"Optional. Roll convention, e.g. Normal, IMM, EOM" );
#endif
