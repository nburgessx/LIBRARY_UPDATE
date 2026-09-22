// aqIRObject.cpp

/*
 * @brief			Swig interface for aqIRFuturePriceToFraRate... and aqIRObjectFra... functions
 */

#include "aqIRObject.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqIRFutureFra.h"
#include "tryAqIRObjectFra.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"

/* @brief			Function to calculate the FRA rate implied by a rate-future price, using a Hull-White convexity adjustment
*  @param [in]		futurePrice			The given future price
*  @param [in]		curveAsOfDate		The yield curve as of or valuation date
*  @param [in]		futuresStartDate	The futures start date
*  @param [in]		futuresEndDate		The futures end date
*  @param [in]		meanReversion		The Hull-White 1F Mean Reversion Parameter
*  @param [in]		volatility			The Hull-White 1F Volatility Parameter
*  @return			The FRA rate
*/
double aqIRFuturePriceToFraRate( const double futurePrice,
                                  const std::string& curveAsOfDate,
                                  const std::string& futuresStartDate,
                                  const std::string& futuresEndDate,
                                  const double meanReversion,
                                  const double volatility )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate curveAsOfDate_( etrading::stringToDate( curveAsOfDate ) );
    AQLDate futuresStartDate_( etrading::stringToDate( futuresStartDate ) );
    AQLDate futuresEndDate_( etrading::stringToDate( futuresEndDate ) );

    // Call Function and Return Result
    double result = validation::tryAqIRFuturePriceToFraRate( futurePrice, curveAsOfDate_, futuresStartDate_, futuresEndDate_, meanReversion, volatility );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the FRA rate implied by a rate-future price and an explicit convexity adjustment
*  @param [in]		futurePrice				The given future price
*  @param [in]		convexityAdjustment		The given convexity adjustment between Future rate and Fra rate
*  @return			The FRA rate
*/
double aqIRFuturePriceToFraRateFromConvAdj( const double futurePrice,
                                             const double convexityAdjustment )
{
    AQ_API_START

    // Call Function and Return Result
    double result = validation::tryAqIRFuturePriceToFraRateFromConvAdj( futurePrice, convexityAdjustment );
    return result;

    AQ_API_END
}

/* @brief			Function to create and store a FRA from a label/value block
*  @param [in]		fraObjectName		Fra object name
*  @param [in]		fraLVB				Fra label value block
*  @param [in]		validateKeys		Optional. Default TRUE. Check the LVB keys
*  @return			Fra object handle
*/
std::string aqIRObjectFraCreate( const std::string& fraObjectName,
                                  const SWIG_STRINGMATRIX& fraLVB,
                                  const bool validateKeys )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix fraLVB_;
    swig::buildStringMatrix( fraLVB_, fraLVB );
    LabelValueBlock fraLVBAsLabelValueBlock( fraLVB_ );

    // Call Function and Return Result
    std::string result = validation::tryAqIRObjectFraCreate( fraObjectName, fraLVBAsLabelValueBlock, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the PV of a cached FRA
*  @param [in]		fraObjectName			Fra object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @return			Fra PV
*/
double aqIRObjectFraPV( const std::string& fraObjectName,
                         const std::vector<std::vector<std::string> >& valuationSettingsLVB )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqIRObjectFraPV( fraObjectName, valuationSettingsLVB_ );
    return result;

    AQ_API_END
}

/* @brief			Function to display a cached FRA's input parameters
*  @param [in]		fraObjectName		Fra object name
*  @return			Display of the Fra input parameters
*/
SWIG_STRINGMATRIX aqIRObjectFraDisplay( const std::string& fraObjectName )
{
    AQ_API_START

    // Call the Function
    AQLStringMatrix display = validation::tryAqIRObjectFraDisplay( fraObjectName );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromStringMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to display the cashflows of a cached FRA
*  @param [in]		fraObjectName			Fra object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @param [in]		showColumnHeaders		Optional. Default TRUE. Include a header row
*  @param [in]		columnList				Optional. Column header names to show specified columns. Default to empty list showing all columns.
*  @return			Fra cashflow display
*/
SWIG_STRINGMATRIX aqIRObjectFraDisplayCashflows( const std::string& fraObjectName,
                                                  const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                                  const bool showColumnHeaders,
                                                  const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call the Function
    AnyTypeMatrix display = validation::tryAqIRObjectFraDisplayCashflows( fraObjectName, valuationSettingsLVB_, showColumnHeaders, columnList );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the fair FRA rate of a cached FRA
*  @param [in]		fraObjectName			Fra object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @return			Fair Fra rate (strikeRate that makes the Fra PV zero)
*/
double aqIRObjectFraRate( const std::string& fraObjectName,
                           const std::vector<std::vector<std::string> >& valuationSettingsLVB )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqIRObjectFraRate( fraObjectName, valuationSettingsLVB_ );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the rate-future price implied by a cached FRA, using a Hull-White convexity adjustment
*  @param [in]		fraObjectName			Fra object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @param [in]		meanReversion			The Hull-White 1F Mean Reversion Parameter
*  @param [in]		volatility				The Hull-White 1F Volatility Parameter
*  @return			Future price
*/
double aqIRObjectFraToFuturePrice( const std::string& fraObjectName,
                                    const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                    const double meanReversion,
                                    const double volatility )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqIRObjectFraToFuturePrice( fraObjectName, valuationSettingsLVB_, meanReversion, volatility );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the rate-future price implied by a cached FRA and an explicit convexity adjustment
*  @param [in]		fraObjectName			Fra object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @param [in]		convexityAdjustment		The convexity adjustment between FraRate and Future rate
*  @return			Future price
*/
double aqIRObjectFraToFuturePriceFromConvAdj( const std::string& fraObjectName,
                                               const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                               const double convexityAdjustment )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqIRObjectFraToFuturePriceFromConvAdj( fraObjectName, valuationSettingsLVB_, convexityAdjustment );
    return result;

    AQ_API_END
}
