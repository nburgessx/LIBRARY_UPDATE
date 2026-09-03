// TestDatesCentralBank.cpp

// Includes: This Library
#include "LAMathCentralBank.h"
#include "LACoreAppError.h"

// Include: Google Test Library
#include <gTest/gTest.h>

namespace google_test
{

    TEST( TestDatesCentralBank, UNIT_ErrorHandling )
    {
        EXPECT_THROW( LAMathCentralBank::meetingSchedule( "XXX" ), LACoreAppError );
    }

    TEST( TestDatesCentralBank, UNIT_AccessECBSchedule )
    {
        const DateVector& ecbDates = LAMathCentralBank::meetingSchedule( "eCb" );

        EXPECT_EQ( ecbDates[0], LADate( "20150122" ) );
        EXPECT_EQ( ecbDates[1], LADate( "20150305" ) );
        EXPECT_EQ( ecbDates[7], LADate( "20151203" ) );
        EXPECT_EQ( ecbDates[8], LADate( "20160121" ) );
        EXPECT_EQ( ecbDates[9], LADate( "20160310" ) );
        EXPECT_EQ( ecbDates[15], LADate( "20161208" ) );
    }

    TEST( TestDatesCentralBank, UNIT_AccessBoeSchedule )
    {
        const DateVector& boeDates = LAMathCentralBank::meetingSchedule( "BoE" );

        EXPECT_EQ( boeDates.size(), 0 );
    }

    TEST( TestDatesCentralBank, UNIT_AccessFedSchedule )
    {
        const DateVector& fedDates = LAMathCentralBank::meetingSchedule( "FED" );

        EXPECT_EQ( fedDates.size(), 0 );
    }

    TEST( TestDatesCentralBank, UNIT_AreThereEnoughDates )
    {
        const DateVector& ecbDates = LAMathCentralBank::meetingSchedule( "ECB" );

        LADate today;
        today.setSystemDate();

        const LADate last = ecbDates.back();

        int yearsToLast = 0;
        int monthsToLast = 0;
        int daysToLast = 0;

        today.intervalYMD( last, yearsToLast, monthsToLast, daysToLast );

        const int totalDaysToLast = 360 * yearsToLast + 30 * monthsToLast + daysToLast;

        // Check that we have at least 6 months of ECB dates, which are published annually
        EXPECT_GE( totalDaysToLast, 150 );
    }
}

