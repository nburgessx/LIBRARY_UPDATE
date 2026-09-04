#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Swap.h"

using etrading::LabelValueBlock;

namespace validation
{

	/* @brief			validation interface for the aqObjectsAssetSwapParParFromCleanPrice method. Calculate the Par/Par Asset Swap Spreads.
	*  @param [in]		bondPrices			Bond prices
	*  @param [in]		bondAccrualStartDates	Bond Accrual Start Dates
	*  @param [in]		isCleanPrice		Bond price is clean or dirty
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		valuationSettingsLVB   A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Par/Par Asset Swap Spreads
	*/
	std::vector< double > tryAqObjectsAssetSwapSpreadFromPrice(	const std::vector< double >& bondPrices, const std::vector< AQLDate >& bondAccrualStartDates, const std::vector< bool >& isCleanPrices,
															const std::string& swapObjectName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames = LabelValueBlock());
  
	/* @brief			Helper interface for the tryAqObjectsAssetSwapSpreadFromPrice method. Calculate the Par/Par Asset Swap Spread.
	*  @param [in]		bondAccrualStartDate	Bond Accrual Start Date
	*  @param [in]		isCleanPrice		Bond price is clean or dirty
	*  @param [in]		bondPrice			Bond price
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		valuationSettingsLVB   A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Par/Par Asset Swap Spread
	*/
	double tryAqObjectsAssetSwapSpreadFromPrice(const double& bondPrice, const AQLDate& bondAccrualStartDate, const bool& isCleanPrice,
											const std::string& swapObjectName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames = LabelValueBlock());

	/* @brief			validation interface for the aqObjectsAssetSwapSpread method. Calculate the Par/Par Asset Swap Spreads.
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		bondPrice		    Bond price
	*  @param [in]		valuationSettingsLVB   A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		fixingTableNames	Fixing table object names
    *  @return			Par/Par Asset Swap Spreads
	*/
    std::vector< double > tryAqObjectsAssetSwapSpread( const std::string& swapObjectName, const std::string& bondObjectName, const std::vector< double >& bondPrices, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames=LabelValueBlock() );

	/* @brief			Helper interface for the tryAqObjectsAssetSwapSpread method. Calculate the Par/Par Asset Swap Spread.
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		bondPrice		    Bond price
	*  @param [in]		valuationSettingsLVB   A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		fixingTableNames	Fixing table object names
    *  @return			Par/Par Asset Swap Spread
	*/
    double tryAqObjectsAssetSwapSpread( const std::string& swapObjectName, const std::string& bondObjectName, const double& bondPrice, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames=LabelValueBlock() );
  
	/* @brief			validation interface for the aqObjectsAssetSwapCleanPrice method. Calculate the Bond Clean Prices.
	*  @param [in]		swapObjectName		Swap object name
    *  @param [in]		assetSwapParParSpread	Asset Swap Par Par Spreads
	*  @param [in]		valuationSettingsLVB   A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		fixingTableNames	Fixing table object names
    *  @return			Par/Par Asset Swap Spreads
	*/
    std::vector< double > tryAqObjectsAssetSwapSpreadToCleanPrice( const std::string& swapObjectName, const std::vector< double >& assetSwapParParSpreads, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames = LabelValueBlock());

	/* @brief			Helper interface for the tryAqObjectsAssetSwapSpreadToCleanPrice method. Calculate the Bond Clean Prices.
	*  @param [in]		swapObjectName		Swap object name
    *  @param [in]		assetSwapParParSpread	Asset Swap Par Par Spread
	*  @param [in]		valuationSettingsLVB   A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		fixingTableNames	Fixing table object names
    *  @return			Par/Par Asset Swap Spread
	*/
    double tryAqObjectsAssetSwapSpreadToCleanPrice( const std::string& swapObjectName, const double& assetSwapParParSpread, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames = LabelValueBlock());


	/* @brief			validation interface for the aqObjectsAssetSwapFixedEqvCoupon method. Calculate the asset swap fixed leg's swap rate, so that the swap PV (excluding accrued interest) matches the bond's parParAdjustment
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		bondPrice		    Bond price
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableNames	Fixing table object names
	*  @return			AssetSwap's swap rate (Fixed Equivalent Coupon)
	*/
	double tryAqObjectsAssetSwapFixedEqvCoupon(const std::string& swapObjectName, const std::string& bondObjectName, const double& bondPrice, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames = LabelValueBlock());


	/* @brief			validation interface for the aqObjectsAssetSwapSpreadFromFixedEqvCoupon method. Calculate the asset swap spread based on the FixedEqvCoupon
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		fixedEqvCoupon		Asset Swap's fixed equivalent coupon (fixed rate that make the swap PV as parParAdjustment)
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Floating Bond Asset Swap Spread based on FixedEqvCoupon
	*/
	double tryAqObjectsAssetSwapSpreadFromFixedEqvCoupon(const std::string& swapObjectName, const double& fixedEqvCoupon, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames = LabelValueBlock());

}

