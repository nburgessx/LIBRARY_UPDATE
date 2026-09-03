// TestUtilitiesTime.cpp
#include <string>

// Includes: This Library
#include <DateUtilities.h>

// Include: Google Test Library
#include <gTest/gTest.h>



TEST( TestUtilitiesTime, UNIT_GetCurrentTime_TestForFailureOnly )
{
    // Check funtion does not fail
    EXPECT_NO_THROW( etrading::getCurrentDateTime() );
    EXPECT_NO_FATAL_FAILURE( etrading::getCurrentDateTime() );
}

TEST( TestUtilitiesTime, UNIT_GetCurrentDecade_TestForFailureOnly )
{
    // Check funtion does not fail
    EXPECT_NO_THROW( etrading::getCurrentDecade() );
    EXPECT_NO_FATAL_FAILURE( etrading::getCurrentDecade() );
}

TEST( TestUtilitiesTime, UNIT_GetCurrentCentury_TestForFailureOnly )
{
    // Check funtion does not fail
    EXPECT_NO_THROW( etrading::getCurrentCentury() );
    EXPECT_NO_FATAL_FAILURE( etrading::getCurrentCentury() );
}