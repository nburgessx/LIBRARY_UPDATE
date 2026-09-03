// SettingsValidation.h

/*
 * @brief			Settings Validation Helper Methods
 * @Created:		6th November 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LADataInstance.h"
#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"
#include "CoreEnumerations.h"

using etrading::LabelValueBlock;

namespace etrading
{
    /* @brief			Get CurveCollection from the valuationSettingsLVB map, by matching the legName 
    *                   *** Supports Curve Objects and the Older Non-Curve Object Types ***
    *
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @output			The matched curveCollection
	*/
	LAString getLWOCurveCollectionFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const LAString legName = "" );

    /* @brief			Get CurveCollection from the valuationSettingsLVB map, by matching the legName 
    *                   *** Supports Curve Objects and the Older Non-Curve Object Types ***
    *
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @output			The matched curveCollection
	*/
	LAString getValidatedLWOCurveCollectionFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const LAString legName = "" );

    /* @brief			Get CurveCollection from the valuationSettingsLVB map, by matching the legName
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @output			The matched curveCollection
	*/
	LAString getOptionalCurveCollectionFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const LAString legName = "" );

	/* @brief			Get CurveCollection from the valuationSettingsLVB map, by matching the legName
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @param [in]		throwIfMissing	    Boolean throw if curve collection is missing True or False
    * @output			The matched curveCollection
	*/
	LAString getCurveCollectionFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const LAString legName = "", const bool throwIfMissing = true );
    
    /* @brief			Get CurveCollection from the valuationSettingsLVB map, by matching the legName and curve asOf date
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @param [in]		throwIfMissing	    Boolean throw if curve collection is missing True or False
    * @output			The matched curveCollection
	*/
	LAString getValidatedCurveCollectionFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const LAString legName = "", const bool throwIfMissing = true );

    /* @brief			Get CurveObject from the valuationSettingsLVB map, by matching the legName
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
    * @output			The yield curve object
	*/
	LAString getCurveObjectFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const LAString legName = "" );

    /* @brief			Get VolatilityModel from the valuationSettingsLVB map, by matching the legName
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @param [in]		throwIfMissing	    Boolean throw if curve collection is missing True or False
	* @output			The volatility model
	*/
	LAString getVolatilityModelFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const LAString legName = "", const bool throwIfMissing = true);

    /* @brief			Get VolatilityModel from the valuationSettingsLVB map, by matching the legName
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @output			The credit model
	*/
	LAString getCreditModelFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const LAString legName = "" );

    /* @brief			Get FXSot from Valuation Settings map
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
    * @param [in]		throwIfMissing	    If TRUE throw an error if the key is missing, otherwise if FALSE return an empty string
	* @output			The credit model
	*/
	double getFXSpotFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing = false  );

	/* @brief			Get FX on as of date from Valuation Settings map
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName					legName, required if curve collection set id
	* @param [in]		throwIfMissing			If TRUE throw an error if the key is missing, otherwise if FALSE return an empty string
	* @output			As of date fx rate
	*/
	double getFXAsOfDateFromValuationSettings(const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing = false);


    /* @brief			Get ValuationDate from the valuationSettingsLVB map
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @output			The ValuationDate
	*/
	LADate getValuationDateFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing = true);

	/* @brief			Get bond SettlementDate from the valuationSettingsLVB map
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @output			The SettlementDate
	*/
	LADate getSettlementDateFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing = false );

	/* @brief			Get the Float-Bond Current CouponRate from the valuationSettingsLVB map. This is annualized coupon rate for
	*					the very next coupon which has already fixed.
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		throwIfMissing	    If TRUE throw an error if the key is missing, otherwise if FALSE return NaN
	* @output			The annualized coupon rate
	*/
	double getFloatBondCurrentCouponRateFromValuationSettings(const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing = false );

	/* @brief			Get te Float-Bond AssumedRate from the valuationSettingsLVB map. This is the assumed average index rate for projected coupons
	*					If data is missing and throwIfMissing is false, returns NaN
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		throwIfMissing	    If TRUE throw an error if the key is missing, otherwise if FALSE return NaN
	* @output			The assumed rate
	*/
	double getFloatBondAssumedRateFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing = false );

	/* @brief			Get Float-Bond Index-to-next-fix from the valuationSettingsLVB map. This is the underlying reference index rate for the current coupon interval.
	*					If data is missing and throwIfMissing is false, returns NaN
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		throwIfMissing	    If TRUE throw an error if the key is missing, otherwise if FALSE return NaN
	* @output			The index-to-next-fix
	*/
	double getFloatBondIndexToNextCouponFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing = false );

	/* @brief			Get Float-Bond quoted-margin from the valuationSettingsLVB map. This is the quoted spread of the bond coupons over the reference index level.
	*					If data is missing and throwIfMissing is false, returns NaN
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		throwIfMissing	    If TRUE throw an error if the key is missing, otherwise if FALSE return NaN
	* @output			The quoted-margin
	*/
	double getFloatBondQuotedMarginFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing = false );

	/* @brief			Get Key from the valuationSettingsLVB map, by matching the legName
	* @param [in]		searchKey               Key to search for in Valuation Settings Map
    * @param [in]		valuationSettingsLVB	    ValuationSettings map
	* @param [in]		legName	                legName, required if curve collection set id
    * @param [in]		throwIfMissing	        If TRUE throw an error if the key is missing, otherwise if FALSE return an empty string
	* @output			The value corresponding to the searchKey
	*/
	LAString getKeyFromValuationSettings( const std::string& searchKey, const LabelValueBlock& valuationSettingsLVB, LAString legName = "", const bool throwIfMissing = true );

	/* @brief			get fixingtable name from the fixingTableNames map, by matching the legname
	* @param [in]		fixingTableNames	fixing table LVB
	* @param [in]		legName	        ID of the leg
	* @param [in]		legType	        Type of the leg, e.g. FIXED, FLOAT, FEE
	* @output			the matched fixingtable name
	*/
	std::string findFixingTableName(const LabelValueBlock& fixingTableNames, const std::string& legName, const ScheduleTypeEnum& legType);


}