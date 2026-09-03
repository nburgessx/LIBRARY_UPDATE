/*
 * @brief			Nelson Siegel Model Fitting
 *					This is used to fit Bond Yields to a Nelson-Siegel / Svensson curve
 * @Created:		5th June 2018
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "NelsonSiegelFitting.h"
#include "BondCurves.h"
#include "ExceptionMacros.h"
 
#include <ql/math/optimization/endcriteria.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>

namespace etrading
{
	/* @brief Constructor of the NelsonSiegel CostFunction which is initialized using bond market data
	 * @param[in]	bondYields		A vector of bond yields observed in the market 
	 * @param[in]	bondMaturities	A vector of bond maturities observed in the market, one per bond yield
	 * @param[in]	curveType		The curvetype parameterisation to use (NelsonSiegel or Svensson)
	 */
	NelsonSiegelCostFunction::NelsonSiegelCostFunction ( const DoubleVector& bondMaturities, const DoubleVector& bondYields, const InterpolationEnum interpolationType )
		: BondCurveCostFunction( bondMaturities, bondYields ), interpolationType_( interpolationType )
	{
		if ( interpolationType_ != NELSON_SIEGEL_INTERPOLATION && interpolationType_ != SVENSSON_INTERPOLATION )
		{
			MLIB_THROW( "Incorrect interpolationType. Only NELSONSIEGEL and SVENSSON interpolations are supported. ");
		}
	}

	/* @brief Copy constructor
	 */
	NelsonSiegelCostFunction::NelsonSiegelCostFunction( const NelsonSiegelCostFunction& rhs ) 
		: BondCurveCostFunction( rhs ), interpolationType_( rhs.interpolationType_ )
	{
	}

	/* @brief Calculates the Nelson Siegel bond yields, given the specified model parameters
	 * @params[in]	modelParams	The model calibration parameters to use
	 * @returns		An array of bond yields calculated from the model 
	 */
	DoubleVector NelsonSiegelCostFunction::getModelValues( const QuantLib::Array& modelParams ) const
	{
		NelsonSiegelSvenssonParameters parameters;

		switch ( interpolationType_ )
		{
		case NELSON_SIEGEL_INTERPOLATION:
			MLIB_REQUIRE( modelParams.size() == 4,  "Nelson-Siegel interpolation should have FOUR fitting parameters: Beta0, Beta1, Beta2, Lambda" );
			parameters.beta0_  = modelParams[0];
			parameters.beta1_  = modelParams[1];
			parameters.beta2_  = modelParams[2];
			parameters.lambda1_ = modelParams[3];
			break;

		case SVENSSON_INTERPOLATION:
			MLIB_REQUIRE( modelParams.size() == 6,  "Svensson interpolation should have SIX fitting parameters: Beta0, Beta1, Beta2, Beta3, Lambda1, Lambda2" );
			parameters.beta0_  = modelParams[0];
			parameters.beta1_  = modelParams[1];
			parameters.beta2_  = modelParams[2];
			parameters.beta3_  = modelParams[3];
			parameters.lambda1_ = modelParams[4];
			parameters.lambda2_ = modelParams[5];
			break;

		default:
			MLIB_THROW( "Incorrect interpolationType. Only NELSONSIEGEL and SVENSSON interpolations are supported. ");
		}
		
		const size_t nBonds = bondMaturities_.size();
		DoubleVector modelValues( nBonds, 0.0 );

		if ( interpolationType_ == NELSON_SIEGEL_INTERPOLATION )
		{
			for (size_t i=0; i< nBonds; i++ )
			{
				double bondMaturity = bondMaturities_[i];
				modelValues[ i ] = NelsonSiegelInterpolator( parameters, bondMaturity );
			}
		}
		else
		{
			// Svensson curve
			for (size_t i=0; i< nBonds; i++ )
			{
				double bondMaturity = bondMaturities_[i];
				modelValues[ i ] = SvenssonInterpolator( parameters, bondMaturity );
			}
		}

		return modelValues;
	}
	
	/* @brief Main API: Calibrates the Nelson-Siegel-Svensson model parameters to the specified bond yields and maturities
	 *
	 * @param[in]	interpolationType	Specify the bond interpolation type to fit: Nelson-Siegel or Svensson
	 * @param[in]	bondMaturities		A vector of bond maturities, to fit
	 * @param[in]	bondYields			A vector of input bond yields to fit
	 * @param[in]	initialGuess		An initial parameter starting point for the calibration.
	 * @param[in]	lowerBounds			A vector of lower bounds, one boundary for each Nelson Siegel parameter
	 * @param[in]	upperBounds			A vector of upper bounds, one boundary for each Nelson Siegel parameter
	 * @param[in]	maxIterations		The maximum number of iterations allowed. On output, the actual number of iterations
	 * @param[in]	maxStationaryStateIterations	The maximum number of iterations around a stationary point
	 * returns		A NelsonSiegelResult structure, containing the fitted parameters and various calibration statistics
	 */
	NelsonSiegelSvenssonCalibrationResults calibrateNelsonSiegelSvenssonToBondYields( const InterpolationEnum& interpolationType,
																			          const DoubleVector& bondMaturities,
																			          const DoubleVector& bondYields,
																			          const NelsonSiegelSvenssonParameters& initialGuess,
																			          const DoubleVector& lowerBounds,
																			          const DoubleVector& upperBounds,
																			          const unsigned int maxIterations,
																			          const unsigned int maxStationaryStateIterations )
	{

		if ( interpolationType != NELSON_SIEGEL_INTERPOLATION && interpolationType != SVENSSON_INTERPOLATION )
		{
			MLIB_THROW( "Incorrect interpolationType. Only NELSONSIEGEL and SVENSSON interpolations are supported. ");
		}

		//Set up Cost Function
		NelsonSiegelCostFunction nelsonSiegelCostFunction( bondMaturities, bondYields, interpolationType );

		//Set up Boundary Constraint
		size_t nExpectedParameters = interpolationType == NELSON_SIEGEL_INTERPOLATION ? 4 : 6;
		MLIB_REQUIRE( lowerBounds.size() == nExpectedParameters, "Invalid Lower bounds vector size. Require size 4 for Nelson-Siegel and size 6 for Svensson Method.");
		MLIB_REQUIRE( upperBounds.size() == nExpectedParameters, "Invalid Upper bounds vector size. Require size 4 for Nelson-Siegel and size 6 for Svensson Method.");

		// Initialize boundary constraint
		QuantLib::Array low( nExpectedParameters );
		QuantLib::Array high( nExpectedParameters );
		for ( size_t i=0; i<nExpectedParameters; i++ )
		{
			low[i] = lowerBounds[i];
			high[i] = upperBounds[i];
		}

		QuantLib::NonhomogeneousBoundaryConstraint nelsonSiegelConstraint( low, high );
		 
		// Initialize Levenberg-Marquardt minimizer
		const double lmEpsilon = 1e-10;
		double levenbergMarquardtEpsfcn = lmEpsilon;      
		double levenbergMarquardtXtol   = lmEpsilon;	    
		double levenbergMarquardtGtol   = lmEpsilon;      

		QuantLib::LevenbergMarquardt solver (levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol);

		// Initialize criteria used to determine when the minimizer should stop
		double rootEpsilon				= lmEpsilon; 
		double functionEpsilon			= lmEpsilon; 
		double gradientNormEpsilon		= lmEpsilon;

		QuantLib::EndCriteria nelsonSiegelEndCriteria( maxIterations, 
													   maxStationaryStateIterations,
													   rootEpsilon, 
													   functionEpsilon, 
													   gradientNormEpsilon );

		// Initialize minimizer Problem, containing the cost function, constraints and initial solution guess
		QuantLib::Array initialValue( nExpectedParameters );
		switch ( interpolationType )
		{
		case NELSON_SIEGEL_INTERPOLATION:
			initialValue[0] = initialGuess.beta0_;
			initialValue[1] = initialGuess.beta1_;
			initialValue[2] = initialGuess.beta2_;
			initialValue[3] = initialGuess.lambda1_;
			break;

		case SVENSSON_INTERPOLATION:
			initialValue[0] = initialGuess.beta0_;
			initialValue[1] = initialGuess.beta1_;
			initialValue[2] = initialGuess.beta2_;
			initialValue[3] = initialGuess.beta3_;
			initialValue[4] = initialGuess.lambda1_;
			initialValue[5] = initialGuess.lambda2_;
			break;

		default:
			MLIB_THROW( "Incorrect interpolationType. Only NELSONSIEGEL and SVENSSON interpolations are supported. ");
			
		}

		QuantLib::Problem nelsonSiegelProblem (nelsonSiegelCostFunction, nelsonSiegelConstraint, initialValue );

		/*
		 * Perform the calibration
		 */
		QuantLib::EndCriteria::Type result = solver.minimize( nelsonSiegelProblem, nelsonSiegelEndCriteria );

		NelsonSiegelSvenssonCalibrationResults calibrationResult;
		calibrationResult.leastSquaresError_	    = nelsonSiegelProblem.functionValue();
		calibrationResult.iterations_		        = nelsonSiegelProblem.functionEvaluation();

		switch ( interpolationType )
		{
		case NELSON_SIEGEL_INTERPOLATION:
			calibrationResult.parameters_.beta0_	= nelsonSiegelProblem.currentValue()[0];
			calibrationResult.parameters_.beta1_	= nelsonSiegelProblem.currentValue()[1];
			calibrationResult.parameters_.beta2_	= nelsonSiegelProblem.currentValue()[2];
			calibrationResult.parameters_.lambda1_	= nelsonSiegelProblem.currentValue()[3];
			break;

		case SVENSSON_INTERPOLATION:
			calibrationResult.parameters_.beta0_	= nelsonSiegelProblem.currentValue()[0];
			calibrationResult.parameters_.beta1_	= nelsonSiegelProblem.currentValue()[1];
			calibrationResult.parameters_.beta2_	= nelsonSiegelProblem.currentValue()[2];
			calibrationResult.parameters_.beta3_	= nelsonSiegelProblem.currentValue()[3];
			calibrationResult.parameters_.lambda1_	= nelsonSiegelProblem.currentValue()[4];
			calibrationResult.parameters_.lambda2_	= nelsonSiegelProblem.currentValue()[5];
			break;

		default:
			MLIB_THROW( "Incorrect interpolationType. Only NELSONSIEGEL and SVENSSON interpolations are supported. ");
			
		}

		return calibrationResult;

	}


}
