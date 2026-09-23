// aqMathCapletFloorlet.h

/*
 * @brief			Swig interface for the aqMathCapletFloorlet* pricing functions
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqMathCapletFloorletPrice
*  @param [in]		capletOrFloorlet	CAPLET or FLOORLET
*  @param [in]		annuityFactor		Discounted year-fraction of the period
*  @param [in]		liborRate			Forward Libor rate for the period
*  @param [in]		strike				Strike rate
*  @param [in]		vol					Volatility
*  @param [in]		time				Time to expiry in years
*  @param [in]		shift				Optional. Lognormal shift, defaults to zero
*  @param [in]		volatilityType		Optional. LOGNORMAL or NORMAL. Default LOGNORMAL
*  @return			The caplet / floorlet price
*/
double aqMathCapletFloorletPrice( const std::string& capletOrFloorlet,
                                   const double annuityFactor,
                                   const double liborRate,
                                   const double strike,
                                   const double vol,
                                   const double time,
                                   const double shift = 0.0,
                                   const std::string& volatilityType = "LOGNORMAL" );

/* @brief			swig interface for aqMathCapletFloorletPrices - optimize = true uses OMP threading
*  @return			The caplet / floorlet prices
*/
std::vector<double> aqMathCapletFloorletPrices( const std::vector<std::string>& capletOrFloorlet,
                                                 const std::vector<double>& annuityFactor,
                                                 const std::vector<double>& liborRate,
                                                 const std::vector<double>& strike,
                                                 const std::vector<double>& vol,
                                                 const std::vector<double>& time,
                                                 const std::vector<double>& shift = std::vector<double>(),
                                                 const std::vector<std::string>& volatilityType = std::vector<std::string>(),
                                                 const bool optimize = false );

/* @brief			swig interface for aqMathCapletFloorletImpliedVol
*  @return			The caplet / floorlet implied volatility
*/
double aqMathCapletFloorletImpliedVol( const double price,
                                        const std::string& capletOrFloorlet,
                                        const double annuityFactor,
                                        const double liborRate,
                                        const double strike,
                                        const double time,
                                        const double shift = 0.0,
                                        const std::string& volatilityType = "LOGNORMAL" );

/* @brief			swig interface for aqMathCapletFloorletImpliedVols - optimize = true uses OMP threading
*  @return			The caplet / floorlet implied volatilities
*/
std::vector<double> aqMathCapletFloorletImpliedVols( const std::vector<double>& price,
                                                      const std::vector<std::string>& capletOrFloorlet,
                                                      const std::vector<double>& annuityFactor,
                                                      const std::vector<double>& liborRate,
                                                      const std::vector<double>& strike,
                                                      const std::vector<double>& time,
                                                      const std::vector<double>& shift = std::vector<double>(),
                                                      const std::vector<std::string>& volatilityType = std::vector<std::string>(),
                                                      const bool optimize = false );
