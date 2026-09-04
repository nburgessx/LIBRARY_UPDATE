#pragma once
#include "CoreEnumerations.h"

namespace validation
{
    // Volatility Helper Methods
    // ----------------------------


    // Function to convert from lognormal to normal volatility using the deterministic drift freezing approach
    double tryAqMathVolatilityToNormalFromLognormal( const double & lognormalVol, const double underlyingRate );
    
    // Function to convert from normal to lognormal volatility using the deterministic drift freezing approach
    double tryAqMathVolatilityToLognormalFromNormal( const double & normalVol, const double underlyingRate );

    // Function to convert from lognormal volatility to shifted lognormal volatility using the deterministic drift freezing approach
    double tryAqMathVolatilityToShiftedLognormalFromLognormal( const double & lognormalVol, const double underlyingRate, const double shiftSize );

    // Function to convert from shifted lognormal volatility to lognormal volatility using the deterministic drift freezing approach
    double tryAqMathVolatilityToLognormalFromShiftedLognormal( const double & shiftedLognormalVol, const double underlyingRate, const double shiftSize );

    // Function to convert from lognormal volatility to shifted lognormal volatility using the deterministic drift freezing approach
    double tryAqMathVolatilityToShiftedLognormalFromNormal( const double & normalVol, const double underlyingRate, const double shiftSize );

    // Function to convert from shifted lognormal volatility to normal volatility using the deterministic drift freezing approach
    double tryAqMathVolatilityToNormalFromShiftedLognormal( const double & shiftedLognormalVol, const double underlyingRate, const double shiftSize );


    // Convexity Adjustment Methods
    // ------------------------------


    /* @brief			Function to calculate the libor in arrears convexity adjustment
                        *** This is a special case Libor Adjustment, where our Libor rate fixes in-arrears on the accrual end date ***
    *  @param [in]		liborRate			    The natural Libor rate in %
    *  @param [in]		couponYearFraction		Coupon Year Fraction
    *  @param [in]		timeToMaturity	        The time to maturity for Libor Coupon
    *  @param [in]		volatility              The volatility in %
    *  @param [in]		volatilityType          The volotility type: NORMAL, LOGNORMAL, SHIFTED_LOGNORMAL 
    *  @param [in]		volatilityShift         The volatility shift for SHIFTED_LOGNORMAL volatility only, defaults to zero
    *  @param [in]		useHullApproximation    Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjustment to the natural Libor rate in %
    */
    double tryAqMathLiborConvexityAdjustmentInArrears( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & volatility, const etrading::VolatilityTypeEnum & volatilityType, const double & volatilityShift = 0.0, const bool useHullApproximation = false );
    
    /* @brief			Function to calculate the libor in arrears convexity adjusted Libor Rate
                        *** This is a special case Libor Adjustment, where our Libor rate fixes in-arrears on the accrual end date ***
    *  @param [in]		liborRate			    The natural Libor rate in %
    *  @param [in]		couponYearFraction		Coupon Year Fraction
    *  @param [in]		timeToMaturity	        The time to maturity for Libor Coupon
    *  @param [in]		volatility              The volatility in %
    *  @param [in]		volatilityType          The volotility type: NORMAL, LOGNORMAL, SHIFTED_LOGNORMAL 
    *  @param [in]		volatilityShift         The volatility shift for SHIFTED_LOGNORMAL volatility only, defaults to zero
    *  @param [in]		useHullApproximation    Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjusted Libor rate in %
    */
    double tryAqMathLiborRateInArrears(const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & volatility, const etrading::VolatilityTypeEnum & volatilityType, const double & volatilityShift = 0.0, const bool useHullApproximation = false );

    /* @brief			Function to calculate the libor in arbitrary time convexity adjustment
    *  @param [in]		liborRate			            The natural Libor rate in %
    *  @param [in]		couponYearFraction		        Coupon Year Fraction
    *  @param [in]		timeToMaturity	                The time to maturity for Libor Coupon
    *  @param [in]		fixingDateYearFraction	        Fixing Year Fraction from Accrual Start to Fixing Date. More precisely from the natural fixing to the unnatural fixing. In AlgoQuantLib curves and swaps we have natural fixing date = accrual start.
    *  @param [in]		startDiscountFactor	            OIS Discount Factor on the Natural Libor Fixing Start Date. This is the Accrual Start Date when there is no fixing lag
    *  @param [in]		endDiscountFactor	            OIS Discount Factor on the Natural Libor Fixing End Date. This is the Accrual End Date when there is no fixing lag
    *  @param [in]		volatility                      The volatility in %
    *  @param [in]		volatilityType                  The volotility type: NORMAL, LOGNORMAL, SHIFTED_LOGNORMAL 
    *  @param [in]		volatilityShift                 The volatility shift for SHIFTED_LOGNORMAL volatility only, defaults to zero
    *  @param [in]		useHullApproximation            Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjustment to the natural Libor rate in %
    */
    double tryAqMathLiborConvexityAdjustmentForArbitraryFixingDate( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & fixingDateYearFraction, const double & startDiscountFactor, const double & endDiscountFactor, const double & volatility, const etrading::VolatilityTypeEnum & volatilityType, const double & volatilityShift = 0.0, const bool useHullApproximation = false );
    
    /* @brief			Function to calculate the libor in arbitrary time convexity adjusted Libor Rate
    *  @param [in]		liborRate			            The natural Libor rate in %
    *  @param [in]		couponYearFraction		        Coupon Year Fraction
    *  @param [in]		timeToMaturity	                The time to maturity for Libor Coupon
    *  @param [in]		fixingDateYearFraction	        Fixing Year Fraction from Accrual Start to Fixing Date. More precisely from the natural fixing to the unnatural fixing. In AlgoQuantLib curves and swaps we have natural fixing date = accrual start.
    *  @param [in]		startDiscountFactor	            OIS Discount Factor on the Natural Libor Fixing Start Date. This is the Accrual Start Date when there is no fixing lag
    *  @param [in]		endDiscountFactor	            OIS Discount Factor on the Natural Libor Fixing End Date. This is the Accrual End Date when there is no fixing lag
    *  @param [in]		volatility                      The volatility in %
    *  @param [in]		volatilityType                  The volotility type: NORMAL, LOGNORMAL, SHIFTED_LOGNORMAL 
    *  @param [in]		volatilityShift                 The volatility shift for SHIFTED_LOGNORMAL volatility only, defaults to zero
    *  @param [in]		useHullApproximation            Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjusted Libor rate in %
    */
    double tryAqMathLiborRateForArbitraryFixingDate(const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & fixingDateYearFraction, const double & startDiscountFactor, const double & endDiscountFactor, const double & volatility, const etrading::VolatilityTypeEnum & volatilityType, const double & volatilityShift = 0.0, const bool useHullApproximation = false );
}
