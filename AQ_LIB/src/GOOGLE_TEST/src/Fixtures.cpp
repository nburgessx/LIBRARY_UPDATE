// Fixtures.cpp

// Includes: Google Test Library
#include <gTest/gTest.h>

// Includes: This Library
#include <LADate.h>

// Namespaces
using namespace std;

namespace google_test
{

    //
    // SetUp Test Fixture
    //
    class LADateTest : public testing::Test
    {
    protected:

        LADateTest()
        {
            date_ = LADate( "20150827" );
        };

        virtual ~LADateTest()       {};
        virtual void SetUp()        {};     // Setup gets invoked immediately before a test starts
        virtual void TearDown()     {};     // TearDown gets invoked immediately after a test finishes

        virtual LADate date()
        {
            return date_;
        };

    private:
        LADate date_;
    };

    //
    // Call Test Fixture
    //

    TEST_F( LADateTest, UNIT_CheckDate )
    {
        EXPECT_EQ( LADate( "20150827" ), date() );
    }
}
