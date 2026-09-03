// Examples.cpp

// Includes: Google Test Library
#include <gTest/gTest.h>

// Includes: Standard Library
#include <string>

// Includes: This Library
#include <AQLString.h>
#include <AQLDate.h>

// Namespaces
using namespace std;

namespace google_test
{

    TEST( ComparisonTest1, UNIT_Integers )
    {
        EXPECT_EQ( 1, 1 );
        EXPECT_EQ( 2, 2 );
        EXPECT_EQ( 3, 3 );
        EXPECT_EQ( 4, 4 );
        EXPECT_EQ( 5, 5 );
    }

    TEST( ComparisionTest2, UNIT_LAStrings )
    {
        EXPECT_EQ( AQLString( "A" ), AQLString( "A" ) );
        EXPECT_EQ( AQLString( "B" ), AQLString( "B" ) );
        EXPECT_EQ( AQLString( "C" ), AQLString( "C" ) );
        EXPECT_EQ( AQLString( "D" ), AQLString( "D" ) );
        EXPECT_EQ( AQLString( "E" ), AQLString( "E" ) );
    }

    TEST( ComparisionTest3, UNIT_LADates )
    {
        EXPECT_EQ( AQLDate( "20150827" ), AQLDate( "20150827" ) );
        EXPECT_EQ( AQLDate( "20160827" ), AQLDate( "20160827" ) );
        EXPECT_EQ( AQLDate( "20170827" ), AQLDate( "20170827" ) );
        EXPECT_EQ( AQLDate( "20180827" ), AQLDate( "20180827" ) );
        EXPECT_EQ( AQLDate( "20190827" ), AQLDate( "20190827" ) );
    }

    TEST( ComparisionTest3, UNIT_Vectors )
    {
        vector< double > a( 5, 0.0 );
        vector< double > b( 5, 0.0 );

        EXPECT_EQ( a.size(), b.size() ) << "Vector sizes are not the same" << endl;

        EXPECT_EQ( a[0], b[0] );
        EXPECT_EQ( a[1], b[1] );
        EXPECT_EQ( a[2], b[2] );
        EXPECT_EQ( a[3], b[3] );
        EXPECT_EQ( a[4], b[4] );
    }

    TEST( ComparisonTest4, UNIT_RealNumbersWithTolerance )
    {
        const double pi         = 3.14159265358979323846261;
        const double epsilon    = 0.00001;
        ASSERT_NEAR( 3.141593, pi, epsilon ) << "The values are outside our tolerance" << endl;
    }

    TEST( ComparisonTest4, UNIT_RealNumbersWithUnitsOfLeastPrecission )
    {
        const double d1         = 3.14159265358979323846261;
        const double d2         = 3.14159265358979323;

        // Assert_Double precission is 4ULP i.e. 4 Units of Least Precission
        ASSERT_DOUBLE_EQ( d1, d2  ) << "The values are not the same" << endl;
    }
}
