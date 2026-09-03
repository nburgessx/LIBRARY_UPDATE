/*
 * @brief			curve validation methods used within validation_api
 * @Created:		17 March 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"
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
    void checkIfCurveExists( LADataInstance* dataInstance, const LAString& curveId );

    /* @brief			Convert the Frequency to Tenor
    * @param [in]		freqquency	Frequencey
    * @output			Term: 1Y, 6M, 3M, 1M, 1W
    */
    LAString fromFrequencyToTerm( const LAString& frequency );

    /* @brief			Validate and populate default frequency
    * @param [in]		isForwardInterp	True to indicate the interpolation is applied on forwards.
    * @param [inout]	frequency	Frequency
    */
    void validateFrequency( bool isForwardInterp, LAString& frequency );

    /* @brief			Get the curve currency
    * @output			currency name
    */
    LAString getCurveCurrency( const LAString& curveCollection );

    /*  @brief is fwdfx constant curve or not, the logic is from LACalibrateModel.cpp
    * @param[in]   key  currency or fx ex.JPY/USD
    * @param[out]   is fwdfx constant curve or not
    */
    bool isFwdFXConst( const LAString& ccy );

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
    FwdInterInfo getfwdInterInfo( const LAString& curveCollection, const LAString& staticDataTable, const BooleanEnum& fwdInterOverride=NONE_BOOL );

	// Convert a frequency to a tenor
    LAString getFrequencyTenor( const FrequencyEnum& frequencyString );

	// Get the frequency string e.g 1D -> DAILY, 1W -> WEEKLY, 1M -> MONTHLY etc ...
    LAString getFrequencyString( const LAString& frequencyTenor );

    /* @brief			return a number to indicate the freqOrTenor in months
    * @param [in]		freqOrTenor Frequency or Tenor
    * @output			number of months
    */
    unsigned int getFrequencyOrTenorMonth( const LAString& freqOrTenor );

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
    LAString validateCurveAndGetCurveFrequency( const LAString& curveCollection, const LAString& curveIndex );

    /* @brief			Function to return the curve frequency as a year fractiongiven the curveCollection and curveName
    * @param [in]		curveCollection	    Curve collection Id
    * @param [in]		curveIndex		    Curve index name
    * @output			curveFrequency as a year fraction
    */
    double getCurveFrequencyAsYearFraction( const LAString& curveCollection, const LAString& curveIndex );
    
    /* @brief			Function to return the curve float daycount fraction given the curveCollection and curveName
    * @param [in]		curveCollection	    Curve collection Id
    * @param [in]		curveIndex		    Curve index name
    * @output			FloatDaycount
    */
    LAString validateCurveAndGetFloatDaycount( const LAString& curveCollection, const LAString& curveIndex );

    /* @brief			Validate if the curve is built from curveCollection and curveIndex, and if so return the staticDataTable(marketName)
    * @param [in]		curveCollection	    Curve collection Id
    * @param [in]		curveIndex		    Curve index name
    * @param [in]		uppercaseResult		The market name result is returned uppercase by default (true), some functions require the marketname to be un touched (false)
    * @output			staticDataTable
    */
    LAString getCurveStaticDataTableName( const LAString& curveCollection, const LAString& curveIndex, const bool& uppercaseResult = true );

    /* @brief			Return interpolation method of the curve

    Note: We use the property file to read the yield curve settings and parameters. Properties are loaded from the ip.properties config file and then
    overridden by the end user. Properties here are the overridden ones, not the defaults in the config file

    * @param [in]		curveCollection	Curve collection Id
    * @param [in]		staticDataTable	Curve staticDataTable(MarketName)
    * @output			Interpolation method
    */
    LAString getCurveInterpolation( const LAString& curveCollection, const LAString& staticDataTable );

    /* @brief			Return the type of a yield curve
    * @param [in]		objPool		                    Object Pool
    * @param [in]		curveCollection		        Curve collection Id
    * @param [in]		upperCaseStaticDataTable    Curve staticDataTable / MarketName - must be in uppercase !!!
    * @output			Curve type
    */
    LAString getCurveType( LAObjectPool& objPool, const LAString& curveCollection, const LAString& upperCaseStaticDataTable );

    /* @brief			Return the type of a yield curve
    * @param [in]		curveCollection		Curve collection Id
    * @param [in]		staticDataTable		Curve staticDataTable(MarketName)
    * @output			Curve type
    */
    LAString getCurveType( const LAString& curveCollection, const LAString& staticDataTable );

	/* @brief			Return the type of a yield curve
	* @param [in]		curveCollection		Curve collection Id
	* @param [in]		curveIndex			Curve index
	* @output			Curve type
	*/
	CurveTypeEnum getCurveTypeFromCurveIndex(const LAString& curveCollection, const LAString& curveIndex);

	/* @brief			Return boolean to confirm if curve exists
    * @param [in]		curveCollection	Curve collection Id
    * @output			Boolean for Does Curve Exist?
    */
	bool doesCurveExist( const LAString& curveCollection );
	
    /* @brief			Return asOfDate of the curve
    * @param [in]		curveCollection	Curve collection Id
    * @output			As of Date
    */
    LADate getCurveAsOfDate( const LAString& curveCollection );

	/* @brief	Function to return the curveFrequencyString and curveFrequencyTenor corresponding to specified curveIndex and CurveCollection
	 * @param [in]		objPool						The EntityPool
	 * @param [in]		marketNameUpperCase		The MarketName / StaticDataTable table name
	 * @param [in]		curveCollection			The curve collection Id
	 * @param [in]		curveIndex				The curveIndex used to look up the curveFrequencyString and curveFrequencyTenor
	 * @param [out]		curveFrequencyString	The curveFrequencyString corresponding to the specified curveIndex
	 * @param [out]		curveFrequencyTenor		The curveFrequencyTenor corresponding to the specified curveIndex
     * @param [in]		enableThrow		        Enable Throw - Throw on Error True or False - Defaults to True
	 */
	void getCurveFrequency(LAObjectPool& objPool, const LAString& marketNameUppercase, const LAString& curveCollection, const LAString& curveIndex, LAString& curveFrequencyString, LAString& curveFrequencyTenor, const bool enableThrow = true);

	/* @brief	Function to return the StaticDataTable, CurveIndex, CurveTenor, for each curveIndex in a given curveCollection
	 * @param [in]		curveCollection	    Curve collection Id
	 * @param [out]		A vector of items, where each item consists of:  StaticDataTable, CurveIndex, CurveTenor
	 */
	std::vector<LAStringVector> getCurveNamesInCurveCollection( const LAString& curveCollection );

    
    /* @brief			convertTermsToDates function which converts discount factor terms or year fractions to payment dates
	*  @param [in]		asOfDate            The start or asOfDate, which is the base date for our terms year fractions
    *  @param [in]		terms               The terms or year fractions corresponding to our discount factors
	*  @param [out]		paymentDates        The corresponding payment dates
	*/
    DateVector convertCurveTermsToDates( const LADate& asOfDate, const DoubleVector& terms );

    /* @brief			convertTermToDate function which converts discount factor terms or year fractions to payment dates
	*  @param [in]		asOfDate            The start or asOfDate, which is the base date for our terms year fractions
    *  @param [in]		term                The term or year fraction
	*  @param [out]		paymentDate         The corresponding payment date
	*/
    LADate convertCurveTermToDate( const LADate& asOfDate, const double& term );

    /* @brief			convertCurveDatesToTerms function which converts discount factor payment dates to terms or year fractions
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		terms               The terms or year fractions 
	*  @param [out]		paymentDates        The corresponding payment dates
	*/
    DateVector convertCurveTermsToDates( const LAString& curveCollection, const DoubleVector& terms );
    
    /* @brief			convertDateToTerm function which converts discount factor payment dates to terms or year fractions
	*  @param [in]		asOfDate            The start or asOfDate, which is the base date for our terms year fractions
    *  @param [in]		paymentDate         The payment date
	*  @param [out]		term                The corresponding term value
	*/
    double convertCurveDateToTerm( const LADate& asOfDate, const LADate& paymentDate );

    /* @brief			convertDatesToTerms function which converts discount factor payment dates to terms or year fractions
	*  @param [in]		asOfDate            The start or asOfDate, which is the base date for our terms year fractions
    *  @param [in]		paymentDates        The payment dates
	*  @param [out]		terms               The corresponding terms values
	*/
    DoubleVector convertCurveDatesToTerms( const LADate& asOfDate, const DateVector& paymentDates );

    /* @brief			convertCurveDatesToTerms function which converts discount factor payment dates to terms or year fractions
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		paymentDates        The payment dates corresponding to our discount factors
	*  @param [out]		terms               The corresponding terms values
	*/
    DoubleVector convertCurveDatesToTerms( const LAString& curveCollection, const DateVector& paymentDates );

    /* @brief			function to format and convert a curve index from uppercase to camel case - needed for object pool data searches
	*  @param [in]		curveIndex  	        The unformatted curve index, usually in uppercase
    *  @param [out]		formattedCurveIndex     The object pool formatted curve index, usually in camel case
	*/
    LAString formatCurveIndex( const LAString& curveIndex );
    
    /* @brief			Function to get a complete list of curve index alias' given one of the curve indices
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveIndex          A single curve index in the curve collection
	*  @param [in]		throwOnError        throw on error if TRUE or return empty string vector if FALSE
	*  @param [out]		A vector of all curve index alias' used
	*/
    LAStringVector curveIndexAliasList( const LAString& curveCollection, const LAString& curveIndex, const bool throwOnError = true );

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
    bool isSTDCurve( const LAString& curveCollection, const LAString& curveIndex );
        
}


