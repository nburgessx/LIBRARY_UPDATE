// aqMathVolatilityConversion.h

/*
 * @brief			Swig interface for the aqMathVolatilityTo* volatility-type conversion functions
 */

#pragma once

/* @brief			swig interface for aqMathVolatilityToNormalFromLognormal. Convert a lognormal volatility to a normal (basis-point) volatility using the deterministic drift-freezing approach
*  @param [in]		lognormalVol	The lognormal volatility
*  @param [in]		underlyingRate	The underlying forward rate
*  @return			The normal volatility
*/
double aqMathVolatilityToNormalFromLognormal( const double lognormalVol, const double underlyingRate );

/* @brief			swig interface for aqMathVolatilityToLognormalFromNormal. Convert a normal (basis-point) volatility to a lognormal volatility
*  @return			The lognormal volatility
*/
double aqMathVolatilityToLognormalFromNormal( const double normalVol, const double underlyingRate );

/* @brief			swig interface for aqMathVolatilityToShiftedLognormalFromLognormal
*  @param [in]		shiftSize	The lognormal shift
*  @return			The shifted-lognormal volatility
*/
double aqMathVolatilityToShiftedLognormalFromLognormal( const double lognormalVol, const double underlyingRate, const double shiftSize );

/* @brief			swig interface for aqMathVolatilityToLognormalFromShiftedLognormal
*  @return			The lognormal volatility
*/
double aqMathVolatilityToLognormalFromShiftedLognormal( const double shiftedLognormalVol, const double underlyingRate, const double shiftSize );

/* @brief			swig interface for aqMathVolatilityToShiftedLognormalFromNormal
*  @return			The shifted-lognormal volatility
*/
double aqMathVolatilityToShiftedLognormalFromNormal( const double normalVol, const double underlyingRate, const double shiftSize );

/* @brief			swig interface for aqMathVolatilityToNormalFromShiftedLognormal
*  @return			The normal volatility
*/
double aqMathVolatilityToNormalFromShiftedLognormal( const double shiftedLognormalVol, const double underlyingRate, const double shiftSize );
