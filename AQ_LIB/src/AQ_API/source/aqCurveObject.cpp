#include "aqCurveResults.h"
#include "tryAqCurveObjectDiscountFactor.h"
#include "tryAqCurveObjectDisplay.h"
#include "tryAqCurveMarketData.h"
#include "tryAqCurveObjectCalibrate.h"
#include "tryAqIRFixingTable.h"

#include "ContainerUtilities.h"
#include "JSONInfoBlock.h"
#include "CoreEnumerations.h"
#include "ExceptionMacros.h"
#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "Variant.h"                // Variant and Variant Matrix Types
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

/* @brief			swig interface for aqCurveObjectDisplay
*  @param [in]		curveHandle		    AQObj curve handle or object name
*  @return			StandardStringMatrix of AQObj Curve static and market data inputs
*/
SWIG_STRINGMATRIX aqCurveObjectDisplay(const std::string& curveHandle)
{
	AQ_API_START
    
	// This returns a ragged matrix
	etrading::VariantMatrix results = validation::tryAqCurveObjectDisplay(curveHandle);

	// Transpose the result so that we have two columns of curve config, and further columns of market data
	etrading::VariantMatrix transposedResults = etrading::transpose( results, true /* pad if input is a ragged matrix */ );

	SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( transposedResults );

    return resultsStringMatrix;

    AQ_API_END
}

/* @brief			swig interface for aqCurveMarketDataDisplay
*  @param [in]		marketDataObjectName	AQObj curve market data handle or object name
*  @param [in]		marketDataKey			The specific market data block of interest, for example "SWAPS"
*  @return			A string matrix containing the specified market data block.
*/
SWIG_STRINGMATRIX aqCurveMarketDataDisplay(const std::string& marketDataObjectName, const std::string marketDataKey )
{
	AQ_API_START
    
	etrading::VariantMatrix results = validation::tryAqCurveMarketDataDisplay( marketDataObjectName, marketDataKey );

    // Marshall Output to Standard String Matrix
	SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( results );

    return resultsStringMatrix;

    AQ_API_END
}

/* @brief			swig interface for aqObjCurvesDisplayMarketDataFromCurve
*  @param [in]		curveObjectName			AQObj curve data handle or object name
*  @param [in]		marketDataKey			The specific market data block of interest, for example "SWAPS"
*  @return			A string matrix containing the specified market data block.
*/
SWIG_STRINGMATRIX aqCurveMarketDataDisplayFromCurve(const std::string& curveObjectName, const std::string marketDataKey )
{
	AQ_API_START
    
	etrading::VariantMatrix results = validation::tryAqCurveMarketDataDisplayFromCurve( curveObjectName, marketDataKey );

    // Marshall Output to Standard String Matrix
	SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( results );

    return resultsStringMatrix;

    AQ_API_END
}

/* @brief Builds a curve using a CurveGenerator object and a CurveMarketData object
* @param [in] aqObjCurveGeneratorName     The name of the AQObjCurveGenerator object to use
* @param [in] aqObjCurveMarketDataName    The name of the AQObjCurveMarketData object to use
* @param [in] domesticCurveCollection   The curveCollection this curve will be stored in ( The Target CurveCollection )
* @param [in] foreignCurveCollection    The curveCollection containing foreign curve dependencies ( The Against CurveCollection )
*                                       Allowed to be blank if this is a single CCY curve (OIS, STD, TenorBasis)
* @param [out]                          The curve build status
*/
std::string aqCurveObjectCalibrate( const std::string& objectName,
								 const std::string& aqObjCurveGeneratorName,
								 const std::string& aqObjCurveMarketDataName,
								 const std::string& domesticCurveCollection,
								 const std::string& foreignCurveCollection )
{
	AQ_API_START

	std::string curveIndexAndAliasNames = validation::tryAqCurveObjectCalibrate( objectName, aqObjCurveGeneratorName, aqObjCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );
	return curveIndexAndAliasNames;

	AQ_API_END
}

	/* @brief			Builds a "TableInfo" tuple from a AQLStringMatrix of marketdata
	*                   This tuple consists of columnNames, columnTypes and the actual data values.
	*  @param [in]		marketDataBlock		A AQLStringMatrix containing key/value market data values
	*/
	etrading::JSONInfoBlockTuple getTableInfoFromStringMatrix( const AQLStringMatrix& marketDataBlock )
	{
		etrading::VariantMatrix variantMatrix;
		swig::buildVariantMatrix( variantMatrix, marketDataBlock );
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


/* @brief Creates a AQObjCurveMarketData object, containing all of the curve properties.
* @param [in] objectName	The name of the Market Data object
* @param [in] key1			The name of datablock1 For example "MARKETDATAPROPERTIES"
* @param [in] value1		The contents of datablock1 
* @param [in] key2			The name of datablock2 For example "OIS"
* @param [in] value2		The contents of datablock2
* @param [in] key3			The name of datablock3 For example "LIBOROISBASISSPREADS"
* @param [in] value3		The contents of datablock3
* @param [in] key4			The name of datablock4 For example "SWAPS"
* @param [in] value4		The contents of datablock4
* @param [out]              The objectName
*/
std::string aqCurveMarketDataCreate( const std::string& objectName,
										const std::string& key1, const SWIG_STRINGMATRIX& value1,
										const std::string& key2, const SWIG_STRINGMATRIX& value2,
										const std::string& key3, const SWIG_STRINGMATRIX& value3,
										const std::string& key4, const SWIG_STRINGMATRIX& value4)
{
	AQ_API_START

	std::vector<std::string> marketDataKeys;
	std::vector<etrading::JSONInfoBlockTuple> infoBlocks;

	AQ_REQUIRE( key1 == etrading::GENERATOR_COMPONENTS::KEY_MARKETDATAPROPERTIES, "Please specify the MARKETDATAPROPERTIES in the first data block." );
	marketDataKeys.push_back( key1 );

	AQLStringMatrix matrix1;
	swig::buildStringMatrix( matrix1, value1 );

	// Extract the CurveType from the MARKETDATAPROPERTIES
	LabelValueBlock propertiesLVB( matrix1 );
	const std::string curveType = propertiesLVB.getCompulsoryValueAsString( etrading::CURVEGENERATOR_CURVEPROPERTIES_KEY::CURVE_TYPE );
	const etrading::CurveTypeEnum curveTypeEnum = etrading::toCurveTypeEnum( curveType );

	infoBlocks.push_back( getTableInfoFromStringMatrix( matrix1 ));

	if ( value2.size() > 0 )
	{
		marketDataKeys.push_back( key2 );
		
		AQLStringMatrix matrix2;
		const etrading::CurveMarketDataEnum curveMarketDataEnum = etrading::toCurveMarketDataEnum( key2 );
		swig::buildStringMatrix( matrix2, value2, curveTypeEnum, curveMarketDataEnum );
		infoBlocks.push_back( getTableInfoFromStringMatrix( matrix2 ));
	}

	if ( value3.size() > 0 )
	{
		marketDataKeys.push_back( key3 );
		
		AQLStringMatrix matrix3;
		const etrading::CurveMarketDataEnum curveMarketDataEnum = etrading::toCurveMarketDataEnum( key3 );
		swig::buildStringMatrix( matrix3, value3, curveTypeEnum, curveMarketDataEnum  );
		infoBlocks.push_back( getTableInfoFromStringMatrix( matrix3 ));
	}

	if ( value4.size() > 0 )
	{
		marketDataKeys.push_back( key4 );
		
		AQLStringMatrix matrix4;
		const etrading::CurveMarketDataEnum curveMarketDataEnum = etrading::toCurveMarketDataEnum( key4 );
		swig::buildStringMatrix( matrix4, value4, curveTypeEnum, curveMarketDataEnum );
		infoBlocks.push_back( getTableInfoFromStringMatrix( matrix4 ));
	}

	std::string result = validation::tryAqCurveMarketDataCreate( objectName, marketDataKeys, infoBlocks );

	return result;

	AQ_API_END

}

/* @brief Creates a AQObjCurveMarketData object, containing all of the curve properties.
*			NOTE: This function not work correctly in R because R cannot cope with vectors of vectors.
*			In particular the vector<SWIG_STRINGMATRIX> parameter is a problem in R.
* @param [in] objectName	The name of the Market Data object
* @param [in] keyVector		The names all the datablocks, for example "MARKETDATAPROPERTIES", "SWAPS", "FUTURES", ...
* @param [in] valueVector	The vector of datablocks
* @param [out]              The objectName
*/
std::string aqObjCurvesMarketDataCreateUsingMultipleBlocks( const std::string& objectName,
												  		   const std::vector<std::string>& keyVector,
														   const std::vector<SWIG_STRINGMATRIX>& valueVector )
{
	AQ_API_START
	
	AQ_REQUIRE( keyVector.size() == valueVector.size(), "Number of keys must match number of value blocks in CurveMarketDataCreate().");
	AQ_REQUIRE( keyVector.size() > 0, "Require 1 or more value blocks in CurveMarketDataCreate().");

	const std::string& key1 = keyVector[0];
	AQ_REQUIRE( key1 == etrading::GENERATOR_COMPONENTS::KEY_MARKETDATAPROPERTIES, "Please specify the MARKETDATAPROPERTIES in the first data block." );
	
	// Read the first block of data to determine the curveType
	AQLStringMatrix matrix1;
	const SWIG_STRINGMATRIX& value1 = valueVector[0];
	swig::buildStringMatrix( matrix1, value1 );

	// Extract the CurveType from the MARKETDATAPROPERTIES
	LabelValueBlock propertiesLVB( matrix1 );
	const std::string curveType = propertiesLVB.getCompulsoryValueAsString( etrading::CURVEGENERATOR_CURVEPROPERTIES_KEY::CURVE_TYPE );
	const etrading::CurveTypeEnum curveTypeEnum = etrading::toCurveTypeEnum( curveType );

	std::vector<etrading::JSONInfoBlockTuple> infoBlocks;
	infoBlocks.push_back( getTableInfoFromStringMatrix( matrix1 ));

	// Process remaining blocks of data
	for (size_t idx=1; idx<keyVector.size(); idx++)
	{		
		const std::string& key = keyVector[idx];
		const SWIG_STRINGMATRIX& value = valueVector[idx];
		const etrading::CurveMarketDataEnum curveMarketDataEnum = etrading::toCurveMarketDataEnum( key );

		AQLStringMatrix matrix;
		swig::buildStringMatrix( matrix, value, curveTypeEnum, curveMarketDataEnum  );
		infoBlocks.push_back( getTableInfoFromStringMatrix( matrix ));
	}

	// Build the curve market data
	std::string result = validation::tryAqCurveMarketDataCreate( objectName, keyVector, infoBlocks );

	return result;

	AQ_API_END
}

/* @brief			function to create a fixing table object
*  @param [in]		tableName		    Fixing Table name
*  @param [in]		currency	        Currency as a string
*  @param [in]		curveTenor	        The curve tenor string: ANNUAL, SEMI-ANNUAL, QUARTERLY, MONTHLY, DAILY ...
*  @param [in]		fixingDates	        Vector of Fixing Dates in String Format
*  @param [in]		fixingValues	    Vector of Fixing Values in Double Format
*  @return			returns the name of the fixing table on the cache
*/
std::string aqIRFixingTableCreate( const std::string& tableName,
                                    const std::string& currency,                                   
                                    const std::string& curveTenor,
                                    const std::vector< std::string >& fixingDates,
                                    const std::vector< double >& fixingValues )
{
    AQ_API_START
    
    // Marshall Inputs
    std::vector<boost::gregorian::date> gregorianFixingDates;
    swig::buildGregorianDateVector( gregorianFixingDates, fixingDates );

    // Call Method
	const std::string result = validation::tryAqIRFixingTableCreate( tableName,
                                                                          currency,
                                                                          curveTenor,
                                                                          gregorianFixingDates, 
                                                                          fixingValues );
    // Output
    return result;

	AQ_API_END
}

/* @brief			function to display a fixing table object
*  @param [in]		tableName		    Fixing Table name
*  @return			returns a VariantMatrix representing the fixing currency, curveTenor, fixingDates and fixingValues
*/
SWIG_STRINGMATRIX aqIRFixingTableDisplay( const std::string& tableName )
{
    AQ_API_START
    
    // Call Method
	etrading::VariantMatrix results = validation::tryAqIRFixingTableDisplay( tableName );

    // Marshall Output to Standard String Matrix
	SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( results );
    return resultsStringMatrix;

    AQ_API_END
}

/* @brief			function to get the fixing value for a particular date
*  @param [in]		tableName		    Fixing Table name
*  @param [in]		fixingDate	        Fixing Date
*  @return			swapName
*/
double aqIRFixingTableValue( const std::string& tableName, const std::string & fixingDate )
{
    AQ_API_START
    
    // Input Marshalling
    boost::gregorian::date gregorianFixingDate( etrading::validateAndConvertStringToGregorianDate( fixingDate ) );
    
    // Call Method
    const double result = validation::tryAqIRFixingTableValue( tableName, gregorianFixingDate );
	
    // Output
    return result;

	AQ_API_END
}

/* @brief			function to get fixing values for a vector of dates
*  @param [in]		tableName		    Fixing Table name
*  @param [in]		fixingDates     	Fixing Dates
*  @return			swapName
*/
std::vector<double> aqIRFixingTableValues( const std::string& tableName, const std::vector< std::string >& fixingDates )
{
    AQ_API_START

    // Input Marshalling
    std::vector<boost::gregorian::date> gregorianFixingDates;
    swig::buildGregorianDateVector( gregorianFixingDates, fixingDates );
    
    // Call Method
    const std::vector<double> results = validation::tryAqIRFixingTableValues( tableName, gregorianFixingDates );
	
    // Output
    return results;

	AQ_API_END
}

/* @brief			Validation method which calculates a set of discount factors for the specified dates.
*					If a spread is provided, calculates the discount factors using the zero discount-margin
*					approach described by O'Kane in "Credit Spreads Explained".
*
*  @param [in]		paymentDates	A vector of one or more dates
*  @param [in]		curveCollection	CurveCollection name
*  @param [in]		curveIndex		Index of the curve.
*  @param [in]		spread			Spread to be added to the curve zero rate when calculating the discount factor
*  @param [in]		fixingTableName An optional fixing table name. This field may be blank if no fixings are required.
*  @return			A vector of discount factors
*/
std::vector<double> aqCurveObjectDiscountFactorsWithSpread( const std::vector<std::string>& paymentDates, const std::string& curveCollection, const std::string& curveIndex, const double& spread, const std::string& fixingTableName )
{
    AQ_API_START

    // Input Marshalling
     DateVector tempPaymentDates;
    swig::buildDateVector( tempPaymentDates, paymentDates );
    
    // Call Method
    const std::vector<double> discountFactors = validation::tryAqCurveObjectDiscountFactorsWithSpread( tempPaymentDates, curveCollection, curveIndex, spread, fixingTableName );
	
    // Output
    return discountFactors;

	AQ_API_END

}

