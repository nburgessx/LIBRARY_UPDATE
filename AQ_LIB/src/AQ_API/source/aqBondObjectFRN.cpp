// aqBondObjectFRN.cpp

/*
 * @brief			Swig interface for aqBondObjectFRN... functions - floating rate note discount margin / price / yield conversions
 */

#include "aqBondObjectFRN.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqBondObject.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqBondObjectFRNPriceFromDiscountMargin function
*  @param [in]		bondObjectName				The name of the cached floating bond object
*  @param [in]		settlementDate				Settlement date
*  @param [in]		discountMargin				Additional discount rate for projected coupons
*  @param [in]		assumedRate					Assumed index rate used for projected coupons
*  @param [in]		indexToNextCoupon			The index discount rate for the next coupon period
*  @param [in]		annualizedNextCouponRate	The rate of the next coupon, already fixed
*  @param [out]		Returns the clean/dirty floating bond price
*/
double aqBondObjectFRNPriceFromDiscountMargin( const std::string& bondObjectName,
                                            const std::string& settlementDate,
                                            const double& discountMargin,
                                            const double& assumedRate,
                                            const double& indexToNextCoupon,
                                            const double& annualizedNextCouponRate )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settlementDate_( etrading::stringToDate( settlementDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectFRNPriceFromDiscountMargin( bondObjectName, settlementDate_, discountMargin, assumedRate, indexToNextCoupon, annualizedNextCouponRate );
    return result;

    AQ_API_END
}

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
                                   const double& annualizedNextCouponRate )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settlementDate_( etrading::stringToDate( settlementDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectFRNPriceFromYield( bondObjectName, settlementDate_, yield, assumedRate, indexToNextCoupon, annualizedNextCouponRate );
    return result;

    AQ_API_END
}

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
                                   const double& annualizedNextCouponRate )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settlementDate_( etrading::stringToDate( settlementDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectFRNYieldFromPrice( bondObjectName, settlementDate_, price, assumedRate, indexToNextCoupon, annualizedNextCouponRate );
    return result;

    AQ_API_END
}

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
                                            const double& annualizedNextCouponRate )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settlementDate_( etrading::stringToDate( settlementDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectFRNDiscountMarginFromPrice( bondObjectName, settlementDate_, price, assumedRate, indexToNextCoupon, annualizedNextCouponRate );
    return result;

    AQ_API_END
}
