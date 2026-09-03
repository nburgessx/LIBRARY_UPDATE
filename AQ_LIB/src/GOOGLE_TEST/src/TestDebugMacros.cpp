/*
 * @brief			Test on MLIBQ Debug Macros
 * @Created:		12th September 2017
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include "DebugMacros.h"

// Include: Google Test Library
#include <gTest/gTest.h>


namespace google_test
{
    
    // Test the MLIB_DEBUG_VALUE function prints to the visual studio output window or google_test.exe command screen
    TEST( TestDebugMacros, UNIT_TestMacro_MLIB_DEBUG_VALUE )
    {
        const double d1 = 1.1;
        const double d2 = 2.2;
        const double d3 = 3.3;
        const double d4 = 4.4;
        const double d5 = 5.5;
        const double d6 = 6.6;
        const double d7 = 7.7;
        const double d8 = 8.8;
        const double d9 = 9.9;
        const double d10 = 10.10;
        const double d11 = 11.11;
        const double d12 = 12.12;
        const double d13 = 13.13;
        const double d14 = 14.14;
        const double d15 = 15.15;


        // MLIB_DEBUG_PARAMETER: Displays the variable names
        MLIB_DEBUG_PARAMETER( d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15 );

        // MLIB_DEBUG_VALUE: Displays the variable values
        MLIB_DEBUG_VALUE( d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15 );
        MLIB_DEBUG_VALUE( d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14 );
        MLIB_DEBUG_VALUE( d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13 );
        MLIB_DEBUG_VALUE( d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12 );
        MLIB_DEBUG_VALUE( d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11 );
        MLIB_DEBUG_VALUE( d1, d2, d3, d4, d5, d6, d7, d8, d9, d10 );
        MLIB_DEBUG_VALUE( d1, d2, d3, d4, d5, d6, d7, d8, d9 );
        MLIB_DEBUG_VALUE( d1, d2, d3, d4, d5, d6, d7, d8 );
        MLIB_DEBUG_VALUE( d1, d2, d3, d4, d5, d6, d7 );
        MLIB_DEBUG_VALUE( d1, d2, d3, d4, d5, d6 );
        MLIB_DEBUG_VALUE( d1, d2, d3, d4, d5 );
        MLIB_DEBUG_VALUE( d1, d2, d3, d4 );
        MLIB_DEBUG_VALUE( d1, d2, d3 );
        MLIB_DEBUG_VALUE( d1, d2 );
        MLIB_DEBUG_VALUE( d1 );
        
    }


    // Test the MLIB_DEBUG_PARAMETER function prints to the visual studio output window or google_test.exe command screen
    TEST( TestDebugMacros, UNIT_TestMacro_MLIB_DEBUG_PARAMETER )
    {
        const std::string s1 = "a";
        const std::string s2 = "b";
        const std::string s3 = "c";
        const std::string s4 = "d";
        const std::string s5 = "e";
        const std::string s6 = "f";
        const std::string s7 = "g";
        const std::string s8 = "h";
        const std::string s9 = "i";
        const std::string s10 = "j";
        const std::string s11 = "k";
        const std::string s12 = "l";
        const std::string s13 = "m";
        const std::string s14 = "n";
        const std::string s15 = "o";


        // MLIB_DEBUG_PARAMETER: Displays the variable names
        MLIB_DEBUG_PARAMETER( s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15 );

        // MLIB_DEBUG_VALUE: Displays the variable values
        MLIB_DEBUG_VALUE( s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15 );
        MLIB_DEBUG_VALUE( s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14 );
        MLIB_DEBUG_VALUE( s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13 );
        MLIB_DEBUG_VALUE( s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12 );
        MLIB_DEBUG_VALUE( s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11 );
        MLIB_DEBUG_VALUE( s1, s2, s3, s4, s5, s6, s7, s8, s9, s10 );
        MLIB_DEBUG_VALUE( s1, s2, s3, s4, s5, s6, s7, s8, s9 );
        MLIB_DEBUG_VALUE( s1, s2, s3, s4, s5, s6, s7, s8 );
        MLIB_DEBUG_VALUE( s1, s2, s3, s4, s5, s6, s7 );
        MLIB_DEBUG_VALUE( s1, s2, s3, s4, s5, s6 );
        MLIB_DEBUG_VALUE( s1, s2, s3, s4, s5 );
        MLIB_DEBUG_VALUE( s1, s2, s3, s4 );
        MLIB_DEBUG_VALUE( s1, s2, s3 );
        MLIB_DEBUG_VALUE( s1, s2 );
        MLIB_DEBUG_VALUE( s1 );
        
    }



}
