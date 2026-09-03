#pragma once

#include "NelsonSiegelFitting.h"
#include "PolynomialFitting.h"
#include "LabelValueBlock.h"
#include "JSONInfoBlock.h"   // JSON Info Blocks


namespace validation_api
{
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
	etrading::NelsonSiegelSvenssonCalibrationResults tryMeBondCurveNelsonSiegelCalibrate( const DoubleVector& bondMaturities,
																				 const DoubleVector& bondYields,
																				 const etrading::NelsonSiegelSvenssonParameters& initialGuess,
																				 const unsigned int& maxIterations,
																				 const unsigned int& maxStationaryStateIterations,
																				 const DoubleVector& lowerBounds,
																				 const DoubleVector& upperBounds );

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
	etrading::NelsonSiegelSvenssonCalibrationResults tryMeBondCurveSvenssonCalibrate( const DoubleVector& bondMaturities,
																			 const DoubleVector& bondYields,
																			 const etrading::NelsonSiegelSvenssonParameters& initialGuess,
																			 const unsigned int& maxIterations,
																			 const unsigned int& maxStationaryStateIterations,
																			 const DoubleVector& lowerBounds,
																			 const DoubleVector& upperBounds );

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
	etrading::PolynomialCalibrationResults tryMeBondCurvePolynomialCalibrate( const unsigned int polynomialOrder,
																			 const DoubleVector& bondMaturities,
																			 const DoubleVector& bondYields,
																		 	 const unsigned int& maxIterations,
																			 const unsigned int maxStationaryStateIterations,
																			 const double lowerBound,
																			 const double upperBound );

	 /* @brief Creates a Bond Curve calibrated from bond quotes
	 * @param [in] objectName        The name of the Bond Curve object to create
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	 *                               BONDCURVE_PROPERTIES, BONDCURVE_MARKETDATA
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
    std::string tryMeBondCurveCreate( const std::string& objectName,
                                      const std::vector<std::string>& dataBlockNames,
                                      const etrading::JSONInfoBlockTuples& infoBlocks ); // TODO: Move the JSON Info Block below the validation_api - does not belong here

	/* @brief Displays the bond curve calibration as a matrix of pillar dates and yield points.
	*  @param[in]	bondCurveName	The name of the bond curve object to display
	*/
	AnyTypeMatrix tryMeBondCurveDisplay( const std::string& bondCurveName );


	/* @brief Inspects the Yield Term Structure of a Bond Curve and returns the yield used on a particular forward reference date
	*  @param[in] bondCurveName		The Bond Curve Name
	*  @param[in] referenceDate		The forward reference date
	*/
	double tryMeBondYield(const std::string& bondCurveName, const LADate& referenceDate );


	/* @brief Calculates the price of a bond using a bond curve to discount the coupons
	*  @param[in] bondObjectName	Name of the bond object to price
	*  @param[in] settlementDate	The settlement date to use for bond pricing
	*  @param[in] bondCurveName		Name of the bond curve
	*/
	double tryMeLWOBondPriceFromBondCurve( const std::string& bondObjectName, const LADate& settlementDate, const std::string & bondCurveName );

	/* @brief Calculates the yield-to-maturity of a bond using a bond curve to discount the coupons
	*  @param[in] bondObjectName	Name of the bond object to price
	*  @param[in] settlementDate	The settlement date to use for bond pricing
	*  @param[in] bondCurveName		Name of the bond curve
	*/
	double tryMeLWOBondYieldFromBondCurve( const std::string& bondObjectName, const LADate& settlementDate, const std::string & bondCurveName );

	/* @brief	Nelson-Siegel interpolation. Given a set of maturities, calculates the corresponding bond yields
	*  @param[in]	beta0						Long term yield
	*  @param[in]	beta1						Slope
	*  @param[in]	beta2						Curvature
	*  @param[in]	lambda						Time decay
	*  @param[in]	maturities				A vector containing the bond maturities to interpolate
	*  @returns		A vector of interpolated bond yields, one yield per supplied maturity.
	*/
	DoubleVector tryMeBondCurveNelsonSiegelYield( const double& beta0,
												  const double& beta1,
												  const double& beta2,
												  const double& lambda,
												  const DoubleVector& bondMaturities );

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
	DoubleVector tryMeBondCurveSvenssonYield( const double& beta0,
											  const double& beta1,
											  const double& beta2,
											  const double& beta3,
											  const double& lambda1,
											  const double& lambda2,
											  const DoubleVector& bondMaturities );

	/* @brief Polynomial interpolation. Given a vector of polynomial coefficients and a bond maturity, interpolates the corresponding bond yield on the curve
	*  @param[in]	coefficients	Polynomial function coefficients: a_0 x^n + a_1 x^(n-1) + ... +a_n i.e. highest order coefficient is first in the vector.
	*  @param[in]	bondMaturities	A vector containing the bond maturities to interpolate
	*  @returns		The bond yield interpolated on the curve, corresponding to the supplied maturity
	*/
	DoubleVector tryMeBondCurvePolynomialYield( const DoubleVector& coefficients, const DoubleVector& bondMaturities );
}