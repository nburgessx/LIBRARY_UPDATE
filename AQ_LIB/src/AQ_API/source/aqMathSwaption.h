// aqMathSwaption.h

/*
 * @brief			Swig interface for the aqMathEuropeanIRSwaption* pricing and Greeks functions
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqMathEuropeanIRSwaptionPrice
*  @param [in]		payerReceiver	PAYER or RECEIVER
*  @param [in]		annuity			The swaption annuity
*  @param [in]		swapRate		The forward swap rate
*  @param [in]		strike			Strike rate
*  @param [in]		vol				Volatility
*  @param [in]		time			Time to expiry in years
*  @param [in]		shift			Optional. Lognormal shift, defaults to zero
*  @param [in]		volatilityType	Optional. LOGNORMAL or NORMAL. Default LOGNORMAL
*  @return			The European IR swaption price
*/
double aqMathEuropeanIRSwaptionPrice( const std::string& payerReceiver,
                                       const double annuity,
                                       const double swapRate,
                                       const double strike,
                                       const double vol,
                                       const double time,
                                       const double shift = 0.0,
                                       const std::string& volatilityType = "LOGNORMAL" );

/* @brief			swig interface for aqMathEuropeanIRSwaptionPrices - optimize = true uses OMP threading
*  @return			The European IR swaption prices
*/
std::vector<double> aqMathEuropeanIRSwaptionPrices( const std::vector<std::string>& payerReceiver,
                                                     const std::vector<double>& annuity,
                                                     const std::vector<double>& swapRate,
                                                     const std::vector<double>& strike,
                                                     const std::vector<double>& vol,
                                                     const std::vector<double>& time,
                                                     const std::vector<double>& shift = std::vector<double>(),
                                                     const std::vector<std::string>& volatilityType = std::vector<std::string>(),
                                                     const bool optimize = false );

/* @brief			swig interface for aqMathEuropeanIRSwaptionImpliedVol
*  @return			The European IR swaption implied volatility
*/
double aqMathEuropeanIRSwaptionImpliedVol( const double price,
                                            const std::string& payerReceiver,
                                            const double annuity,
                                            const double swapRate,
                                            const double strike,
                                            const double time,
                                            const double shift = 0.0,
                                            const std::string& volatilityType = "LOGNORMAL" );

/* @brief			swig interface for aqMathEuropeanIRSwaptionImpliedVols - optimize = true uses OMP threading
*  @return			The European IR swaption implied volatilities
*/
std::vector<double> aqMathEuropeanIRSwaptionImpliedVols( const std::vector<double>& price,
                                                          const std::vector<std::string>& payerReceiver,
                                                          const std::vector<double>& annuity,
                                                          const std::vector<double>& swapRate,
                                                          const std::vector<double>& strike,
                                                          const std::vector<double>& time,
                                                          const std::vector<double>& shift = std::vector<double>(),
                                                          const std::vector<std::string>& volatilityType = std::vector<std::string>(),
                                                          const bool optimize = false );

/* @brief			swig interface for aqMathEuropeanIRSwaptionCashAnnuity
*  @param [in]		notional			Notional
*  @param [in]		swapRate			The swap rate
*  @param [in]		nCouponsPerYear		Number of coupons per year
*  @param [in]		tenorInYears		Swap tenor in years
*  @param [in]		stubType			Optional. Default SHORT_START
*  @return			The cash annuity
*/
double aqMathEuropeanIRSwaptionCashAnnuity( const double notional,
                                             const double swapRate,
                                             const unsigned int nCouponsPerYear,
                                             const double tenorInYears,
                                             const std::string& stubType = "SHORT_START" );

/* @brief			swig interface for aqMathEuropeanIRSwaptionDelta
*  @return			The European IR swaption delta
*/
double aqMathEuropeanIRSwaptionDelta( const std::string& payerReceiver,
                                       const double annuity,
                                       const double swapRate,
                                       const double strike,
                                       const double vol,
                                       const double time,
                                       const double shift = 0.0,
                                       const std::string& volatilityType = "LOGNORMAL" );

/* @brief			swig interface for aqMathEuropeanIRSwaptionGamma
*  @return			The European IR swaption gamma
*/
double aqMathEuropeanIRSwaptionGamma( const std::string& payerReceiver,
                                       const double annuity,
                                       const double swapRate,
                                       const double strike,
                                       const double vol,
                                       const double time,
                                       const double shift = 0.0,
                                       const std::string& volatilityType = "LOGNORMAL" );

/* @brief			swig interface for aqMathEuropeanIRSwaptionVega
*  @return			The European IR swaption vega
*/
double aqMathEuropeanIRSwaptionVega( const std::string& payerReceiver,
                                      const double annuity,
                                      const double swapRate,
                                      const double strike,
                                      const double vol,
                                      const double time,
                                      const double shift = 0.0,
                                      const std::string& volatilityType = "LOGNORMAL" );

/* @brief			swig interface for aqMathEuropeanIRSwaptionTheta
*  @return			The European IR swaption theta
*/
double aqMathEuropeanIRSwaptionTheta( const std::string& payerReceiver,
                                       const double annuity,
                                       const double swapRate,
                                       const double strike,
                                       const double vol,
                                       const double time,
                                       const double shift = 0.0,
                                       const std::string& volatilityType = "LOGNORMAL" );
