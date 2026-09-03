// TestUtilitiesString.cpp

// Includes: This Library
#include <StringUtilities.h>

// Include: Google Test Library
#include <gTest/gTest.h>

TEST( TestUtilitiesString, UppercaseTest )
{
    // Test Uppercase
    string expected = string( "FOO" );
    string result   = ETrading::Utilities::uppercase( string( "foo" ) );
    EXPECT_EQ( expected, result );
}

TEST( TestUtilitiesString, IntToStringTest )
{
    // Test Uppercase
    string expected = string( "123456789" );
    string result   = ETrading::Utilities::intToString( 123456789 );
    EXPECT_EQ( expected, result );
}
TEST( TestUtilitiesString, StringToIntTest )
{
    // Test Uppercase
    int expected = 123456789;
    int result   = ETrading::Utilities::stringToInt( string( "123456789" ) );
    EXPECT_EQ( expected, result );
}
