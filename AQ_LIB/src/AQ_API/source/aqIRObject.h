// aqIRObject.h

/*
 * @brief			Swig interface for aqIRFuturePriceToFraRate... and aqIRObjectFra... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqIRFuturePriceToFraRate. Calculate the FRA rate implied by a rate-future price, using a Hull-White convexity adjustment.
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
                                  const double volatility );

/* @brief			swig interface for aqIRFuturePriceToFraRateFromConvAdj. Calculate the FRA rate implied by a rate-future price and an explicit convexity adjustment.
*  @param [in]		futurePrice				The given future price
*  @param [in]		convexityAdjustment		The given convexity adjustment between Future rate and Fra rate
*  @return			The FRA rate
*/
double aqIRFuturePriceToFraRateFromConvAdj( const double futurePrice,
                                             const double convexityAdjustment );

/* @brief			swig interface for aqIRObjectFraCreate
*  @param [in]		fraObjectName		Fra object name
*  @param [in]		fraLVB				Fra label value block
*  @param [in]		validateKeys		Optional. Default TRUE. Check the LVB keys
*  @return			Fra object handle
*/
std::string aqIRObjectFraCreate( const std::string& fraObjectName,
                                  const SWIG_STRINGMATRIX& fraLVB,
                                  const bool validateKeys = true );

/* @brief			swig interface for aqIRObjectFraPV
*  @param [in]		fraObjectName			Fra object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @return			Fra PV
*/
double aqIRObjectFraPV( const std::string& fraObjectName,
                         const std::vector<std::vector<std::string> >& valuationSettingsLVB );

/* @brief			swig interface for aqIRObjectFraDisplay, which displays the INPUT parameters of the cached FRA
*  @param [in]		fraObjectName		Fra object name
*  @return			Display of the Fra input parameters
*/
SWIG_STRINGMATRIX aqIRObjectFraDisplay( const std::string& fraObjectName );

/* @brief			swig interface for aqIRObjectFraDisplayCashflows
*  @param [in]		fraObjectName			Fra object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @param [in]		showColumnHeaders		Optional. Default TRUE. Include a header row
*  @param [in]		columnList				Optional. Column header names to show specified columns. Default to empty list showing all columns.
*  @return			Fra cashflow display
*/
SWIG_STRINGMATRIX aqIRObjectFraDisplayCashflows( const std::string& fraObjectName,
                                                  const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                                  const bool showColumnHeaders = true,
                                                  const std::vector<std::string>& columnList = std::vector<std::string>() );

/* @brief			swig interface for aqIRObjectFraRate
*  @param [in]		fraObjectName			Fra object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @return			Fair Fra rate (strikeRate that makes the Fra PV zero)
*/
double aqIRObjectFraRate( const std::string& fraObjectName,
                           const std::vector<std::vector<std::string> >& valuationSettingsLVB );

/* @brief			swig interface for aqIRObjectFraToFuturePrice. Calculate the rate-future price implied by a cached FRA, using a Hull-White convexity adjustment.
*  @param [in]		fraObjectName			Fra object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @param [in]		meanReversion			The Hull-White 1F Mean Reversion Parameter
*  @param [in]		volatility				The Hull-White 1F Volatility Parameter
*  @return			Future price
*/
double aqIRObjectFraToFuturePrice( const std::string& fraObjectName,
                                    const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                    const double meanReversion,
                                    const double volatility );

/* @brief			swig interface for aqIRObjectFraToFuturePriceFromConvAdj. Calculate the rate-future price implied by a cached FRA and an explicit convexity adjustment.
*  @param [in]		fraObjectName			Fra object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @param [in]		convexityAdjustment		The convexity adjustment between FraRate and Future rate
*  @return			Future price
*/
double aqIRObjectFraToFuturePriceFromConvAdj( const std::string& fraObjectName,
                                               const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                               const double convexityAdjustment );
