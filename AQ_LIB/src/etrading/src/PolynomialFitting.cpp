/*
 * @brief			Polynomial-interpolation coefficient Fitting
 *					This is used to fit Bond Yields to a polynomial
 * @Created:		18th June 2018
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "PolynomialFitting.h"
#include "BondCurves.h"
#include "ExceptionMacros.h"
 
#include <ql/math/optimization/endcriteria.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>

namespace etrading
{
	/* @brief Constructor of the Polynomial interpolator CostFunction which is initialized using bond market data
	 * @param[in]	bondYields		A vector of bond yields observed in the market 
	 * @param[in]	bondMaturities	A vector of bond maturities observed in the market, one per bond yield
	 */
	PolynomialCostFunction::PolynomialCostFunction ( const DoubleVector& bondMaturities, const DoubleVector& bondYields )
		: BondCurveCostFunction( bondMaturities, bondYields )
	{
	}

	/* @brief Copy constructor
	 */
	PolynomialCostFunction::PolynomialCostFunction( const PolynomialCostFunction& rhs ) 
		: BondCurveCostFunction( rhs )
	{
	}

	/* @brief Calculates the Polynomial interpolator bond yields, given the specified model parameters
	 * @params[in]	modelParams	The model calibration parameters to use
	 * @returns		An array of bond yields calculated from the model 
	 */
	DoubleVector PolynomialCostFunction::getModelValues( const QuantLib::Array& modelParams ) const
	{
		const size_t nCoefficients = modelParams.size();
		DoubleVector coefficients ( nCoefficients );
		for ( size_t i=0; i< nCoefficients; i++ )
		{
			coefficients[i] = modelParams[i];
		}
		
		const size_t nBonds = bondMaturities_.size();
		DoubleVector modelValues( nBonds, 0.0 );

		for (size_t i=0; i< nBonds; i++ )
		{
			double bondMaturity = bondMaturities_[i];
			modelValues[ i ] = PolynomialInterpolator( coefficients, bondMaturity );
		}
		
		return modelValues;
	}

	
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
															   const double upperBound )
	{
		//Set up Cost Function
		PolynomialCostFunction polynomialCostFunction( bondMaturities, bondYields );

		//Set up Boundary Constraint
		size_t nCoefficients = polynomialOrder + 1;

		// Initialize boundary constraint
		QuantLib::Array low( nCoefficients );
		QuantLib::Array high( nCoefficients );
		for ( size_t i=0; i<nCoefficients; i++ )
		{
			low[i] = lowerBound;
			high[i] = upperBound;
		}

		QuantLib::NonhomogeneousBoundaryConstraint polynomialConstraint( low, high );
		 
		// Initialize Levenberg-Marquardt minimizer
		const double lmEpsilon = 1e-10;
		double levenbergMarquardtEpsfcn = lmEpsilon;      
		double levenbergMarquardtXtol   = lmEpsilon;	    
		double levenbergMarquardtGtol   = lmEpsilon;      

		QuantLib::LevenbergMarquardt solver( levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol );

		// Initialize criteria used to determine when the minimizer should stop
		double rootEpsilon				= lmEpsilon; 
		double functionEpsilon			= lmEpsilon; 
		double gradientNormEpsilon		= lmEpsilon;

		QuantLib::EndCriteria polynomialEndCriteria( maxIterations, 
													 maxStationaryStateIterations,
													 rootEpsilon, 
													 functionEpsilon, 
													 gradientNormEpsilon );

		// Initialize minimizer Problem, containing the cost function, constraints and initial solution guess
		QuantLib::Array initialValue( nCoefficients );
		for ( size_t i=0; i<nCoefficients; i++ )
		{
			initialValue[i] = 0.0;  // As an initial guess, set all coefficients to 0.0
		}
		
		QuantLib::Problem polynomialProblem ( polynomialCostFunction, polynomialConstraint, initialValue );

		/*
		 * Perform the calibration
		 */
		QuantLib::EndCriteria::Type result = solver.minimize( polynomialProblem, polynomialEndCriteria );

		PolynomialCalibrationResults calibrationResult;
		calibrationResult.leastSquaresError_    = polynomialProblem.functionValue();
		calibrationResult.iterations_		    = polynomialProblem.functionEvaluation();

		// Store the final result
		
		calibrationResult.coefficients_.resize( nCoefficients );
		for ( size_t i=0; i<nCoefficients; i++ )
		{
			calibrationResult.coefficients_[i] = polynomialProblem.currentValue()[i];
		}
		
		return calibrationResult;

	}


}
