// aqMathConvexity.h

/*
 * @brief			Swig interface for the aqMathLibor* convexity-adjustment functions
 */

#pragma once

#include <string>

/* @brief			swig interface for aqMathLiborConvexityAdjustmentInArrears.
*                   *** This is a special case Libor Adjustment, where our Libor rate fixes in-arrears on the accrual end date ***
*  @param [in]		liborRate				The natural Libor rate in %
*  @param [in]		couponYearFraction		Coupon Year Fraction
*  @param [in]		timeToMaturity			The time to maturity for the Libor coupon
*  @param [in]		volatility				The volatility in %
*  @param [in]		volatilityType			The volatility type: NORMAL, LOGNORMAL, SHIFTED_LOGNORMAL
*  @param [in]		volatilityShift			Optional. The volatility shift for SHIFTED_LOGNORMAL volatility only, defaults to zero
*  @param [in]		useHullApproximation	Optional. Default FALSE. Use Hull's approximation: in the lognormal case exp(x) is approximated as (1+x)
*  @return			The convexity adjustment to the natural Libor rate in %
*/
double aqMathLiborConvexityAdjustmentInArrears( const double liborRate,
                                                 const double couponYearFraction,
                                                 const double timeToMaturity,
                                                 const double volatility,
                                                 const std::string& volatilityType,
                                                 const double volatilityShift = 0.0,
                                                 const bool useHullApproximation = false );

/* @brief			swig interface for aqMathLiborRateInArrears - the convexity-adjusted Libor-in-arrears rate
*  @return			The convexity adjusted Libor rate in %
*/
double aqMathLiborRateInArrears( const double liborRate,
                                  const double couponYearFraction,
                                  const double timeToMaturity,
                                  const double volatility,
                                  const std::string& volatilityType,
                                  const double volatilityShift = 0.0,
                                  const bool useHullApproximation = false );

/* @brief			swig interface for aqMathLiborConvexityAdjustmentForArbitraryFixingDate
*  @param [in]		liborRate				The natural Libor rate in %
*  @param [in]		couponYearFraction		Coupon Year Fraction
*  @param [in]		timeToMaturity			The time to maturity for the Libor coupon
*  @param [in]		fixingDateYearFraction	Fixing year fraction from accrual start to fixing date
*  @param [in]		startDiscountFactor		OIS discount factor on the natural Libor fixing start date
*  @param [in]		endDiscountFactor		OIS discount factor on the natural Libor fixing end date
*  @param [in]		volatility				The volatility in %
*  @param [in]		volatilityType			The volatility type: NORMAL, LOGNORMAL, SHIFTED_LOGNORMAL
*  @param [in]		volatilityShift			Optional. The volatility shift for SHIFTED_LOGNORMAL volatility only, defaults to zero
*  @param [in]		useHullApproximation	Optional. Default FALSE. Use Hull's approximation
*  @return			The convexity adjustment to the natural Libor rate in %
*/
double aqMathLiborConvexityAdjustmentForArbitraryFixingDate( const double liborRate,
                                                               const double couponYearFraction,
                                                               const double timeToMaturity,
                                                               const double fixingDateYearFraction,
                                                               const double startDiscountFactor,
                                                               const double endDiscountFactor,
                                                               const double volatility,
                                                               const std::string& volatilityType,
                                                               const double volatilityShift = 0.0,
                                                               const bool useHullApproximation = false );

/* @brief			swig interface for aqMathLiborRateForArbitraryFixingDate - the convexity-adjusted Libor rate for an arbitrary fixing date
*  @return			The convexity adjusted Libor rate in %
*/
double aqMathLiborRateForArbitraryFixingDate( const double liborRate,
                                               const double couponYearFraction,
                                               const double timeToMaturity,
                                               const double fixingDateYearFraction,
                                               const double startDiscountFactor,
                                               const double endDiscountFactor,
                                               const double volatility,
                                               const std::string& volatilityType,
                                               const double volatilityShift = 0.0,
                                               const bool useHullApproximation = false );
