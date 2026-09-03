// ExceptionMacros.h

#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <boost/math/special_functions/round.hpp>
#include "LACoreAppError.h"

//
// PURPOSE
//    To create MACROS to simplify and standardize the error messaging
//

// Macro to print to console
#ifndef MLIB_PRINT
#define MLIB_PRINT( message ) \
	std::cout << message << std::endl;
#endif

// Macro to throw an error message
#ifndef MLIB_THROW
#define MLIB_THROW( error_message ) \
{ \
	std::ostringstream MLIB_VALUE; \
    MLIB_VALUE << "#Error: " << error_message; \
	throw LACoreInvalidData( MLIB_VALUE.str().c_str(), __FILE__, __LINE__ ); \
}
#endif

// Macro to throw an error message if a given condition is not satisfied (false)
#ifndef MLIB_REQUIRE
#define MLIB_REQUIRE( condition, error_message ) \
if ( !(condition) ) \
{ \
    std::ostringstream MLIB_VALUE; \
    MLIB_VALUE << "#Error: " << error_message; \
	throw LACoreInvalidData( MLIB_VALUE.str().c_str(), __FILE__, __LINE__ ); \
}
#endif

// Macro to throw an error message if a given condition is satisfied (true)
#ifndef MLIB_THROW_IF
#define MLIB_THROW_IF( condition, error_message ) \
if ( (condition) ) \
{ \
    std::ostringstream MLIB_VALUE; \
    MLIB_VALUE << "#Error: " << error_message; \
	throw LACoreInvalidData( MLIB_VALUE.str().c_str(), __FILE__, __LINE__ ); \
}
#endif

// Macro to catch an error message
#ifndef MLIB_CATCH
#define MLIB_CATCH \
    catch(const LACoreError& e) \
    { \
        throw e; \
    } \
    catch(const std::exception& e) \
    { \
        throw e; \
    }
#endif

// Macro to check the size of a matrix and throw if the minimum column size is not satisfied
#ifndef MLIB_MATRIX_CHECK
#define MLIB_MATRIX_CHECK( matrix, minColumnSize, error_message ) \
if ( !matrix.empty() && minColumnSize>0 ) \
{ \
    std::ostringstream nested_ss; \
    nested_ss << error_message << " - At least " << minColumnSize << " columns required"; \
    MLIB_REQUIRE( matrix[0].size() >= minColumnSize, nested_ss.str().c_str() ) \
}
#endif

// Macro to check the size of a matrix and throw if the matrix does not have at least 2 columns
#ifndef MLIB_2D_MATRIX_CHECK
#define MLIB_2D_MATRIX_CHECK( matrix, error_message ) \
if ( !matrix.empty() ) \
{ \
    std::ostringstream nested_ss; \
    nested_ss << error_message << " - At least 2 columns required"; \
    MLIB_REQUIRE( matrix[0].size() >= 2, nested_ss.str().c_str() )  \
}
#endif

// Macro to make a function call and throw an error message if a given function call throws
#ifndef MLIB_TRY
#define MLIB_TRY( function_call, error_message ) \
try \
{ \
    function_call; \
} \
catch(...) \
{ \
    std::ostringstream MLIB_VALUE; \
    MLIB_VALUE << "#Error: " << error_message; \
    throw LACoreInvalidData( MLIB_VALUE.str().c_str(), __FILE__, __LINE__ ); \
}
#endif

// Macro to populate a variable using a function call or value
#ifndef MLIB_SET_VARIABLE
#define MLIB_SET_VARIABLE( variable, function_call ) \
    variable = function_call;
#endif

// Macro to populate a variable using a function call or override the value on failure
#ifndef MLIB_TRY_SET_VARIABLE
#define MLIB_TRY_SET_VARIABLE( variable, function_call, error_message ) \
try \
{ \
    variable = function_call; \
} \
catch(...) \
{ \
    std::ostringstream MLIB_VALUE; \
    MLIB_VALUE << "#Error: " << error_message; \
    throw LACoreInvalidData( MLIB_VALUE.str().c_str(), __FILE__, __LINE__ ); \
}
#endif

// Macro to populate a variable using a function call or override the value on failure
#ifndef MLIB_SET_VARIABLE_OR_OVERRIDE_ON_FAILURE
#define MLIB_SET_VARIABLE_OR_OVERRIDE_ON_FAILURE( variable, function_call, error_value ) \
try \
{ \
    variable = function_call; \
} \
catch(...) \
{ \
    variable = error_value; \
}
#endif

// Epsilion Precision Definition
#ifndef MLIB_EPSILON
#define MLIB_EPSILON 1e-14
#endif

// Macro to round a variable to precision MLIB_EPSILON
#ifndef MLIB_ROUND
#define MLIB_ROUND( variable, precision ) \
    boost::math::round( variable / precision ) * precision
#endif

// Macro to test if a variable is equal to a value with precision MLIB_EPSILON
#ifndef MLIB_IS_EQUAL
#define MLIB_IS_EQUAL( variable, equal ) \
	MLIB_IS_EQUAL_WITH_TOLERANCE( variable, equal, MLIB_EPSILON )
#endif

// Macro to test if a variable is equal to a value with precision given by tolerance
#ifndef MLIB_IS_EQUAL_WITH_TOLERANCE
#define MLIB_IS_EQUAL_WITH_TOLERANCE( variable, equal, tolerance ) \
    ( variable - equal > -tolerance && variable - equal < tolerance ) ? true : false
#endif

// Macro to test if a variable is less than a value to precision MLIB_EPSILON
#ifndef MLIB_IS_LESS_THAN
#define MLIB_IS_LESS_THAN( variable, lessThan ) \
    MLIB_IS_LESS_THAN_WITH_TOLERANCE( variable, lessThan, MLIB_EPSILON )
#endif

#ifndef MLIB_IS_LESS_THAN_WITH_TOLERANCE
#define MLIB_IS_LESS_THAN_WITH_TOLERANCE( variable, lessThan, tolerance ) \
    ( variable - lessThan < -tolerance ) ? true : false
#endif

// Macro to test if a variable is less than a value to precision MLIB_EPSILON
#ifndef MLIB_IS_LESS_THAN_OR_EQUAL
#define MLIB_IS_LESS_THAN_OR_EQUAL( variable, lessThan ) \
    MLIB_IS_LESS_THAN_OR_EQUAL_WITH_TOLERANCE( variable, lessThan, MLIB_EPSILON )
#endif

#ifndef MLIB_IS_LESS_THAN_OR_EQUAL_WITH_TOLERANCE
#define MLIB_IS_LESS_THAN_OR_EQUAL_WITH_TOLERANCE( variable, lessThan, tolerance ) \
    ( variable - lessThan <= tolerance ) ? true : false
#endif

// Macro to test if a variable is greater than a value to precision MLIB_EPSILON
#ifndef MLIB_IS_GREATER_THAN
#define MLIB_IS_GREATER_THAN( variable, greaterThan ) \
    MLIB_IS_GREATER_THAN_WITH_TOLERANCE( variable, greaterThan, MLIB_EPSILON )
#endif

#ifndef MLIB_IS_GREATER_THAN_WITH_TOLERANCE
#define MLIB_IS_GREATER_THAN_WITH_TOLERANCE( variable, greaterThan, tolerance ) \
    ( variable - greaterThan > tolerance ) ? true : false
#endif

// Macro to test if a variable is greater than a value to precision MLIB_EPSILON
#ifndef MLIB_IS_GREATER_THAN_OR_EQUAL
#define MLIB_IS_GREATER_THAN_OR_EQUAL( variable, greaterThan ) \
    MLIB_IS_GREATER_THAN_OR_EQUAL_WITH_TOLERANCE( variable, greaterThan, MLIB_EPSILON )
#endif

#ifndef MLIB_IS_GREATER_THAN_OR_EQUAL_WITH_TOLERANCE
#define MLIB_IS_GREATER_THAN_OR_EQUAL_WITH_TOLERANCE( variable, greaterThan, tolerance ) \
    ( variable - greaterThan >= -tolerance ) ? true : false
#endif

// Macro to test if a variable is equal to zero to precision MLIB_EPSILON
#ifndef MLIB_IS_EQUAL_ZERO
#define MLIB_IS_EQUAL_ZERO( variable ) \
    MLIB_IS_EQUAL_ZERO_WITH_TOLERANCE( variable, MLIB_EPSILON )
#endif

#ifndef MLIB_IS_EQUAL_ZERO_WITH_TOLERANCE
#define MLIB_IS_EQUAL_ZERO_WITH_TOLERANCE( variable, tolerance ) \
    ( variable > -tolerance && variable < tolerance ) ? true : false
#endif

// Macro to test if a variable is less than a value to precision MLIB_EPSILON
#ifndef MLIB_IS_LESS_THAN_ZERO
#define MLIB_IS_LESS_THAN_ZERO( variable ) \
    MLIB_IS_LESS_THAN_ZERO_WITH_TOLERANCE( variable, MLIB_EPSILON )
#endif

#ifndef MLIB_IS_LESS_THAN_ZERO_WITH_TOLERANCE
#define MLIB_IS_LESS_THAN_ZERO_WITH_TOLERANCE( variable, tolerance ) \
    ( variable < -tolerance ) ? true : false
#endif

// Macro to test if a variable is less than a value to precision MLIB_EPSILON
#ifndef MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO
#define MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO( variable ) \
    MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO_WITH_TOLERANCE( variable, MLIB_EPSILON )
#endif

#ifndef MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO_WITH_TOLERANCE
#define MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO_WITH_TOLERANCE( variable, tolerance ) \
    ( variable <= tolerance ) ? true : false
#endif

// Macro to test if a variable is greater than a value to precision MLIB_EPSILON
#ifndef MLIB_IS_GREATER_THAN_ZERO
#define MLIB_IS_GREATER_THAN_ZERO( variable ) \
    MLIB_IS_GREATER_THAN_ZERO_WITH_TOLERANCE( variable, MLIB_EPSILON )
#endif

#ifndef MLIB_IS_GREATER_THAN_ZERO_WITH_TOLERANCE
#define MLIB_IS_GREATER_THAN_ZERO_WITH_TOLERANCE( variable, tolerance ) \
    ( variable > tolerance ) ? true : false
#endif

// Macro to test if a variable is greater than a value to precision MLIB_EPSILON
#ifndef MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO
#define MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( variable ) \
    MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO_WITH_TOLERANCE( variable, MLIB_EPSILON )
#endif

#ifndef MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO_WITH_TOLERANCE
#define MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO_WITH_TOLERANCE( variable, tolerance ) \
    ( variable >= -tolerance ) ? true : false
#endif

// Namespace to protect against clashes with std methods
namespace etrading
{ 
	// Template to Convert Numbers to Standard String, defaults to zero decimal places of precision
	template <typename T>
	std::string toStandardString(const T value, const int decimalPlacePrecision = 6) // use 6 decimal places by default, same as std::to_string for consistency
	{
		std::ostringstream DATA_VALUE;
		DATA_VALUE.precision(decimalPlacePrecision);
		DATA_VALUE << std::fixed << value;
		return DATA_VALUE.str();
	}
}

// Macros to convert from a number to a string
#ifndef MLIB_TO_STRING_FROM_SIZE_T
#define MLIB_TO_STRING_FROM_SIZE_T( input )											etrading::toStandardString< size_t >( input )	
#endif
	
#ifndef MLIB_TO_STRING_FROM_LONG
#define MLIB_TO_STRING_FROM_LONG( input )											etrading::toStandardString< long   >( input )
#endif

#ifndef MLIB_TO_STRING_FROM_INT
#define MLIB_TO_STRING_FROM_INT( input )											etrading::toStandardString< int    >( input )
#endif

#ifndef MLIB_TO_STRING_FROM_FLOAT
#define MLIB_TO_STRING_FROM_FLOAT( input )											etrading::toStandardString< float  >( input )
#endif

#ifndef MLIB_TO_STRING_FROM_DOUBLE
#define MLIB_TO_STRING_FROM_DOUBLE( input )											etrading::toStandardString< double >( input )
#endif
	
#ifndef MLIB_TO_STRING_FROM_FLOAT_WITH_PRECISION
#define MLIB_TO_STRING_FROM_FLOAT_WITH_PRECISION( input, decimalPlacePrecision )	etrading::toStandardString< float  >( input, decimalPlacePrecision )
#endif
	
#ifndef MLIB_TO_STRING_FROM_DOUBLE_WITH_PRECISION
#define MLIB_TO_STRING_FROM_DOUBLE_WITH_PRECISION( input, decimalPlacePrecision )	etrading::toStandardString< double >( input, decimalPlacePrecision )
#endif