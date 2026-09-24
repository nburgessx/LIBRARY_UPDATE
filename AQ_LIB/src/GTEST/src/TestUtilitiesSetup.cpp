
// TestUtilitiesSetup.cpp
#include <string>
#include "tryAqToolSetup.h"
#include "OMPThreadManager.h"

// Include: Google Test Library
#include <gTest/gTest.h>



TEST( TestUtilitiesSetupAQL, UNIT_SetupAQ_and_TearDownAQ_CheckForSuccess )
{
    // Defensive: tryAqToolInitialize is now idempotent (a no-op if already initialized -- see the
    // note above its definition), so start from a guaranteed clean slate rather than relying on
    // whatever an earlier test in the binary happened to leave behind.
    EXPECT_NO_THROW( validation::tryAqToolTearDown() );

    // Parameters: configFolder, calendarPath, cbSchedulePath, startupConfigPath, irPropsPath
    EXPECT_NO_THROW( validation::tryAqToolInitialize( "", "", "", "", "" ) );
    EXPECT_NO_THROW( validation::tryAqToolTearDown() );
}


TEST( TestUtilitiesSetupAQL, UNIT_Initialize_IsIdempotent_DoesNotThrowOrRebuildOnRepeatCall )
{
    // tryAqToolInitialize no longer tears down and rebuilds on every call (decided, Nicholas
    // 2026-09-24 -- see the note above its definition and tryAqToolReset). A second call while
    // already initialized must be a safe no-op: it must not throw, and it must not silently wipe
    // whatever the first call (or anything built on top of it) set up.
    EXPECT_NO_THROW( validation::tryAqToolTearDown() );
    EXPECT_NO_THROW( validation::tryAqToolInitialize( "", "", "", "", "" ) );
    EXPECT_NO_THROW( validation::tryAqToolInitialize( "", "", "", "", "" ) );
    EXPECT_NO_THROW( validation::tryAqToolTearDown() );
}


TEST( TestUtilitiesSetupAQL, UNIT_SetupAQ_and_TearDownAQ_CheckForInvalidPathInputs )
{
    // tryAqToolInitialize is idempotent, so each case below explicitly tears down first to
    // guarantee it actually (re)validates the override path being tested rather than
    // short-circuiting on a still-live instance from the previous case.

    // Check function correctly fails on an invalid ir.properties path
    EXPECT_NO_THROW( validation::tryAqToolTearDown() );
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


TEST( TestUtilitiesSetupAQL, UNIT_Reset_AlwaysReloads_CheckForInvalidPathInputs )
{
    // tryAqToolReset owns the "always tear down and rebuild" contract tryAqToolInitialize used to
    // have -- unlike tryAqToolInitialize, no explicit tryAqToolTearDown() is needed between cases
    // here, since tryAqToolReset does that itself on every call, whether or not anything is
    // currently initialized.

    // Check function correctly fails on an invalid ir.properties path
    EXPECT_ANY_THROW( validation::tryAqToolReset( "", "", "", "", "invalid ir.properties filepath" ) );

    // Check function correctly fails on an invalid calendar path
    EXPECT_ANY_THROW( validation::tryAqToolReset( "", "invalid calendar filepath", "", "", "" ) );

    // Check function correctly fails on an invalid central bank calendar path
    EXPECT_ANY_THROW( validation::tryAqToolReset( "", "", "Invalid central bank calendar filepath", "", "" ) );

    // Check function correctly passes with no overrides, and actually reinitializes even though
    // the previous (failed) calls above may have left AlgoQuantLib in a partially-initialized state
    EXPECT_NO_THROW( validation::tryAqToolReset( "", "", "", "", "" ) );
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

