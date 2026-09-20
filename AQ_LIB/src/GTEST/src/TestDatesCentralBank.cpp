// TestDatesCentralBank.cpp

// Includes: This Library
#include "AQLCentralBank.h"
#include "AQLCoreAppError.h"
#include "TestHelperUtilities.h"

// Include: Google Test Library
#include <gTest/gTest.h>

namespace google_test
{

    TEST( TestDatesCentralBank, UNIT_ErrorHandling )
    {
        EXPECT_THROW( AQLCentralBank::meetingSchedule( "XXX" ), AQLCoreAppError );
    }

    TEST( TestDatesCentralBank, UNIT_AccessECBSchedule )
    {
        const DateVector& ecbDates = AQLCentralBank::meetingSchedule( "eCb" );

        EXPECT_EQ( ecbDates[0], AQLDate( "20150122" ) );
        EXPECT_EQ( ecbDates[1], AQLDate( "20150305" ) );
        EXPECT_EQ( ecbDates[7], AQLDate( "20151203" ) );
        EXPECT_EQ( ecbDates[8], AQLDate( "20160121" ) );
        EXPECT_EQ( ecbDates[9], AQLDate( "20160310" ) );
        EXPECT_EQ( ecbDates[15], AQLDate( "20161208" ) );
    }

    TEST( TestDatesCentralBank, UNIT_AccessBoeSchedule )
    {
        const DateVector& boeDates = AQLCentralBank::meetingSchedule( "BoE" );

        EXPECT_EQ( boeDates.size(), 0 );
    }

    TEST( TestDatesCentralBank, UNIT_AccessFedSchedule )
    {
        const DateVector& fedDates = AQLCentralBank::meetingSchedule( "FED" );

        EXPECT_EQ( fedDates.size(), 0 );
    }

    TEST( TestDatesCentralBank, UNIT_AreThereEnoughDates )
    {
        const DateVector& ecbDates = AQLCentralBank::meetingSchedule( "ECB" );

        AQLDate today;
        today.setSystemDate();

        const AQLDate last = ecbDates.back();

        int yearsToLast = 0;
        int monthsToLast = 0;
        int daysToLast = 0;

        today.intervalYMD( last, yearsToLast, monthsToLast, daysToLast );

        const int totalDaysToLast = 360 * yearsToLast + 30 * monthsToLast + daysToLast;

        // Check that we have at least 6 months of ECB dates, which are published annually.
        // Warning, not a hard failure (GTEST_CONDITIONAL_WARNING, not EXPECT_GE) - ECB meeting
        // dates are discretionary policy decisions published roughly a year at a time, not a
        // rule-based calendar that self-extends, so this data will predictably run low well
        // before anyone gets around to refreshing it. A hard test failure here just means
        // "nobody has manually updated AQLCentralBank.cpp/CBSchedule.csv recently enough" - worth
        // knowing about, not worth blocking a build over. Matches
        // ECB_Calendars.UNIT_ECB_Calendar_Expiry_Test's own warning-vs-error split in
        // TestAqDateECB.cpp.
        GTEST_CONDITIONAL_WARNING( totalDaysToLast >= 150,
            "AQLCentralBank's ECB meeting schedule has fewer than 150 days of forward dates left "
            "(" + std::to_string( totalDaysToLast ) + ") - consider refreshing AQLCentralBank.cpp's "
            "hardcoded table and resources/config/CBSchedule.csv from the ECB's published calendar." );
    }
}

