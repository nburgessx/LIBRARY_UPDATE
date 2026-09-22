// aqInflationObject.h

/*
 * @brief			Swig interface for aqInflationCurve... / aqInflationObject... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqInflationCurveCreate
*  @param [in]		inflationCurveName	Name for the inflation curve object
*  @param [in]		key1				Name of the first data block
*  @param [in]		value1				First data block, as a key/value matrix
*  @param [in]		key2				Optional. Name of the second data block
*  @param [in]		value2				Optional. Second data block, as a key/value matrix
*  @return			Inflation curve object handle
*/
std::string aqInflationCurveCreate( const std::string& inflationCurveName,
                                     const std::string& key1,
                                     const SWIG_STRINGMATRIX& value1,
                                     const std::string& key2 = std::string(),
                                     const SWIG_STRINGMATRIX& value2 = SWIG_STRINGMATRIX() );

/* @brief			swig interface for aqInflationCurveCalibrationParameters
*  @param [in]		inflationCurveName	Inflation curve object name
*  @return			A matrix containing node dates and calibrated index levels
*/
SWIG_STRINGMATRIX aqInflationCurveCalibrationParameters( const std::string& inflationCurveName );

/* @brief			swig interface for aqInflationObjectCPI
*  @param [in]		inflationCurveName	Inflation curve object name
*  @param [in]		date				Return the CPI for this date
*  @param [in]		inflationResetType	Reset convention: MonthlyInterpolation or DailyInterpolation
*  @param [in]		lag					Adjust the specified date backwards by this lag tenor
*  @return			The calculated CPI level
*/
double aqInflationObjectCPI( const std::string& inflationCurveName,
                              const std::string& date,
                              const std::string& inflationResetType,
                              const std::string& lag );

/* @brief			swig interface for aqInflationObjectZCSwapPV
*  @param [in]		swapName			Zero coupon inflation swap object name
*  @param [in]		inflationCurveName	Inflation curve object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		legName				If specified, calculate the PV of the single swap leg
*  @return			Present value of the zero coupon inflation swap
*/
double aqInflationObjectZCSwapPV( const std::string& swapName,
                                   const std::string& inflationCurveName,
                                   const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                   const std::string& legName );

/* @brief			swig interface for aqInflationObjectZCSwapPVFromIndex
*  @param [in]		swapName			Zero coupon inflation swap object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		baseIndex			The inflation level at the effective date of the swap
*  @param [in]		resetIndex			The inflation level at the maturity of the swap
*  @param [in]		legName				If specified, calculate the PV of the single swap leg
*  @return			Present value of the zero coupon inflation swap
*/
double aqInflationObjectZCSwapPVFromIndex( const std::string& swapName,
                                            const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                            const double baseIndex,
                                            const double resetIndex,
                                            const std::string& legName );

/* @brief			swig interface for aqInflationObjectZCSwapParRate
*  @param [in]		swapName			Zero coupon inflation swap object name
*  @param [in]		inflationCurveName	Inflation curve object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @return			Break-even par rate of the zero coupon inflation swap
*/
double aqInflationObjectZCSwapParRate( const std::string& swapName,
                                        const std::string& inflationCurveName,
                                        const std::vector<std::vector<std::string> >& valuationSettingsLVB );

/* @brief			swig interface for aqInflationObjectZCSwapParRateFromIndex
*  @param [in]		swapName			Zero coupon inflation swap object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		baseIndex			The inflation level at the effective date of the swap
*  @param [in]		resetIndex			The inflation level at the maturity of the swap
*  @return			Break-even par rate of the zero coupon inflation swap
*/
double aqInflationObjectZCSwapParRateFromIndex( const std::string& swapName,
                                                 const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                                 const double baseIndex,
                                                 const double resetIndex );
