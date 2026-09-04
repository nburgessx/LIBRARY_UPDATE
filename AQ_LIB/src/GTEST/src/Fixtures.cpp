// Fixtures.cpp

// Includes: Google Test Library
#include <gTest/gTest.h>

// Includes: This Library
#include <AQLDate.h>

// Namespaces
using namespace std;

namespace google_test
{

    //
    // SetUp Test Fixture
    //
    class AQLDateTest : public testing::Test
    {
    protected:

        AQLDateTest()
        {
            date_ = AQLDate( "20150827" );
        };

        virtual ~AQLDateTest()       {};
        virtual void SetUp()        {};     // Setup gets invoked immediately before a test starts
        virtual void TearDown()     {};     // TearDown gets invoked immediately after a test finishes

        virtual AQLDate date()
        {
            return date_;
        };

    private:
        AQLDate date_;
    };

    //
    // Call Test Fixture
    //

    TEST_F( AQLDateTest, UNIT_CheckDate )
    {
        EXPECT_EQ( AQLDate( "20150827" ), date() );
    }
}
