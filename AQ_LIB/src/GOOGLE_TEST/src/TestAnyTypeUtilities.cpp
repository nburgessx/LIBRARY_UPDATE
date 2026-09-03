/*
 * @brief			Unit Tests for Boost AnyType Utilities
 * @Created:		23rd October 2018
 * @Author:			Nicholas Burgess
 * @Department:		MHI Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once
#include "AnyTypeUtilities.h"

// Include: Google Test Library
#include <gTest/gTest.h>


namespace google_test
{
    // Test: Can we cast from boost::AnyType to std::string
    TEST( TestAnyTypeUtilities, UNIT_CastAnyTypeToString )
    {
        // Double
        std::string actual0     = etrading::fromAnyTypeToString( 1.123456789 );
        std::string expected0   = "1.1234567890";
        EXPECT_EQ( expected0, actual0 );

        std::string actual1     = etrading::fromAnyTypeToString( 1.1 );
        std::string expected1   = "1.1000000000";
        EXPECT_EQ( expected1, actual1 );

        // Integer
        std::string actual2     = etrading::fromAnyTypeToString( 2 );
        std::string expected2   = "2";
        EXPECT_EQ( expected2, actual2 );

        // LAString
        std::string actual3     = etrading::fromAnyTypeToString( LAString("Three") );
        std::string expected3   = "Three";
        EXPECT_EQ( expected3, actual3 );

        // std::string
        std::string actual4     = etrading::fromAnyTypeToString( "Four" );
        std::string expected4   = "Four";
        EXPECT_EQ( expected4, actual4 );

        // Boolean
        std::string actual5     = etrading::fromAnyTypeToString( true );
        std::string expected5   = "true";
        EXPECT_EQ( expected5, actual5 );

        std::string actual6     = etrading::fromAnyTypeToString( false );
        std::string expected6   = "false";
        EXPECT_EQ( expected6, actual6 );
    }

}
