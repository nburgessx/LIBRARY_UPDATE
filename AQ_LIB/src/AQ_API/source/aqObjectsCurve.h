#pragma once

#include <string>
#include <vector>
#include "SwigTypes.h"	// For SWIG_STRINGMATRIX which manages the AQLStringMatrix idiosyncrasies type for R API

/* @brief			swig interface for aqObjectsCurveDisplay
*  @param [in]		curveHandle		    AQO curve handle or object name
*  @return			StandardStringMatrix of AQO Curve static and market data inputs
*/
SWIG_STRINGMATRIX aqObjectsCurveDisplay(const std::string& curveHandle);

/* @brief			swig interface for aqObjectsCurveMarketDataDisplay
*  @param [in]		marketDataObjectName	AQO curve market data handle or object name
*  @param [in]		marketDataKey			The specific market data block of interest, for example "SWAPS"
*  @return			A string matrix containing the specified market data block.
*/
SWIG_STRINGMATRIX aqObjectsCurveMarketDataDisplay(const std::string& marketDataObjectName, const std::string marketDataKey );

/* @brief			swig interface for aqObjectsCurveDisplayMarketDataFromCurve
*  @param [in]		curveObjectName			AQO curve data handle or object name
*  @param [in]		marketDataKey			The specific market data block of interest, for example "SWAPS"
*  @return			A string matrix containing the specified market data block.
*/
SWIG_STRINGMATRIX aqObjectsCurveMarketDataDisplayFromCurve(const std::string& curveObjectName, const std::string marketDataKey );


/* @brief Builds a curve using a CurveGenerator object and a CurveMarketData object
* @param [in] lwoCurveGeneratorName     The name of the AQOCurveGenerator object to use
* @param [in] lwoCurveMarketDataName    The name of the AQOCurveMarketData object to use
* @param [in] domesticCurveCollection   The curveCollection this curve will be stored in ( The Target CurveCollection )
* @param [in] foreignCurveCollection    The curveCollection containing foreign curve dependencies ( The Against CurveCollection )
*                                       Allowed to be blank if this is a single CCY curve (OIS, STD, TenorBasis)
* @param [out]                          The curve build status
*/
std::string aqObjectsCurveCalibrate( const std::string& objectName,
								 const std::string& lwoCurveGeneratorName,
								 const std::string& lwoCurveMarketDataName,
								 const std::string& domesticCurveCollection,
								 const std::string& foreignCurveCollection );


/* @brief Creates a AQOCurveMarketData object, containing all of the curve properties.
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
std::string aqObjectsCurveMarketDataCreate( const std::string& objectName,
										const std::string& key1, const SWIG_STRINGMATRIX& value1,
										const std::string& key2, const SWIG_STRINGMATRIX& value2,
										const std::string& key3, const SWIG_STRINGMATRIX& value3,
										const std::string& key4, const SWIG_STRINGMATRIX& value4) ;


/* @brief Creates a AQOCurveMarketData object, containing all of the curve properties.
*			NOTE: This function not work correctly in R because R cannot cope with vectors of vectors.
*			In particular the vector<SWIG_STRINGMATRIX> parameter is a problem in R.
* @param [in] objectName	The name of the Market Data object
* @param [in] keyVector		The names all the datablocks, for example "MARKETDATAPROPERTIES", "SWAPS", "FUTURES", ...
* @param [in] valueVector	The vector of datablocks
* @param [out]              The objectName
*/
std::string aqObjectsCurveMarketDataCreateUsingMultipleBlocks( const std::string& objectName,
														   const std::vector<std::string>& keyVector,
														   const std::vector<SWIG_STRINGMATRIX>& valueVector );
										

/* @brief			function to create a fixing table object
*  @param [in]		tableName		    Fixing Table name
*  @param [in]		currency	        Currency as a string
*  @param [in]		curveTenor	        The curve tenor string: ANNUAL, SEMI-ANNUAL, QUARTERLY, MONTHLY, DAILY ...
*  @param [in]		fixingDates	        Vector of Fixing Dates in String Format
*  @param [in]		fixingValues	    Vector of Fixing Values in Double Format
*  @return			returns the name of the fixing table on the cache
*/
std::string aqObjectsFixingTableCreate( const std::string& tableName,
                                    const std::string& currency,                                   
                                    const std::string& curveTenor,
                                    const std::vector< std::string >& fixingDates,
                                    const std::vector< double >& fixingValues );

/* @brief			function to display a fixing table object
*  @param [in]		tableName		    Fixing Table name
*  @return			returns a VariantMatrix representing the fixing currency, curveTenor, fixingDates and fixingValues
*/
SWIG_STRINGMATRIX aqObjectsFixingTableDisplay( const std::string& tableName );

/* @brief			function to get the fixing value for a particular date
*  @param [in]		tableName		    Fixing Table name
*  @param [in]		fixingDate	        Fixing Date
*  @return			fixing value
*/
double aqObjectsFixingTableValue( const std::string& tableName, const std::string & fixingDate );

/* @brief			function to get fixing values for a vector of dates
*  @param [in]		tableName		    Fixing Table name
*  @param [in]		fixingDates     	Fixing Dates
*  @return			fixing values
*/
std::vector<double> aqObjectsFixingTableValues( const std::string& tableName, const std::vector< std::string >& fixingDates );

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
std::vector<double> aqObjectsCurveDiscountFactorsWithSpread( const std::vector<std::string>& paymentDates, const std::string& curveCollection, const std::string& curveIndex, const double& spread, const std::string& fixingTableName );
