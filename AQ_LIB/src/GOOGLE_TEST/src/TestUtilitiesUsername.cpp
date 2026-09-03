// TestUtilitiesUsername.cpp

// Include: Google Test Library
#include <gTest/gTest.h>

#include <string>

// Includes: This Library
#include "UserUtilities.h"


TEST( TestUtilitiesUsername, UNIT_GetUserNameTest )
{
    // Check Valid UserName
    std::string result = etrading::getUserName();
    EXPECT_EQ( result, result ); // Redundant test, but allows me to see the result locally
}
