/*
 * @brief			Polynomial-interpolation coefficient Fitting
 *					This is used to fit Bond Yields to a polynomial
 * @Created:		18th June 2018
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "BondCurveCostFunction.h"
#include "BondCurves.h"
#include "LACoreTemplateType.h"

namespace etrading
{

	// A result structure containing the calibration results
	struct PolynomialCalibrationResults
	{
		DoubleVector coefficients_;	// The calibrated model parameters
		double leastSquaresError_;	// Minimum error achieved by the minimizer corresponding to the final calibration parameters
		double iterations_;			// Number of iterations the minimizer used
	};

	// The Polynomial-interpolator cost function, which the optimizer will attempt to minimise.
	// The main purpose of the cost function is to calculate the difference between the model yields and the target yields.
	class PolynomialCostFunction : public BondCurveCostFunction
	{

	public:

		PolynomialCostFunction() {}
		virtual ~PolynomialCostFunction(){}

		/* @brief Constructor of the Polynomial Cost Function which is initialized using bond market data
		 * @param[in]	bondMaturities		A vector of bond maturities observed in the market
		 * @param[in]	bondYields			A vector of bond yields observed in the market 
		*/
		PolynomialCostFunction ( const DoubleVector& bondMaturities, const DoubleVector& bondYields );

		/* @brief Copy constructor
		*/
		PolynomialCostFunction( const PolynomialCostFunction& rhs );
		
	protected:
		/* @brief Calculates the Polynomial-interpolator bond yields, given the specified model parameters
		 * @params[in]	modelParams	The polynomial coefficients to use
		 * @returns		An array of bond yields calculated from the model 
		 */
		virtual DoubleVector getModelValues( const QuantLib::Array& modelParams ) const;

	};

	/* @brief Main API: Calibrates Polynomial-interpolation coefficients to the specified bond yields and maturities
	 *
	 * @param[in]	polynomialOrder		The order of the polynomial: 0 = constant line, 1 = linear, 2 = quadratic etc
	 * @param[in]	bondMaturities		A vector of bond maturities, to fit
	 * @param[in]	bondYields			A vector of input bond yields to fit
	 * @param[in]	maxIterations		The maximum number of iterations allowed. On output, the actual number of iterations
	 * @param[in]	maxStationaryStateIterations	The maximum number of iterations around a stationary point
	 * @param[in]	lowerBound			Lower bound to be applied to each each polynomial coefficient
	 * @param[in]	upperBound			Upper bound to be applied to each each polynomial coefficient
	 * returns		A PolynomialCalibrationResults structure, containing the fitted parameters and various calibration statistics
	 */
	PolynomialCalibrationResults calibratePolynomialBondYields( const unsigned int polynomialOrder,
															   const DoubleVector& bondMaturities,
															   const DoubleVector& bondYields,
															   const unsigned int maxIterations,
															   const unsigned int maxStationaryStateIterations,
															   const double lowerBound,
															   const double upperBound );

}

