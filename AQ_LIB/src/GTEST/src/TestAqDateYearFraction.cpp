// Includes: This Library
#include "tryAqDate.h"
#include "AQLCoreAppError.h"

// Include: Google Test Library
#include <gTest/gTest.h>

namespace google_test
{

	/*
	 * @brief These tests exercise tryAqDateYearFraction(). It checks that we obtain the expected dates.
	 */
    TEST( TestAqDateYearFraction, UNIT_with_ACT_365FJ )
    {
		AQLString dayCount("ACT/365FJ");

        AQLDate fromDate1("20000101");
		AQLDate toDate1("20000202");
        double yearFaction1 = validation::tryAqDateYearFraction(fromDate1, toDate1, dayCount, false);
		EXPECT_EQ( 32, yearFaction1 * 365 );

        AQLDate fromDate2("20000101");
		AQLDate toDate2("20000301");
        double yearFaction2 = validation::tryAqDateYearFraction(fromDate2, toDate2, dayCount, false);
		EXPECT_EQ( 59, yearFaction2 * 365 );

        AQLDate fromDate3("20000201");
		AQLDate toDate3("20000202");
        double yearFaction3 = validation::tryAqDateYearFraction(fromDate3, toDate3, dayCount, false);
		EXPECT_EQ( 1, yearFaction3 * 365 );

        AQLDate fromDate4("20000201");
		AQLDate toDate4("20000229");
        double yearFaction4 = validation::tryAqDateYearFraction(fromDate4, toDate4, dayCount, false);
		EXPECT_EQ( 28, yearFaction4 * 365 );

        AQLDate fromDate5("20000201");
		AQLDate toDate5("20000228");
        double yearFaction5 = validation::tryAqDateYearFraction(fromDate5, toDate5, dayCount, false);
		EXPECT_EQ( 27, yearFaction5 * 365 );

        AQLDate fromDate6("20000201");
		AQLDate toDate6("20000301");
        double yearFaction6 = validation::tryAqDateYearFraction(fromDate6, toDate6, dayCount, false);
		EXPECT_EQ( 28, yearFaction6 * 365 );

        AQLDate fromDate7("20170418");
		AQLDate toDate7("20210320");
        double yearFaction7 = validation::tryAqDateYearFraction(fromDate7, toDate7, dayCount, false);
		EXPECT_EQ( 1431, yearFaction7 * 365 );



    }



}

