/*
 * @brief			Class which defines the Zero Coupon Inflation Swap product
 * @Created:		21 May 2020
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>
#include "Swap.h"
#include "InflationLeg.h"


namespace etrading
{
    class ZeroCouponInflationSwap : public Swap 
    {
	public:
		ZeroCouponInflationSwap( const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB );
   		ZeroCouponInflationSwap( const ZeroCouponInflationSwap& rhs );

		virtual ~ZeroCouponInflationSwap() {}
        
        std::shared_ptr<Swap> clone();

		/* @brief	Calculates the total PV of all of the Zero Coupon Inflation Swap legs.
		*			This simple PV calculation requires the user to specify the inflation fixings at start and end of the trade
		*
		* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
		* @param[in]	baseIndex				The inflation level at the effective date of the swap
		* @param[in]	resetIndex				The inflation level at the maturity of the swap
		* @param[in]	legName					Optionally calculate the PV of the specified leg only.
		* @returns	The calculated PV value
		*/
		double pvFromInflationIndex( const LabelValueBlock& valuationSettingsLVB, const double baseIndex, const double resetIndex, const std::string& legName = "" ) const;

		/* @brief	Calculates the total PV of all of the Zero Coupon Inflation Swap legs.
		*			This version obtains the inflation resets from a supplied InflationCurve.
		*
		* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
		* @param[in]	inflationCurve			InflationCurve used to obtain inflation index reset values
		* @param[in]	legName					Optionally calculate the PV of the specified leg only.
		* @returns	The calculated PV value
		*/
		double pvFromInflationCurve( const LabelValueBlock& valuationSettingsLVB, const InflationCurve& inflationCurve, const std::string& legName = "" ) const;

		/* @brief	Calculates the par rate of the Zero Coupon Inflation Swap
		*			i.e. the break-even coupon rate of the fixed leg which causes the swap to PV to zero.
		*			This simple calculation requires the user to specify the inflation fixings at start and end of the trade
		*
		* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
		* @param[in]	baseIndex				The inflation level at the effective date of the swap
		* @param[in]	resetIndex				The inflation level at the maturity of the swap
		* @returns	The calculated par rate value
		*/
		double parRateFromInflationIndex( const LabelValueBlock& valuationSettingsLVB, const double baseIndex, const double resetIndex ) const;

		/* @brief	Calculates the par rate of the Zero Coupon Inflation Swap
		*			i.e. the break-even coupon rate of the fixed leg which causes the swap to PV to zero.
		*			This simple calculation requires the user to specify the inflation fixings at start and end of the trade
		*
		* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
		* @param[in]	inflationCurve			InflationCurve used to obtain inflation index reset values
		* @returns	The calculated par rate value
		*/
		double parRateFromInflationCurve( const LabelValueBlock& valuationSettingsLVB, const InflationCurve& inflationCurve ) const;

		/* @brief Solves for the inflation index which prices the swap at par
		*         NOTE: This modifies the inflation index within the Inflation Curve. Used in calibration.
		*
		* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
		* @param[inout]	inflationCurve			The calibrated inflation curve
		* @returns	The implied inflation index to make swap price at par
		*/
		double impliedInflationIndexAtPar( const LabelValueBlock& valuationSettingsLVB,  InflationCurve& inflationCurve ) const;

		/* @brief	Helper function: identify which of the swap's legs is the Fixed Leg and which is the Inflation Leg
		*			Throws an exception if there are not exactly 2 legs in the swap
		*
		*  @param[out]	fixedLeg		The identified fixed leg of the swap
		*  @param[out]	inflationLeg	The identified inflation leg of the swap
		*/
		void identifyLegs( std::shared_ptr<FixedLeg>& fixedLeg, std::shared_ptr<InflationLeg>& inflationLeg ) const;

	private:
		/* @brief	Internal implementation method for par-rate calculation
		*			i.e. the break-even coupon rate of the fixed leg which causes the swap to PV to zero.
		*
		* @param[in]	inflationLegPV			PV of the inflation leg
		* @param[in]	fixedLeg				The fixed leg of the swap.
		* @param[in]	fixedLegDataProvider	Holds discount factors used to PV the coupons of the fixed leg
		* @returns	The calculated par rate value
		*/
		double parRate_impl( const double inflationLegPV, const std::shared_ptr<FixedLeg>& fixedLeg, DataProvider& fixedLegDataProvider ) const;



	};

}

