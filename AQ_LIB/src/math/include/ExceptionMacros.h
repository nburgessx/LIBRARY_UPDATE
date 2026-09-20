// ExceptionMacros.h

#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <boost/math/special_functions/round.hpp>
#include "AQLCoreAppError.h"

//
// PURPOSE
//    To create MACROS to simplify and standardize the error messaging
//

// Macro to print to console
#ifndef AQ_PRINT
#define AQ_PRINT( message ) \
	std::cout << message << std::endl;
#endif

// Macro to throw an error message
#ifndef AQ_THROW
#define AQ_THROW( error_message ) \
{ \
	std::ostringstream AQ_VALUE; \
    AQ_VALUE << "#Error: " << error_message; \
	throw AQLCoreInvalidData( AQ_VALUE.str().c_str(), __FILE__, __LINE__ ); \
}
#endif

// Macro to throw an error message if a given condition is not satisfied (false)
#ifndef AQ_REQUIRE
#define AQ_REQUIRE( condition, error_message ) \
if ( !(condition) ) \
{ \
    std::ostringstream AQ_VALUE; \
    AQ_VALUE << "#Error: " << error_message; \
	throw AQLCoreInvalidData( AQ_VALUE.str().c_str(), __FILE__, __LINE__ ); \
}
#endif

// Macro to throw an error message if a given condition is satisfied (true)
#ifndef AQ_THROW_IF
#define AQ_THROW_IF( condition, error_message ) \
if ( (condition) ) \
{ \
    std::ostringstream AQ_VALUE; \
    AQ_VALUE << "#Error: " << error_message; \
	throw AQLCoreInvalidData( AQ_VALUE.str().c_str(), __FILE__, __LINE__ ); \
}
#endif

// Macro to catch an error message and rethrow it unchanged.
//
// Rethrows with a bare `throw;`, not `throw e;`. `throw e` constructs a NEW exception object of
// e's *static* catch-clause type (AQLCoreError / std::exception) and throws that - it slices away
// whatever derived type (AQLCoreInvalidData, AQLCoreNumericalError, AQLCoreSystemError, or any
// std::exception subclass) the original throw site actually used. A caller further up the stack
// doing catch(const AQLCoreNumericalError&) to handle numerical failures differently from input
// validation failures would silently stop matching once the exception passed through this macro -
// no compiler warning, just a catch clause that quietly stops firing. `throw;` (bare, no operand)
// rethrows the original exception object with its original dynamic type intact - the standard,
// idiomatic way to "catch, do nothing extra, propagate" in C++.
#ifndef AQ_CATCH
#define AQ_CATCH \
    catch(const AQLCoreError&) \
    { \
        throw; \
    } \
    catch(const std::exception&) \
    { \
        throw; \
    }
#endif

// Macro to check the size of a matrix and throw if the minimum column size is not satisfied
#ifndef AQ_MATRIX_CHECK
#define AQ_MATRIX_CHECK( matrix, minColumnSize, error_message ) \
if ( !matrix.empty() && minColumnSize>0 ) \
{ \
    std::ostringstream nested_ss; \
    nested_ss << error_message << " - At least " << minColumnSize << " columns required"; \
    AQ_REQUIRE( matrix[0].size() >= minColumnSize, nested_ss.str().c_str() ) \
}
#endif

// Macro to check the size of a matrix and throw if the matrix does not have at least 2 columns
#ifndef AQ_2D_MATRIX_CHECK
#define AQ_2D_MATRIX_CHECK( matrix, error_message ) \
if ( !matrix.empty() ) \
{ \
    std::ostringstream nested_ss; \
    nested_ss << error_message << " - At least 2 columns required"; \
    AQ_REQUIRE( matrix[0].size() >= 2, nested_ss.str().c_str() )  \
}
#endif

// Macro to make a function call and throw an error message if a given function call throws
#ifndef AQ_TRY
#define AQ_TRY( function_call, error_message ) \
try \
{ \
    function_call; \
} \
catch(...) \
{ \
    std::ostringstream AQ_VALUE; \
    AQ_VALUE << "#Error: " << error_message; \
    throw AQLCoreInvalidData( AQ_VALUE.str().c_str(), __FILE__, __LINE__ ); \
}
#endif

// Macro to populate a variable using a function call or value
#ifndef AQ_SET_VARIABLE
#define AQ_SET_VARIABLE( variable, function_call ) \
    variable = function_call;
#endif

// Macro to populate a variable using a function call or override the value on failure
#ifndef AQ_TRY_SET_VARIABLE
#define AQ_TRY_SET_VARIABLE( variable, function_call, error_message ) \
try \
{ \
    variable = function_call; \
} \
catch(...) \
{ \
    std::ostringstream AQ_VALUE; \
    AQ_VALUE << "#Error: " << error_message; \
    throw AQLCoreInvalidData( AQ_VALUE.str().c_str(), __FILE__, __LINE__ ); \
}
#endif

// Macro to populate a variable using a function call or override the value on failure
#ifndef AQ_SET_VARIABLE_OR_OVERRIDE_ON_FAILURE
#define AQ_SET_VARIABLE_OR_OVERRIDE_ON_FAILURE( variable, function_call, error_value ) \
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
#ifndef AQ_EPSILON
#define AQ_EPSILON 1e-14
#endif

// Macro to round a variable to precision AQ_EPSILON
#ifndef AQ_ROUND
#define AQ_ROUND( variable, precision ) \
    boost::math::round( variable / precision ) * precision
#endif

// Macro to test if a variable is equal to a value with precision AQ_EPSILON
#ifndef AQ_IS_EQUAL
#define AQ_IS_EQUAL( variable, equal ) \
	AQ_IS_EQUAL_WITH_TOLERANCE( variable, equal, AQ_EPSILON )
#endif

// Macro to test if a variable is equal to a value with precision given by tolerance
//
// Every _WITH_TOLERANCE macro below is wrapped in an outer set of parens around its whole
// ternary, not just around the condition. Unparenthesized, `COND ? true : false` composed into a
// larger expression is a footgun: `?:` binds looser than `&&`/`||`, so
// `AQ_IS_EQUAL_WITH_TOLERANCE(a,b,c) && somethingElse` would expand to
// `COND ? true : (false && somethingElse)` - i.e. `COND ? true : false`, silently dropping
// `somethingElse` whenever COND is false, not evaluating it at all. The outer parens make the
// whole macro a single, safely-composable boolean subexpression, matching how a real bool-valued
// function call already behaves.
#ifndef AQ_IS_EQUAL_WITH_TOLERANCE
#define AQ_IS_EQUAL_WITH_TOLERANCE( variable, equal, tolerance ) \
    ( ( variable - equal > -tolerance && variable - equal < tolerance ) ? true : false )
#endif

// Macro to test if a variable is less than a value to precision AQ_EPSILON
#ifndef AQ_IS_LESS_THAN
#define AQ_IS_LESS_THAN( variable, lessThan ) \
    AQ_IS_LESS_THAN_WITH_TOLERANCE( variable, lessThan, AQ_EPSILON )
#endif

#ifndef AQ_IS_LESS_THAN_WITH_TOLERANCE
#define AQ_IS_LESS_THAN_WITH_TOLERANCE( variable, lessThan, tolerance ) \
    ( ( variable - lessThan < -tolerance ) ? true : false )
#endif

// Macro to test if a variable is less than a value to precision AQ_EPSILON
#ifndef AQ_IS_LESS_THAN_OR_EQUAL
#define AQ_IS_LESS_THAN_OR_EQUAL( variable, lessThan ) \
    AQ_IS_LESS_THAN_OR_EQUAL_WITH_TOLERANCE( variable, lessThan, AQ_EPSILON )
#endif

#ifndef AQ_IS_LESS_THAN_OR_EQUAL_WITH_TOLERANCE
#define AQ_IS_LESS_THAN_OR_EQUAL_WITH_TOLERANCE( variable, lessThan, tolerance ) \
    ( ( variable - lessThan <= tolerance ) ? true : false )
#endif

// Macro to test if a variable is greater than a value to precision AQ_EPSILON
#ifndef AQ_IS_GREATER_THAN
#define AQ_IS_GREATER_THAN( variable, greaterThan ) \
    AQ_IS_GREATER_THAN_WITH_TOLERANCE( variable, greaterThan, AQ_EPSILON )
#endif

#ifndef AQ_IS_GREATER_THAN_WITH_TOLERANCE
#define AQ_IS_GREATER_THAN_WITH_TOLERANCE( variable, greaterThan, tolerance ) \
    ( ( variable - greaterThan > tolerance ) ? true : false )
#endif

// Macro to test if a variable is greater than a value to precision AQ_EPSILON
#ifndef AQ_IS_GREATER_THAN_OR_EQUAL
#define AQ_IS_GREATER_THAN_OR_EQUAL( variable, greaterThan ) \
    AQ_IS_GREATER_THAN_OR_EQUAL_WITH_TOLERANCE( variable, greaterThan, AQ_EPSILON )
#endif

#ifndef AQ_IS_GREATER_THAN_OR_EQUAL_WITH_TOLERANCE
#define AQ_IS_GREATER_THAN_OR_EQUAL_WITH_TOLERANCE( variable, greaterThan, tolerance ) \
    ( ( variable - greaterThan >= -tolerance ) ? true : false )
#endif

// Macro to test if a variable is equal to zero to precision AQ_EPSILON
#ifndef AQ_IS_EQUAL_ZERO
#define AQ_IS_EQUAL_ZERO( variable ) \
    AQ_IS_EQUAL_ZERO_WITH_TOLERANCE( variable, AQ_EPSILON )
#endif

#ifndef AQ_IS_EQUAL_ZERO_WITH_TOLERANCE
#define AQ_IS_EQUAL_ZERO_WITH_TOLERANCE( variable, tolerance ) \
    ( ( variable > -tolerance && variable < tolerance ) ? true : false )
#endif

// Macro to test if a variable is less than a value to precision AQ_EPSILON
#ifndef AQ_IS_LESS_THAN_ZERO
#define AQ_IS_LESS_THAN_ZERO( variable ) \
    AQ_IS_LESS_THAN_ZERO_WITH_TOLERANCE( variable, AQ_EPSILON )
#endif

#ifndef AQ_IS_LESS_THAN_ZERO_WITH_TOLERANCE
#define AQ_IS_LESS_THAN_ZERO_WITH_TOLERANCE( variable, tolerance ) \
    ( ( variable < -tolerance ) ? true : false )
#endif

// Macro to test if a variable is less than a value to precision AQ_EPSILON
#ifndef AQ_IS_LESS_THAN_OR_EQUAL_TO_ZERO
#define AQ_IS_LESS_THAN_OR_EQUAL_TO_ZERO( variable ) \
    AQ_IS_LESS_THAN_OR_EQUAL_TO_ZERO_WITH_TOLERANCE( variable, AQ_EPSILON )
#endif

#ifndef AQ_IS_LESS_THAN_OR_EQUAL_TO_ZERO_WITH_TOLERANCE
#define AQ_IS_LESS_THAN_OR_EQUAL_TO_ZERO_WITH_TOLERANCE( variable, tolerance ) \
    ( ( variable <= tolerance ) ? true : false )
#endif

// Macro to test if a variable is greater than a value to precision AQ_EPSILON
#ifndef AQ_IS_GREATER_THAN_ZERO
#define AQ_IS_GREATER_THAN_ZERO( variable ) \
    AQ_IS_GREATER_THAN_ZERO_WITH_TOLERANCE( variable, AQ_EPSILON )
#endif

#ifndef AQ_IS_GREATER_THAN_ZERO_WITH_TOLERANCE
#define AQ_IS_GREATER_THAN_ZERO_WITH_TOLERANCE( variable, tolerance ) \
    ( ( variable > tolerance ) ? true : false )
#endif

// Macro to test if a variable is greater than a value to precision AQ_EPSILON
#ifndef AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO
#define AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( variable ) \
    AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO_WITH_TOLERANCE( variable, AQ_EPSILON )
#endif

#ifndef AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO_WITH_TOLERANCE
#define AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO_WITH_TOLERANCE( variable, tolerance ) \
    ( ( variable >= -tolerance ) ? true : false )
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
#ifndef AQ_TO_STRING_FROM_SIZE_T
#define AQ_TO_STRING_FROM_SIZE_T( input )											etrading::toStandardString< size_t >( input )	
#endif
	
#ifndef AQ_TO_STRING_FROM_LONG
#define AQ_TO_STRING_FROM_LONG( input )											etrading::toStandardString< long   >( input )
#endif

#ifndef AQ_TO_STRING_FROM_INT
#define AQ_TO_STRING_FROM_INT( input )											etrading::toStandardString< int    >( input )
#endif

#ifndef AQ_TO_STRING_FROM_FLOAT
#define AQ_TO_STRING_FROM_FLOAT( input )											etrading::toStandardString< float  >( input )
#endif

#ifndef AQ_TO_STRING_FROM_DOUBLE
#define AQ_TO_STRING_FROM_DOUBLE( input )											etrading::toStandardString< double >( input )
#endif
	
#ifndef AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION
#define AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION( input, decimalPlacePrecision )	etrading::toStandardString< float  >( input, decimalPlacePrecision )
#endif
	
#ifndef AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION
#define AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( input, decimalPlacePrecision )	etrading::toStandardString< double >( input, decimalPlacePrecision )
#endif