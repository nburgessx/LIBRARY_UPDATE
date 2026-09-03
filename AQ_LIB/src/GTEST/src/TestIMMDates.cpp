// TestIMMDates.cpp
#include "FuturesDates.h"

// Include: Google Test Library
#include <gTest/gTest.h>


TEST( TestIMMUtilityMethods, UNIT_monthYearCurrentIMM )
{
    etrading::MonthYear     monthYear;

    // Jan2018 - Current IMM Started in Dec 2017
    AQLDate jan2018 = AQLDate("20180101");
    monthYear = etrading::monthYearCurrentIMM( jan2018 );
    EXPECT_EQ( monthYear.month_, 12 );
    EXPECT_EQ( monthYear.year_,  2017 );

    // Feb2018 - Current IMM Started in Dec 2017
    AQLDate feb2018 = AQLDate("20180201");
    monthYear = etrading::monthYearCurrentIMM( feb2018 );
    EXPECT_EQ( monthYear.month_, 12 );
    EXPECT_EQ( monthYear.year_,  2017 );
    
    // Mar2018 - Current IMM Started in Mar 2018
    AQLDate mar2018 = AQLDate("20180301");
    monthYear = etrading::monthYearCurrentIMM( mar2018 );
    EXPECT_EQ( monthYear.month_, 3 );
    EXPECT_EQ( monthYear.year_,  2018 );

    // Apr2018 - Current IMM Started in Mar 2018
    AQLDate apr2018 = AQLDate("20180401");
    monthYear = etrading::monthYearCurrentIMM( apr2018 );
    EXPECT_EQ( monthYear.month_, 3 );
    EXPECT_EQ( monthYear.year_,  2018 );

    // May2018 - Current IMM Started in Mar 2018
    AQLDate may2018 = AQLDate("20180501");
    monthYear = etrading::monthYearCurrentIMM( may2018 );
    EXPECT_EQ( monthYear.month_, 3 );
    EXPECT_EQ( monthYear.year_,  2018 );

    // Jun2018 - Current IMM Started in Jun 2018
    AQLDate jun2018 = AQLDate("20180601");
    monthYear = etrading::monthYearCurrentIMM( jun2018 );
    EXPECT_EQ( monthYear.month_, 6 );
    EXPECT_EQ( monthYear.year_,  2018 );

    // Jul2018 - Current IMM Started in Jun 2018
    AQLDate jul2018 = AQLDate("20180701");
    monthYear = etrading::monthYearCurrentIMM( jul2018 );
    EXPECT_EQ( monthYear.month_, 6 );
    EXPECT_EQ( monthYear.year_,  2018 );

    // Aug2018 - Current IMM Started in Jun 2018
    AQLDate aug2018 = AQLDate("20180801");
    monthYear = etrading::monthYearCurrentIMM( aug2018 );
    EXPECT_EQ( monthYear.month_, 6 );
    EXPECT_EQ( monthYear.year_,  2018 );

    // Sep2018 - Current IMM Started in Sep 2018
    AQLDate sep2018 = AQLDate("20180901");
    monthYear = etrading::monthYearCurrentIMM( sep2018 );
    EXPECT_EQ( monthYear.month_, 9 );
    EXPECT_EQ( monthYear.year_,  2018 );

    // Oct2018 - Current IMM Started in Sep 2018
    AQLDate oct2018 = AQLDate("20181001");
    monthYear = etrading::monthYearCurrentIMM( oct2018 );
    EXPECT_EQ( monthYear.month_, 9 );
    EXPECT_EQ( monthYear.year_,  2018 );

    // Nov2018 - Current IMM Started in Sep 2018
    AQLDate nov2018 = AQLDate("20181101");
    monthYear = etrading::monthYearCurrentIMM( nov2018 );
    EXPECT_EQ( monthYear.month_, 9 );
    EXPECT_EQ( monthYear.year_,  2018 );

    // Dec2018 - Current IMM Started in Dec 2018
    AQLDate dec2018 = AQLDate("20181201");
    monthYear = etrading::monthYearCurrentIMM( dec2018 );
    EXPECT_EQ( monthYear.month_, 12 );
    EXPECT_EQ( monthYear.year_,  2018 );

    // Jan2019 - Current IMM Started in Dec 2018
    AQLDate jan2019 = AQLDate("20190101");
    monthYear = etrading::monthYearCurrentIMM( jan2019 );
    EXPECT_EQ( monthYear.month_, 12 );
    EXPECT_EQ( monthYear.year_,  2018 );
}

TEST( TestIMMUtilityMethods, UNIT_monthYearRollIMMForwards )
{
    etrading::MonthYear monthYear;
    monthYear.month_    = 1;
    monthYear.year_     = 2018;

    monthYearRollIMMForwards( monthYear, 0 );
    EXPECT_EQ( monthYear.month_, 1 );
    EXPECT_EQ( monthYear.year_,  2018 );

    monthYearRollIMMForwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 2 );
    EXPECT_EQ( monthYear.year_,  2018 );

    monthYearRollIMMForwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 3 );
    EXPECT_EQ( monthYear.year_,  2018 );

    monthYearRollIMMForwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 4 );
    EXPECT_EQ( monthYear.year_,  2018 );

    monthYearRollIMMForwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 5 );
    EXPECT_EQ( monthYear.year_,  2018 );

    monthYearRollIMMForwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 6 );
    EXPECT_EQ( monthYear.year_,  2018 );

    monthYearRollIMMForwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 7 );
    EXPECT_EQ( monthYear.year_,  2018 );

    monthYearRollIMMForwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 8 );
    EXPECT_EQ( monthYear.year_,  2018 );

    monthYearRollIMMForwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 9 );
    EXPECT_EQ( monthYear.year_,  2018 );

    monthYearRollIMMForwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 10 );
    EXPECT_EQ( monthYear.year_,  2018 );

    monthYearRollIMMForwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 11 );
    EXPECT_EQ( monthYear.year_,  2018 );

    monthYearRollIMMForwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 12 );
    EXPECT_EQ( monthYear.year_,  2018 );

    // *** IMPORTANT TESTS ***
    // -------------------------------------------
    monthYearRollIMMForwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 1 );
    EXPECT_EQ( monthYear.year_,  2019 );

    monthYearRollIMMForwards( monthYear, 2 );
    EXPECT_EQ( monthYear.month_, 3 );
    EXPECT_EQ( monthYear.year_,  2019 );

    monthYearRollIMMForwards( monthYear, 3 );
    EXPECT_EQ( monthYear.month_, 6 );
    EXPECT_EQ( monthYear.year_,  2019 );

    monthYearRollIMMForwards( monthYear, 3 );
    EXPECT_EQ( monthYear.month_, 9 );
    EXPECT_EQ( monthYear.year_,  2019 );

    monthYearRollIMMForwards( monthYear, 3 );
    EXPECT_EQ( monthYear.month_, 12 );
    EXPECT_EQ( monthYear.year_,  2019 );

    monthYearRollIMMForwards( monthYear, 3 );
    EXPECT_EQ( monthYear.month_, 3 );
    EXPECT_EQ( monthYear.year_,  2020 );
}


TEST( TestIMMUtilityMethods, UNIT_monthYearRollIMMBackwards )
{
    etrading::MonthYear monthYear;
    monthYear.month_    = 1;
    monthYear.year_     = 2018;

    // Roll 0 Months Backwards
    monthYearRollIMMBackwards( monthYear, 0 );
    EXPECT_EQ( monthYear.month_, 1 );
    EXPECT_EQ( monthYear.year_,  2018 );

    // Roll Back 1 Month
    monthYearRollIMMBackwards( monthYear, 1 );
    EXPECT_EQ( monthYear.month_, 12 );
    EXPECT_EQ( monthYear.year_,  2017 );

    // Roll Back 3 Months
    monthYearRollIMMBackwards( monthYear, 3 );
    EXPECT_EQ( monthYear.month_, 9 );
    EXPECT_EQ( monthYear.year_,  2017 );

    // Roll Back 3 Months
    monthYearRollIMMBackwards( monthYear, 3 );
    EXPECT_EQ( monthYear.month_, 6 );
    EXPECT_EQ( monthYear.year_,  2017 );

    // Roll Back 3 Months
    monthYearRollIMMBackwards( monthYear, 3 );
    EXPECT_EQ( monthYear.month_, 3 );
    EXPECT_EQ( monthYear.year_,  2017 );

    // Roll Back 3 Months
    monthYearRollIMMBackwards( monthYear, 3 );
    EXPECT_EQ( monthYear.month_, 12 );
    EXPECT_EQ( monthYear.year_,  2016 );
}


TEST( TestIMMDates, UNIT_currentIMMDate )
{
    AQLDate result_IMM_Date;

    // IMM Dates
    AQLDate dec2017_IMM_Date("20171220");
    AQLDate mar2018_IMM_Date("20180321");
    AQLDate jun2018_IMM_Date("20180620");
    AQLDate sep2018_IMM_Date("20180919");
    AQLDate dec2018_IMM_Date("20181219");
    
    // 1. Test the Include Today Parameter
    // -----------------------------------

    // Given Sep-2018 IMM Date the Current IMM is Sep-2018 when including today
    result_IMM_Date = etrading::currentIMMDate( sep2018_IMM_Date, true ); // IncludeToday = true - since jun2-018 IMM has rolled today!!!
    EXPECT_EQ( sep2018_IMM_Date, result_IMM_Date );

    // Given Sep-2018 IMM Date the Current IMM is the previous IMM Jun-2018 when *** NOT *** including today - since previous IMM not rolled yet
    result_IMM_Date = etrading::currentIMMDate( sep2018_IMM_Date, false ); // IncludeToday = false
    EXPECT_EQ( jun2018_IMM_Date, result_IMM_Date );

    // 2. Test the Current IMM Date Function using every month of 2018 as valudation date input
    AQLDate jan2018("20180101");
    AQLDate feb2018("20180201");
    AQLDate mar2018("20180301");
    AQLDate apr2018("20180401");
    AQLDate may2018("20180501");
    AQLDate jun2018("20180601");
    AQLDate jul2018("20180701");
    AQLDate aug2018("20180801");
    AQLDate sep2018("20180901");
    AQLDate oct2018("20181001");
    AQLDate nov2018("20181101");
    AQLDate dec2018("20181201");
    
    result_IMM_Date = etrading::currentIMMDate( jan2018, true ); // IncludeToday = true
    EXPECT_EQ( dec2017_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::currentIMMDate( feb2018, true ); // IncludeToday = true
    EXPECT_EQ( dec2017_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::currentIMMDate( mar2018, true ); // IncludeToday = true
    EXPECT_EQ( dec2017_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::currentIMMDate( apr2018, true ); // IncludeToday = true
    EXPECT_EQ( mar2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::currentIMMDate( may2018, true ); // IncludeToday = true
    EXPECT_EQ( mar2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::currentIMMDate( jun2018, true ); // IncludeToday = true
    EXPECT_EQ( mar2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::currentIMMDate( jul2018, true ); // IncludeToday = true
    EXPECT_EQ( jun2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::currentIMMDate( aug2018, true ); // IncludeToday = true
    EXPECT_EQ( jun2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::currentIMMDate( sep2018, true ); // IncludeToday = true
    EXPECT_EQ( jun2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::currentIMMDate( oct2018, true ); // IncludeToday = true
    EXPECT_EQ( sep2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::currentIMMDate( nov2018, true ); // IncludeToday = true
    EXPECT_EQ( sep2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::currentIMMDate( dec2018, true ); // IncludeToday = true
    EXPECT_EQ( sep2018_IMM_Date, result_IMM_Date );
}


TEST( TestIMMDates, UNIT_nextIMMDate )
{
    AQLDate result_IMM_Date;

    // IMM Dates
    AQLDate dec2017_IMM_Date("20171220");
    AQLDate mar2018_IMM_Date("20180321");
    AQLDate jun2018_IMM_Date("20180620");
    AQLDate sep2018_IMM_Date("20180919");
    AQLDate dec2018_IMM_Date("20181219");
    AQLDate mar2019_IMM_Date("20190320");

    // Next IMM from Jan 2018 is Mar 2018
    AQLDate referenceDate("20180101");
    result_IMM_Date = etrading:: nextIMMDate( referenceDate );
    EXPECT_EQ( result_IMM_Date, mar2018_IMM_Date );

    // Iterate Over IMMs for Next IMMs
    result_IMM_Date = etrading:: nextIMMDate( result_IMM_Date );
    EXPECT_EQ( result_IMM_Date, jun2018_IMM_Date );

    result_IMM_Date = etrading:: nextIMMDate( result_IMM_Date );
    EXPECT_EQ( result_IMM_Date, sep2018_IMM_Date );

    result_IMM_Date = etrading:: nextIMMDate( result_IMM_Date );
    EXPECT_EQ( result_IMM_Date, dec2018_IMM_Date );

    result_IMM_Date = etrading:: nextIMMDate( result_IMM_Date );
    EXPECT_EQ( result_IMM_Date, mar2019_IMM_Date );
}

TEST( TestIMMDates, UNIT_previousIMMDate )
{
    AQLDate result_IMM_Date;

    // IMM Dates
    AQLDate dec2017_IMM_Date("20171220");
    AQLDate mar2018_IMM_Date("20180321");
    AQLDate jun2018_IMM_Date("20180620");
    AQLDate sep2018_IMM_Date("20180919");
    AQLDate dec2018_IMM_Date("20181219");
    AQLDate mar2019_IMM_Date("20190320");

    // From 1st April 2018 viewpoint the current IMM is Mar-2018 ... hence previous is Dec-2107
    AQLDate referenceDate("20190401");
    result_IMM_Date = etrading:: previousIMMDate( referenceDate );
    EXPECT_EQ( dec2018_IMM_Date, result_IMM_Date );

    // Iterate Over IMMs for Next IMMs
    result_IMM_Date = etrading:: previousIMMDate( result_IMM_Date );
    EXPECT_EQ( sep2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading:: previousIMMDate( result_IMM_Date );
    EXPECT_EQ( jun2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading:: previousIMMDate( result_IMM_Date );
    EXPECT_EQ( mar2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading:: previousIMMDate( result_IMM_Date );
    EXPECT_EQ( dec2017_IMM_Date, result_IMM_Date );
}

TEST( TestIMMDates, UNIT_nthIMMDate )
{
    AQLDate result_IMM_Date;

    // IMM Dates
    AQLDate jun2017_IMM_Date("20170621");
    AQLDate sep2017_IMM_Date("20170920");
    AQLDate dec2017_IMM_Date("20171220"); // <----- Current IMM relative to Reference Date 1st-Jan-2018
    AQLDate mar2018_IMM_Date("20180321");
    AQLDate jun2018_IMM_Date("20180620");
    AQLDate sep2018_IMM_Date("20180919");
    AQLDate dec2018_IMM_Date("20181219");
    AQLDate mar2019_IMM_Date("20190320");

    // Reference Date Jan-2018, Zero-th IMM is Dec-17
    // ----------------------------------------------
    AQLDate referenceDate("20180101");
    result_IMM_Date = etrading::nthIMMDate( referenceDate, 0 );
    EXPECT_EQ( result_IMM_Date, dec2017_IMM_Date );

    // Roll IMM Dates Forwards
    // ----------------------------------------------
    result_IMM_Date = etrading::nthIMMDate( referenceDate, 1 );
    EXPECT_EQ( mar2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::nthIMMDate( referenceDate, 2 );
    EXPECT_EQ( jun2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::nthIMMDate( referenceDate, 3 );
    EXPECT_EQ( sep2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::nthIMMDate( referenceDate, 4 );
    EXPECT_EQ( dec2018_IMM_Date, result_IMM_Date );

    result_IMM_Date = etrading::nthIMMDate( referenceDate, 5 );
    EXPECT_EQ( mar2019_IMM_Date, result_IMM_Date );

    // Roll IMM Dates Backwards
    // ----------------------------------------------

    // Removed because no longer supported ... does this make sense

    // result_IMM_Date = etrading::nthIMMDate( referenceDate, -1 );
    // EXPECT_EQ( sep2017_IMM_Date, result_IMM_Date );

    // result_IMM_Date = etrading::nthIMMDate( referenceDate, -2 );
    // EXPECT_EQ( jun2017_IMM_Date, result_IMM_Date );

}