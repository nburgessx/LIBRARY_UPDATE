// RecordMacros.h

#pragma once
#include <string>
#include "CreateDataFile.h"

namespace
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;
}

//
// PURPOSE
//    To create MACROS to simplify and standardize the recording of Google Test inputs and outputs for test, logging and replay purposes
//
// SYNPOSIS
//    The RECORD_INPUTS nad RECORD_OUTPUTS macros records the input and output parameters for a given function, see examples below.
//    IMPORTANT NOTE: The record ouputs macro will also return the test result
//    
//      Example 1. - Recording Function Inputs
//      --------------------------------------
//      
//      Use 
//      RECORD_INPUTS( swapName, curveCollections, fixingTableNames )
//
//      Instead of ...
//        
//      // RECORDING INPUTS
//	    if (CreateDataFile::recordEnabled()) 
//	    {
//	  	    CreateDataFile file(decorateFilename("tryMeLWOXccySwapSpread_inputs", swapName.c_str()));
//	  	    file.write("generatorFunction", "tryMeLWOXccySwapSpread");
//	  	    file.write("swapName", swapName);
//	  	    file.write("curveCollections", curveCollections);
//	  	    file.write("fixingTableNames", fixingTableNames);
//	    }
//
//      Example 2. - Recording Function Outputs
//      ---------------------------------------
//
//      Use
//      RECORD_OUTPUTS_AND_RETURN_RESULT( resultValue )
//
//      Instead of ...
//
//      // RECORDING OUTPUTS
//      if (CreateDataFile::recordEnabled()) 
//	    {
//	    	CreateDataFile file(decorateFilename("tryMeLWOXccySwapSpread_outputs", swapName.c_str()));
//	    	file.write("output", resultValue );
//	    }
//

// Macro to write a parameter to file, note #=stringify and converts a variable to a string
#define WRITE_PARAMETER(P) \
    file.write( #P, (P) );

// Macro to write a timestamp to file
#define WRITE_TIMESTAMP() \
    char buff[20]; \
    CreateDataFile::currentTime(buff); \
    file.write( "=========", "===================" ); \
    file.write( "TIMESTAMP", buff ); \
    file.write( "=========", "===================" );

// Macro to get the in-scope function name with the namespace removed
#define FUNCTION_NAME \
    std::string decoratedFunctionName = __FUNCTION__; \
    std::size_t positionOfLastNamespaceColon = decoratedFunctionName.find_last_of(":"); \
    std::string functionName = decoratedFunctionName.substr( positionOfLastNamespaceColon + 1 );


// Macro to record a function's output to a file	
#define RECORD_OUTPUTS(result) \
	if ( CreateDataFile::recordEnabled() ) { \
    	FUNCTION_NAME \
		std::string fileName = functionName; \
		fileName += "_outputs"; \
		CreateDataFile outputFile( LAString( fileName.c_str() ) ); \
		outputFile.write( "output", (result) ); }


// Macro to record a function's output to a file	
#define RECORD_OUTPUTS_AND_RETURN_RESULT(result) \
	if ( CreateDataFile::recordEnabled() ) { \
    	FUNCTION_NAME \
		std::string fileName = functionName; \
		fileName += "_outputs"; \
		CreateDataFile outputFile( LAString( fileName.c_str() ) ); \
		outputFile.write( "output", (result) ); } \
    return result;


// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_NO_ARGUMENTS() \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_1(P1) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_2(P1, P2) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_3(P1, P2, P3) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_4(P1, P2, P3, P4) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_5(P1, P2, P3, P4, P5) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_6(P1, P2, P3, P4, P5, P6) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_7(P1, P2, P3, P4, P5, P6, P7) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_8(P1, P2, P3, P4, P5, P6, P7, P8) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_9(P1, P2, P3, P4, P5, P6, P7, P8, P9) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_10(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_11(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_12(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_13(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_14(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_15(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_16(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
        WRITE_PARAMETER( P15 ) \
		WRITE_PARAMETER( P16 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_17(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
        WRITE_PARAMETER( P15 ) \
        WRITE_PARAMETER( P16 ) \
		WRITE_PARAMETER( P17 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_18(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
        WRITE_PARAMETER( P15 ) \
        WRITE_PARAMETER( P16 ) \
        WRITE_PARAMETER( P17 ) \
		WRITE_PARAMETER( P18 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_19(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
        WRITE_PARAMETER( P15 ) \
        WRITE_PARAMETER( P16 ) \
        WRITE_PARAMETER( P17 ) \
        WRITE_PARAMETER( P18 ) \
		WRITE_PARAMETER( P19 ) }

// Macro to record a function's input parameters to a file	
#define RECORD_INPUTS_20(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName += "_inputs"; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
        WRITE_PARAMETER( P15 ) \
        WRITE_PARAMETER( P16 ) \
        WRITE_PARAMETER( P17 ) \
        WRITE_PARAMETER( P18 ) \
        WRITE_PARAMETER( P19 ) \
		WRITE_PARAMETER( P20 ) }

// ---------------------------------------------------------------------------------------------------------------
// MACRO EXPANSION AND VARIABLE ARGUMENT ROUTINES (__VA_ARGS__)
// ---------------------------------------------------------------------------------------------------------------

// Macro to expand a variable X. This resolves a Microsoft Visual Studio 2010 compiler error
// whereby the preprocessor expands __VA_ARGS__ multiple variables into a single concatinated variable
#define EXPAND(X) X

// Define a macro that uses the "paired, sliding arg list" technique to select the appropriate macro. 
#define GET_MACRO_NAME( _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, NAME, ... ) NAME

// Define a macro to record a functions's input parameters to a file
// This macro dynamically records up to 20 input parameters.
#define RECORD_INPUTS(...) EXPAND( GET_MACRO_NAME( __VA_ARGS__,       \
                                                   RECORD_INPUTS_20,  \
                                                   RECORD_INPUTS_19,  \
                                                   RECORD_INPUTS_18,  \
                                                   RECORD_INPUTS_17,  \
                                                   RECORD_INPUTS_16,  \
                                                   RECORD_INPUTS_15,  \
                                                   RECORD_INPUTS_14,  \
                                                   RECORD_INPUTS_13,  \
                                                   RECORD_INPUTS_12,  \
                                                   RECORD_INPUTS_11,  \
                                                   RECORD_INPUTS_10,  \
                                                   RECORD_INPUTS_9,   \
                                                   RECORD_INPUTS_8,   \
                                                   RECORD_INPUTS_7,   \
                                                   RECORD_INPUTS_6,   \
                                                   RECORD_INPUTS_5,   \
                                                   RECORD_INPUTS_4,   \
                                                   RECORD_INPUTS_3,   \
                                                   RECORD_INPUTS_2,   \
                                                   RECORD_INPUTS_1 )( __VA_ARGS__ ) )

// ---------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_1(P1) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_2(P1, P2) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_3(P1, P2, P3) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_4(P1, P2, P3, P4) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_5(P1, P2, P3, P4, P5) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_6(P1, P2, P3, P4, P5, P6) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_7(P1, P2, P3, P4, P5, P6, P7) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_8(P1, P2, P3, P4, P5, P6, P7, P8) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_9(P1, P2, P3, P4, P5, P6, P7, P8, P9) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_10(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_11(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_12(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_13(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_14(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_15(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_16(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) \
        WRITE_PARAMETER( P16 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_17(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) \
        WRITE_PARAMETER( P16 ) \
        WRITE_PARAMETER( P17 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_18(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) \
        WRITE_PARAMETER( P16 ) \
        WRITE_PARAMETER( P17 ) \
        WRITE_PARAMETER( P18 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_19(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) \
        WRITE_PARAMETER( P16 ) \
        WRITE_PARAMETER( P17 ) \
        WRITE_PARAMETER( P18 ) \
        WRITE_PARAMETER( P19 ) }

// Macro to log a function and a choice of parameters to a file	
#define CREATE_LOGFILE_20(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
		WRITE_TIMESTAMP() \
        file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) \
        WRITE_PARAMETER( P16 ) \
        WRITE_PARAMETER( P17 ) \
        WRITE_PARAMETER( P18 ) \
        WRITE_PARAMETER( P19 ) \
        WRITE_PARAMETER( P20 ) }

// Define a macro to create a log file of a functions's input parameters
// This macro dynamically records up to 20 input parameters.
#define CREATE_LOGFILE(...) EXPAND( GET_MACRO_NAME( __VA_ARGS__,       \
                                                    CREATE_LOGFILE_20,  \
                                                    CREATE_LOGFILE_19,  \
                                                    CREATE_LOGFILE_18,  \
                                                    CREATE_LOGFILE_17,  \
                                                    CREATE_LOGFILE_16,  \
                                                    CREATE_LOGFILE_15,  \
                                                    CREATE_LOGFILE_14,  \
                                                    CREATE_LOGFILE_13,  \
                                                    CREATE_LOGFILE_12,  \
                                                    CREATE_LOGFILE_11,  \
                                                    CREATE_LOGFILE_10,  \
                                                    CREATE_LOGFILE_9,   \
                                                    CREATE_LOGFILE_8,   \
                                                    CREATE_LOGFILE_7,   \
                                                    CREATE_LOGFILE_6,   \
                                                    CREATE_LOGFILE_5,   \
                                                    CREATE_LOGFILE_4,   \
                                                    CREATE_LOGFILE_3,   \
                                                    CREATE_LOGFILE_2,   \
                                                    CREATE_LOGFILE_1 )( __VA_ARGS__ ) )

// Macro to add a comment to a logfile
#define LOGFILE_ADD_COMMENT(comment) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string fileName = functionName; \
		fileName = "log_" + fileName; \
		CreateDataFile file( LAString( fileName.c_str() ), ".csv", true ); \
        std::ostringstream ss; \
        ss << comment; \
        file.write( ss.str().c_str() ); }

// Macro to record a function's output to a file and decorate the file name with a prefix and suffix	
#define RECORD_DECORATED_OUTPUTS(PREFIX, SUFFIX, result) \
	if ( CreateDataFile::recordEnabled() ) { \
    	FUNCTION_NAME \
		std::string decoratedOutputFunction = functionName + "_outputs"; \
        if ( PREFIX != "" ) decoratedOutputFunction = "_" + decoratedOutputFunction; \
        if ( SUFFIX != "" ) decoratedOutputFunction = decoratedOutputFunction + "_"; \
        std::string fileName = PREFIX + decoratedOutputFunction + SUFFIX; \
		CreateDataFile outputFile( LAString( fileName.c_str() ) ); \
		outputFile.write( "output", (result) ); }


// Macro to record a function's output to a file and decorate the file name with a prefix and suffix	
#define RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(PREFIX, SUFFIX, result) \
	if ( CreateDataFile::recordEnabled() ) { \
    	FUNCTION_NAME \
		std::string decoratedOutputFunction = functionName + "_outputs"; \
        if ( PREFIX != "" ) decoratedOutputFunction = "_" + decoratedOutputFunction; \
        if ( SUFFIX != "" ) decoratedOutputFunction = decoratedOutputFunction + "_"; \
        std::string fileName = PREFIX + decoratedOutputFunction + SUFFIX; \
		CreateDataFile outputFile( LAString( fileName.c_str() ) ); \
		outputFile.write( "output", (result) ); } \
    return result;


// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_1(PREFIX) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        std::string fileName = PREFIX + decoratedInputFunction + inputs; \
        CreateDataFile outputFile( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); }


// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_2(PREFIX, SUFFIX) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_3(PREFIX, SUFFIX, P1) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
        WRITE_PARAMETER( P1 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_4(PREFIX, SUFFIX, P1, P2) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_5(PREFIX, SUFFIX, P1, P2, P3) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_6(PREFIX, SUFFIX, P1, P2, P3, P4) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_7(PREFIX, SUFFIX, P1, P2, P3, P4, P5) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_8(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_9(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_10(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_11(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8, P9) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_12(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_13(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_14(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_15(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_16(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_17(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_18(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) \
		WRITE_PARAMETER( P16 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_19(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) \
		WRITE_PARAMETER( P16 ) \
		WRITE_PARAMETER( P17 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_20(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) \
		WRITE_PARAMETER( P16 ) \
		WRITE_PARAMETER( P17 ) \
		WRITE_PARAMETER( P18 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_21(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) \
		WRITE_PARAMETER( P16 ) \
		WRITE_PARAMETER( P17 ) \
		WRITE_PARAMETER( P18 ) \
		WRITE_PARAMETER( P19 ) }

// Macro to record a function's input parameters to a file, which we decorate with a prefix	and suffix
#define RECORD_DECORATED_INPUTS_22(PREFIX, SUFFIX, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, P20) \
    if ( CreateDataFile::recordEnabled() ) { \
        FUNCTION_NAME \
        std::string decoratedInputFunction = functionName + "_inputs"; \
        if ( PREFIX != "" ) decoratedInputFunction = "_" + decoratedInputFunction; \
        if ( SUFFIX != "" ) decoratedInputFunction = decoratedInputFunction + "_"; \
        std::string fileName = PREFIX + decoratedInputFunction + SUFFIX; \
		CreateDataFile file( LAString( fileName.c_str() ) ); \
		file.write( "generatorFunction", functionName ); \
		WRITE_PARAMETER( P1 ) \
		WRITE_PARAMETER( P2 ) \
		WRITE_PARAMETER( P3 ) \
		WRITE_PARAMETER( P4 ) \
		WRITE_PARAMETER( P5 ) \
		WRITE_PARAMETER( P6 ) \
		WRITE_PARAMETER( P7 ) \
		WRITE_PARAMETER( P8 ) \
		WRITE_PARAMETER( P9 ) \
		WRITE_PARAMETER( P10 ) \
		WRITE_PARAMETER( P11 ) \
		WRITE_PARAMETER( P12 ) \
		WRITE_PARAMETER( P13 ) \
		WRITE_PARAMETER( P14 ) \
		WRITE_PARAMETER( P15 ) \
		WRITE_PARAMETER( P16 ) \
		WRITE_PARAMETER( P17 ) \
		WRITE_PARAMETER( P18 ) \
		WRITE_PARAMETER( P19 ) \
		WRITE_PARAMETER( P20 ) }

// ---------------------------------------------------------------------------------------------------------------
// MACRO EXPANSION AND VARIABLE ARGUMENT ROUTINES (__VA_ARGS__)
// ---------------------------------------------------------------------------------------------------------------

// Define a macro to record a functions's input parameters to a file, which we decorate with a prefix
// This macro dynamically records up to 20 input parameters, including the file prefix and suffix which are the first and second arguments

#define GET_MACRO_NAME_UPTO_22_ARGS( _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, NAME, ... ) NAME

#define RECORD_DECORATED_INPUTS(...) EXPAND( GET_MACRO_NAME_UPTO_22_ARGS( __VA_ARGS__,       \
                                                                          RECORD_DECORATED_INPUTS_22,  \
                                                                          RECORD_DECORATED_INPUTS_21,  \
                                                                          RECORD_DECORATED_INPUTS_20,  \
                                                                          RECORD_DECORATED_INPUTS_19,  \
                                                                          RECORD_DECORATED_INPUTS_18,  \
                                                                          RECORD_DECORATED_INPUTS_17,  \
                                                                          RECORD_DECORATED_INPUTS_16,  \
                                                                          RECORD_DECORATED_INPUTS_15,  \
                                                                          RECORD_DECORATED_INPUTS_14,  \
                                                                          RECORD_DECORATED_INPUTS_13,  \
                                                                          RECORD_DECORATED_INPUTS_12,  \
                                                                          RECORD_DECORATED_INPUTS_11,  \
                                                                          RECORD_DECORATED_INPUTS_10,  \
                                                                          RECORD_DECORATED_INPUTS_9,   \
                                                                          RECORD_DECORATED_INPUTS_8,   \
                                                                          RECORD_DECORATED_INPUTS_7,   \
                                                                          RECORD_DECORATED_INPUTS_6,   \
                                                                          RECORD_DECORATED_INPUTS_5,   \
                                                                          RECORD_DECORATED_INPUTS_4,   \
                                                                          RECORD_DECORATED_INPUTS_3,   \
                                                                          RECORD_DECORATED_INPUTS_2,   \
                                                                          RECORD_DECORATED_INPUTS_1,   \
                                                                          RECORD_DECORATED_INPUTS_0, )( __VA_ARGS__ ) )

// ---------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------