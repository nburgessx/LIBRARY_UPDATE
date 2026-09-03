// ExceptionMacros.h

#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include "RecordMacros.h"   // <--- The EXPAND, GET_MACRO_NAME and VARGS macros are here

#ifdef _MSC_VER
#include "Windows.h"        // <---- OutputDebugStringA
#endif
// Important Note: To Resolve a Macro Conflict when using Windows.h
// -----------------------------------------------------------------
// Windows.h has a macro defined min / max that prevents the std::min and std:max functions from working
// So we undefine these macros here
#undef min
#undef max


//
// PURPOSE
//    To create MACROS to simplify debugging and logging
//


// Macro to write a parameter name to a string stream, note #=stringify and converts a variable to a string
#define DEBUG_PARAMETER_NAME( ss, D ) \
    ss << #D << "," ;


// Macro to write a parameter value to a string stream, note #=stringify and converts a variable to a string
#define DEBUG_PARAMETER_VALUE( ss, D ) \
    ss << D << "," ;


// Macro to write a parameter name to a string stream, note #=stringify and converts a variable to a string
#define DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D ) \
    ss << #D;


// Macro to write a parameter value to a string stream, note #=stringify and converts a variable to a string
#define DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D ) \
    ss << D;


// Macro to add a final new line to the debug output
#define DEBUG_NEWLINE( ss ) \
    ss << std::endl;


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_1( D1 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D1 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_1( D1 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_2( D1, D2 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D2 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_2( D1, D2 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_3( D1, D2, D3 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D3 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_3( D1, D2, D3 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_4( D1, D2, D3, D4 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D4 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_4( D1, D2, D3, D4 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_5( D1, D2, D3, D4, D5 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D5 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_5( D1, D2, D3, D4, D5 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_6( D1, D2, D3, D4, D5, D6 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D6 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_6( D1, D2, D3, D4, D5, D6 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_7( D1, D2, D3, D4, D5, D6, D7 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D7 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_7( D1, D2, D3, D4, D5, D6, D7 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_8( D1, D2, D3, D4, D5, D6, D7, D8 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D8 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_8( D1, D2, D3, D4, D5, D6, D7, D8 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_9( D1, D2, D3, D4, D5, D6, D7, D8, D9 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE( ss, D8 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D9 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_9( D1, D2, D3, D4, D5, D6, D7, D8, D9 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_10( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE( ss, D8 ) \
        DEBUG_PARAMETER_VALUE( ss, D9 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D10 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_10( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_11( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE( ss, D8 ) \
        DEBUG_PARAMETER_VALUE( ss, D9 ) \
        DEBUG_PARAMETER_VALUE( ss, D10 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D11 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_11( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_12( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE( ss, D8 ) \
        DEBUG_PARAMETER_VALUE( ss, D9 ) \
        DEBUG_PARAMETER_VALUE( ss, D10 ) \
        DEBUG_PARAMETER_VALUE( ss, D11 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D12 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_12( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_13( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE( ss, D8 ) \
        DEBUG_PARAMETER_VALUE( ss, D9 ) \
        DEBUG_PARAMETER_VALUE( ss, D10 ) \
        DEBUG_PARAMETER_VALUE( ss, D11 ) \
        DEBUG_PARAMETER_VALUE( ss, D12 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D13 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_13( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_14( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE( ss, D8 ) \
        DEBUG_PARAMETER_VALUE( ss, D9 ) \
        DEBUG_PARAMETER_VALUE( ss, D10 ) \
        DEBUG_PARAMETER_VALUE( ss, D11 ) \
        DEBUG_PARAMETER_VALUE( ss, D12 ) \
        DEBUG_PARAMETER_VALUE( ss, D13 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D14 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_14( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_15( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE( ss, D8 ) \
        DEBUG_PARAMETER_VALUE( ss, D9 ) \
        DEBUG_PARAMETER_VALUE( ss, D10 ) \
        DEBUG_PARAMETER_VALUE( ss, D11 ) \
        DEBUG_PARAMETER_VALUE( ss, D12 ) \
        DEBUG_PARAMETER_VALUE( ss, D13 ) \
        DEBUG_PARAMETER_VALUE( ss, D14 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D15 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_15( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15 )
#endif

// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_16( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE( ss, D8 ) \
        DEBUG_PARAMETER_VALUE( ss, D9 ) \
        DEBUG_PARAMETER_VALUE( ss, D10 ) \
        DEBUG_PARAMETER_VALUE( ss, D11 ) \
        DEBUG_PARAMETER_VALUE( ss, D12 ) \
        DEBUG_PARAMETER_VALUE( ss, D13 ) \
        DEBUG_PARAMETER_VALUE( ss, D14 ) \
        DEBUG_PARAMETER_VALUE( ss, D15 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D16 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_16( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16 )
#endif

// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_17( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE( ss, D8 ) \
        DEBUG_PARAMETER_VALUE( ss, D9 ) \
        DEBUG_PARAMETER_VALUE( ss, D10 ) \
        DEBUG_PARAMETER_VALUE( ss, D11 ) \
        DEBUG_PARAMETER_VALUE( ss, D12 ) \
        DEBUG_PARAMETER_VALUE( ss, D13 ) \
        DEBUG_PARAMETER_VALUE( ss, D14 ) \
        DEBUG_PARAMETER_VALUE( ss, D15 ) \
        DEBUG_PARAMETER_VALUE( ss, D16 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D17 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_17( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17 )
#endif

// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_18( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE( ss, D8 ) \
        DEBUG_PARAMETER_VALUE( ss, D9 ) \
        DEBUG_PARAMETER_VALUE( ss, D10 ) \
        DEBUG_PARAMETER_VALUE( ss, D11 ) \
        DEBUG_PARAMETER_VALUE( ss, D12 ) \
        DEBUG_PARAMETER_VALUE( ss, D13 ) \
        DEBUG_PARAMETER_VALUE( ss, D14 ) \
        DEBUG_PARAMETER_VALUE( ss, D15 ) \
        DEBUG_PARAMETER_VALUE( ss, D16 ) \
        DEBUG_PARAMETER_VALUE( ss, D17 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D18 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_18( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18 )
#endif

// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_19( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18, D19 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE( ss, D8 ) \
        DEBUG_PARAMETER_VALUE( ss, D9 ) \
        DEBUG_PARAMETER_VALUE( ss, D10 ) \
        DEBUG_PARAMETER_VALUE( ss, D11 ) \
        DEBUG_PARAMETER_VALUE( ss, D12 ) \
        DEBUG_PARAMETER_VALUE( ss, D13 ) \
        DEBUG_PARAMETER_VALUE( ss, D14 ) \
        DEBUG_PARAMETER_VALUE( ss, D15 ) \
        DEBUG_PARAMETER_VALUE( ss, D16 ) \
        DEBUG_PARAMETER_VALUE( ss, D17 ) \
        DEBUG_PARAMETER_VALUE( ss, D18 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D19 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_19( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18, D19 )
#endif

// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_VALUE_20( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18, D19, D20 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_VALUE( ss, D1 ) \
        DEBUG_PARAMETER_VALUE( ss, D2 ) \
        DEBUG_PARAMETER_VALUE( ss, D3 ) \
        DEBUG_PARAMETER_VALUE( ss, D4 ) \
        DEBUG_PARAMETER_VALUE( ss, D5 ) \
        DEBUG_PARAMETER_VALUE( ss, D6 ) \
        DEBUG_PARAMETER_VALUE( ss, D7 ) \
        DEBUG_PARAMETER_VALUE( ss, D8 ) \
        DEBUG_PARAMETER_VALUE( ss, D9 ) \
        DEBUG_PARAMETER_VALUE( ss, D10 ) \
        DEBUG_PARAMETER_VALUE( ss, D11 ) \
        DEBUG_PARAMETER_VALUE( ss, D12 ) \
        DEBUG_PARAMETER_VALUE( ss, D13 ) \
        DEBUG_PARAMETER_VALUE( ss, D14 ) \
        DEBUG_PARAMETER_VALUE( ss, D15 ) \
        DEBUG_PARAMETER_VALUE( ss, D16 ) \
        DEBUG_PARAMETER_VALUE( ss, D17 ) \
        DEBUG_PARAMETER_VALUE( ss, D18 ) \
        DEBUG_PARAMETER_VALUE( ss, D19 ) \
        DEBUG_PARAMETER_VALUE_NO_TRAILING_COMMA( ss, D20 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_VALUE_20( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18, D19, D20 )
#endif

// Define a macro to record a functions's input parameters to a file
// This macro dynamically records up to 15 input parameters.
#define AQ_DEBUG_VALUE(...) EXPAND( GET_MACRO_NAME( __VA_ARGS__,    \
                                                      AQ_DEBUG_VALUE_20,  \
                                                      AQ_DEBUG_VALUE_19,  \
                                                      AQ_DEBUG_VALUE_18,  \
                                                      AQ_DEBUG_VALUE_17,  \
                                                      AQ_DEBUG_VALUE_16,  \
                                                      AQ_DEBUG_VALUE_15,  \
                                                      AQ_DEBUG_VALUE_14,  \
                                                      AQ_DEBUG_VALUE_13,  \
                                                      AQ_DEBUG_VALUE_12,  \
                                                      AQ_DEBUG_VALUE_11,  \
                                                      AQ_DEBUG_VALUE_10,  \
                                                      AQ_DEBUG_VALUE_9,   \
                                                      AQ_DEBUG_VALUE_8,   \
                                                      AQ_DEBUG_VALUE_7,   \
                                                      AQ_DEBUG_VALUE_6,   \
                                                      AQ_DEBUG_VALUE_5,   \
                                                      AQ_DEBUG_VALUE_4,   \
                                                      AQ_DEBUG_VALUE_3,   \
                                                      AQ_DEBUG_VALUE_2,   \
                                                      AQ_DEBUG_VALUE_1 )( __VA_ARGS__ ) )


//-------------------------------------------------------------------------------------------------------------------


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_1( D1 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D1 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_1( D1 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_2( D1, D2 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D2 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_2( D1, D2 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_3( D1, D2, D3 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D3 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_3( D1, D2, D3 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_4( D1, D2, D3, D4 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D4 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_4( D1, D2, D3, D4 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_5( D1, D2, D3, D4, D5 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D5 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_5( D1, D2, D3, D4, D5 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_6( D1, D2, D3, D4, D5, D6 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D6 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_6( D1, D2, D3, D4, D5, D6 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_7( D1, D2, D3, D4, D5, D6, D7 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D7 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_7( D1, D2, D3, D4, D5, D6, D7 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_8( D1, D2, D3, D4, D5, D6, D7, D8 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D8 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_8( D1, D2, D3, D4, D5, D6, D7, D8 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_9( D1, D2, D3, D4, D5, D6, D7, D8, D9 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME( ss, D8 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D9 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_9( D1, D2, D3, D4, D5, D6, D7, D8, D9 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_10( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME( ss, D8 ) \
        DEBUG_PARAMETER_NAME( ss, D9 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D10 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_10( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_11( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME( ss, D8 ) \
        DEBUG_PARAMETER_NAME( ss, D9 ) \
        DEBUG_PARAMETER_NAME( ss, D10 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D11 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_11( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_12( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME( ss, D8 ) \
        DEBUG_PARAMETER_NAME( ss, D9 ) \
        DEBUG_PARAMETER_NAME( ss, D10 ) \
        DEBUG_PARAMETER_NAME( ss, D11 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D12 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_12( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_13( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME( ss, D8 ) \
        DEBUG_PARAMETER_NAME( ss, D9 ) \
        DEBUG_PARAMETER_NAME( ss, D10 ) \
        DEBUG_PARAMETER_NAME( ss, D11 ) \
        DEBUG_PARAMETER_NAME( ss, D12 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D13 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_13( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_14( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME( ss, D8 ) \
        DEBUG_PARAMETER_NAME( ss, D9 ) \
        DEBUG_PARAMETER_NAME( ss, D10 ) \
        DEBUG_PARAMETER_NAME( ss, D11 ) \
        DEBUG_PARAMETER_NAME( ss, D12 ) \
        DEBUG_PARAMETER_NAME( ss, D13 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D14 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_14( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_15( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME( ss, D8 ) \
        DEBUG_PARAMETER_NAME( ss, D9 ) \
        DEBUG_PARAMETER_NAME( ss, D10 ) \
        DEBUG_PARAMETER_NAME( ss, D11 ) \
        DEBUG_PARAMETER_NAME( ss, D12 ) \
        DEBUG_PARAMETER_NAME( ss, D13 ) \
        DEBUG_PARAMETER_NAME( ss, D14 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D15 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_15( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15 )
#endif

// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_16( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME( ss, D8 ) \
        DEBUG_PARAMETER_NAME( ss, D9 ) \
        DEBUG_PARAMETER_NAME( ss, D10 ) \
        DEBUG_PARAMETER_NAME( ss, D11 ) \
        DEBUG_PARAMETER_NAME( ss, D12 ) \
        DEBUG_PARAMETER_NAME( ss, D13 ) \
        DEBUG_PARAMETER_NAME( ss, D14 ) \
        DEBUG_PARAMETER_NAME( ss, D15 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D16 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_16( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_17( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME( ss, D8 ) \
        DEBUG_PARAMETER_NAME( ss, D9 ) \
        DEBUG_PARAMETER_NAME( ss, D10 ) \
        DEBUG_PARAMETER_NAME( ss, D11 ) \
        DEBUG_PARAMETER_NAME( ss, D12 ) \
        DEBUG_PARAMETER_NAME( ss, D13 ) \
        DEBUG_PARAMETER_NAME( ss, D14 ) \
        DEBUG_PARAMETER_NAME( ss, D15 ) \
        DEBUG_PARAMETER_NAME( ss, D16 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D17 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_17( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17 )
#endif


// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_18( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME( ss, D8 ) \
        DEBUG_PARAMETER_NAME( ss, D9 ) \
        DEBUG_PARAMETER_NAME( ss, D10 ) \
        DEBUG_PARAMETER_NAME( ss, D11 ) \
        DEBUG_PARAMETER_NAME( ss, D12 ) \
        DEBUG_PARAMETER_NAME( ss, D13 ) \
        DEBUG_PARAMETER_NAME( ss, D14 ) \
        DEBUG_PARAMETER_NAME( ss, D15 ) \
        DEBUG_PARAMETER_NAME( ss, D16 ) \
        DEBUG_PARAMETER_NAME( ss, D17 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D18 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_18( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18 )
#endif

// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_19( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18, D19 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME( ss, D8 ) \
        DEBUG_PARAMETER_NAME( ss, D9 ) \
        DEBUG_PARAMETER_NAME( ss, D10 ) \
        DEBUG_PARAMETER_NAME( ss, D11 ) \
        DEBUG_PARAMETER_NAME( ss, D12 ) \
        DEBUG_PARAMETER_NAME( ss, D13 ) \
        DEBUG_PARAMETER_NAME( ss, D14 ) \
        DEBUG_PARAMETER_NAME( ss, D15 ) \
        DEBUG_PARAMETER_NAME( ss, D16 ) \
        DEBUG_PARAMETER_NAME( ss, D17 ) \
        DEBUG_PARAMETER_NAME( ss, D18 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D19 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_19( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18, D19 )
#endif

// Macro to write a debug variable to the visual studio output screen
// Expand macro to nothing when using release configuration(s)
#if defined(_DEBUG)
    #define AQ_DEBUG_PARAMETER_20( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18, D19, D20 ) \
    { \
        std::ostringstream ss; \
        DEBUG_PARAMETER_NAME( ss, D1 ) \
        DEBUG_PARAMETER_NAME( ss, D2 ) \
        DEBUG_PARAMETER_NAME( ss, D3 ) \
        DEBUG_PARAMETER_NAME( ss, D4 ) \
        DEBUG_PARAMETER_NAME( ss, D5 ) \
        DEBUG_PARAMETER_NAME( ss, D6 ) \
        DEBUG_PARAMETER_NAME( ss, D7 ) \
        DEBUG_PARAMETER_NAME( ss, D8 ) \
        DEBUG_PARAMETER_NAME( ss, D9 ) \
        DEBUG_PARAMETER_NAME( ss, D10 ) \
        DEBUG_PARAMETER_NAME( ss, D11 ) \
        DEBUG_PARAMETER_NAME( ss, D12 ) \
        DEBUG_PARAMETER_NAME( ss, D13 ) \
        DEBUG_PARAMETER_NAME( ss, D14 ) \
        DEBUG_PARAMETER_NAME( ss, D15 ) \
        DEBUG_PARAMETER_NAME( ss, D16 ) \
        DEBUG_PARAMETER_NAME( ss, D17 ) \
        DEBUG_PARAMETER_NAME( ss, D18 ) \
        DEBUG_PARAMETER_NAME( ss, D19 ) \
        DEBUG_PARAMETER_NAME_NO_TRAILING_COMMA( ss, D20 ) \
        DEBUG_NEWLINE( ss ) \
        OutputDebugStringA( ss.str().c_str() ); \
    } 
#else
    #define AQ_DEBUG_PARAMETER_20( D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18, D19, D20 )
#endif

// Define a macro to record a functions's input parameters to a file
// This macro dynamically records up to 15 input parameters.
#define AQ_DEBUG_PARAMETER(...) EXPAND( GET_MACRO_NAME( __VA_ARGS__,    \
                                                          AQ_DEBUG_PARAMETER_20,  \
                                                          AQ_DEBUG_PARAMETER_19,  \
                                                          AQ_DEBUG_PARAMETER_18,  \
                                                          AQ_DEBUG_PARAMETER_17,  \
                                                          AQ_DEBUG_PARAMETER_16,  \
                                                          AQ_DEBUG_PARAMETER_15,  \
                                                          AQ_DEBUG_PARAMETER_14,  \
                                                          AQ_DEBUG_PARAMETER_13,  \
                                                          AQ_DEBUG_PARAMETER_12,  \
                                                          AQ_DEBUG_PARAMETER_11,  \
                                                          AQ_DEBUG_PARAMETER_10,  \
                                                          AQ_DEBUG_PARAMETER_9,   \
                                                          AQ_DEBUG_PARAMETER_8,   \
                                                          AQ_DEBUG_PARAMETER_7,   \
                                                          AQ_DEBUG_PARAMETER_6,   \
                                                          AQ_DEBUG_PARAMETER_5,   \
                                                          AQ_DEBUG_PARAMETER_4,   \
                                                          AQ_DEBUG_PARAMETER_3,   \
                                                          AQ_DEBUG_PARAMETER_2,   \
                                                          AQ_DEBUG_PARAMETER_1 )( __VA_ARGS__ ) )

