
// TestUtilitiesSetup.cpp
#include <string>
#include "tryAqToolSetup.h"
#include "OMPThreadManager.h"

// Include: Google Test Library
#include <gTest/gTest.h>



TEST( TestUtilitiesSetupAQL, UNIT_SetupAQ_and_TearDownAQ_CheckForSuccess )
{
    // Parameters: configFolder, calendarPath, cbSchedulePath, startupConfigPath, irPropsPath
    EXPECT_NO_THROW( validation::tryAqToolInitialize( "", "", "", "", "" ) );
    EXPECT_NO_THROW( validation::tryAqToolTearDown() );
}


TEST( TestUtilitiesSetupAQL, UNIT_SetupAQ_and_TearDownAQ_CheckForInvalidPathInputs )
{
    // tryAqToolInitialize always tears down first (see the note above its definition), so each
    // case below is guaranteed to actually (re)validate the override path being tested rather
    // than short-circuiting on a still-live instance from the previous case.

    // Check function correctly fails on an invalid ir.properties path
    EXPECT_ANY_THROW( validation::tryAqToolInitialize( "", "", "", "", "invalid ir.properties filepath" ) );
    EXPECT_NO_THROW( validation::tryAqToolTearDown() );

    // Check function correctly fails on an invalid calendar path
    EXPECT_ANY_THROW( validation::tryAqToolInitialize( "", "invalid calendar filepath", "", "", "" ) );
    EXPECT_NO_THROW( validation::tryAqToolTearDown() );

    // Check function correctly fails on an invalid central bank calendar path
    EXPECT_ANY_THROW( validation::tryAqToolInitialize( "", "", "Invalid central bank calendar filepath", "", "" ) );
    EXPECT_NO_THROW( validation::tryAqToolTearDown() );

    // Check function correctly passes with no overrides
    EXPECT_NO_THROW( validation::tryAqToolInitialize( "", "", "", "", "" ) );
    EXPECT_NO_THROW( validation::tryAqToolTearDown() );
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

