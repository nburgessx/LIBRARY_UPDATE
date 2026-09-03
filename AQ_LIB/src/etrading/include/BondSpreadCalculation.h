#pragma once

#include "LabelValueBlock.h"
#include "Bond.h"
#include "BondEnumerations.h"

#include <string>
#include <vector>

namespace etrading
{

	/* @brief			function to calculate the Bond Z-Spread from discount factor input.
	*                   This function accepts zeroRates as input to allow us to test the result against different systems.
	*  @param [in]		bondDirtyPrice		Bond Dirty Price
	*  @param [in]		bond				Pointer to the Bond Object
	*  @param [in]		zeroRates			External ZeroRates
	*  @param [in]		settleDate	        Bond Settlement Date
	*  @param [in]		continuouslyCompounding	        True to calculate z-spread using continouslyCompounding
	*  @return			returns the Z-Spread
	*/
	double calculateBondZSpread(const double& bondDirtyPrice, const std::shared_ptr< Bond >& bond, const DoubleVector& zeroRates, const AQLDate& settleDate, const bool& continuouslyCompounding);

	/* @brief			function to calculate the Bond Z-Spread
	*  @param [in]		bondDirtyPrice		Bond Dirty Price
	*  @param [in]		bond			    Pointer to the Bond Object
	*  @param [in]		curveCollection	    The name of the curve collection set
	*  @param [in]		forecastCurve       The forecast curve name
	*  @param [in]		settleDate	        Bond Settlement Date
	*  @param [in]		continuouslyCompounding	        True to calculate z-spread using continouslyCompounding
	*  @return			returns the Z-Spread
	*/
	double calculateBondZSpread(const double& bondDirtyPrice, const std::shared_ptr< Bond >& bond, const std::string& curveCollection, const std::string& forecastCurve, const AQLDate& settleDate, const bool& continuouslyCompounding);

	double calculateAssetSwapSpreadParParFromBondCleanPrice(const double& bondCleanPrice, const LegPtr& fixedLeg, const LegPtr& floatLeg, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames);

	double calculateBondCleanPriceFromAssetSwapSpread(const double& assetSwapParParSpread, const SwapPtr& swap, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames);

	// Calculate the Floating Bond asset swap's FixedEqvCoupon - the swap fixed leg's swap rate so that the swap PV (excluding accrued interest) matches the bond's parParAdjustment
	double calculateFloatingBondAssetSwapFixedEqvCoupon(const double& bondCleanPrice, const LegPtr& fixedLeg, const LegPtr& floatLeg, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames);

	// Calculate the Floating Bond asset swap spread based on the given FixedEqvCoupon
	double calculateFloatingBondAssetSwapSpreadFromFixedEqvCoupon(const double& fixedRate, const LegPtr& fixedLeg, const LegPtr& floatLeg, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames);

	// Calculate the Floating Bond asset swap spread
	double calculateFloatingBondAssetSwapSpread(const double& bondCleanPrice, const LegPtr& refLeg, const LegPtr& origSpreadLeg, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames);

	// Helper function to solve the spread of the float leg, given target SwapPV
	double calculateFloatSpreadFromTargetSwapPV(DataProvider& dataProviderRefLeg, DataProvider& dataProviderSpreadLeg, const LegPtr& refLeg, const LegPtr& spreadLeg, const double& targetSwapPV);

	// Helper function to solve the fixed rate of the swap fixed leg, so that swap PV can match the targetSwapPV
	double calculateSwapRateFromTargetSwapPV(DataProvider& dataProviderFixedLeg, DataProvider& dataProviderFloatLeg, const LegPtr& fixedLeg, const LegPtr& floatLeg, const double& targetSwapPV);

	//Helper function to solve either fixed rate or float spread so that the swap PV match the targetSwapPV
	double calculateSwapPvByChangingSpread(DataProvider& spreadDataProvider, const double& refLegPV, const LegPtr& spreadLeg, const double& spread);

	// Calculate the Floating Bond asset swap's FixedEqvCoupon from AssetSwapSpread - by solving the swap fixed leg's swap rate so that the swap PV zero, given the ASWSpread in the float leg's spread
	double calculateFixedEqvCouponFromFloatingBondAssetSwapSpread(const double& assetSwapParParSpread, const SwapPtr& origSwap, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames);

	// Calculate the Floating Bond Clean Price from AssetSwapSpread - by solving the swap fixed leg's swap rate so that the swap PV matches parParAjustment, given the float leg's spread as quotedMargin
	double calculateBondCleanPriceFromFloatingBondAssetSwapSpread(const double& assetSwapParParSpread, const SwapPtr& origSwap, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames);

}

