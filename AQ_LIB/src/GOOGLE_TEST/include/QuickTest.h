// QuickTest.h

// AlgoQuantLib test base class
#include "InitializeAQGoogleTest.h"

// Google Test Library
#include <gTest/gTest.h>

namespace google_test
{


    // Timeout Fixture Class
    class QuickTest
        : public virtual google_test::InitializeAQGoogleTest
        , public virtual testing::Test
    {
    protected:

        QuickTest() {};
        virtual ~QuickTest() {};

        // Remember that SetUp() is run immediately before a test starts.
        // This is a good place to record the start time.
        virtual void SetUp()
        {
            start_time_ = time( NULL );
        }

        // TearDown() is invoked immediately after a test finishes.  Here we
        // check if the test was too slow.
        virtual void TearDown()
        {
            // Gets the time when the test finishes
            const time_t end_time = time( NULL );

            // Asserts that the test took no more than ~5 seconds.  Did you
            // know that you can use assertions in SetUp() and TearDown() as well?
            EXPECT_TRUE( end_time - start_time_ <= 5 ) << "TIMEOUT: The test took too long.";
        }

        // The UTC time (in seconds) when the test starts
        time_t start_time_;

    };
}
