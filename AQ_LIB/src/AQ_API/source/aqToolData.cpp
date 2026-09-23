// aqToolData.cpp

/*
 * @brief			Swig interface for the aqToolAppend, aqToolClean, aqToolDataFilter and
 *					aqToolValuationSettingsDisplay data-reshaping functions
 */

#include "aqToolData.h"
#include "tryAqToolAppend.h"
#include "tryAqToolClean.h"
#include "tryAqToolDataFilter.h"
#include "tryAqToolValuationSettings.h"

#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "Variant.h"                // Variant and Variant Matrix Types
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

// aqToolAppend and aqToolClean take arbitrary-shape range(s); see the matching note in aqToolData.h.
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
                                 const SWIG_STRINGMATRIX& matrix2,
                                 const SWIG_STRINGMATRIX& matrix3,
                                 const SWIG_STRINGMATRIX& matrix4,
                                 const SWIG_STRINGMATRIX& matrix5,
                                 const SWIG_STRINGMATRIX& matrix6,
                                 const SWIG_STRINGMATRIX& matrix7,
                                 const SWIG_STRINGMATRIX& matrix8,
                                 const SWIG_STRINGMATRIX& matrix9,
                                 const SWIG_STRINGMATRIX& matrix10 )
{
    AQ_API_START

    // Marshall Inputs
    etrading::VariantMatrix matrix1_; swig::buildVariantMatrix( matrix1_, matrix1 );
    etrading::VariantMatrix matrix2_; swig::buildVariantMatrix( matrix2_, matrix2 );
    etrading::VariantMatrix matrix3_; swig::buildVariantMatrix( matrix3_, matrix3 );
    etrading::VariantMatrix matrix4_; swig::buildVariantMatrix( matrix4_, matrix4 );
    etrading::VariantMatrix matrix5_; swig::buildVariantMatrix( matrix5_, matrix5 );
    etrading::VariantMatrix matrix6_; swig::buildVariantMatrix( matrix6_, matrix6 );
    etrading::VariantMatrix matrix7_; swig::buildVariantMatrix( matrix7_, matrix7 );
    etrading::VariantMatrix matrix8_; swig::buildVariantMatrix( matrix8_, matrix8 );
    etrading::VariantMatrix matrix9_; swig::buildVariantMatrix( matrix9_, matrix9 );
    etrading::VariantMatrix matrix10_; swig::buildVariantMatrix( matrix10_, matrix10 );

    // Call Function and Return Result
    etrading::VariantMatrix result = validation::tryAqToolAppend( appendByRow,
        matrix1_, matrix2_, matrix3_, matrix4_, matrix5_, matrix6_, matrix7_, matrix8_, matrix9_, matrix10_ );

    return swig::fromVariantMatrixToMatrixOfString( result );

    AQ_API_END
}

/* @brief			swig interface for aqToolClean. Remove blank and/or error cells from a range, row by row or column by column.
*  @param [in]		inputMatrix				The matrix to clean
*  @param [in]		cleanByRow				Optional. Default TRUE. FALSE cleans column by column
*  @param [in]		checkRowColumnNumber	Optional. Row/column index whose blanks/errors drive the removal. Default 0
*  @param [in]		removeBlanks			Optional. Default TRUE. Remove blank cells
*  @param [in]		removeErrors			Optional. Default TRUE. Remove error cells
*  @return			The cleaned matrix
*/
SWIG_STRINGMATRIX aqToolClean( const SWIG_STRINGMATRIX& inputMatrix,
                                const bool cleanByRow,
                                const int checkRowColumnNumber,
                                const bool removeBlanks,
                                const bool removeErrors )
{
    AQ_API_START

    // Marshall Inputs
    etrading::VariantMatrix inputMatrix_;
    swig::buildVariantMatrix( inputMatrix_, inputMatrix );

    // Call Function and Return Result
    etrading::VariantMatrix result = validation::tryAqToolClean( inputMatrix_, cleanByRow, checkRowColumnNumber, removeBlanks, removeErrors );

    return swig::fromVariantMatrixToMatrixOfString( result );

    AQ_API_END
}
#endif

/* @brief			swig interface for aqToolDataFilter. Filter a vector of input data and remove duplicates.
*  @param [in]		dataVector		The vector of mixed values
*  @param [in]		displayByRow	Optional. Default FALSE. TRUE lays the result out by row
*  @return			A filtered data list with duplicates removed
*/
SWIG_STRINGMATRIX aqToolDataFilter( const std::vector<std::string>& dataVector, const bool displayByRow )
{
    AQ_API_START

    // Marshall Inputs
    etrading::VariantVector dataVector_;
    swig::buildVariantVector( dataVector_, dataVector );

    // Call Function and Return Result
    etrading::VariantMatrix result = validation::tryAqToolDataFilter( dataVector_, displayByRow );

    return swig::fromVariantMatrixToMatrixOfString( result );

    AQ_API_END
}

/* @brief			swig interface for aqToolValuationSettingsDisplay. Resolve a raw valuation-settings block
*                   into the settings actually used for pricing.
*  @param [in]		rawInput	The raw valuation-settings block, rows of (key, value)
*  @return			The resolved valuation settings
*/
SWIG_STRINGMATRIX aqToolValuationSettingsDisplay( const SWIG_STRINGMATRIX& rawInput )
{
    AQ_API_START

#if defined(SWIG_R) || defined(SWIGR)
    // rawInput is R's flat, column-by-column vector<string>; a valuation-settings block is always
    // (key, value) - 2 columns - and tryAqToolValuationSettingsDisplay returns "the modified input",
    // i.e. the same 2-column shape it was given, so both directions unflatten/flatten at nCols = 2.
    const size_t nCols = 2;
    const size_t nRows = rawInput.size() / nCols;

    StandardStringMatrix input;
    for ( size_t i = 0; i < nRows; ++i )
    {
        StandardStringVector row;
        for ( size_t j = 0; j < nCols; ++j )
        {
            row.push_back( rawInput[ nRows * j + i ] );
        }
        input.push_back( row );
    }

    StandardStringMatrix result = validation::tryAqToolValuationSettingsDisplay( input );

    SWIG_STRINGMATRIX flat;
    for ( size_t j = 0; j < nCols; ++j )
    {
        for ( size_t i = 0; i < result.size(); ++i )
        {
            flat.push_back( j < result[i].size() ? result[i][j] : std::string() );
        }
    }
    return flat;
#else
    StandardStringMatrix result = validation::tryAqToolValuationSettingsDisplay( rawInput );
    return result;
#endif

    AQ_API_END
}
