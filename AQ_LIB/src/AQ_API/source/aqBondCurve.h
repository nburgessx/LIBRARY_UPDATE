// aqBondCurve.h

/*
 * @brief			Swig Interface file for Bond Curve Fitting
 *					This is used to fit Bond Prices to a curve
 */

#pragma once

#include "tryAqBondCurves.h"
#include "BondTypes.h"

#include <string>
#include <vector>


/* @brief Fits a Nelson-Siegel parameterized curve to the supplied bond maturities and yields.
*  @param[in]		bondMaturities					A vector containing the bond maturities to fit
*  @param[in]		bondYields						A vector containing the bond yields to fit
*  @param[in]		initialGuess					A structure containing the initial guess for beta0, beta1, beta2 and lambda
*  @param[in]		maxIterations					The maximum number of iterations allowed.
*  @param[in]		maxStationaryStateIterations	The maximum number of iterations around a stationary point
*  @param[in]		lowerBounds						A vector containing the lower bounds for beta0, beta1, beta2, lambda. If empty vector, default is -50 for betas, 0 for lambda
*  @param[in]		upperBounds						A vector containing the upper bounds for beta0, beta1, beta2, lambda. If empty vector, default is +50 for betas and lambda
*  @returns		The calibrated parameters, store in a NelsonSiegelSvenssonCalibrationResults structure
*/
swig::NelsonSiegelSvenssonCalibrationResults aqBondCurveNelsonSiegelCalibrate( const std::vector<double>& bondMaturities,
																               const std::vector<double>& bondYields,
																               const swig::NelsonSiegelSvenssonParameters& initialGuess,
																               const unsigned int& maxIterations,
																               const unsigned int& maxStationaryStateIterations,
																               const std::vector<double>& lowerBounds,
																               const std::vector<double>& upperBounds );

/* @brief Fits a Svensson parameterized curve to the supplied bond maturities and yields.
*  @param[in]		bondMaturities					A vector containing the bond maturities to fit
*  @param[in]		bondYields						A vector containing the bond yields to fit
*  @param[in]		initialGuess					A structure containing the initial guess for beta0, beta1, beta2, beta3, lambda1 and lambda2
*  @param[in]		maxIterations					The maximum number of iterations allowed.
*  @param[in]		maxStationaryStateIterations	The maximum number of iterations around a stationary point
*  @param[in]		lowerBounds						A vector containing the lower bounds for beta0, beta1, beta2, lambda. If empty vector, default is -50 for betas, 0 for lambdas
*  @param[in]		upperBounds						A vector containing the upper bounds for beta0, beta1, beta2, lambda. If empty vector, default is +50 for betas and lambdas
*  @returns		The calibrated parameters, store in a NelsonSiegelSvenssonCalibrationResults structure
*/
swig::NelsonSiegelSvenssonCalibrationResults aqBondCurveSvenssonCalibrate( const std::vector<double>& bondMaturities,
															               const std::vector<double>& bondYields,
															               const swig::NelsonSiegelSvenssonParameters& initialGuess,
															               const unsigned int& maxIterations,
															               const unsigned int& maxStationaryStateIterations,
															               const std::vector<double>& lowerBounds,
															               const std::vector<double>& upperBounds );

/* @brief Main API: Calibrates Polynomial-interpolation coefficients to the specified bond yields and maturities
	*
	* @param[in]		polynomialOrder		The order of the polynomial: 0 = constant line, 1 = linear, 2 = quadratic etc
	* @param[in]		bondMaturities		A vector of bond maturities, to fit
	* @param[in]		bondYields			A vector of input bond yields to fit
	* @param[in]		maxIterations		The maximum number of iterations allowed.
	* @param[in]		maxStationaryStateIterations	The maximum number of iterations around a stationary point
	* @param[in]		lowerBound			Lower bound to be applied to each each polynomial coefficient
	* @param[in]		upperBound			Upper bound to be applied to each each polynomial coefficient
	* returns		The calibrated parameters, store in a PolynomialCalibrationResults structure
	*/
swig::PolynomialCalibrationResults aqBondCurvePolynomialCalibrate( const unsigned int polynomialOrder,
																   const std::vector<double>& bondMaturities,
																   const std::vector<double>& bondYields,
																   const unsigned int& maxIterations,
																   const unsigned int maxStationaryStateIterations,
																   const double lowerBound,
																   const double upperBound );

/* @brief	Nelson-Siegel interpolation. Given a set of maturities, calculates the corresponding bond yields
*  @param[in]	beta0						Long term yield
*  @param[in]	beta1						Slope
*  @param[in]	beta2						Curvature
*  @param[in]	lambda						Time decay
*  @param[in]	maturities				A vector containing the bond maturities to interpolate
*  @returns		A vector of interpolated bond yields, one yield per supplied maturity.
*/
std::vector<double> aqBondCurveNelsonSiegelYield( const double& beta0,
										          const double& beta1,
										          const double& beta2,
										          const double& lambda,
										          const std::vector<double>& bondMaturities );

/* @brief	Nelson-Siegel-Svensson interpolation. Given a set of maturities, calculates the corresponding bond yields
*  @param[in]	beta0						Long term yield
*  @param[in]	beta1						Slope
*  @param[in]	beta2						Curvature
*  @param[in]	beta3						Secondary curvature
*  @param[in]	lambda1						Time decay
*  @param[in]	lambda2						Time decay
*  @param[in]	bondMaturities				A vector containing the bond maturities to interpolate
*  @returns		A vector of interpolated bond yields, one yield per supplied maturity.
*/
std::vector<double> aqBondCurveSvenssonYield( const double& beta0,
									          const double& beta1,
									          const double& beta2,
									          const double& beta3,
									          const double& lambda1,
									          const double& lambda2,
									          const std::vector<double>& bondMaturities );

/* @brief Polynomial interpolation. Given a vector of polynomial coefficients and a bond maturity, interpolates the corresponding bond yield on the curve
*  @param[in]	coefficients	Polynomial function coefficients: a_0 x^n + a_1 x^(n-1) + ... +a_n i.e. highest order coefficient is first in the vector.
*  @param[in]	bondMaturities	A vector containing the bond maturities to interpolate
*  @returns		The bond yield interpolated on the curve, corresponding to the supplied maturity
*/
std::vector<double> aqBondCurvePolynomialYield( const std::vector<double>& coefficients, const std::vector<double>& bondMaturities );


/* @brief Function to display bond cashflows
*  @param[in]   bondObjectName
*  @param[in]   settlementDate
*  @param[in]   yield
*  @param[in]   yieldCalculationType
*  @param[in]   showColumnHeaders
*  @returns		Cashflow Display Information
*/
SWIG_STRINGMATRIX aqBondObjectDisplayCashflows( const std::string& bondObjectName, const std::string& settlementDate, const double& yield, const std::string& yieldCalculationType, const bool& showColumnHeaders );


/* @brief Function to display bond cashflows
*  @param[in]   bondObjectName
*  @param[in]   settlementDate
*  @param[in]   yield
*  @param[in]   yieldCalculationType
*  @param[in]   showColumnHeaders
*  @param[in]   columnList
*  @returns		Cashflow Display Information
*/
SWIG_STRINGMATRIX aqBondObjectDisplayCashflows( const std::string& bondObjectName, const std::string& settlementDate, const double& yield, const std::string& yieldCalculationType, const bool& showColumnHeaders, const std::vector<std::string>& columnList );


/* @brief Function to display bond cashflows
*  @param[in]   bondObjectName
*  @param[in]   showColumnHeaders
*  @returns		Cashflow Display Information
*/
SWIG_STRINGMATRIX aqBondObjectDisplaySchedule( const std::string& bondObjectName, const bool& showColumnHeaders );


/* @brief Function to display bond cashflows
*  @param[in]   bondObjectName
*  @param[in]   showColumnHeaders
*  @param[in]   columnList
*  @returns		Cashflow Display Information
*/
SWIG_STRINGMATRIX aqBondObjectDisplaySchedule( const std::string& bondObjectName, const bool& showColumnHeaders, const std::vector<std::string>& columnList );
