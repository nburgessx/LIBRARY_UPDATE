// TestDatesCentralBank.cpp

// Includes: This Library
#include "AQLDateScheduleHelpers.h"
#include "AQLCoreAppError.h"

// Include: Google Test Library
#include <gTest/gTest.h>

namespace google_test
{

	/*
	 * @brief These tests exercise getDateFromTerm() and getTerm(). It checks that we obtain the expected dates
	 * and that the implied term matches the input term.
	 */
    TEST( TestMirDateFunctions, UNIT_checkDateFromTerm_ACT_365_ISDA )
    {
		// Test adding a year fraction onto a date
		AQLDate baseDate("20160914");
		double inputTerm = 0.122950819672131;
		AQLString dayCount("ACT/365_ISDA");
		AQLDate dateFromTerm1 = etrading::AQLDateScheduleHelpers::getDateFromTerm(baseDate, inputTerm, dayCount);

		EXPECT_EQ( AQLDate( "20161029" ), dateFromTerm1 );

		// Test calculating the year fraction between two dates. This should match the input year fraction
		double impliedTerm = etrading::AQLDateScheduleHelpers::getTerm(baseDate, dateFromTerm1, dayCount);
		double tolerance = 1.0e-9;

		EXPECT_NEAR( inputTerm, impliedTerm, tolerance );

		// Verify that the date from impliedTerm again reproduces the expected date
		AQLDate dateFromTerm2 = etrading::AQLDateScheduleHelpers::getDateFromTerm(baseDate, impliedTerm, dayCount);

		EXPECT_EQ( AQLDate( "20161029" ), dateFromTerm2 );
    }

	TEST( TestMirDateFunctions, UNIT_checkDateFromTerm_ACT_360 )
    {
		// Test adding a year fraction on to a date
		AQLDate baseDate("20160914");
		double inputTerm = 0.122950819672131;
		AQLString dayCount("ACT/360");
		AQLDate dateFromTerm1 = etrading::AQLDateScheduleHelpers::getDateFromTerm(baseDate, inputTerm, dayCount);

		EXPECT_EQ( AQLDate( "20161028" ), dateFromTerm1 );

		// NOTE: The impliedTerm is different from the inputTerm because the impliedTerm is a full-day term.
		// The difference is expected.
		double impliedTerm = etrading::AQLDateScheduleHelpers::getTerm(baseDate, dateFromTerm1, dayCount);
		double tolerance = 1.0e-9;

		EXPECT_NEAR( 0.122222222, impliedTerm, tolerance );

		// Most importantly, we reproduce the end date calculated in the first step
		AQLDate dateFromTerm2 = etrading::AQLDateScheduleHelpers::getDateFromTerm(baseDate, impliedTerm, dayCount);

		EXPECT_EQ( dateFromTerm1, dateFromTerm2 );
    }

	TEST( TestMirDateFunctions, UNIT_checkDateFromTerm_ACT_ACT )
    {
		// Test adding a year fraction onto a date
		AQLDate baseDate("20160914");
		double inputTerm = 0.122950819672131;    
		AQLString dayCount("ACT/ACT");
		AQLDate dateFromTerm1 = etrading::AQLDateScheduleHelpers::getDateFromTerm(baseDate, inputTerm, dayCount);

		EXPECT_EQ( AQLDate( "20161029" ), dateFromTerm1 );

		// Test calculating the year fraction between two dates. This should match the input year fraction
		double impliedTerm = etrading::AQLDateScheduleHelpers::getTerm(baseDate, dateFromTerm1, dayCount);
		double tolerance = 1.0e-9;

		EXPECT_NEAR( inputTerm, impliedTerm, tolerance );

		// Verify that the date from impliedTerm reproduces the expected date
		AQLDate dateFromTerm2 = etrading::AQLDateScheduleHelpers::getDateFromTerm(baseDate, impliedTerm, dayCount);

		EXPECT_EQ( AQLDate( "20161029" ), dateFromTerm2 );
	}

}

