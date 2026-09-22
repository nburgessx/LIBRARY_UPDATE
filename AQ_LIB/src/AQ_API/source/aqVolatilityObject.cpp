// aqVolatilityObject.cpp

/*
 * @brief			Swig interface for aqVolatilityObject... functions
 */

#include "aqVolatilityObject.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "ContainerUtilities.h"
#include "JSONInfoBlock.h"
#include "CoreEnumerations.h"
#include "ExceptionMacros.h"
#include "Variant.h"                // Variant and Variant Matrix Types
#include "tryAqVolatilityObject.h"
#include "APISetUp.h"					// AQ_API_START and AQ_API_END Macros

namespace
{
    /* @brief			Build a JSONInfoBlockTuple (column names, column types, transposed data) from a key/value matrix.
    *  @param [in]		block			The data block, as an AQLStringMatrix
    *  @return			The equivalent JSONInfoBlockTuple
    */
    etrading::JSONInfoBlockTuple aqVolatilityTableInfo( const AQLStringMatrix& block )
    {
        etrading::VariantMatrix variantMatrix;
        swig::buildVariantMatrix( variantMatrix, block );
        const etrading::VariantMatrix transposedMatrix = etrading::transpose( variantMatrix );

        size_t numColumns = transposedMatrix.size();

        // EnumTypes for each column
        std::vector<etrading::ContainedTypeEnum> columnEnumTypes = etrading::Variant::getContainedTypeInfo( transposedMatrix );

        // Construct dummy column headings
        std::vector<int> nColCounters;
        boost::push_back( nColCounters, boost::irange( 1, static_cast<int>( numColumns ) + 1 ) );
        std::vector<std::string> columnNames( numColumns, std::string( "COL_" ) );
        columnNames = etrading::zip_paste<std::vector<std::string>, std::vector<int>, std::string>( columnNames, nColCounters );

        // Construct the TableInfo
        return std::make_tuple( columnNames, columnEnumTypes, transposedMatrix );
    }
}

/* @brief			Function to create and store a volatility surface from a label/value block
*  @param [in]		objectName			Volatility-surface object name
*  @param [in]		volLVB				Volatility-surface definition as a key/value matrix
*  @param [in]		validateKeys		Optional. Default TRUE. Check the LVB keys
*  @return			Volatility-surface object handle
*/
std::string aqVolatilityObjectCreate( const std::string& objectName,
                                       const SWIG_STRINGMATRIX& volLVB,
                                       const bool validateKeys )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix volLVB_;
    swig::buildStringMatrix( volLVB_, volLVB );
    LabelValueBlock volLVBAsLabelValueBlock( volLVB_ );

    // Call Function and Return Result
    std::string result = validation::tryAqVolatilityObjectCreate( objectName, volLVBAsLabelValueBlock, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			Function to create and store SABR market data from one or two named data blocks
*  @param [in]		objectName			Name for the SABR market-data object
*  @param [in]		key1				Name of the first data block
*  @param [in]		value1				First data block, as a key/value matrix
*  @param [in]		key2				Optional. Name of the second data block
*  @param [in]		value2				Optional. Second data block, as a key/value matrix
*  @return			SABR market-data object handle
*/
std::string aqVolatilityObjectSabrMarketDataCreate( const std::string& objectName,
                                                     const std::string& key1,
                                                     const SWIG_STRINGMATRIX& value1,
                                                     const std::string& key2,
                                                     const SWIG_STRINGMATRIX& value2 )
{
    AQ_API_START

    // Marshall Inputs
    std::vector<std::string> dataBlockNames;
    etrading::JSONInfoBlockTuples infoBlocks;

    AQLStringMatrix matrix1;
    swig::buildStringMatrix( matrix1, value1 );
    dataBlockNames.push_back( key1 );
    infoBlocks.push_back( aqVolatilityTableInfo( matrix1 ) );

    if ( value2.size() > 0 )
    {
        AQLStringMatrix matrix2;
        swig::buildStringMatrix( matrix2, value2 );
        dataBlockNames.push_back( key2 );
        infoBlocks.push_back( aqVolatilityTableInfo( matrix2 ) );
    }

    // Call Function and Return Result
    std::string result = validation::tryAqVolatilityObjectSabrMarketDataCreate( objectName, dataBlockNames, infoBlocks );
    return result;

    AQ_API_END
}

/* @brief			Function to calibrate and store a SABR model from one or two named data blocks
*  @param [in]		objectName			Name for the SABR model object, calibrated to market data
*  @param [in]		key1				Name of the first data block
*  @param [in]		value1				First data block, as a key/value matrix
*  @param [in]		key2				Optional. Name of the second data block
*  @param [in]		value2				Optional. Second data block, as a key/value matrix
*  @return			SABR model object handle
*/
std::string aqVolatilityObjectSabrModelCalibrate( const std::string& objectName,
                                                   const std::string& key1,
                                                   const SWIG_STRINGMATRIX& value1,
                                                   const std::string& key2,
                                                   const SWIG_STRINGMATRIX& value2 )
{
    AQ_API_START

    // Marshall Inputs
    std::vector<std::string> dataBlockNames;
    etrading::JSONInfoBlockTuples infoBlocks;

    AQLStringMatrix matrix1;
    swig::buildStringMatrix( matrix1, value1 );
    dataBlockNames.push_back( key1 );
    infoBlocks.push_back( aqVolatilityTableInfo( matrix1 ) );

    if ( value2.size() > 0 )
    {
        AQLStringMatrix matrix2;
        swig::buildStringMatrix( matrix2, value2 );
        dataBlockNames.push_back( key2 );
        infoBlocks.push_back( aqVolatilityTableInfo( matrix2 ) );
    }

    // Call Function and Return Result
    std::string result = validation::tryAqVolatilityObjectSabrModelCalibrate( objectName, dataBlockNames, infoBlocks );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the SABR-implied volatility at an expiry/tenor/strike/forward point
*  @param [in]		volSurfaceName		A volatility-surface handle
*  @param [in]		expiry				Option expiry, e.g. 3M, 1Y
*  @param [in]		tenor				Underlying tenor, e.g. 5Y, 10Y
*  @param [in]		strike				Strike
*  @param [in]		forward				Forward rate
*  @return			SABR-implied volatility
*/
double aqVolatilityObjectSabrVolatility( const std::string& volSurfaceName,
                                          const std::string& expiry,
                                          const std::string& tenor,
                                          const double strike,
                                          const double forward )
{
    AQ_API_START

    // Call Function and Return Result
    double result = validation::tryAqVolatilityObjectSabrVolatility( volSurfaceName, expiry, tenor, strike, forward );
    return result;

    AQ_API_END
}

/* @brief			Function to obtain one SABR parameter at an expiry/tenor point
*  @param [in]		volSurfaceName		A volatility-surface handle
*  @param [in]		expiry				Option expiry, e.g. 3M, 1Y
*  @param [in]		tenor				Underlying tenor, e.g. 5Y, 10Y
*  @param [in]		paramName			Which parameter, e.g. ALPHA, BETA, RHO, NU
*  @return			The SABR parameter value
*/
double aqVolatilityObjectSabrParameter( const std::string& volSurfaceName,
                                         const std::string& expiry,
                                         const std::string& tenor,
                                         const std::string& paramName )
{
    AQ_API_START

    // Call Function and Return Result
    double result = validation::tryAqVolatilityObjectSabrParameter( volSurfaceName, expiry, tenor, paramName );
    return result;

    AQ_API_END
}

/* @brief			Function to display stored SABR market data
*  @param [in]		marketDataObjectName	A SABR market-data handle
*  @param [in]		marketDataKey			The data block to display
*  @return			Stored SABR market data, as a string matrix
*/
SWIG_STRINGMATRIX aqVolatilityObjectSabrMarketDataDisplay( const std::string& marketDataObjectName,
                                                            const std::string& marketDataKey )
{
    AQ_API_START

    // Call the Function
    etrading::VariantMatrix results = validation::tryAqVolatilityObjectSabrMarketDataDisplay( marketDataObjectName, marketDataKey );

    // Marshall Output to Standard String Matrix
    SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( results );
    return resultsStringMatrix;

    AQ_API_END
}

/* @brief			Function to display a stored SABR model
*  @param [in]		modelObjectName		A SABR model handle
*  @param [in]		modelKey			The model block to display
*  @return			Stored SABR model, as a string matrix
*/
SWIG_STRINGMATRIX aqVolatilityObjectSabrModelDisplay( const std::string& modelObjectName,
                                                       const std::string& modelKey )
{
    AQ_API_START

    // Call the Function
    etrading::VariantMatrix results = validation::tryAqVolatilityObjectSabrModelDisplay( modelObjectName, modelKey );

    // Marshall Output to Standard String Matrix
    SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( results );
    return resultsStringMatrix;

    AQ_API_END
}
