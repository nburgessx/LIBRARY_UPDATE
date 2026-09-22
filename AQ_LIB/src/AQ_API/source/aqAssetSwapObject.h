// aqAssetSwapObject.h

/*
 * @brief			Swig interface for aqAssetSwapObject... functions
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqAssetSwapObjectSpread. Calculate the Par/Par Asset Swap Spread.
*  @param [in]		swapObjectName		Swap object name
*  @param [in]		bondObjectName		Bond object name
*  @param [in]		bondPrice			Bond price
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		fixingTableNames	Optional. Fixing-table name overrides as a label/value block
*  @return			Par/Par Asset Swap Spread
*/
double aqAssetSwapObjectSpread( const std::string& swapObjectName,
                                 const std::string& bondObjectName,
                                 const double bondPrice,
                                 const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                 const std::vector<std::vector<std::string> >& fixingTableNames = std::vector<std::vector<std::string> >() );

/* @brief			swig interface for aqAssetSwapObjectSpreadFromPrice. Calculate the Par/Par Asset Swap Spread implied by a bond price.
*  @param [in]		bondPrice			The bond price
*  @param [in]		bondAccrualStartDate	Bond Accrual Start Date
*  @param [in]		isCleanPrice		Bond price is clean or dirty
*  @param [in]		swapObjectName		Swap object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		fixingTableNames	Optional. Fixing-table name overrides as a label/value block
*  @return			Par/Par Asset Swap Spread
*/
double aqAssetSwapObjectSpreadFromPrice( const double bondPrice,
                                          const std::string& bondAccrualStartDate,
                                          const bool isCleanPrice,
                                          const std::string& swapObjectName,
                                          const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                          const std::vector<std::vector<std::string> >& fixingTableNames = std::vector<std::vector<std::string> >() );

/* @brief			swig interface for aqAssetSwapObjectSpreadToCleanPrice. Calculate the Bond Clean Price implied by an asset-swap par-par spread.
*  @param [in]		swapObjectName			Swap object name
*  @param [in]		assetSwapParSpread		The par-par asset-swap spread
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		fixingTableNames		Optional. Fixing-table name overrides as a label/value block
*  @return			Bond Clean Price
*/
double aqAssetSwapObjectSpreadToCleanPrice( const std::string& swapObjectName,
                                             const double assetSwapParSpread,
                                             const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                             const std::vector<std::vector<std::string> >& fixingTableNames = std::vector<std::vector<std::string> >() );

/* @brief			swig interface for aqAssetSwapObjectFixedEqvCoupon. Calculate the asset swap fixed leg's swap rate, so that the swap PV (excluding accrued interest) matches the bond's parParAdjustment.
*  @param [in]		swapObjectName		Swap object name
*  @param [in]		bondObjectName		Bond object name
*  @param [in]		bondPrice			Bond price
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		fixingTableNames	Optional. Fixing-table name overrides as a label/value block
*  @return			AssetSwap's fixed equivalent coupon
*/
double aqAssetSwapObjectFixedEqvCoupon( const std::string& swapObjectName,
                                         const std::string& bondObjectName,
                                         const double bondPrice,
                                         const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                         const std::vector<std::vector<std::string> >& fixingTableNames = std::vector<std::vector<std::string> >() );

/* @brief			swig interface for aqAssetSwapObjectSpreadFromFixedEqvCoupon. Calculate the asset swap spread based on the FixedEqvCoupon.
*  @param [in]		swapObjectName		Swap object name
*  @param [in]		fixedEqvCoupon		Asset Swap's fixed equivalent coupon (fixed rate that makes the swap PV equal to parParAdjustment)
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		fixingTableNames	Optional. Fixing-table name overrides as a label/value block
*  @return			Floating Bond Asset Swap Spread based on FixedEqvCoupon
*/
double aqAssetSwapObjectSpreadFromFixedEqvCoupon( const std::string& swapObjectName,
                                                   const double fixedEqvCoupon,
                                                   const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                                   const std::vector<std::vector<std::string> >& fixingTableNames = std::vector<std::vector<std::string> >() );
