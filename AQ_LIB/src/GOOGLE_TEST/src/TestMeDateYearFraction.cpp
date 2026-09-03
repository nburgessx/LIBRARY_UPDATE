// Includes: This Library
#include "tryMeDate.h"
#include "LACoreAppError.h"

// Include: Google Test Library
#include <gTest/gTest.h>

namespace google_test
{

	/*
	 * @brief These tests exercise tryMeDateYearFraction(). It checks that we obtain the expected dates.
	 */
    TEST( TestMeDateYearFraction, UNIT_with_ACT_365FJ )
    {
		LAString dayCount("ACT/365FJ");

        LADate fromDate1("20000101");
		LADate toDate1("20000202");
        double yearFaction1 = validation_api::tryMeDateYearFraction(fromDate1, toDate1, dayCount, false);
		EXPECT_EQ( 32, yearFaction1 * 365 );

        LADate fromDate2("20000101");
		LADate toDate2("20000301");
        double yearFaction2 = validation_api::tryMeDateYearFraction(fromDate2, toDate2, dayCount, false);
		EXPECT_EQ( 59, yearFaction2 * 365 );

        LADate fromDate3("20000201");
		LADate toDate3("20000202");
        double yearFaction3 = validation_api::tryMeDateYearFraction(fromDate3, toDate3, dayCount, false);
		EXPECT_EQ( 1, yearFaction3 * 365 );

        LADate fromDate4("20000201");
		LADate toDate4("20000229");
        double yearFaction4 = validation_api::tryMeDateYearFraction(fromDate4, toDate4, dayCount, false);
		EXPECT_EQ( 28, yearFaction4 * 365 );

        LADate fromDate5("20000201");
		LADate toDate5("20000228");
        double yearFaction5 = validation_api::tryMeDateYearFraction(fromDate5, toDate5, dayCount, false);
		EXPECT_EQ( 27, yearFaction5 * 365 );

        LADate fromDate6("20000201");
		LADate toDate6("20000301");
        double yearFaction6 = validation_api::tryMeDateYearFraction(fromDate6, toDate6, dayCount, false);
		EXPECT_EQ( 28, yearFaction6 * 365 );

        LADate fromDate7("20170418");
		LADate toDate7("20210320");
        double yearFaction7 = validation_api::tryMeDateYearFraction(fromDate7, toDate7, dayCount, false);
		EXPECT_EQ( 1431, yearFaction7 * 365 );



    }



}

