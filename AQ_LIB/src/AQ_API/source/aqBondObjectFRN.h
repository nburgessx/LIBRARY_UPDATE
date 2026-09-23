// aqBondObjectFRN.h

/*
 * @brief			Swig interface for aqBondObjectFRN... functions - floating rate note discount margin / price / yield conversions,
 *					computed without requiring discount and forecast curves (see BBG CalcType 21; Stigum and Robinson,
 *					"Money Market and Bond Calculations" pp259-271)
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqBondObjectFRNPriceFromDiscountMargin function
*  @param [in]		bondObjectName				The name of the cached floating bond object
*  @param [in]		settlementDate				Settlement date
*  @param [in]		discountMargin				Additional discount rate for projected coupons
*  @param [in]		assumedRate					Assumed index rate used for projected coupons
*  @param [in]		indexToNextCoupon			The index discount rate for the next coupon period, i.e. the Libor rate from settlement date to next coupon
*  @param [in]		annualizedNextCouponRate	The rate of the next coupon, already fixed
*  @param [out]		Returns the clean/dirty floating bond price
*/
double aqBondObjectFRNPriceFromDiscountMargin( const std::string& bondObjectName,
                                            const std::string& settlementDate,
                                            const double& discountMargin,
                                            const double& assumedRate,
                                            const double& indexToNextCoupon,
                                            const double& annualizedNextCouponRate );

/* @brief			swig interface for aqBondObjectFRNPriceFromYield function
*  @param [in]		bondObjectName				The name of the cached floating bond object
*  @param [in]		settlementDate				Settlement date
*  @param [in]		yield						The bond yield to maturity
*  @param [in]		assumedRate					Assumed index rate used for projected coupons
*  @param [in]		indexToNextCoupon			The index discount rate for the next coupon period
*  @param [in]		annualizedNextCouponRate	The rate of the next coupon, already fixed
*  @param [out]		Returns the clean/dirty floating bond price
*/
double aqBondObjectFRNPriceFromYield( const std::string& bondObjectName,
                                   const std::string& settlementDate,
                                   const double& yield,
                                   const double& assumedRate,
                                   const double& indexToNextCoupon,
                                   const double& annualizedNextCouponRate );

/* @brief			swig interface for aqBondObjectFRNYieldFromPrice function
*  @param [in]		bondObjectName				The name of the cached floating bond object
*  @param [in]		settlementDate				Settlement date
*  @param [in]		price						The bond price, in the quoted convention (clean/dirty)
*  @param [in]		assumedRate					Assumed index rate used for projected coupons
*  @param [in]		indexToNextCoupon			The index discount rate for the next coupon period
*  @param [in]		annualizedNextCouponRate	The rate of the next coupon, already fixed
*  @param [out]		Returns the bond yield
*/
double aqBondObjectFRNYieldFromPrice( const std::string& bondObjectName,
                                   const std::string& settlementDate,
                                   const double& price,
                                   const double& assumedRate,
                                   const double& indexToNextCoupon,
                                   const double& annualizedNextCouponRate );

/* @brief			swig interface for aqBondObjectFRNDiscountMarginFromPrice function
*  @param [in]		bondObjectName				The name of the cached floating bond object
*  @param [in]		settlementDate				Settlement date
*  @param [in]		price						The bond price, in the quoted convention (clean/dirty)
*  @param [in]		assumedRate					Assumed index rate used for projected coupons
*  @param [in]		indexToNextCoupon			The index discount rate for the next coupon period
*  @param [in]		annualizedNextCouponRate	The rate of the next coupon, already fixed
*  @param [out]		Returns the bond discount margin
*/
double aqBondObjectFRNDiscountMarginFromPrice( const std::string& bondObjectName,
                                            const std::string& settlementDate,
                                            const double& price,
                                            const double& assumedRate,
                                            const double& indexToNextCoupon,
                                            const double& annualizedNextCouponRate );
