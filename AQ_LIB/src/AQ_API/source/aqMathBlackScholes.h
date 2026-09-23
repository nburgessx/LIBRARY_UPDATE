// aqMathBlackScholes.h

/*
 * @brief			Swig interface for the aqMathBlackScholes* pricing and Greeks functions
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqMathBlackScholesPrice
*  @param [in]		callOrPut	CALL or PUT
*  @param [in]		spot		Spot
*  @param [in]		strike		Strike
*  @param [in]		vol			Volatility (%)
*  @param [in]		time		Time to expiry in years
*  @param [in]		rate		Interest rate (%)
*  @param [in]		carry		Cost of carry (%)
*  @param [in]		shift		Optional. Lognormal shift, defaults to zero
*  @return			The Black-Scholes price
*/
double aqMathBlackScholesPrice( const std::string& callOrPut,
                                 const double spot,
                                 const double strike,
                                 const double vol,
                                 const double time,
                                 const double rate,
                                 const double carry,
                                 const double shift = 0.0 );

/* @brief			swig interface for aqMathBlackScholesPrices - optimize = true uses OMP threading
*  @param [in]		callOrPut	Column of CALL/PUT
*  @param [in]		spot		Column of spot prices
*  @param [in]		strike		Column of strikes
*  @param [in]		vol			Column of volatilities
*  @param [in]		time		Column of times to expiry, in years
*  @param [in]		rate		Column of interest rates
*  @param [in]		carry		Column of cost-of-carry rates
*  @param [in]		shift		Optional. Column of lognormal shifts
*  @param [in]		optimize	Optional. Default FALSE. Use OMP threading
*  @return			The Black-Scholes prices
*/
std::vector<double> aqMathBlackScholesPrices( const std::vector<std::string>& callOrPut,
                                               const std::vector<double>& spot,
                                               const std::vector<double>& strike,
                                               const std::vector<double>& vol,
                                               const std::vector<double>& time,
                                               const std::vector<double>& rate,
                                               const std::vector<double>& carry,
                                               const std::vector<double>& shift = std::vector<double>(),
                                               const bool optimize = false );

/* @brief			swig interface for aqMathBlackScholesImpliedVol
*  @param [in]		price		The option price
*  @param [in]		callOrPut	CALL or PUT
*  @param [in]		spot		Spot
*  @param [in]		strike		Strike
*  @param [in]		time		Time to expiry in years
*  @param [in]		rate		Interest rate (%)
*  @param [in]		carry		Cost of carry (%)
*  @param [in]		shift		Optional. Lognormal shift, defaults to zero
*  @return			The Black-Scholes implied volatility
*/
double aqMathBlackScholesImpliedVol( const double price,
                                      const std::string& callOrPut,
                                      const double spot,
                                      const double strike,
                                      const double time,
                                      const double rate,
                                      const double carry,
                                      const double shift = 0.0 );

/* @brief			swig interface for aqMathBlackScholesImpliedVols - optimize = true uses OMP threading
*  @param [in]		price		Column of option prices
*  @param [in]		callOrPut	Column of CALL/PUT
*  @param [in]		spot		Column of spot prices
*  @param [in]		strike		Column of strikes
*  @param [in]		time		Column of times to expiry, in years
*  @param [in]		rate		Column of interest rates
*  @param [in]		carry		Column of cost-of-carry rates
*  @param [in]		shift		Optional. Column of lognormal shifts
*  @param [in]		optimize	Optional. Default FALSE. Use OMP threading
*  @return			The Black-Scholes implied volatilities
*/
std::vector<double> aqMathBlackScholesImpliedVols( const std::vector<double>& price,
                                                    const std::vector<std::string>& callOrPut,
                                                    const std::vector<double>& spot,
                                                    const std::vector<double>& strike,
                                                    const std::vector<double>& time,
                                                    const std::vector<double>& rate,
                                                    const std::vector<double>& carry,
                                                    const std::vector<double>& shift = std::vector<double>(),
                                                    const bool optimize = false );

/* @brief			swig interface for aqMathBlackScholesDeltaForward
*  @return			The Black-Scholes forward delta
*/
double aqMathBlackScholesDeltaForward( const std::string& callOrPut,
                                        const double spot,
                                        const double strike,
                                        const double vol,
                                        const double time,
                                        const double rate,
                                        const double carry,
                                        const double shift = 0.0 );

/* @brief			swig interface for aqMathBlackScholesDeltaSpot
*  @return			The Black-Scholes spot delta
*/
double aqMathBlackScholesDeltaSpot( const std::string& callOrPut,
                                     const double spot,
                                     const double strike,
                                     const double vol,
                                     const double time,
                                     const double rate,
                                     const double carry,
                                     const double shift = 0.0 );

/* @brief			swig interface for aqMathBlackScholesGamma
*  @return			The Black-Scholes gamma
*/
double aqMathBlackScholesGamma( const std::string& callOrPut,
                                 const double spot,
                                 const double strike,
                                 const double vol,
                                 const double time,
                                 const double rate,
                                 const double carry,
                                 const double shift = 0.0 );

/* @brief			swig interface for aqMathBlackScholesVega
*  @return			The Black-Scholes vega
*/
double aqMathBlackScholesVega( const std::string& callOrPut,
                                const double spot,
                                const double strike,
                                const double vol,
                                const double time,
                                const double rate,
                                const double carry,
                                const double shift = 0.0 );

/* @brief			swig interface for aqMathBlackScholesTheta
*  @return			The Black-Scholes theta
*/
double aqMathBlackScholesTheta( const std::string& callOrPut,
                                 const double spot,
                                 const double strike,
                                 const double vol,
                                 const double time,
                                 const double rate,
                                 const double carry,
                                 const double shift = 0.0 );

/* @brief			swig interface for aqMathBlackScholesRho
*  @return			The Black-Scholes rho
*/
double aqMathBlackScholesRho( const std::string& callOrPut,
                               const double spot,
                               const double strike,
                               const double vol,
                               const double time,
                               const double rate,
                               const double carry,
                               const double shift = 0.0 );
