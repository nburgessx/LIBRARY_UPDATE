// aqToolData.h

/*
 * @brief			Swig interface for the aqToolAppend, aqToolClean, aqToolDataFilter and
 *					aqToolValuationSettingsDisplay data-reshaping functions
 */

#pragma once

#include <string>
#include <vector>

#include "SwigTypes.h"	// For SWIG_STRINGMATRIX which manages the AQLStringMatrix idiosyncrasies type for R API

// aqToolAppend and aqToolClean take arbitrary-shape range(s) with no fixed column count. R represents a
// matrix as a flattened vector with no shape metadata, so there is no way to unflatten a generic-shape
// range on the R side; excluded from the R binding rather than guessed at.
#if (!defined(SWIG_R)) && (!defined(SWIGR))
/* @brief			swig interface for aqToolAppend. Append up to ten ranges into one, stacking by row or by column.
*  @param [in]		appendByRow		TRUE stacks the ranges vertically, FALSE horizontally
*  @param [in]		matrix1			Range 1
*  @param [in]		matrix2			Optional. Range 2
*  @param [in]		matrix3			Optional. Range 3
*  @param [in]		matrix4			Optional. Range 4
*  @param [in]		matrix5			Optional. Range 5
*  @param [in]		matrix6			Optional. Range 6
*  @param [in]		matrix7			Optional. Range 7
*  @param [in]		matrix8			Optional. Range 8
*  @param [in]		matrix9			Optional. Range 9
*  @param [in]		matrix10		Optional. Range 10
*  @return			The appended matrix
*/
SWIG_STRINGMATRIX aqToolAppend( const bool appendByRow,
                                 const SWIG_STRINGMATRIX& matrix1,
                                 const SWIG_STRINGMATRIX& matrix2 = SWIG_STRINGMATRIX(),
                                 const SWIG_STRINGMATRIX& matrix3 = SWIG_STRINGMATRIX(),
                                 const SWIG_STRINGMATRIX& matrix4 = SWIG_STRINGMATRIX(),
                                 const SWIG_STRINGMATRIX& matrix5 = SWIG_STRINGMATRIX(),
                                 const SWIG_STRINGMATRIX& matrix6 = SWIG_STRINGMATRIX(),
                                 const SWIG_STRINGMATRIX& matrix7 = SWIG_STRINGMATRIX(),
                                 const SWIG_STRINGMATRIX& matrix8 = SWIG_STRINGMATRIX(),
                                 const SWIG_STRINGMATRIX& matrix9 = SWIG_STRINGMATRIX(),
                                 const SWIG_STRINGMATRIX& matrix10 = SWIG_STRINGMATRIX() );

/* @brief			swig interface for aqToolClean. Remove blank and/or error cells from a range, row by row or column by column.
*  @param [in]		inputMatrix				The matrix to clean
*  @param [in]		cleanByRow				Optional. Default TRUE. FALSE cleans column by column
*  @param [in]		checkRowColumnNumber	Optional. Row/column index whose blanks/errors drive the removal. Default 0
*  @param [in]		removeBlanks			Optional. Default TRUE. Remove blank cells
*  @param [in]		removeErrors			Optional. Default TRUE. Remove error cells
*  @return			The cleaned matrix
*/
SWIG_STRINGMATRIX aqToolClean( const SWIG_STRINGMATRIX& inputMatrix,
                                const bool cleanByRow = true,
                                const int checkRowColumnNumber = 0,
                                const bool removeBlanks = true,
                                const bool removeErrors = true );
#endif

/* @brief			swig interface for aqToolDataFilter. Filter a vector of input data and remove duplicates.
*  @param [in]		dataVector		The vector of mixed values
*  @param [in]		displayByRow	Optional. Default FALSE. TRUE lays the result out by row
*  @return			A filtered data list with duplicates removed
*/
SWIG_STRINGMATRIX aqToolDataFilter( const std::vector<std::string>& dataVector, const bool displayByRow = false );

/* @brief			swig interface for aqToolValuationSettingsDisplay. Resolve a raw valuation-settings block
*                   into the settings actually used for pricing.
*  @param [in]		rawInput	The raw valuation-settings block, rows of (key, value)
*  @return			The resolved valuation settings
*/
SWIG_STRINGMATRIX aqToolValuationSettingsDisplay( const SWIG_STRINGMATRIX& rawInput );
