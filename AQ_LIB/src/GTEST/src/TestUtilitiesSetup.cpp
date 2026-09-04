
// TestUtilitiesSetup.cpp
#include <string>
#include "tryAqToolsSetup.h"
#include "OMPThreadManager.h"

// Include: Google Test Library
#include <gTest/gTest.h>



TEST( TestUtilitiesSetupAQL, UNIT_SetupAQ_and_TearDownAQ_CheckForSuccess )
{
    // Check funtion does not fail
    // Parameters: ir.properties filepath, calendar filepath and central bank calendar filepath respectively
    EXPECT_NO_THROW( validation::trySetupAQL( "", "", "" ));
    EXPECT_NO_THROW( validation::tryTearDownAQL() );
}


TEST( TestUtilitiesSetupAQL, UNIT_SetupAQ_and_TearDownAQ_CheckForInvalidPathInputs )
{
    // Check funtion correctly fails
    // Parameters: ir.properties filepath, calendar filepath and central bank calendar filepath respectively
    EXPECT_ANY_THROW( validation::trySetupAQL( "invalid ir.properties filepath", "", "" ));
    EXPECT_NO_THROW( validation::tryTearDownAQL() );

    EXPECT_ANY_THROW( validation::trySetupAQL( "", "invalid calendar filepath", "" ));
    EXPECT_NO_THROW( validation::tryTearDownAQL() );

    EXPECT_ANY_THROW( validation::trySetupAQL( "", "", "Invalid central bank calendar filepath" ));
    EXPECT_NO_THROW( validation::tryTearDownAQL() );

    // Check function correctly passes
    // Parameters: ir.properties filepath, calendar filepath and central bank calendar filepath respectively
    EXPECT_NO_THROW( validation::trySetupAQL( "", "", "" ));
    EXPECT_NO_THROW( validation::tryTearDownAQL() );
}

TEST( TestParallelModeUsingOMP, UNIT_ParallelModeUsingOMP_EnableAndStatus_Methods )
{
    // Default Mode should be enabled = true
    const bool defaultMode = etrading::getIsOMPEnabled();
    EXPECT_EQ( true, defaultMode );

    // Disable Parallel Mode
    etrading::setIsOMPEnabled( false );
    const bool disabledParallelMode = etrading::getIsOMPEnabled();
    EXPECT_EQ( false, disabledParallelMode );

    // Enable Parallel Mode
    etrading::setIsOMPEnabled( true );
    const bool enableParallelMode = etrading::getIsOMPEnabled();
    EXPECT_EQ( true, enableParallelMode );
}

