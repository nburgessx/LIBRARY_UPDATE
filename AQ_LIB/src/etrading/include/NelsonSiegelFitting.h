#pragma once

#include "BondCurveCostFunction.h"
#include "BondCurves.h"
#include "LACoreTemplateType.h"
#include "CoreEnumerations.h"

namespace etrading
{

	// A result structure containing the calibration results
	struct NelsonSiegelSvenssonCalibrationResults
	{
		NelsonSiegelSvenssonParameters parameters_;	// The calibrated model parameters
		double leastSquaresError_;		            // Minimum error achieved by the minimizer corresponding to the final calibration parameters
		double iterations_;				            // Number of iterations the minimizer used
	};

	// The Nelson Seigel cost function, which the optimizer will attempt to minimise.
	// The main purpose of the cost function is to calculate the difference between the model yields and the target yields.
	class NelsonSiegelCostFunction : public BondCurveCostFunction
	{

	public:

		NelsonSiegelCostFunction() {}
		virtual ~NelsonSiegelCostFunction(){}

		/* @brief Constructor of the NelsonSiegel CostFunction which is initialized using bond market data
		 * @param[in]	bondMaturities		A vector of bond maturities observed in the market
		 * @param[in]	bondYields			A vector of bond yields observed in the market 
		 * @param[in]	interpolationType	The curvetype parameterisation to use (NelsonSiegel or Svensson)
		*/
		NelsonSiegelCostFunction ( const DoubleVector& bondMaturities, const DoubleVector& bondYields, const InterpolationEnum interpolationType );

		/* @brief Copy constructor
		*/
		NelsonSiegelCostFunction( const NelsonSiegelCostFunction& rhs );

	protected:
		/* @brief Calculates the Nelson Siegel bond yields, given the specified model parameters
		 * @params[in]	modelParams	The model calibration parameters to use
		 * @returns		An array of bond yields calculated from the model 
		 */
		virtual DoubleVector getModelValues( const QuantLib::Array& modelParams ) const;

	private :
		InterpolationEnum interpolationType_;

	};

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
																			 const unsigned int maxStationaryStateIteration );

}

