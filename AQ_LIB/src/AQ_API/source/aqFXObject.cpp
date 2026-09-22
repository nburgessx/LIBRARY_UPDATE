// aqFXObject.cpp

/*
 * @brief			Swig interface for aqFXCurveCreate / aqFXObject... functions
 */

#include "aqFXObject.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqFXObject.h"
#include "APISetUp.h"					// AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"		// etrading::stringToDate

/* @brief			Function to create and store an FX curve from a curve generator and market data
*  @param [in]		objectName				FX curve object name
*  @param [in]		curveGeneratorName		A curve-generator handle
*  @param [in]		curveMarketDataName		A curve-market-data handle
*  @return			FX curve object handle
*/
std::string aqFXCurveCreate( const std::string& objectName,
                              const std::string& curveGeneratorName,
                              const std::string& curveMarketDataName )
{
    AQ_API_START

    // Call Function and Return Result
    std::string result = validation::tryAqFXCurveCreate( objectName, curveGeneratorName, curveMarketDataName );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate FX forwards derived from base/term discount curves
*  @param [in]		objectName				FX curve object name
*  @param [in]		settleDatesOrTenors		Settlement dates or tenors
*  @param [in]		baseCurveCollection		Base-currency discount curve collection
*  @param [in]		termCurveCollection		Term-currency discount curve collection
*  @param [in]		showColumnHeaders		Optional. Default FALSE. Include a header row
*  @param [in]		columnList				Optional. Columns to include. Default to empty list showing all columns.
*  @return			FX forwards outright rates and points
*/
SWIG_STRINGMATRIX aqFXObjectForwardsFromDiscountCurves( const std::string& objectName,
                                                     const std::vector<std::string>& settleDatesOrTenors,
                                                     const std::string& baseCurveCollection,
                                                     const std::string& termCurveCollection,
                                                     const bool showColumnHeaders,
                                                     const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqFXObjectForwardsFromDiscountCurves(
        objectName, settleDatesOrTenors, baseCurveCollection, termCurveCollection, showColumnHeaders, columnList );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate FX forwards from a cached FX curve
*  @param [in]		objectName				FX curve object name
*  @param [in]		settleDatesOrTenors		Settlement dates or tenors
*  @param [in]		showColumnHeaders		Optional. Default FALSE. Include a header row
*  @param [in]		columnList				Optional. Columns to include. Default to empty list showing all columns.
*  @return			FX forwards outright rates and points
*/
SWIG_STRINGMATRIX aqFXObjectForwards( const std::string& objectName,
                                                     const std::vector<std::string>& settleDatesOrTenors,
                                                     const bool showColumnHeaders,
                                                     const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqFXObjectForwards( objectName, settleDatesOrTenors, showColumnHeaders, columnList );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate FX swap points derived from base/term discount curves
*  @param [in]		objectName								FX curve object name
*  @param [in]		nearLegSettlementDatesOrTenors			Near leg settlement dates or tenors
*  @param [in]		farLegSettlementDatesOrTenors			Far leg settlement dates or tenors
*  @param [in]		baseCurveCollection						Base-currency discount curve collection
*  @param [in]		termCurveCollection						Term-currency discount curve collection
*  @param [in]		outputFarLeg							TRUE to output the far leg's rates, FALSE the near leg's rates
*  @param [in]		showColumnHeaders						Optional. Default FALSE. Include a header row
*  @param [in]		columnList								Optional. Columns to include. Default to empty list showing all columns.
*  @return			FX swap outright rates and points
*/
SWIG_STRINGMATRIX aqFXObjectSwapFromDiscountCurves( const std::string& objectName,
                                                     const std::vector<std::string>& nearLegSettlementDatesOrTenors,
                                                     const std::vector<std::string>& farLegSettlementDatesOrTenors,
                                                     const std::string& baseCurveCollection,
                                                     const std::string& termCurveCollection,
                                                     const bool outputFarLeg,
                                                     const bool showColumnHeaders,
                                                     const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqFXObjectSwapFromDiscountCurves(
        objectName, nearLegSettlementDatesOrTenors, farLegSettlementDatesOrTenors, baseCurveCollection, termCurveCollection,
        outputFarLeg, showColumnHeaders, columnList );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate FX swap points from a cached FX curve
*  @param [in]		objectName								FX curve object name
*  @param [in]		nearLegSettlementDatesOrTenors			Near leg settlement dates or tenors
*  @param [in]		farLegSettlementDatesOrTenors			Far leg settlement dates or tenors
*  @param [in]		outputFarLeg							TRUE to output the far leg's rates, FALSE the near leg's rates
*  @param [in]		showColumnHeaders						Optional. Default FALSE. Include a header row
*  @param [in]		columnList								Optional. Columns to include. Default to empty list showing all columns.
*  @return			FX swap outright rates and points
*/
SWIG_STRINGMATRIX aqFXObjectSwap( const std::string& objectName,
                                                     const std::vector<std::string>& nearLegSettlementDatesOrTenors,
                                                     const std::vector<std::string>& farLegSettlementDatesOrTenors,
                                                     const bool outputFarLeg,
                                                     const bool showColumnHeaders,
                                                     const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqFXObjectSwap(
        objectName, nearLegSettlementDatesOrTenors, farLegSettlementDatesOrTenors, outputFarLeg, showColumnHeaders, columnList );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate FX forwards from a cached cross-currency curve object
*  @param [in]		xccyCurveObjectName				A cross-currency curve handle
*  @param [in]		settleDatesOrTenors				Settlement dates or tenors
*  @param [in]		xccyCurveSwapRateBumpSize			Bump size applied to the xccy swap rate
*  @param [in]		showColumnHeaders					Optional. Default FALSE. Include a header row
*  @param [in]		columnList							Optional. Columns to include. Default to empty list showing all columns.
*  @return			FX forwards outright rates and points
*/
SWIG_STRINGMATRIX aqFXObjectForwardsFromXccyCurveObject( const std::string& xccyCurveObjectName,
                                                     const std::vector<std::string>& settleDatesOrTenors,
                                                     const double xccyCurveSwapRateBumpSize,
                                                     const bool showColumnHeaders,
                                                     const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqFXObjectForwardsFromXccyCurveObject(
        xccyCurveObjectName, settleDatesOrTenors, xccyCurveSwapRateBumpSize, showColumnHeaders, columnList );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate FX forwards from base/term curve collections via a cross-currency basis
*  @param [in]		baseCurveCollection					Base-currency curve collection
*  @param [in]		baseCurveIndex						Base-currency curve index
*  @param [in]		termCurveCollection					Term-currency curve collection
*  @param [in]		termCurveIndex						Term-currency curve index
*  @param [in]		fxSpotRate							FX spot rate
*  @param [in]		pipSize								Pip size for quoting
*  @param [in]		xccyCurveSwapRateBumpSize			Bump size applied to the xccy swap rate
*  @param [in]		settleDatesOrTenors					Settlement dates or tenors
*  @param [in]		showColumnHeaders					Optional. Default FALSE. Include a header row
*  @param [in]		columnList							Optional. Columns to include. Default to empty list showing all columns.
*  @return			FX forwards outright rates and points
*/
SWIG_STRINGMATRIX aqFXObjectForwardsFromXccyCurve( const std::string& baseCurveCollection,
                                                     const std::string& baseCurveIndex,
                                                     const std::string& termCurveCollection,
                                                     const std::string& termCurveIndex,
                                                     const double fxSpotRate,
                                                     const double pipSize,
                                                     const double xccyCurveSwapRateBumpSize,
                                                     const std::vector<std::string>& settleDatesOrTenors,
                                                     const bool showColumnHeaders,
                                                     const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqFXObjectForwardsFromXccyCurve(
        baseCurveCollection, baseCurveIndex, termCurveCollection, termCurveIndex, fxSpotRate, pipSize,
        xccyCurveSwapRateBumpSize, settleDatesOrTenors, showColumnHeaders, columnList );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate FX swap points from a cached cross-currency curve object
*  @param [in]		xccyCurveObjectName						A cross-currency curve handle
*  @param [in]		nearLegSettlementDatesOrTenors			Near leg settlement dates or tenors
*  @param [in]		farLegSettlementDatesOrTenors			Far leg settlement dates or tenors
*  @param [in]		outputFarLeg							TRUE to output the far leg's rates, FALSE the near leg's rates
*  @param [in]		showColumnHeaders						Optional. Default FALSE. Include a header row
*  @param [in]		columnList								Optional. Columns to include. Default to empty list showing all columns.
*  @return			FX swap outright rates and points
*/
SWIG_STRINGMATRIX aqFXObjectSwapFromXccyCurveObject( const std::string& xccyCurveObjectName,
                                                     const std::vector<std::string>& nearLegSettlementDatesOrTenors,
                                                     const std::vector<std::string>& farLegSettlementDatesOrTenors,
                                                     const bool outputFarLeg,
                                                     const bool showColumnHeaders,
                                                     const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqFXObjectSwapFromXccyCurveObject(
        xccyCurveObjectName, nearLegSettlementDatesOrTenors, farLegSettlementDatesOrTenors, outputFarLeg, showColumnHeaders, columnList );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate FX swap points from base/term curve collections via a cross-currency basis
*  @param [in]		baseCurveCollection						Base-currency curve collection
*  @param [in]		baseCurveIndex							Base-currency curve index
*  @param [in]		termCurveCollection						Term-currency curve collection
*  @param [in]		termCurveIndex							Term-currency curve index
*  @param [in]		fxSpotRate								FX spot rate
*  @param [in]		pipSize									Pip size for quoting
*  @param [in]		nearLegSettlementDatesOrTenors			Near leg settlement dates or tenors
*  @param [in]		farLegSettlementDatesOrTenors			Far leg settlement dates or tenors
*  @param [in]		outputFarLeg							TRUE to output the far leg's rates, FALSE the near leg's rates
*  @param [in]		showColumnHeaders						Optional. Default FALSE. Include a header row
*  @param [in]		columnList								Optional. Columns to include. Default to empty list showing all columns.
*  @return			FX swap outright rates and points
*/
SWIG_STRINGMATRIX aqFXObjectSwapFromXccyCurve( const std::string& baseCurveCollection,
                                                     const std::string& baseCurveIndex,
                                                     const std::string& termCurveCollection,
                                                     const std::string& termCurveIndex,
                                                     const double fxSpotRate,
                                                     const double pipSize,
                                                     const std::vector<std::string>& nearLegSettlementDatesOrTenors,
                                                     const std::vector<std::string>& farLegSettlementDatesOrTenors,
                                                     const bool outputFarLeg,
                                                     const bool showColumnHeaders,
                                                     const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqFXObjectSwapFromXccyCurve(
        baseCurveCollection, baseCurveIndex, termCurveCollection, termCurveIndex, fxSpotRate, pipSize,
        nearLegSettlementDatesOrTenors, farLegSettlementDatesOrTenors, outputFarLeg, showColumnHeaders, columnList );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the as-of-date FX rate implied by a spot rate
*  @param [in]		fxSpot						FX rate on FX spot date
*  @param [in]		fxSpotDate					FX spot date
*  @param [in]		baseCurveCollection			Base-currency discount curve collection
*  @param [in]		baseDiscountCurveIndex		Base-currency discount curve index
*  @param [in]		termCurveCollection			Term-currency discount curve collection
*  @param [in]		termDiscountCurveIndex		Term-currency discount curve index
*  @return			FX rate on curve as-of date
*/
double aqFXObjectSpotToAsOfDate( const double fxSpot,
                                  const std::string& fxSpotDate,
                                  const std::string& baseCurveCollection,
                                  const std::string& baseDiscountCurveIndex,
                                  const std::string& termCurveCollection,
                                  const std::string& termDiscountCurveIndex )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate fxSpotDate_( etrading::stringToDate( fxSpotDate ) );

    // Call Function and Return Result
    double result = validation::tryAqFXObjectSpotToAsOfDate(
        fxSpot, fxSpotDate_, baseCurveCollection, baseDiscountCurveIndex, termCurveCollection, termDiscountCurveIndex );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the FX spot rate implied by an as-of-date rate
*  @param [in]		fxAsOfDateRate				FX rate on as-of date
*  @param [in]		fxSpotDate					FX spot date
*  @param [in]		baseCurveCollection			Base-currency discount curve collection
*  @param [in]		baseDiscountCurveIndex		Base-currency discount curve index
*  @param [in]		termCurveCollection			Term-currency discount curve collection
*  @param [in]		termDiscountCurveIndex		Term-currency discount curve index
*  @return			FX spot rate implied by the as-of-date rate
*/
double aqFXObjectAsOfDateToSpot( const double fxAsOfDateRate,
                                  const std::string& fxSpotDate,
                                  const std::string& baseCurveCollection,
                                  const std::string& baseDiscountCurveIndex,
                                  const std::string& termCurveCollection,
                                  const std::string& termDiscountCurveIndex )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate fxSpotDate_( etrading::stringToDate( fxSpotDate ) );

    // Call Function and Return Result
    double result = validation::tryAqFXObjectAsOfDateToSpot(
        fxAsOfDateRate, fxSpotDate_, baseCurveCollection, baseDiscountCurveIndex, termCurveCollection, termDiscountCurveIndex );
    return result;

    AQ_API_END
}
