/*
 * @brief			Vallidation API functions for Inflation
 * @Created:		22 May 2020
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Swap.h"
#include "JSONInfoBlock.h"

using etrading::LabelValueBlock;

namespace validation_api
{
	/* @brief Creates an Inflation Curve, calibrated from Zero Coupon Inflation Swap market data
	 * @param [in] objectName        The name of the Inflation Curve object to create
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	 *                               CURVE_PROPERTIES, ZC_INFLATIONSWAPS, SEASONALITY
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
	std::string tryMeLWOInflationCurveCreate( const std::string& objectName,
											  const std::vector<std::string>& dataBlockNames,
											  const etrading::JSONInfoBlockTuples& infoBlocks ); // TODO: Move the JSON Info Block below the validation_api - does not belong here

	/* @brief Obtains the calibration parameters from the specified inflation curve
	 * @param[in] inflationCurveName	Inflation Curve object name
	 * @param[out]						A matrix containing node dates and calibrated index levels.
	 */
	AnyTypeMatrix tryMeLWOInflationCurveCalibrationParameters( const std::string& inflationCurveName );


	/* @brief	validation interface for the neLWOInflationZCSwapPVFromIndex method.
	*			Calculates the PV of the specified Zero Coupon Inflation Swap.
	*			This function requires the user to provide the inflation fixing level at the start and end of the trade,
	*			taking into account the correct lag.
	*
	* @param[in]	swapName				Swap object name
	* @param[in]	valuationSettingsLVB	Contains the curveCollection, used for discounting cashflows
	* @param[in]	baseIndex				The inflation level at the effective date of the swap
	* @param[in]	resetIndex				The inflation level at the maturity of the swap
	* @param[in]	legName					If specified, calculate the PV of the single swap leg
	* @returns	The calculated PV value
	*/
	double tryMeLWOInflationZCSwapPVFromIndex( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double baseIndex, const double resetIndex, const std::string& legName );

	/* @brief	validation interface for the meLWOInflationZCSwapPV method.
	*			Calculates the PV of the specified Zero Coupon Inflation Swap.
	*			This function takes an inflation curve as input.
	*
	* @param[in]	swapName				Swap object name
	* @param[in]	inflationCurveName		Inflation Curve object name
	* @param[in]	valuationSettingsLVB	Contains the curveCollection, used for discounting cashflows
	* @param[in]	legName					If specified, calculate the PV of the single swap leg
	* @returns	The calculated PV value
	*/
	double tryMeLWOInflationZCSwapPV( const std::string& swapName, const std::string& inflationCurveName, const LabelValueBlock& valuationSettingsLVB, const std::string& legName );

	/* @brief	validation interface for the meLWOInflationZCSwapParRateFromIndex method.
	*			Calculates the par rate of the Zero Coupon Inflation Swap
	*			i.e. the coupon rate of the fixed leg which causes the swap to PV to zero.
	*			This simple calculation requires the user to specify the inflation fixings at start and end of the trade
	*
	* @param[in]	swapName				Swap object name
	* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
	* @param[in]	baseIndex				The inflation level at the effective date of the swap
	* @param[in]	resetIndex				The inflation level at the maturity of the swap
	* @returns	The calculated break-even par-rate value
	*/
	double tryMeLWOInflationZCSwapParRateFromIndex( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double baseIndex, const double resetIndex );

	/* @brief	validation interface for the meLWOInflationZCSwapParRate method.
	*			Calculates the Break-even par rate of the specified Zero Coupon Inflation Swap.
	*			This function takes an inflation curve as input
	*
	* @param[in]	swapName				Swap object name
	* @param[in]	inflationCurveName		Inflation Curve object name
	* @param[in]	valuationSettingsLVB	Contains the curveCollection, used for discounting cashflows
	* @returns	The calculated break-even par-rate value
	*/
	double tryMeLWOInflationZCSwapParRate( const std::string& swapName, const std::string& inflationCurveName, const LabelValueBlock& valuationSettingsLVB );

	/* @brief	validation interface for the meLWOInflationCPI method.
	*			Obtains the CPI level from the suuplied inflation curve for the specified date, taking into account the specified lag
	*
	* @param[in]	inflationCurveName		Inflation Curve object name
	* @param[in]	date					Return the CPI for this date
	* @param[in]	inflationResetType		Specifies how the lag should be taken into account: MonthlyInterpolation or DailyInterpolation
	* @param[in]	lag						Adjust the specified date backwards by this lag tenor
	* @returns	The calculated CPI level
	*/
	double tryMeLWOInflationCPI( const std::string& inflationCurveName, const LADate& date, const std::string& inflationResetType, const std::string& lag );


}
