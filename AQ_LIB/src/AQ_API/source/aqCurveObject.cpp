#include "aqCurveResults.h"
#include "tryAqCurveObjectDiscountFactor.h"
#include "tryAqCurveObjectDisplay.h"
#include "tryAqCurveMarketData.h"
#include "tryAqCurveObjectCalibrate.h"
#include "tryAqIRFixingTable.h"
#include "tryAqCurveObjectUtilities.h"       // validation::tryAqCurveObject{List,Delete,DeleteAll,Save,Load}
#include "tryAqCurveObjectData.h"            // validation::tryAqCurveObjectDataCreate/Display
#include "tryAqCurveGenerator.h"             // validation::tryAqCurveObjectDisplayConventions
#include "tryAqCurveObjectDualBootstrap.h"   // validation::tryAqCurveObjectDualBootstrap
#include "tryAqCurveObjectCalibrateHedge.h"  // validation::tryAqCurveObjectCalibrateHedge
#include "tryAqCurveObjectEngineCalibrate.h" // validation::tryAqCurveObjectEngineCalibrate
#include "tryAqCurveObjectCreateBasis.h"     // validation::tryAqCurveObjectCreateBasis
#include "tryAqCurveObjectCreateFXForwards.h" // validation::tryAqCurveObjectCreateFXForwards
#include "tryAqCurveObjectCreateOIS.h"        // validation::tryAqCurveObjectCreateOIS
#include "tryAqCurveObjectCreateSwap.h"       // validation::tryAqCurveObjectCreateSwap
#include "tryAqCurveObjectForwardRate.h"      // validation::tryAqCurveObjectForwardRates*
#include "tryAqCurveObjectJacobianDisplay.h"  // validation::tryAqCurveObjectEngineJacobianDisplay / tryAqCurveObjectJacobianDisplay
#include "tryAqCurveDelete.h"                 // validation::tryAqCurveDelete
#include "tryAqCurveDiscountFactor.h"         // validation::tryAqCurveDiscountFactors*, tryAqCurveDatesToTerms etc
#include "tryAqCurveForwardRate.h"            // validation::tryAqCurveForwardRates*
#include "tryAqCurveResults.h"                // validation::tryAqCurveResults*, tryAqCurveGroup*
#include "tryAqCurveCompoundRate.h"           // validation::tryAqCurveCompoundRateWithFixingTable
#include "tryAqCurveCalibrateCTD.h"           // validation::tryAqCurveCalibrateCTD
#include "tryAqCurveVasicek.h"                // validation::tryAqCurveVasicekChecking / tryAqCurveVasicekForwardRates
#include "tryAqCurveHullWhite.h"              // validation::tryAqCurveHullWhiteForwardRates
#include "tryAqCurveFrequency.h"              // validation::tryAqCurveFrequency
#include "tryAqCurveEuroDollarConvexityAdjustment.h" // validation::tryAqCurveEuroDollarConvexityAdjustment

#include "ContainerUtilities.h"
#include "JSONInfoBlock.h"
#include "CoreEnumerations.h"
#include "ExceptionMacros.h"
#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "Variant.h"                // Variant and Variant Matrix Types
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"    // etrading::stringToDate

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
std::string aqCurveMarketDataCreate( const std::string& objectName,
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

/* -------------------------------------------------------------------------
 *  Object lifecycle
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectList
*  @return			The names of every cached curve
*/
std::vector<std::string> aqCurveObjectList()
{
	AQ_API_START

	return validation::tryAqCurveObjectList();

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectDelete
*  @param [in]		curveName		A curve handle
*  @return			TRUE on success
*/
bool aqCurveObjectDelete( const std::string& curveName )
{
	AQ_API_START

	return validation::tryAqCurveObjectDelete( curveName );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectDeleteAll
*  @return			The number of curves removed
*/
int aqCurveObjectDeleteAll()
{
	AQ_API_START

	return validation::tryAqCurveObjectDeleteAll();

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectSave
*  @param [in]		aqObjCurveName		A curve handle
*  @param [in]		fileNameToWriteTo	Full path to write the curve to
*  @return			A status string
*/
std::string aqCurveObjectSave( const std::string& aqObjCurveName, const std::string& fileNameToWriteTo )
{
	AQ_API_START

	return validation::tryAqCurveObjectSave( aqObjCurveName, fileNameToWriteTo );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectLoad
*  @param [in]		fileName		Full path to the curve file
*  @return			A status string
*/
std::string aqCurveObjectLoad( const std::string& fileName )
{
	AQ_API_START

	return validation::tryAqCurveObjectLoad( fileName ).second;

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve market data object ("ObjectData" LVB family)
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectDataCreate: build curve market data from raw
*					swap/FRA/futures/central-bank/FX static data blocks.
*  @param [in]		mdcName					Name for the market-data object
*  @param [in]		currency				Currency
*  @param [in]		tenorString				Swap tenors to include
*  @param [in]		swapType				Swap instrument type (optional, may be blank)
*  @param [in]		swapStringBlock			Swap rates block (optional, may be empty)
*  @param [in]		toTenorString			FRA/futures end tenors (optional, may be blank)
*  @param [in]		fraStringBlock			FRA rates block (optional, may be empty)
*  @param [in]		irFuturesStringBlock	IR futures rates block (optional, may be empty)
*  @param [in]		centralBankTypeString	Central bank meeting type (optional, may be blank)
*  @param [in]		centralBankStringBlock	Central bank meeting dates block (optional, may be empty)
*  @param [in]		fxStringBlock			FX rates block (optional, may be empty)
*  @param [in]		unitCurrency			FX unit currency (optional, may be blank)
*  @param [in]		isInvertedFX			TRUE if the FX quote is inverted
*  @return			The name of the market-data object on the cache
*/
std::string aqCurveObjectDataCreate( const std::string& mdcName,
                                      const std::string& currency,
                                      const std::string& tenorString,
                                      const std::string& swapType,
                                      const SWIG_STRINGMATRIX& swapStringBlock,
                                      const std::string& toTenorString,
                                      const SWIG_STRINGMATRIX& fraStringBlock,
                                      const SWIG_STRINGMATRIX& irFuturesStringBlock,
                                      const std::string& centralBankTypeString,
                                      const SWIG_STRINGMATRIX& centralBankStringBlock,
                                      const SWIG_STRINGMATRIX& fxStringBlock,
                                      const std::string& unitCurrency,
                                      const bool isInvertedFX )
{
	AQ_API_START

	AQLStringMatrix tmp_swapStringBlock;
	swig::buildStringMatrix( tmp_swapStringBlock, swapStringBlock );

	AQLStringMatrix tmp_fraStringBlock;
	swig::buildStringMatrix( tmp_fraStringBlock, fraStringBlock );

	AQLStringMatrix tmp_irFuturesStringBlock;
	swig::buildStringMatrix( tmp_irFuturesStringBlock, irFuturesStringBlock );

	AQLStringMatrix tmp_centralBankStringBlock;
	swig::buildStringMatrix( tmp_centralBankStringBlock, centralBankStringBlock );

	AQLStringMatrix tmp_fxStringBlock;
	swig::buildStringMatrix( tmp_fxStringBlock, fxStringBlock );

	return validation::tryAqCurveObjectDataCreate( mdcName, currency, tenorString,
		swapType, tmp_swapStringBlock, toTenorString, tmp_fraStringBlock,
		tmp_irFuturesStringBlock, centralBankTypeString, tmp_centralBankStringBlock,
		tmp_fxStringBlock, unitCurrency, isInvertedFX );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectDataDisplay
*  @param [in]		mdcName		A market-data object handle built via aqCurveObjectDataCreate
*  @return			The market-data object's inputs, as a string matrix
*/
SWIG_STRINGMATRIX aqCurveObjectDataDisplay( const std::string& mdcName )
{
	AQ_API_START

	etrading::VariantMatrix results = validation::tryAqCurveObjectDataDisplay( mdcName );

	return swig::fromVariantMatrixToMatrixOfString( results );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve object conventions / dual bootstrap / engine calibration
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectDisplayConventions
*  @param [in]		objectName		A curve handle
*  @param [in]		propertyKey		The convention block to display
*  @return			The curve object's resolved conventions, as a string matrix
*/
SWIG_STRINGMATRIX aqCurveObjectDisplayConventions( const std::string& objectName, const std::string& propertyKey )
{
	AQ_API_START

	etrading::VariantMatrix results = validation::tryAqCurveObjectDisplayConventions( objectName, propertyKey );

	return swig::fromVariantMatrixToMatrixOfString( results );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectDualBootstrap: dual-bootstrap OIS and swap curves together
*  @param [in]		objectName				Name for the dual-bootstrapped object
*  @param [in]		curveCollection			Curve collection the calibrated curves belong to
*  @param [in]		swapCurveGeneratorName	A curve-generator handle defining the swap curve's conventions
*  @param [in]		oisCurveGeneratorName	A curve-generator handle defining the OIS curve's conventions
*  @param [in]		aqObjSwapMarketObj		A curve-market-data handle for the swap curve
*  @param [in]		aqObjOISMarketObj		A curve-market-data handle for the OIS curve
*  @param [in]		commonParams			Parameters common across both curves
*  @return			The resulting curve index names, as a key/value string matrix
*/
SWIG_STRINGMATRIX aqCurveObjectDualBootstrap( const std::string& objectName,
                                               const std::string& curveCollection,
                                               const std::string& swapCurveGeneratorName,
                                               const std::string& oisCurveGeneratorName,
                                               const std::string& aqObjSwapMarketObj,
                                               const std::string& aqObjOISMarketObj,
                                               const SWIG_STRINGMATRIX& commonParams )
{
	AQ_API_START

	AQLStringMatrix tmp_commonParams;
	swig::buildStringMatrix( tmp_commonParams, commonParams );

	const std::map<std::string, std::string> curveIndexes = validation::tryAqCurveObjectDualBootstrap(
		objectName, curveCollection, swapCurveGeneratorName, oisCurveGeneratorName,
		aqObjSwapMarketObj, aqObjOISMarketObj, tmp_commonParams );

	AQLStringMatrix result;
	for ( const auto& entry : curveIndexes )
	{
		std::vector<AQLString> row;
		row.push_back( AQLString( entry.first.c_str() ) );
		row.push_back( AQLString( entry.second.c_str() ) );
		result.push_back( row );
	}

	return swig::fromStringMatrixToMatrixOfString( result );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectCalibrateHedge: calibrate a hedge-curve pairing
*					(OIS discounting + Libor forwarding).
*  @param [in]		oisCurveObjectName		Name for the OIS discount curve
*  @param [in]		swapCurveObjectName		Name for the swap forward curve
*  @param [in]		pricingCurveCollection	Curve collection to price the hedge instruments off
*  @param [in]		hedgeCurveCollection	Curve collection the hedge curves are stored under
*  @param [in]		oisCurveGeneratorName	A curve-generator handle for the OIS curve
*  @param [in]		oisCurveMarketDataName	A curve-market-data handle for the OIS curve
*  @param [in]		swapCurveGeneratorName	A curve-generator handle for the swap curve
*  @param [in]		swapCurveMarketDataName	A curve-market-data handle for the swap curve
*  @param [in]		swapGeneratorName		A swap-generator handle for the hedge instruments
*  @return			The hedge curve names/indices, as a key/value string matrix
*/
SWIG_STRINGMATRIX aqCurveObjectCalibrateHedge( const std::string& oisCurveObjectName,
                                                const std::string& swapCurveObjectName,
                                                const std::string& pricingCurveCollection,
                                                const std::string& hedgeCurveCollection,
                                                const std::string& oisCurveGeneratorName,
                                                const std::string& oisCurveMarketDataName,
                                                const std::string& swapCurveGeneratorName,
                                                const std::string& swapCurveMarketDataName,
                                                const std::string& swapGeneratorName )
{
	AQ_API_START

	const etrading::HedgeCurveInfo info = validation::tryAqCurveObjectCalibrateHedge(
		oisCurveObjectName, swapCurveObjectName, pricingCurveCollection, hedgeCurveCollection,
		oisCurveGeneratorName, oisCurveMarketDataName, swapCurveGeneratorName,
		swapCurveMarketDataName, swapGeneratorName );

	AQLStringMatrix result;
	auto row = [&result]( const char* key, const std::string& value )
	{
		std::vector<AQLString> r;
		r.push_back( AQLString( key ) );
		r.push_back( AQLString( value.c_str() ) );
		result.push_back( r );
	};
	row( "OisCurveName",  info.oisCurveName );
	row( "OisCurveIndex", info.oisCurveIndex );
	row( "SwapCurveName", info.swapCurveName );
	row( "SwapCurveIndex", info.swapCurveIndex );

	return swig::fromStringMatrixToMatrixOfString( result );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectEngineCalibrate: calibrate a global yield curve
*					engine, producing several synchronous curves at once.
*  @param [in]		engineObjectName	Name for the curve engine object
*  @param [in]		curveCollection		Curve collection the calibrated curves belong to
*  @param [in]		engineSettings		Engine-level parameters, as a label/value block
*  @param [in]		curveGeneratorNames	Curve-generator handles, one per curve
*  @param [in]		marketDataObjects	Curve-market-data handles, aligned with curveGeneratorNames
*  @return			The resulting curve index names
*/
std::vector<std::string> aqCurveObjectEngineCalibrate( const std::string& engineObjectName,
                                                         const std::string& curveCollection,
                                                         const SWIG_STRINGMATRIX& engineSettings,
                                                         const std::vector<std::string>& curveGeneratorNames,
                                                         const std::vector<std::string>& marketDataObjects )
{
	AQ_API_START

	AQLStringMatrix tmp_engineSettings;
	swig::buildStringMatrix( tmp_engineSettings, engineSettings );

	const AQLStringVector curveIndexNames = validation::tryAqCurveObjectEngineCalibrate(
		engineObjectName, curveCollection, tmp_engineSettings, curveGeneratorNames, marketDataObjects );

	std::vector<std::string> result;
	result.reserve( curveIndexNames.size() );
	for ( const AQLString& indexName : curveIndexNames )
	{
		result.push_back( indexName.getCString() );
	}

	return result;

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve object one-shot creation from raw conventions/rates
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectCreateBasis
*/
std::string aqCurveObjectCreateBasis( const std::string& aqObjCurveName,
                                       const std::string& curveCollection,
                                       const std::string& staticDataTable,
                                       const std::string& curveIndex,
                                       const SWIG_STRINGMATRIX& curveConv,
                                       const SWIG_STRINGMATRIX& basisConv,
                                       const SWIG_STRINGMATRIX& basisRates,
                                       const SWIG_STRINGMATRIX& fxFwdConv,
                                       const SWIG_STRINGMATRIX& fxFwdRates,
                                       const SWIG_STRINGMATRIX& spotFxRates )
{
	AQ_API_START

	AQLStringMatrix tmp_curveConv;     swig::buildStringMatrix( tmp_curveConv, curveConv );
	AQLStringMatrix tmp_basisConv;     swig::buildStringMatrix( tmp_basisConv, basisConv );
	AQLStringMatrix tmp_basisRates;    swig::buildStringMatrix( tmp_basisRates, basisRates );
	AQLStringMatrix tmp_fxFwdConv;     swig::buildStringMatrix( tmp_fxFwdConv, fxFwdConv );
	AQLStringMatrix tmp_fxFwdRates;    swig::buildStringMatrix( tmp_fxFwdRates, fxFwdRates );
	AQLStringMatrix tmp_spotFxRates;   swig::buildStringMatrix( tmp_spotFxRates, spotFxRates );

	return std::string( validation::tryAqCurveObjectCreateBasis( aqObjCurveName,
		curveCollection.c_str(), staticDataTable.c_str(), curveIndex.c_str(),
		tmp_curveConv, tmp_basisConv, tmp_basisRates, tmp_fxFwdConv, tmp_fxFwdRates, tmp_spotFxRates ).getCString() );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectCreateFXForwards
*/
std::string aqCurveObjectCreateFXForwards( const std::string& aqObjCurveName,
                                            const std::string& curveCollection,
                                            const std::string& staticDataTable,
                                            const std::string& curveIndex,
                                            const SWIG_STRINGMATRIX& curveConv,
                                            const SWIG_STRINGMATRIX& fxFwdConv )
{
	AQ_API_START

	AQLStringMatrix tmp_curveConv;  swig::buildStringMatrix( tmp_curveConv, curveConv );
	AQLStringMatrix tmp_fxFwdConv;  swig::buildStringMatrix( tmp_fxFwdConv, fxFwdConv );

	return std::string( validation::tryAqCurveObjectCreateFXForwards( aqObjCurveName,
		curveCollection.c_str(), staticDataTable.c_str(), curveIndex.c_str(),
		tmp_curveConv, tmp_fxFwdConv ).getCString() );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectCreateOIS
*/
std::string aqCurveObjectCreateOIS( const std::string& aqObjCurveName,
                                     const std::string& curveCollection,
                                     const std::string& staticDataTable,
                                     const std::string& curveIndex,
                                     const SWIG_STRINGMATRIX& curveConv,
                                     const SWIG_STRINGMATRIX& oisConv,
                                     const SWIG_STRINGMATRIX& oisRates,
                                     const SWIG_STRINGMATRIX& oisHistoricalRates,
                                     const SWIG_STRINGMATRIX& liborOisBasisConv,
                                     const SWIG_STRINGMATRIX& liborOisBasisRates,
                                     const SWIG_STRINGMATRIX& swapConv,
                                     const SWIG_STRINGMATRIX& swapRates )
{
	AQ_API_START

	AQLStringMatrix tmp_curveConv;           swig::buildStringMatrix( tmp_curveConv, curveConv );
	AQLStringMatrix tmp_oisConv;              swig::buildStringMatrix( tmp_oisConv, oisConv );
	AQLStringMatrix tmp_oisRates;              swig::buildStringMatrix( tmp_oisRates, oisRates );
	AQLStringMatrix tmp_oisHistoricalRates;    swig::buildStringMatrix( tmp_oisHistoricalRates, oisHistoricalRates );
	AQLStringMatrix tmp_liborOisBasisConv;     swig::buildStringMatrix( tmp_liborOisBasisConv, liborOisBasisConv );
	AQLStringMatrix tmp_liborOisBasisRates;    swig::buildStringMatrix( tmp_liborOisBasisRates, liborOisBasisRates );
	AQLStringMatrix tmp_swapConv;              swig::buildStringMatrix( tmp_swapConv, swapConv );
	AQLStringMatrix tmp_swapRates;              swig::buildStringMatrix( tmp_swapRates, swapRates );

	return std::string( validation::tryAqCurveObjectCreateOIS( aqObjCurveName,
		curveCollection.c_str(), staticDataTable.c_str(), curveIndex.c_str(),
		tmp_curveConv, tmp_oisConv, tmp_oisRates, tmp_oisHistoricalRates,
		tmp_liborOisBasisConv, tmp_liborOisBasisRates, tmp_swapConv, tmp_swapRates ).getCString() );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectCreateSwap
*/
std::string aqCurveObjectCreateSwap( const std::string& aqObjCurveName,
                                      const std::string& curveCollection,
                                      const std::string& staticDataTable,
                                      const std::string& curveIndex,
                                      const SWIG_STRINGMATRIX& curveConv,
                                      const SWIG_STRINGMATRIX& moneyMarketConv,
                                      const SWIG_STRINGMATRIX& liborConv,
                                      const SWIG_STRINGMATRIX& liborRates,
                                      const SWIG_STRINGMATRIX& swapConv,
                                      const SWIG_STRINGMATRIX& swapRates,
                                      const SWIG_STRINGMATRIX& fraConv,
                                      const SWIG_STRINGMATRIX& fra3mRates,
                                      const SWIG_STRINGMATRIX& fra6mRates,
                                      const SWIG_STRINGMATRIX& futureConv,
                                      const SWIG_STRINGMATRIX& futureRates,
                                      const SWIG_STRINGMATRIX& convexityAdjConv,
                                      const SWIG_STRINGMATRIX& convexityAdjRates )
{
	AQ_API_START

	AQLStringMatrix tmp_curveConv;         swig::buildStringMatrix( tmp_curveConv, curveConv );
	AQLStringMatrix tmp_moneyMarketConv;    swig::buildStringMatrix( tmp_moneyMarketConv, moneyMarketConv );
	AQLStringMatrix tmp_liborConv;          swig::buildStringMatrix( tmp_liborConv, liborConv );
	AQLStringMatrix tmp_liborRates;         swig::buildStringMatrix( tmp_liborRates, liborRates );
	AQLStringMatrix tmp_swapConv;           swig::buildStringMatrix( tmp_swapConv, swapConv );
	AQLStringMatrix tmp_swapRates;          swig::buildStringMatrix( tmp_swapRates, swapRates );
	AQLStringMatrix tmp_fraConv;            swig::buildStringMatrix( tmp_fraConv, fraConv );
	AQLStringMatrix tmp_fra3mRates;         swig::buildStringMatrix( tmp_fra3mRates, fra3mRates );
	AQLStringMatrix tmp_fra6mRates;         swig::buildStringMatrix( tmp_fra6mRates, fra6mRates );
	AQLStringMatrix tmp_futureConv;         swig::buildStringMatrix( tmp_futureConv, futureConv );
	AQLStringMatrix tmp_futureRates;        swig::buildStringMatrix( tmp_futureRates, futureRates );
	AQLStringMatrix tmp_convexityAdjConv;   swig::buildStringMatrix( tmp_convexityAdjConv, convexityAdjConv );
	AQLStringMatrix tmp_convexityAdjRates;  swig::buildStringMatrix( tmp_convexityAdjRates, convexityAdjRates );

	return std::string( validation::tryAqCurveObjectCreateSwap( aqObjCurveName,
		curveCollection.c_str(), staticDataTable.c_str(), curveIndex.c_str(),
		tmp_curveConv, tmp_moneyMarketConv, tmp_liborConv, tmp_liborRates,
		tmp_swapConv, tmp_swapRates, tmp_fraConv, tmp_fra3mRates, tmp_fra6mRates,
		tmp_futureConv, tmp_futureRates, tmp_convexityAdjConv, tmp_convexityAdjRates ).getCString() );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve object discount factors / forward rates (handle-based)
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectDiscountFactors
*/
std::vector<double> aqCurveObjectDiscountFactors( const std::string& aqObjCurveName, const std::vector<std::string>& paymentDates )
{
	AQ_API_START

	std::vector<boost::gregorian::date> tmp_paymentDates;
	swig::buildGregorianDateVector( tmp_paymentDates, paymentDates );

	return validation::tryAqCurveObjectDiscountFactors( aqObjCurveName, tmp_paymentDates );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectDiscountFactorsForwardStarting
*/
std::vector<double> aqCurveObjectDiscountFactorsForwardStarting( const std::string& aqObjCurveName,
                                                                   const std::vector<std::string>& fromDates,
                                                                   const std::vector<std::string>& toDates )
{
	AQ_API_START

	std::vector<boost::gregorian::date> tmp_fromDates;
	swig::buildGregorianDateVector( tmp_fromDates, fromDates );
	std::vector<boost::gregorian::date> tmp_toDates;
	swig::buildGregorianDateVector( tmp_toDates, toDates );

	return validation::tryAqCurveObjectDiscountFactorsForwardStarting( aqObjCurveName, tmp_fromDates, tmp_toDates );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectDiscountFactorsForwardStartingFromTenors
*/
std::vector<double> aqCurveObjectDiscountFactorsForwardStartingFromTenors( const std::string& aqObjCurveName,
                                                                             const std::vector<std::string>& fromDates,
                                                                             const std::vector<std::string>& tenors,
                                                                             const std::string& businessDayAdj,
                                                                             const std::string& calendar )
{
	AQ_API_START

	std::vector<boost::gregorian::date> tmp_fromDates;
	swig::buildGregorianDateVector( tmp_fromDates, fromDates );

	return validation::tryAqCurveObjectDiscountFactorsForwardStartingFromTenors(
		aqObjCurveName, tmp_fromDates, tenors, businessDayAdj, calendar );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectDiscountFactorsForwardStartingFromYearFractions
*/
std::vector<double> aqCurveObjectDiscountFactorsForwardStartingFromYearFractions( const std::string& aqObjCurveName,
                                                                                    const std::vector<std::string>& fromDates,
                                                                                    const std::vector<double>& yearFractions,
                                                                                    const std::string& dayCount )
{
	AQ_API_START

	std::vector<boost::gregorian::date> tmp_fromDates;
	swig::buildGregorianDateVector( tmp_fromDates, fromDates );

	return validation::tryAqCurveObjectDiscountFactorsForwardStartingFromYearFractions(
		aqObjCurveName, tmp_fromDates, yearFractions, dayCount.c_str() );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectDiscountFactorsFromTenors
*/
std::vector<double> aqCurveObjectDiscountFactorsFromTenors( const std::string& aqObjCurveName,
                                                              const std::vector<std::string>& tenors,
                                                              const std::string& businessDayAdj,
                                                              const std::string& calendar )
{
	AQ_API_START

	return validation::tryAqCurveObjectDiscountFactorsFromTenors( aqObjCurveName, tenors, businessDayAdj, calendar );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectDiscountFactorsFromYearFractions
*/
std::vector<double> aqCurveObjectDiscountFactorsFromYearFractions( const std::string& aqObjCurveName,
                                                                     const std::vector<double>& yearFractions,
                                                                     const std::string& dayCount )
{
	AQ_API_START

	return validation::tryAqCurveObjectDiscountFactorsFromYearFractions( aqObjCurveName, yearFractions, dayCount.c_str() );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectDiscountFactorsTable
*/
SWIG_STRINGMATRIX aqCurveObjectDiscountFactorsTable( const std::string& curveCollection,
                                                       const std::vector<std::string>& curveIndices,
                                                       const std::string& startDate,
                                                       const std::string& maturity,
                                                       const std::string& businessDayAdjust,
                                                       const std::string& calendar,
                                                       const std::string& rollConvention,
                                                       const std::string& frequency )
{
	AQ_API_START

	AQLStringVector tmp_curveIndices;
	swig::buildStringVector( tmp_curveIndices, curveIndices );

	DateVector   paymentDates;
	DoubleMatrix discountFactors;

	validation::tryAqCurveObjectDiscountFactorsTable( paymentDates, discountFactors,
		curveCollection.c_str(), tmp_curveIndices, startDate.c_str(), maturity.c_str(),
		businessDayAdjust.c_str(), calendar.c_str(), rollConvention.c_str(), frequency.c_str() );

	etrading::VariantMatrix result;
	etrading::VariantVector header;
	header.push_back( etrading::Variant( "Date" ) );
	for ( const AQLString& index : tmp_curveIndices )
	{
		header.push_back( etrading::Variant( index.getCString() ) );
	}
	result.push_back( header );

	std::vector<std::string> paymentDateStrings;
	swig::buildStringVectorFromDateVector( paymentDateStrings, paymentDates );

	for ( std::size_t i = 0; i < paymentDateStrings.size(); ++i )
	{
		etrading::VariantVector row;
		row.push_back( etrading::Variant( paymentDateStrings[i] ) );
		if ( i < discountFactors.size() )
		{
			for ( double df : discountFactors[i] )
			{
				row.push_back( etrading::Variant( df ) );
			}
		}
		result.push_back( row );
	}

	return swig::fromVariantMatrixToMatrixOfString( result );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectForwardRates
*/
std::vector<double> aqCurveObjectForwardRates( const std::string& aqObjCurveName, const std::vector<std::string>& fixingDates )
{
	AQ_API_START

	std::vector<boost::gregorian::date> tmp_fixingDates;
	swig::buildGregorianDateVector( tmp_fixingDates, fixingDates );

	return validation::tryAqCurveObjectForwardRates( aqObjCurveName, tmp_fixingDates );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectForwardRatesFromForwardDates
*/
std::vector<double> aqCurveObjectForwardRatesFromForwardDates( const std::string& aqObjCurveName,
                                                                  const std::vector<std::string>& fromDates,
                                                                  const std::vector<std::string>& toDates )
{
	AQ_API_START

	std::vector<boost::gregorian::date> tmp_fromDates;
	swig::buildGregorianDateVector( tmp_fromDates, fromDates );
	std::vector<boost::gregorian::date> tmp_toDates;
	swig::buildGregorianDateVector( tmp_toDates, toDates );

	return validation::tryAqCurveObjectForwardRatesFromForwardDates( aqObjCurveName, tmp_fromDates, tmp_toDates );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectForwardRatesFromYearFraction
*/
std::vector<double> aqCurveObjectForwardRatesFromYearFraction( const std::string& aqObjCurveName,
                                                                  const std::vector<std::string>& fromDates,
                                                                  double yearFraction,
                                                                  const std::string& dayCount )
{
	AQ_API_START

	std::vector<boost::gregorian::date> tmp_fromDates;
	swig::buildGregorianDateVector( tmp_fromDates, fromDates );

	return validation::tryAqCurveObjectForwardRatesFromYearFraction( aqObjCurveName, tmp_fromDates, yearFraction, dayCount );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectForwardRatesTable
*/
SWIG_STRINGMATRIX aqCurveObjectForwardRatesTable( const std::string& curveCollection,
                                                    const std::vector<std::string>& curveIndices,
                                                    const std::string& startDate,
                                                    const std::string& maturity,
                                                    const std::string& businessDayAdjust,
                                                    const std::string& calendar,
                                                    const std::string& rollConvention,
                                                    const std::string& frequency,
                                                    const std::vector<std::string>& fwdInterps )
{
	AQ_API_START

	AQLStringVector tmp_curveIndices;
	swig::buildStringVector( tmp_curveIndices, curveIndices );
	AQLStringVector tmp_fwdInterps;
	swig::buildStringVector( tmp_fwdInterps, fwdInterps );

	DateVector   fixingDates;
	DoubleMatrix forwardRates;

	validation::tryAqCurveObjectForwardRatesTable( fixingDates, forwardRates,
		curveCollection.c_str(), tmp_curveIndices, startDate.c_str(), maturity.c_str(),
		businessDayAdjust.c_str(), calendar.c_str(), rollConvention.c_str(), frequency.c_str(), tmp_fwdInterps );

	etrading::VariantMatrix result;
	etrading::VariantVector header;
	header.push_back( etrading::Variant( "Date" ) );
	for ( const AQLString& index : tmp_curveIndices )
	{
		header.push_back( etrading::Variant( index.getCString() ) );
	}
	result.push_back( header );

	std::vector<std::string> fixingDateStrings;
	swig::buildStringVectorFromDateVector( fixingDateStrings, fixingDates );

	for ( std::size_t i = 0; i < fixingDateStrings.size(); ++i )
	{
		etrading::VariantVector row;
		row.push_back( etrading::Variant( fixingDateStrings[i] ) );
		if ( i < forwardRates.size() )
		{
			for ( double fwd : forwardRates[i] )
			{
				row.push_back( etrading::Variant( fwd ) );
			}
		}
		result.push_back( row );
	}

	return swig::fromVariantMatrixToMatrixOfString( result );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve object / market data bumping
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectBumpAll
*/
std::string aqCurveObjectBumpAll( const std::string& objectName, double bumpSize, bool onlyBumpOutrightInstruments )
{
	AQ_API_START

	return validation::tryAqCurveObjectBumpAll( objectName, bumpSize, onlyBumpOutrightInstruments );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectBumpInstrument
*/
std::string aqCurveObjectBumpInstrument( const std::string& objectName, const std::string& marketDataType, double bumpSize, bool clearExistingBumps )
{
	AQ_API_START

	return validation::tryAqCurveObjectBumpInstrument( objectName, marketDataType, bumpSize, clearExistingBumps );

	AQ_API_END
}

/* @brief			swig interface for aqCurveMarketDataBumpAll
*/
std::string aqCurveMarketDataBumpAll( const std::string& objectName, double bumpSize, bool onlyBumpOutrightInstruments )
{
	AQ_API_START

	return validation::tryAqCurveMarketDataBumpAll( objectName, bumpSize, onlyBumpOutrightInstruments );

	AQ_API_END
}

/* @brief			swig interface for aqCurveMarketDataBumpClear
*/
std::string aqCurveMarketDataBumpClear( const std::string& objectName )
{
	AQ_API_START

	return validation::tryAqCurveMarketDataBumpClear( objectName );

	AQ_API_END
}

/* @brief			swig interface for aqCurveMarketDataBumpInstrument
*/
std::string aqCurveMarketDataBumpInstrument( const std::string& objectName, const std::string& marketDataType, double bumpSize, bool clearExistingBumps )
{
	AQ_API_START

	return validation::tryAqCurveMarketDataBumpInstrument( objectName, marketDataType, bumpSize, clearExistingBumps );

	AQ_API_END
}

/* @brief			swig interface for aqCurveMarketDataColumn
*/
SWIG_STRINGMATRIX aqCurveMarketDataColumn( const std::string& curveObjectName, const std::string& marketDataKey, int columnNumber )
{
	AQ_API_START

	const etrading::VariantVector column = validation::tryAqCurveMarketDataColumn( curveObjectName, marketDataKey, columnNumber );

	etrading::VariantMatrix asMatrix;
	for ( const etrading::Variant& cell : column )
	{
		etrading::VariantVector row;
		row.push_back( cell );
		asMatrix.push_back( row );
	}

	return swig::fromVariantMatrixToMatrixOfString( asMatrix );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Legacy stateless curveCollection+curveIndex surface
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveDelete
*/
std::string aqCurveDelete( const std::string& curveCollection, const std::string& curveIndex )
{
	AQ_API_START

	return std::string( validation::tryAqCurveDelete( curveCollection.c_str(), curveIndex.c_str() ).getCString() );

	AQ_API_END
}

/* @brief			swig interface for aqCurveDiscountFactorsDisplay
*/
SWIG_STRINGMATRIX aqCurveDiscountFactorsDisplay( const std::string& curveCollection, const std::string& curveIndex )
{
	AQ_API_START

	const ::DiscountFactorTable table = validation::tryAqCurveDiscountFactorsDisplay( curveCollection.c_str(), curveIndex.c_str() );

	etrading::VariantMatrix result;
	etrading::VariantVector header;
	header.push_back( etrading::Variant( "Term" ) );
	header.push_back( etrading::Variant( "PaymentDate" ) );
	header.push_back( etrading::Variant( "DiscountFactor" ) );
	result.push_back( header );

	std::vector<std::string> paymentDateStrings;
	swig::buildStringVectorFromDateVector( paymentDateStrings, table.paymentDates_ );

	for ( std::size_t i = 0; i < paymentDateStrings.size(); ++i )
	{
		etrading::VariantVector row;
		row.push_back( etrading::Variant( i < table.terms_.size() ? table.terms_[i] : 0.0 ) );
		row.push_back( etrading::Variant( paymentDateStrings[i] ) );
		row.push_back( etrading::Variant( i < table.discountFactors_.size() ? table.discountFactors_[i] : 0.0 ) );
		result.push_back( row );
	}

	return swig::fromVariantMatrixToMatrixOfString( result );

	AQ_API_END
}

/* @brief			swig interface for aqCurveDiscountFactorsOverride
*/
std::string aqCurveDiscountFactorsOverride( const std::string& curveCollection,
                                             const std::string& curveIndex,
                                             const std::vector<std::string>& paymentDates,
                                             const std::vector<double>& discountFactors,
                                             bool setCorrespondingForwards )
{
	AQ_API_START

	DateVector tmp_paymentDates;
	swig::buildDateVector( tmp_paymentDates, paymentDates );

	return std::string( validation::tryAqCurveDiscountFactorsOverride( curveCollection.c_str(), curveIndex.c_str(),
		tmp_paymentDates, discountFactors, setCorrespondingForwards ).getCString() );

	AQ_API_END
}

/* @brief			swig interface for aqCurveDiscountFactorsSetToOne
*/
std::string aqCurveDiscountFactorsSetToOne( const std::string& curveCollection, const std::string& curveIndex )
{
	AQ_API_START

	return std::string( validation::tryAqCurveDiscountFactorsSetToOne( curveCollection.c_str(), curveIndex.c_str() ).getCString() );

	AQ_API_END
}

/* @brief			swig interface for aqCurveForwardRatesOverride
*/
std::string aqCurveForwardRatesOverride( const std::string& curveCollection,
                                          const std::string& curveIndex,
                                          const std::vector<std::string>& fixingDates,
                                          const std::vector<double>& forwardRates,
                                          bool setCorrespondingDiscountFactors )
{
	AQ_API_START

	DateVector tmp_fixingDates;
	swig::buildDateVector( tmp_fixingDates, fixingDates );

	return std::string( validation::tryAqCurveForwardRatesOverride( curveCollection.c_str(), curveIndex.c_str(),
		tmp_fixingDates, forwardRates, setCorrespondingDiscountFactors ).getCString() );

	AQ_API_END
}

/* @brief			swig interface for aqCurveForwardRatesFromForwardDatesFromObject
*/
std::vector<double> aqCurveForwardRatesFromForwardDatesFromObject( const std::vector<std::string>& fromDates,
                                                                     const std::vector<std::string>& toDates,
                                                                     const std::string& curveCollectionOrHandle,
                                                                     const std::string& curveIndex,
                                                                     const std::string& fwdInter,
                                                                     const std::string& businessDayAdjust )
{
	AQ_API_START

	DateVector tmp_fromDates;
	swig::buildDateVector( tmp_fromDates, fromDates );
	DateVector tmp_toDates;
	swig::buildDateVector( tmp_toDates, toDates );

	return validation::tryAqCurveForwardRatesFromForwardDatesFromObject( tmp_fromDates, tmp_toDates,
		curveCollectionOrHandle.c_str(), curveIndex.c_str(), fwdInter, businessDayAdjust );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve Jacobian risk display
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectEngineJacobianDisplay
*/
SWIG_STRINGMATRIX aqCurveObjectEngineJacobianDisplay( const std::string& curveEngineObject,
                                                        const std::string& curveCollection,
                                                        bool displayLabels,
                                                        bool displayInverseMatrix )
{
	AQ_API_START

	DoubleMatrix    matrix;
	AQLStringMatrix labelMatrix;

	validation::tryAqCurveObjectEngineJacobianDisplay( matrix, labelMatrix,
		curveEngineObject.c_str(), curveCollection.c_str(), displayLabels, displayInverseMatrix );

	if ( displayLabels )
	{
		return swig::fromStringMatrixToMatrixOfString( labelMatrix );
	}

	etrading::VariantMatrix result;
	for ( const DoubleVector& row : matrix )
	{
		etrading::VariantVector variantRow;
		for ( double value : row )
		{
			variantRow.push_back( etrading::Variant( value ) );
		}
		result.push_back( variantRow );
	}

	return swig::fromVariantMatrixToMatrixOfString( result );

	AQ_API_END
}

/* @brief			swig interface for aqCurveObjectJacobianDisplay
*/
SWIG_STRINGMATRIX aqCurveObjectJacobianDisplay( const std::string& curveCollection, const std::string& curveName, bool displayInverseMatrix )
{
	AQ_API_START

	DoubleMatrix matrix;
	validation::tryAqCurveObjectJacobianDisplay( matrix, curveCollection.c_str(), curveName.c_str(), displayInverseMatrix );

	etrading::VariantMatrix result;
	for ( const DoubleVector& row : matrix )
	{
		etrading::VariantVector variantRow;
		for ( double value : row )
		{
			variantRow.push_back( etrading::Variant( value ) );
		}
		result.push_back( variantRow );
	}

	return swig::fromVariantMatrixToMatrixOfString( result );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve results / Jacobian risk store
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveResultsEnable
*/
std::string aqCurveResultsEnable( bool enable )
{
	AQ_API_START

	return validation::tryAqCurveResultsEnable( enable );

	AQ_API_END
}

/* @brief			swig interface for aqCurveResultsIsEnabled
*/
std::string aqCurveResultsIsEnabled()
{
	AQ_API_START

	return validation::tryAqCurveResultsIsEnabled();

	AQ_API_END
}

// forwardAdjustments is arbitrary-shape; see the matching note in aqCurveObject.h.
#if (!defined(SWIG_R)) && (!defined(SWIGR))
/* @brief			swig interface for aqCurveResultsDiscountFactorsUpdate
*/
std::string aqCurveResultsDiscountFactorsUpdate( const SWIG_STRINGMATRIX& curveLVB,
                                                   const SWIG_STRINGMATRIX& parameterLVB,
                                                   const SWIG_STRINGMATRIX& discountFactorLVB,
                                                   const SWIG_STRINGMATRIX& forwardAdjustments )
{
	AQ_API_START

	AQLStringMatrix tmp_curveLVB;          swig::buildStringMatrix( tmp_curveLVB, curveLVB );
	AQLStringMatrix tmp_parameterLVB;      swig::buildStringMatrix( tmp_parameterLVB, parameterLVB );
	AQLStringMatrix tmp_discountFactorLVB; swig::buildStringMatrix( tmp_discountFactorLVB, discountFactorLVB );

	// StandardStringMatrix and (non-R) SWIG_STRINGMATRIX are both std::vector<std::vector<std::string>> -
	// no conversion required for the optional forwardAdjustments block, only an empty-check
	const StandardStringMatrix tmp_forwardAdjustments = forwardAdjustments.empty() ? StandardStringMatrix() : StandardStringMatrix( forwardAdjustments );

	return validation::tryAqCurveResultsDiscountFactorsUpdate( tmp_curveLVB, tmp_parameterLVB, tmp_discountFactorLVB, tmp_forwardAdjustments );

	AQ_API_END
}
#endif

/* @brief			swig interface for aqCurveResultsDiscountFactorsDisplay
*/
std::vector<double> aqCurveResultsDiscountFactorsDisplay( const std::string& curveCollection,
                                                             const std::string& curveIndex,
                                                             const std::vector<std::string>& paymentDates )
{
	AQ_API_START

	DateVector tmp_paymentDates;
	swig::buildDateVector( tmp_paymentDates, paymentDates );

	return validation::tryAqCurveResultsDiscountFactorsDisplay( curveCollection, curveIndex, tmp_paymentDates );

	AQ_API_END
}

/* @brief			swig interface for aqCurveResultsDelete
*/
std::string aqCurveResultsDelete( const std::string& curveCollection, const std::string& curveIndex )
{
	AQ_API_START

	return validation::tryAqCurveResultsDelete( curveCollection, curveIndex );

	AQ_API_END
}

/* @brief			swig interface for aqCurveResultsDeleteAll
*/
std::string aqCurveResultsDeleteAll()
{
	AQ_API_START

	return validation::tryAqCurveResultsDeleteAll();

	AQ_API_END
}

/* @brief			swig interface for aqCurveResultsForwardRatesDisplay
*/
std::vector<double> aqCurveResultsForwardRatesDisplay( const std::string& curveCollection,
                                                          const std::string& curveIndex,
                                                          const std::vector<std::string>& fixingDates,
                                                          bool isFwdInter,
                                                          const std::string& fixingBusinessDayAdj,
                                                          const std::string& fixingCalendar )
{
	AQ_API_START

	DateVector tmp_fixingDates;
	swig::buildDateVector( tmp_fixingDates, fixingDates );

	const etrading::BusinessDayAdjustmentEnum businessDayAdjEnum = fixingBusinessDayAdj.empty()
		? etrading::NONE_BUSINESS_DAY_ADJ
		: etrading::toBusinessDayAdjustmentEnum( fixingBusinessDayAdj );

	return validation::tryAqCurveResultsForwardRatesDisplay( curveCollection, curveIndex, tmp_fixingDates,
		isFwdInter, businessDayAdjEnum, fixingCalendar );

	AQ_API_END
}

/* @brief			swig interface for aqCurveResultsJacobianUpdate
*/
std::string aqCurveResultsJacobianUpdate( const SWIG_STRINGMATRIX& curveLVB,
                                            const SWIG_STRINGMATRIX& discountFactorParameterLVB,
                                            const SWIG_STRINGMATRIX& discountFactors,
                                            const SWIG_STRINGMATRIX& jacobianParameterLVB,
                                            const std::vector<bool>& outrightInstruments,
                                            const std::vector<double>& marketDataShiftSizeInPercent,
                                            const SWIG_STRINGMATRIX& jacobianMatrix )
{
	AQ_API_START

	AQLStringMatrix tmp_curveLVB;                   swig::buildStringMatrix( tmp_curveLVB, curveLVB );
	AQLStringMatrix tmp_discountFactorParameterLVB; swig::buildStringMatrix( tmp_discountFactorParameterLVB, discountFactorParameterLVB );
	AQLStringMatrix tmp_discountFactors;            swig::buildStringMatrix( tmp_discountFactors, discountFactors );
	AQLStringMatrix tmp_jacobianParameterLVB;       swig::buildStringMatrix( tmp_jacobianParameterLVB, jacobianParameterLVB );
	AQLStringMatrix tmp_jacobianMatrix;             swig::buildStringMatrix( tmp_jacobianMatrix, jacobianMatrix );

	return validation::tryAqCurveResultsJacobianUpdate( tmp_curveLVB, tmp_discountFactorParameterLVB, tmp_discountFactors,
		tmp_jacobianParameterLVB, outrightInstruments, marketDataShiftSizeInPercent, tmp_jacobianMatrix );

	AQ_API_END
}

/* @brief			swig interface for aqCurveResultsJacobianDisplay
*/
SWIG_STRINGMATRIX aqCurveResultsJacobianDisplay( const std::string& curveCollection, const std::string& curveIndex, const std::string& riskType )
{
	AQ_API_START

	const etrading::VariantMatrix result = validation::tryAqCurveResultsJacobianDisplay(
		curveCollection, curveIndex, etrading::toRiskTypeEnum( riskType ) );

	return swig::fromVariantMatrixToMatrixOfString( result );

	AQ_API_END
}

/* @brief			swig interface for aqCurveResultsJacobianDiscountFactorDelta
*/
SWIG_STRINGMATRIX aqCurveResultsJacobianDiscountFactorDelta( const std::string& curveCollection, const std::string& curveIndex )
{
	AQ_API_START

	const etrading::VariantMatrix result = validation::tryAqCurveResultsJacobianDiscountFactorDelta( curveCollection, curveIndex );

	return swig::fromVariantMatrixToMatrixOfString( result );

	AQ_API_END
}

/* @brief			swig interface for aqCurveResultsJacobianRiskTotals
*/
SWIG_STRINGMATRIX aqCurveResultsJacobianRiskTotals( const std::string& curveCollection,
                                                      const std::string& curveIndex,
                                                      const std::string& riskType,
                                                      bool useOutrightInstrumentsOnly )
{
	AQ_API_START

	const etrading::VariantMatrix result = validation::tryAqCurveResultsJacobianRiskTotals(
		curveCollection, curveIndex, etrading::toRiskTypeEnum( riskType ), useOutrightInstrumentsOnly );

	return swig::fromVariantMatrixToMatrixOfString( result );

	AQ_API_END
}

/* @brief			swig interface for aqCurveResultsJacobianImplyNewDiscountFactors
*/
SWIG_STRINGMATRIX aqCurveResultsJacobianImplyNewDiscountFactors( const std::string& curveCollection, const std::string& curveIndex )
{
	AQ_API_START

	const etrading::VariantMatrix result = validation::tryAqCurveResultsJacobianImplyNewDiscountFactors( curveCollection, curveIndex );

	return swig::fromVariantMatrixToMatrixOfString( result );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve groups
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveGroupCreate
*/
std::string aqCurveGroupCreate( const std::string& groupName, const std::vector<std::string>& curveHandles )
{
	AQ_API_START

	// StandardStringVector is std::vector<std::string> - no conversion required
	return validation::tryAqCurveGroupCreate( groupName, curveHandles );

	AQ_API_END
}

/* @brief			swig interface for aqCurveGroupCollectionName
*/
std::string aqCurveGroupCollectionName( const std::string& groupName )
{
	AQ_API_START

	return validation::tryAqCurveGroupCollectionName( groupName );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve terms / dates
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveTermsToDates
*/
std::vector<std::string> aqCurveTermsToDates( const std::string& curveCollection, const std::vector<double>& terms )
{
	AQ_API_START

	const DateVector paymentDates = validation::tryAqCurveTermsToDates( curveCollection, terms );

	std::vector<std::string> result;
	swig::buildStringVectorFromDateVector( result, paymentDates );

	return result;

	AQ_API_END
}

/* @brief			swig interface for aqCurveDatesToTerms
*/
std::vector<double> aqCurveDatesToTerms( const std::string& curveCollection, const std::vector<std::string>& paymentDates )
{
	AQ_API_START

	DateVector tmp_paymentDates;
	swig::buildDateVector( tmp_paymentDates, paymentDates );

	return validation::tryAqCurveDatesToTerms( curveCollection, tmp_paymentDates );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve compound rate with fixing table
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveCompoundRateWithFixingTable (vector form)
*/
std::vector<double> aqCurveCompoundRateWithFixingTable( const std::vector<std::string>& startDates,
                                                           const std::vector<std::string>& endDates,
                                                           const std::string& curveCollection,
                                                           const std::string& forecastCurveIndex,
                                                           const std::string& frequency,
                                                           double spread,
                                                           const std::string& stubType,
                                                           const std::string& rollDayInput,
                                                           const std::string& calendar,
                                                           const std::string& businessDayAdj,
                                                           const std::string& dayCount,
                                                           const std::string& interpolation,
                                                           const std::string& compoundType,
                                                           const std::string& firstStubDate,
                                                           const std::string& lastStubDate,
                                                           const std::string& fixingTableName,
                                                           bool annualized )
{
	AQ_API_START

	DateVector tmp_startDates;
	swig::buildDateVector( tmp_startDates, startDates );
	DateVector tmp_endDates;
	swig::buildDateVector( tmp_endDates, endDates );

	return validation::tryAqCurveCompoundRateWithFixingTable( tmp_startDates, tmp_endDates,
		curveCollection, forecastCurveIndex, frequency, spread, stubType, rollDayInput,
		calendar, businessDayAdj, dayCount, interpolation, compoundType,
		firstStubDate, lastStubDate, fixingTableName, annualized );

	AQ_API_END
}

/* @brief			swig interface for aqCurveCompoundRateWithFixingTable (single-date form)
*/
double aqCurveCompoundRateWithFixingTable( const std::string& startDate,
                                             const std::string& endDate,
                                             const std::string& curveCollection,
                                             const std::string& forecastCurveIndex,
                                             const std::string& frequency,
                                             double spread,
                                             const std::string& stubType,
                                             const std::string& rollDayInput,
                                             const std::string& calendar,
                                             const std::string& businessDayAdj,
                                             const std::string& dayCount,
                                             const std::string& interpolation,
                                             const std::string& compoundType,
                                             const std::string& firstStubDate,
                                             const std::string& lastStubDate,
                                             const std::string& fixingTableName,
                                             bool annualized )
{
	AQ_API_START

	AQLDate tmp_startDate( etrading::stringToDate( startDate ) );
	AQLDate tmp_endDate( etrading::stringToDate( endDate ) );

	return validation::tryAqCurveCompoundRateWithFixingTable( tmp_startDate, tmp_endDate,
		curveCollection, forecastCurveIndex, frequency, spread, stubType, rollDayInput,
		calendar, businessDayAdj, dayCount, interpolation, compoundType,
		firstStubDate, lastStubDate, fixingTableName, annualized );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve one-shot calibration - cheapest-to-deliver collateral curve
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveCalibrateCTD
*/
std::string aqCurveCalibrateCTD( const std::string& curveCollection,
                                   const std::string& curveName,
                                   const std::string& curveIndex,
                                   const SWIG_STRINGMATRIX& curveConv,
                                   const std::vector<std::string>& collateralCurves )
{
	AQ_API_START

	AQLStringMatrix tmp_curveConv;
	swig::buildStringMatrix( tmp_curveConv, curveConv );

	AQLStringVector tmp_collateralCurves;
	swig::buildStringVector( tmp_collateralCurves, collateralCurves );

	return std::string( validation::tryAqCurveCalibrateCTD( curveCollection, curveName, curveIndex,
		tmp_curveConv, tmp_collateralCurves ).getCString() );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve floating index frequency / EuroDollar convexity adjustment
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveFrequency
*/
std::string aqCurveFrequency( const std::string& curveCollection, const std::string& curveIndex )
{
	AQ_API_START

	return std::string( validation::tryAqCurveFrequency( curveCollection, curveIndex ).getCString() );

	AQ_API_END
}

/* @brief			swig interface for aqCurveEuroDollarConvexityAdjustment
*/
double aqCurveEuroDollarConvexityAdjustment( const std::string& curveAsOfDate,
                                               const std::string& futuresStartDate,
                                               const std::string& futuresEndDate,
                                               double meanReversion,
                                               double volatility )
{
	AQ_API_START

	AQLDate tmp_curveAsOfDate( etrading::stringToDate( curveAsOfDate ) );
	AQLDate tmp_futuresStartDate( etrading::stringToDate( futuresStartDate ) );
	AQLDate tmp_futuresEndDate( etrading::stringToDate( futuresEndDate ) );

	return validation::tryAqCurveEuroDollarConvexityAdjustment(
		tmp_curveAsOfDate, tmp_futuresStartDate, tmp_futuresEndDate, meanReversion, volatility );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Short-rate model checks (Hull-White, Vasicek)
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveHullWhiteForwardRates
*/
std::vector<double> aqCurveHullWhiteForwardRates( const std::vector<std::string>& fixingDates,
                                                     const std::string& curveCollection,
                                                     const std::string& curveIndex,
                                                     double sigma,
                                                     double alpha,
                                                     double rt,
                                                     const std::string& valuationDate )
{
	AQ_API_START

	DateVector tmp_fixingDates;
	swig::buildDateVector( tmp_fixingDates, fixingDates );

	const AQLDate tmp_valuationDate = valuationDate.empty() ? AQLDate() : AQLDate( etrading::stringToDate( valuationDate ) );

	return validation::tryAqCurveHullWhiteForwardRates( tmp_fixingDates, curveCollection, curveIndex,
		sigma, alpha, rt, tmp_valuationDate );

	AQ_API_END
}

/* @brief			swig interface for aqCurveVasicekChecking
*/
SWIG_STRINGMATRIX aqCurveVasicekChecking( const std::vector<std::string>& fixingDates,
                                            const std::vector<double>& targetForwardRates,
                                            const std::string& curveCollection,
                                            const std::string& curveIndex,
                                            double initialTheta,
                                            double initialSigma,
                                            double alpha,
                                            double rt,
                                            const std::string& valuationDate,
                                            bool showColumnHeaders )
{
	AQ_API_START

	DateVector tmp_fixingDates;
	swig::buildDateVector( tmp_fixingDates, fixingDates );

	const AQLDate tmp_valuationDate = valuationDate.empty() ? AQLDate() : AQLDate( etrading::stringToDate( valuationDate ) );

	const AnyTypeMatrix result = validation::tryAqCurveVasicekChecking( tmp_fixingDates, targetForwardRates,
		curveCollection, curveIndex, initialTheta, initialSigma, alpha, rt, tmp_valuationDate, showColumnHeaders );

	return swig::fromAnyTypeMatrixToMatrixOfString( result );

	AQ_API_END
}

/* @brief			swig interface for aqCurveVasicekForwardRates
*/
std::vector<double> aqCurveVasicekForwardRates( const std::vector<std::string>& fixingDates,
                                                   const std::string& curveCollection,
                                                   const std::string& curveIndex,
                                                   double theta,
                                                   double sigma,
                                                   double alpha,
                                                   double rt,
                                                   const std::string& valuationDate )
{
	AQ_API_START

	DateVector tmp_fixingDates;
	swig::buildDateVector( tmp_fixingDates, fixingDates );

	const AQLDate tmp_valuationDate = valuationDate.empty() ? AQLDate() : AQLDate( etrading::stringToDate( valuationDate ) );

	return validation::tryAqCurveVasicekForwardRates( tmp_fixingDates, curveCollection, curveIndex,
		theta, sigma, alpha, rt, tmp_valuationDate );

	AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Curve generator (a named sub-object - conventions)
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveGeneratorCreate
*/
std::string aqCurveGeneratorCreate( const std::string& objectName,
                                      const std::string& key1, const SWIG_STRINGMATRIX& value1,
                                      const std::string& key2, const SWIG_STRINGMATRIX& value2 )
{
	AQ_API_START

	std::vector<std::string>           propertyNames;
	std::vector<validation::TableInfo> infoBlocks;

	AQLStringMatrix tmp_value1;
	swig::buildStringMatrix( tmp_value1, value1 );
	propertyNames.push_back( key1 );
	infoBlocks.push_back( getTableInfoFromStringMatrix( tmp_value1 ) );

	if ( value2.size() > 0 )
	{
		AQLStringMatrix tmp_value2;
		swig::buildStringMatrix( tmp_value2, value2 );
		propertyNames.push_back( key2 );
		infoBlocks.push_back( getTableInfoFromStringMatrix( tmp_value2 ) );
	}

	const std::string storedName = validation::tryAqCurveGeneratorCreate( objectName, propertyNames, infoBlocks );

	return storedName;

	AQ_API_END
}

/* @brief			swig interface for aqCurveGeneratorDisplay
*/
SWIG_STRINGMATRIX aqCurveGeneratorDisplay( const std::string& objectName, const std::string& propertyName )
{
	AQ_API_START

	const etrading::VariantMatrix result = validation::tryAqCurveGeneratorDisplay( objectName, propertyName );

	return swig::fromVariantMatrixToMatrixOfString( result );

	AQ_API_END
}

/* @brief			swig interface for aqCurveGeneratorModify
*/
std::string aqCurveGeneratorModify( const std::string& newObjectName,
                                      const std::string& baseObjectName,
                                      const SWIG_STRINGMATRIX& modifiedValues )
{
	AQ_API_START

	const LabelValueBlock modifiedValuesLVB = swig::buildSingleLabelValueBlock( modifiedValues );

	const std::string storedName = validation::tryAqCurveGeneratorModify( newObjectName, baseObjectName, modifiedValuesLVB );

	return storedName;

	AQ_API_END
}

