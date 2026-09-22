// aqFXObject.h

/*
 * @brief			Swig interface for aqFXCurveCreate / aqFXObject... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqFXCurveCreate, creating an FX curve from a curve generator and market data
*  @param [in]		objectName				FX curve object name
*  @param [in]		curveGeneratorName		A curve-generator handle
*  @param [in]		curveMarketDataName		A curve-market-data handle
*  @return			FX curve object handle
*/
std::string aqFXCurveCreate( const std::string& objectName,
                              const std::string& curveGeneratorName,
                              const std::string& curveMarketDataName );

/* @brief			swig interface for aqFXObjectForwardsFromDiscountCurves
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
                                                     const bool showColumnHeaders = false,
                                                     const std::vector<std::string>& columnList = std::vector<std::string>() );

/* @brief			swig interface for aqFXObjectForwards, from a cached FX curve
*  @param [in]		objectName				FX curve object name
*  @param [in]		settleDatesOrTenors		Settlement dates or tenors
*  @param [in]		showColumnHeaders		Optional. Default FALSE. Include a header row
*  @param [in]		columnList				Optional. Columns to include. Default to empty list showing all columns.
*  @return			FX forwards outright rates and points
*/
SWIG_STRINGMATRIX aqFXObjectForwards( const std::string& objectName,
                                                     const std::vector<std::string>& settleDatesOrTenors,
                                                     const bool showColumnHeaders = false,
                                                     const std::vector<std::string>& columnList = std::vector<std::string>() );

/* @brief			swig interface for aqFXObjectSwapFromDiscountCurves
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
                                                     const bool showColumnHeaders = false,
                                                     const std::vector<std::string>& columnList = std::vector<std::string>() );

/* @brief			swig interface for aqFXObjectSwap, from a cached FX curve
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
                                                     const bool showColumnHeaders = false,
                                                     const std::vector<std::string>& columnList = std::vector<std::string>() );

/* @brief			swig interface for aqFXObjectForwardsFromXccyCurveObject, from a cached cross-currency curve object
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
                                                     const bool showColumnHeaders = false,
                                                     const std::vector<std::string>& columnList = std::vector<std::string>() );

/* @brief			swig interface for aqFXObjectForwardsFromXccyCurve, from base/term curve collections via a cross-currency basis
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
                                                     const bool showColumnHeaders = false,
                                                     const std::vector<std::string>& columnList = std::vector<std::string>() );

/* @brief			swig interface for aqFXObjectSwapFromXccyCurveObject, from a cached cross-currency curve object
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
                                                     const bool showColumnHeaders = false,
                                                     const std::vector<std::string>& columnList = std::vector<std::string>() );

/* @brief			swig interface for aqFXObjectSwapFromXccyCurve, from base/term curve collections via a cross-currency basis
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
                                                     const bool showColumnHeaders = false,
                                                     const std::vector<std::string>& columnList = std::vector<std::string>() );

/* @brief			swig interface for aqFXObjectSpotToAsOfDate
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
                                  const std::string& termDiscountCurveIndex );

/* @brief			swig interface for aqFXObjectAsOfDateToSpot
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
                                  const std::string& termDiscountCurveIndex );
