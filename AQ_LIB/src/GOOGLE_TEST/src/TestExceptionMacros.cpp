#pragma once
#include "ExceptionMacros.h"

// Include: Google Test Library
#include <gTest/gTest.h>


namespace google_test
{
    // Test Function to simulate a throw on demand
    double throwFunction( bool doThrow, double value )
    {
        if ( doThrow ) throw "ErrorMessage";
        return value;
    }

    // Test Function to return a value
    double goodFunction( double value )
    {
        return value;
    }
    
    // Test Function to throw an error a value
    double badFunction( double value )
    {
        throw "ErrorMessage";
        return value;
    }

    // Test the MLIB_THROW Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_THROW )
    {
        // MLIB_THROW( Message )
        EXPECT_THROW( MLIB_THROW( "ErrorMessage" ), LACoreAppError );
    }


    // Test the MLIB_REQUIRE Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_REQUIRE )
    {
        // MLIB_REQUIRE( Condition, Message ) - Will throw Message if Condition false
        EXPECT_THROW( MLIB_REQUIRE( false, "ErrorMessage" ), LACoreAppError );
        EXPECT_NO_THROW( MLIB_REQUIRE( true, "ErrorMessage") );
    }

	// Test the MLIB_THROW_IF Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_THROW_IF )
    {
        // MLIB_THROW_IF( Condition, Message ) - Will throw Message if Condition true
        EXPECT_THROW( MLIB_THROW_IF( true, "ErrorMessage" ), LACoreAppError );
        EXPECT_NO_THROW( MLIB_THROW_IF( false, "ErrorMessage") );
    }

    // Test the MLIB_TRY Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_TRY )
    {
        // MLIB_TRY( FunctionCall, Message ) - Will throw if function call fails
        EXPECT_THROW( MLIB_TRY( throwFunction( true, 1.0 ), "ErrorMessage" ), LACoreAppError );
        EXPECT_NO_THROW( MLIB_TRY( throwFunction( false, 1.0 ), "ErrorMessage" ) );
    }


    // Test the MLIB_SET_VARIABLE_OR_OVERRIDE_ON_FAILURE Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_SET_VARIABLE_OR_OVERRIDE_ON_FAILURE )
    {
        double result_of_echo;
        MLIB_SET_VARIABLE_OR_OVERRIDE_ON_FAILURE( result_of_echo, goodFunction( 1.0 ), 999.0 );
        EXPECT_EQ( result_of_echo, 1.0 );

        double result_of_failure;
        MLIB_SET_VARIABLE_OR_OVERRIDE_ON_FAILURE( result_of_failure, badFunction( 1.0 ), 999.0 );
        EXPECT_EQ( result_of_failure, 999.0 );
    }


    // Test the MLIB_IS_EQUAL Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_ROUND )
    {
        EXPECT_EQ( MLIB_ROUND( 123, 1.0 ),          123.0 );
        EXPECT_EQ( MLIB_ROUND( 123, 10.0 ),         120.0 );
        EXPECT_EQ( MLIB_ROUND( 123, 100.0 ),        100.0 );

        EXPECT_EQ( MLIB_ROUND( 187, 1.0 ),          187.0 );
        EXPECT_EQ( MLIB_ROUND( 187, 10.0 ),         190.0 );
        EXPECT_EQ( MLIB_ROUND( 187, 100.0 ),        200.0 );

        EXPECT_EQ( MLIB_ROUND( -123, 1.0 ),         -123.0 );
        EXPECT_EQ( MLIB_ROUND( -123, 10.0 ),        -120.0 );
        EXPECT_EQ( MLIB_ROUND( -123, 100.0 ),       -100.0 );

        EXPECT_EQ( MLIB_ROUND( -187, 1.0 ),         -187.0 );
        EXPECT_EQ( MLIB_ROUND( -187, 10.0 ),        -190.0 );
        EXPECT_EQ( MLIB_ROUND( -187, 100.0 ),       -200.0 );

        EXPECT_EQ( MLIB_ROUND( 10.0001, 1.0 ),      10.0 );
        EXPECT_EQ( MLIB_ROUND( 10.0001, 0.001 ),    10.0 );
        EXPECT_EQ( MLIB_ROUND( 10.0001, 0.0001 ),   10.0001 );
    }


    // Test the MLIB_IS_EQUAL Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_IS_EQUAL )
    {
        EXPECT_TRUE( MLIB_IS_EQUAL( 10, 10 ) );
        EXPECT_TRUE( MLIB_IS_EQUAL( 9.9999999999999999999999, 10 ) );
        EXPECT_TRUE( MLIB_IS_EQUAL( 10.0000000000000000000001, 10 ) );
        
        EXPECT_FALSE( MLIB_IS_EQUAL( 9, 10 ) );
        EXPECT_FALSE( MLIB_IS_EQUAL( 9.9999, 10 ) );
        EXPECT_FALSE( MLIB_IS_EQUAL( 10.0001, 10 ) );
    }


    // Test the MLIB_IS_LESS_THAN Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_IS_LESS_THAN )
    {
        EXPECT_TRUE( MLIB_IS_LESS_THAN( -10, 10 ) );
        EXPECT_TRUE( MLIB_IS_LESS_THAN( 9.9, 10 ) );

        EXPECT_FALSE( MLIB_IS_LESS_THAN( 9.99999999999999999999999, 10 ) );
        EXPECT_FALSE( MLIB_IS_LESS_THAN( 10.1, 10 ) );
    }


    // Test theMLIB_IS_LESS_THAN_OR_EQUAL Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_IS_LESS_THAN_OR_EQUAL )
    {
        EXPECT_TRUE( MLIB_IS_LESS_THAN_OR_EQUAL( 10.000000000000000000000001, 10 ) );
        EXPECT_TRUE( MLIB_IS_LESS_THAN_OR_EQUAL( 9.9999999999999999999999999, 10 ) );
        
        EXPECT_TRUE( MLIB_IS_LESS_THAN_OR_EQUAL( -10, 10 ) );
        EXPECT_TRUE( MLIB_IS_LESS_THAN_OR_EQUAL( 9.9, 10 ) );

        EXPECT_FALSE( MLIB_IS_LESS_THAN_OR_EQUAL( 10.1, 10 ) );
    }


    // Test the MLIB_IS_GREATER_THAN Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_IS_GREATER_THAN )
    {
        EXPECT_TRUE( MLIB_IS_GREATER_THAN( 15, 10 ) );
        EXPECT_TRUE( MLIB_IS_GREATER_THAN( 10.1, 10 ) );

        EXPECT_FALSE( MLIB_IS_GREATER_THAN( -15, 10 ) );
        EXPECT_FALSE( MLIB_IS_GREATER_THAN( 10.0000000000000000000001, 10 ) );
    }


    // Test the MLIB_IS_GREATER_THAN_OR_EQUAL Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_IS_GREATER_THAN_OR_EQUAL )
    {
        EXPECT_TRUE( MLIB_IS_GREATER_THAN_OR_EQUAL( 10.000000000000000000000001, 10 ) );
        EXPECT_TRUE( MLIB_IS_GREATER_THAN_OR_EQUAL( 9.9999999999999999999999999, 10 ) );

        EXPECT_TRUE( MLIB_IS_GREATER_THAN_OR_EQUAL( 15, 10 ) );
        EXPECT_TRUE( MLIB_IS_GREATER_THAN_OR_EQUAL( 10.1, 10 ) );

        EXPECT_FALSE( MLIB_IS_GREATER_THAN_OR_EQUAL( -15, 10 ) );
        EXPECT_FALSE( MLIB_IS_GREATER_THAN_OR_EQUAL( 9.99999, 10 ) );
    }


    // Test the MLIB_IS_EQUAL_ZERO Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_IS_EQUAL_ZERO )
    {
        EXPECT_TRUE( MLIB_IS_EQUAL_ZERO( 0.00000000000000000001 ) );
        EXPECT_TRUE( MLIB_IS_EQUAL_ZERO( -0.00000000000000000001 ) );

        EXPECT_FALSE( MLIB_IS_EQUAL_ZERO( 0.0001 ) );
        EXPECT_FALSE( MLIB_IS_EQUAL_ZERO( -0.0001 ) );
    }


    // Test the MLIB_IS_LESS_THAN_ZERO Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_IS_LESS_THAN_ZERO )
    {
        EXPECT_TRUE( MLIB_IS_LESS_THAN_ZERO( -0.00001 ) );
        EXPECT_TRUE( MLIB_IS_LESS_THAN_ZERO( -10 ) );

        EXPECT_FALSE( MLIB_IS_LESS_THAN_ZERO( 0.00000000000000000001 ) );
        EXPECT_FALSE( MLIB_IS_LESS_THAN_ZERO( -0.00000000000000000001 ) );
    }


    // Test the MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO )
    {
        EXPECT_TRUE( MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO( 0.0000000000000000000000001 ) );
        EXPECT_TRUE( MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO( -0.0000000000000000000000001 ) );

        EXPECT_TRUE( MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO( -0.00001 ) );
        EXPECT_TRUE( MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO( -10 ) );
    }


    // Test the MLIB_IS_GREATER_THAN_ZERO Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_IS_GREATER_THAN_ZERO )
    {
        EXPECT_TRUE( MLIB_IS_GREATER_THAN_ZERO( 0.00001 ) );
        EXPECT_TRUE( MLIB_IS_GREATER_THAN_ZERO( 10 ) );

        EXPECT_FALSE( MLIB_IS_GREATER_THAN_ZERO( 0.000000000000000001 ) );
        EXPECT_FALSE( MLIB_IS_GREATER_THAN_ZERO( -10 ) );
    }


    // Test the MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO Exception Macro
    TEST( TestExceptionMacros, UNIT_TestMacro_MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO )
    {
        EXPECT_TRUE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( 0.0000000000000000000000001 ) );
        EXPECT_TRUE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( -0.0000000000000000000000001 ) );

        EXPECT_TRUE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( 0.00001 ) );
        EXPECT_TRUE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( 10 ) );

        EXPECT_FALSE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( -10 ) );
    }

}
