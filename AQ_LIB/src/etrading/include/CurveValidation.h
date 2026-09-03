#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include <boost/lexical_cast.hpp>
#include "LabelValueBlock.h"
#include "LACurveCalibrationHelpers.h"
#include <vector>

using etrading::LabelValueBlock;

namespace etrading
{
    /* @brief			Validate if the curve exists
    *  @param [in]		dataInstance		Pointer to the object pool
    *  @param [in]		curveId		CurveID
    */
    void checkIfCurveExists( AQLDataInstance* dataInstance, const AQLString& curveId );

    /* @brief			Convert the Frequency to Tenor
    * @param [in]		freqquency	Frequencey
    * @output			Term: 1Y, 6M, 3M, 1M, 1W
    */
    AQLString fromFrequencyToTerm( const AQLString& frequency );

    /* @brief			Validate and populate default frequency
    * @param [in]		isForwardInterp	True to indicate the interpolation is applied on forwards.
    * @param [inout]	frequency	Frequency
    */
    void validateFrequency( bool isForwardInterp, AQLString& frequency );

    /* @brief			Get the curve currency
    * @output			currency name
    */
    AQLString getCurveCurrency( const AQLString& curveCollection );

    /*  @brief is fwdfx constant curve or not, the logic is from AQLCalibrateModel.cpp
    * @param[in]   key  currency or fx ex.JPY/USD
    * @param[out]   is fwdfx constant curve or not
    */
    bool isFwdFXConst( const AQLString& ccy );

	// Struct to Store the FwdInterInfo
    struct FwdInterInfo
    {
        bool isFwdInter;
        bool useFwdData;
    };

    /* @brief			Get the forward interpolation parameters
    * @param [in]		curveCollection		Curve collection
    * @param [in]		staticDataTable		Market name
    * @param [in]		fwdInterOverride			User specified fwdInter flag
    * @output			FwdInterInfo
    */
    FwdInterInfo getfwdInterInfo( const AQLString& curveCollection, const AQLString& staticDataTable, const BooleanEnum& fwdInterOverride=NONE_BOOL );

	// Convert a frequency to a tenor
    AQLString getFrequencyTenor( const FrequencyEnum& frequencyString );

	// Get the frequency string e.g 1D -> DAILY, 1W -> WEEKLY, 1M -> MONTHLY etc ...
    AQLString getFrequencyString( const AQLString& frequencyTenor );

    /* @brief			return a number to indicate the freqOrTenor in months
    * @param [in]		freqOrTenor Frequency or Tenor
    * @output			number of months
    */
    unsigned int getFrequencyOrTenorMonth( const AQLString& freqOrTenor );

    /* @brief			return a number to indicate the freq in months
    * @param [in]		freq Frequency
    * @output			number of months 
    */
    unsigned int getFrequencyNumber(const FrequencyEnum& freq);

    /* @brief			Function to return the curve frequency given the curveCollection and curveName
    * @param [in]		curveCollection	    Curve collection Id
    * @param [in]		curveIndex		    Curve index name
    * @output			curveFrequency
    */
    AQLString validateCurveAndGetCurveFrequency( const AQLString& curveCollection, const AQLString& curveIndex );

    /* @brief			Function to return the curve frequency as a year fractiongiven the curveCollection and curveName
    * @param [in]		curveCollection	    Curve collection Id
    * @param [in]		curveIndex		    Curve index name
    * @output			curveFrequency as a year fraction
    */
    double getCurveFrequencyAsYearFraction( const AQLString& curveCollection, const AQLString& curveIndex );
    
    /* @brief			Function to return the curve float daycount fraction given the curveCollection and curveName
    * @param [in]		curveCollection	    Curve collection Id
    * @param [in]		curveIndex		    Curve index name
    * @output			FloatDaycount
    */
    AQLString validateCurveAndGetFloatDaycount( const AQLString& curveCollection, const AQLString& curveIndex );

    /* @brief			Validate if the curve is built from curveCollection and curveIndex, and if so return the staticDataTable(marketName)
    * @param [in]		curveCollection	    Curve collection Id
    * @param [in]		curveIndex		    Curve index name
    * @param [in]		uppercaseResult		The market name result is returned uppercase by default (true), some functions require the marketname to be un touched (false)
    * @output			staticDataTable
    */
    AQLString getCurveStaticDataTableName( const AQLString& curveCollection, const AQLString& curveIndex, const bool& uppercaseResult = true );

    /* @brief			Return interpolation method of the curve

    Note: We use the property file to read the yield curve settings and parameters. Properties are loaded from the ip.properties config file and then
    overridden by the end user. Properties here are the overridden ones, not the defaults in the config file

    * @param [in]		curveCollection	Curve collection Id
    * @param [in]		staticDataTable	Curve staticDataTable(MarketName)
    * @output			Interpolation method
    */
    AQLString getCurveInterpolation( const AQLString& curveCollection, const AQLString& staticDataTable );

    /* @brief			Return the type of a yield curve
    * @param [in]		objPool		                    Object Pool
    * @param [in]		curveCollection		        Curve collection Id
    * @param [in]		upperCaseStaticDataTable    Curve staticDataTable / MarketName - must be in uppercase !!!
    * @output			Curve type
    */
    AQLString getCurveType( AQLObjectPool& objPool, const AQLString& curveCollection, const AQLString& upperCaseStaticDataTable );

    /* @brief			Return the type of a yield curve
    * @param [in]		curveCollection		Curve collection Id
    * @param [in]		staticDataTable		Curve staticDataTable(MarketName)
    * @output			Curve type
    */
    AQLString getCurveType( const AQLString& curveCollection, const AQLString& staticDataTable );

	/* @brief			Return the type of a yield curve
	* @param [in]		curveCollection		Curve collection Id
	* @param [in]		curveIndex			Curve index
	* @output			Curve type
	*/
	CurveTypeEnum getCurveTypeFromCurveIndex(const AQLString& curveCollection, const AQLString& curveIndex);

	/* @brief			Return boolean to confirm if curve exists
    * @param [in]		curveCollection	Curve collection Id
    * @output			Boolean for Does Curve Exist?
    */
	bool doesCurveExist( const AQLString& curveCollection );
	
    /* @brief			Return asOfDate of the curve
    * @param [in]		curveCollection	Curve collection Id
    * @output			As of Date
    */
    AQLDate getCurveAsOfDate( const AQLString& curveCollection );

	/* @brief	Function to return the curveFrequencyString and curveFrequencyTenor corresponding to specified curveIndex and CurveCollection
	 * @param [in]		objPool						The EntityPool
	 * @param [in]		marketNameUpperCase		The MarketName / StaticDataTable table name
	 * @param [in]		curveCollection			The curve collection Id
	 * @param [in]		curveIndex				The curveIndex used to look up the curveFrequencyString and curveFrequencyTenor
	 * @param [out]		curveFrequencyString	The curveFrequencyString corresponding to the specified curveIndex
	 * @param [out]		curveFrequencyTenor		The curveFrequencyTenor corresponding to the specified curveIndex
     * @param [in]		enableThrow		        Enable Throw - Throw on Error True or False - Defaults to True
	 */
	void getCurveFrequency(AQLObjectPool& objPool, const AQLString& marketNameUppercase, const AQLString& curveCollection, const AQLString& curveIndex, AQLString& curveFrequencyString, AQLString& curveFrequencyTenor, const bool enableThrow = true);

	/* @brief	Function to return the StaticDataTable, CurveIndex, CurveTenor, for each curveIndex in a given curveCollection
	 * @param [in]		curveCollection	    Curve collection Id
	 * @param [out]		A vector of items, where each item consists of:  StaticDataTable, CurveIndex, CurveTenor
	 */
	std::vector<AQLStringVector> getCurveNamesInCurveCollection( const AQLString& curveCollection );

    
    /* @brief			convertTermsToDates function which converts discount factor terms or year fractions to payment dates
	*  @param [in]		asOfDate            The start or asOfDate, which is the base date for our terms year fractions
    *  @param [in]		terms               The terms or year fractions corresponding to our discount factors
	*  @param [out]		paymentDates        The corresponding payment dates
	*/
    DateVector convertCurveTermsToDates( const AQLDate& asOfDate, const DoubleVector& terms );

    /* @brief			convertTermToDate function which converts discount factor terms or year fractions to payment dates
	*  @param [in]		asOfDate            The start or asOfDate, which is the base date for our terms year fractions
    *  @param [in]		term                The term or year fraction
	*  @param [out]		paymentDate         The corresponding payment date
	*/
    AQLDate convertCurveTermToDate( const AQLDate& asOfDate, const double& term );

    /* @brief			convertCurveDatesToTerms function which converts discount factor payment dates to terms or year fractions
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		terms               The terms or year fractions 
	*  @param [out]		paymentDates        The corresponding payment dates
	*/
    DateVector convertCurveTermsToDates( const AQLString& curveCollection, const DoubleVector& terms );
    
    /* @brief			convertDateToTerm function which converts discount factor payment dates to terms or year fractions
	*  @param [in]		asOfDate            The start or asOfDate, which is the base date for our terms year fractions
    *  @param [in]		paymentDate         The payment date
	*  @param [out]		term                The corresponding term value
	*/
    double convertCurveDateToTerm( const AQLDate& asOfDate, const AQLDate& paymentDate );

    /* @brief			convertDatesToTerms function which converts discount factor payment dates to terms or year fractions
	*  @param [in]		asOfDate            The start or asOfDate, which is the base date for our terms year fractions
    *  @param [in]		paymentDates        The payment dates
	*  @param [out]		terms               The corresponding terms values
	*/
    DoubleVector convertCurveDatesToTerms( const AQLDate& asOfDate, const DateVector& paymentDates );

    /* @brief			convertCurveDatesToTerms function which converts discount factor payment dates to terms or year fractions
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		paymentDates        The payment dates corresponding to our discount factors
	*  @param [out]		terms               The corresponding terms values
	*/
    DoubleVector convertCurveDatesToTerms( const AQLString& curveCollection, const DateVector& paymentDates );

    /* @brief			function to format and convert a curve index from uppercase to camel case - needed for object pool data searches
	*  @param [in]		curveIndex  	        The unformatted curve index, usually in uppercase
    *  @param [out]		formattedCurveIndex     The object pool formatted curve index, usually in camel case
	*/
    AQLString formatCurveIndex( const AQLString& curveIndex );
    
    /* @brief			Function to get a complete list of curve index alias' given one of the curve indices
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveIndex          A single curve index in the curve collection
	*  @param [in]		throwOnError        throw on error if TRUE or return empty string vector if FALSE
	*  @param [out]		A vector of all curve index alias' used
	*/
    AQLStringVector curveIndexAliasList( const AQLString& curveCollection, const AQLString& curveIndex, const bool throwOnError = true );

    /* @brief			Function to get a complete list of curve index alias' given one of the curve indices
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveIndex          A single curve index in the curve collection
	*  @param [out]		A vector of all curve index alias' used
	*/
    StandardStringVector curveIndexAliasListAsStandardString( const StandardString& curveCollection, const StandardString& curveIndex );

    /* @brief			Function to identify if a curve is an STD curve
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveIndex          A single curve index in the curve collection
	*  @param [out]		A boolean of true if STD curve and false otherwise
	*/
    bool isSTDCurve( const AQLString& curveCollection, const AQLString& curveIndex );
        
}


