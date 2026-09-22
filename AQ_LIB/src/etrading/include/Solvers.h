#pragma once

#include <functional>
#include <vector>
#include <limits>		// for numerical limits / quiet nan
#include <cmath>		// for std::isnan

#include "AQLNumericMatrix.h"

namespace etrading
{
	namespace solvers
	{
        // Struct to Store the Solver Results
        // ----------------------------------
        // Store the solution, number of iterations, the slope / Jacobian and other useful information.
        struct SolverResults
        {
            double solution;
            unsigned int numberOfIterations;
            double jacobian;    // Solver Slope. Newton-Raphson has this, however not all solvers will have this information
            double epsilon;     // Solver Error
        };

		/* @brief A dataInstance finder which uses TOMS Algorithm 748, which does not require derivatives:
		*         "Enclosing Zeros of Continuous Functions"
		*         ACM Transactions on Mathematical Software by Alefeld, Potra, Shi
		*         http://na.math.kit.edu/alefeld/download/1995_Algorithm_748_Enclosing_Zeros_of_Continuous_Functions.pdf
		*
		*         This algorithm is similar-to and superior to the Brent-Dekker dataInstance finder.
		*
		* @param[in] function			The objective function: This takes a single double (the x value), and returns a double (the y value)
		* @param[in] initialGuess		An initial guess for the x value
		* @param[in] tolerance			The y-axis tolerance to achieve
		* @param[in] maxIterations		The maximum number of Newton Raphson iterations to perform before giving up
		* @param[in] functionIsRising	Set to true if f(x) is rising on x and false if f(x) is falling on x. Default true
		* @param[in] bracketFactor		A scaling factor that is used to bracket the dataInstance: the value guess is multiplied (or divided as appropriate) by bracketFactor until two values are found that bracket the dataInstance. 
		* @returns   SolverResults		Returns a struct containing:- 1) The value of x for which: f(x) = target  2) the number of solver interations 3) epsilon
		*/
		SolverResults toms748( std::function<double (double)> function,
							   const double initialGuess,	   
							   const double tolerance,
							   const size_t maxIterations,
							   const bool functionIsRising = true,
							   const double bracketFactor = 2.0 );

        
        /* @brief The One-dimensional Bisection Method solver / rootfinder. Given an objective function y = f(x), the solver searches the interval [lowerBound, UpperBound]
        * to find the value of x for which f(x) equals zero. The target function is therefore ( f(x) - target value )
		* @param[in] function		                        The objective function: A one dimensional funcion such as i.e. y = f(x) - target value. Note the target value is subtracted from the function value a priori.
		* @param[in] initialLowerBound 	                    The Search Interval Lower Bound
        * @param[in] initialUpperBound 	                    The Search Interval Upper Bound
        * @param[in] minimumIntervalSize 	                The mimimum search interval size
		* @param[in] tolerance		                        The y-axis tolerance to achieve
        * @param[in] gradientTolerance                      Tolerance check on the gradient.
		* @param[in] maxIterations	                        The maximum number of Bisection Method iterations to perform before giving up
        * @param[in] maxIntervalExpansionAttempts           The number of attempts to expand the initial search range if it does not contain the solution, defaults to 10
        * @param[in] throwIfMultipleSolutions               Throw an error if multiple solutions exist, defaults to false
        * @param[in] ShiftSizeForMultipleSolutionCheck      Multiple Solutitons Exist when the function derivative is zero, defaults to 1e-8 This shift size specifies the shift to evaluate the gradient numerically.
		* @returns   SolverResults                          Returns a struct containing:- 1) The value of x for which: f(x) = target  2) the number of solver interations  3) the slope jacobian value, here zero since no jacobian in bisection method
		*/
		SolverResults bisectionMethod( std::function<double (double)> function,
							           const double initialLowerBound,
							           const double initialUpperBound,
                                       const double minimumIntervalSize,
                                       const double tolerance,
                                       const double gradientTolerance,
							           const size_t maxIterations,
                                       const size_t maxIntervalExpansionAttempts = 10,
                                       const bool throwIfMultipleSolutions = false,
                                       const double shiftSizeForMultipleSolutionCheck = 1e-8 );


	   /* @brief The One-dimensional Newton Raphson solver / rootfinder. Given an objective function y = f(x), and a target y value,
	    * the solver attempts to find the value of x for which f(x) matches the target.
		* @param[in] function		The objective function: This takes a single double (the x value), and returns a double (the y value)
		* @param[in] target			The target y value to solve for.
		* @param[in] initialGuess	An initial guess for the x value.
		* @param[in] tolerance		The y-axis tolerance to achieve
		* @param[in] maxIterations	The maximum number of Newton Raphson iterations to perform before giving up
		* @param[in] bumpSize		The small bump in x value used when calculating a local gradient.
		* @returns   SolverResults  Returns a struct containing:- 1) The value of x for which: f(x) = target  2) the number of solver interations  3) the slope jacobian value
		*/
		SolverResults newtonRaphson( std::function<double (double)> function,
							         const double target,
							         const double initialGuess,
							         const double tolerance,
							         const size_t maxIterations,
							         const double bumpSize );

		/* @brief The One-dimensional Newton Raphson Minimizer. Given an objective function y = f(x), and a target y value,
	    * the solver attempts to find the value of x for which f(x) minimizes the target.
		* @param[in] function						The objective function: This takes a single double (the x value), and returns a double (the y value)
		* @param[in] initialGuess					An initial guess for the x value.
		* @param[in] tolerance						The y-axis tolerance to achieve
		* @param[in] maxIterations					The maximum number of Newton Raphson iterations to perform before giving up
		* @param[in] bumpSize						The small bump in x value used when calculating a local gradient.
		* @param[in] intervalLowerBound				[OPTIONAL] The interval lowerbound for the problem to be solved
		* @param[in] intervalUpperBound				[OPTIONAL] The interval upperbound for the problem to be solved
		* @param[in] defaultValueToUseOnFailure		[OPTIONAL] Default value on failure, will throw on failure if missing
		* @returns   SolverResults  Returns a struct containing:- 1) The value of x for which: f(x) = target  2) the number of solver interations  3) the slope jacobian value
		*/
		SolverResults newtonRaphsonMinimizer( std::function<double (double)> function,
											  const double initialGuess,
											  const double tolerance,
											  const size_t maxIterations,
											  const double bumpSize,
											  const double intervalLowerBound = std::numeric_limits<double>::quiet_NaN(),
											  const double intervalUpperBound = std::numeric_limits<double>::quiet_NaN(),
											  const double defaultValueToUseOnFailure = std::numeric_limits<double>::quiet_NaN() );

		struct MultiVariateSolverResults
		{
			MultiVariateSolverResults()
				:	solution(std::vector<double>()), 
					numberOfIterations(0),
					jacobian(AQLNumericMatrix()),
					inverseJacobian(AQLNumericMatrix()),
					epsilon(std::vector<double>())
			{};

			~MultiVariateSolverResults() {};

			std::vector<double>		solution;
			unsigned int			numberOfIterations;
			AQLNumericMatrix				jacobian;
			AQLNumericMatrix				inverseJacobian;
            std::vector<double>		epsilon;     // Solver Error
		};

	   /* @brief A multi-dimensional Newton Raphson solver / rootfinder. Given an objective function y = f(x), 
	    * the solver attempts to find the x for which f(x) = 0. Note that x and y are vector quantities.
		* The solver is capable of solving for a subsection of the provided x vector. This is controlled by
		* the inputs "offset" and "numPoints".
		* @param[in]    targetFunction			The objective function: This	takes a double vector (the x value), and returns a double vector (the y values)
		* @param[inout] stateVariables			The x vector values. On input, may contain an initial solution estimate
		*										On input, must be allocated to the correct dimension and initialised either to zero or to
		*										the initial starting estimate of the solution.
		* @param[in]    offset					Starting index into stateVariable vector
		* @param[in]    numPoints				Number of stateVariables ( x values ) to solve for
		* @param[in]    tolerance				Accuracy to which the solution is to be found
		* @param[in]    gradientTolerance       Tolerance check on the gradient. The jacobian gradient must always be larger than the gradientTolerance
		*                                       in order for the solver to continue the search.
		* @param[in]    maxIterations			Maximum number of Newton Raphson iterations to perform
		* @param[in]    initialBumpSize			The bump size to apply to the initial solution estimate, before the main Newton-Raphson loop
		* @param[in]    bumpSize				The delta-bump to use when calculating the jacobian gradient
		* @param[in]	useInverseJacobian		A flag which specifies whether the solver will use the initialInverseJacobian parameter in the first step,
		*										or whether the solver will calculate its own inverseJacobian. Providing the algorithm with the
		*										inverseJacobian and stateVariables from a previous solution will often yield a large performance improvement.
		* @param[in]    initialInverseJacobian	An optional matrix containing an inverse-jacobian gradient.
		* @param[in]    fastConvergenceCheck	Determines how convergence is measured. true = check a single point is converged; false = check all points.
		* @returns		MultiVariateSolverResults Returns a struct containing:- 1) The vector of x for which: f(x) = 0  2) the number of solver interations  3) the inverseJacobian gradient 4) The solver error
		*/
		MultiVariateSolverResults multiVariateNewtonRaphson( std::function<std::vector<double> ( const std::vector<double>& )> targetFunction,
															 std::vector<double>& stateVariables,
															 const size_t offset,
															 const size_t numPoints,
															 const double tolerance,
															 const double gradientTolerance,
															 const size_t maxIterations,
															 const double initialBumpSize,
															 const double bumpSize,
															 const bool useInverseJacobian,
															 const DoubleMatrix& initialInverseJacobian = DoubleMatrix(),
															 const bool fastConvergenceCheck = false );


		/* @brief A multi-dimensional Newton Raphson solver / rootfinder. Given an objective function y = f(x).
		*         This version is a simpler / cutdown version of the above with many parameters defaulted for convenience.
		*
		* @param[in]    targetFunction			The objective function: This	takes a double vector (the x value), and returns a double vector (the y values)
		* @param[inout] stateVariables			The x vector values. On input, may contain an initial solution estimate
		*										On input, must be allocated to the correct dimension and initialised either to zero or to
		*										the initial starting estimate of the solution.
		* @param[in]    tolerance				Accuracy to which the solution is to be found
		* @param[in]    gradientTolerance       Tolerance check on the gradient. The jacobian gradient must always be larger than the gradientTolerance
		*                                       in order for the solver to continue the search.
		* @param[in]    maxIterations			Maximum number of Newton Raphson iterations to perform
		* @param[in]    bumpSize				The delta-bump to use when calculating the jacobian gradient
		* @param[in]	useInverseJacobian		A flag which specifies whether the solver will use the initialInverseJacobian parameter in the first step,
		*										or whether the solver will calculate its own inverseJacobian. Providing the algorithm with the
		*										inverseJacobian and stateVariables from a previous solution will often yield a large performance improvement.
		* @param[in]    initialInverseJacobian	An optional matrix containing an inverse-jacobian gradient.
		* @returns		MultiVariateSolverResults Returns a struct containing:- 1) The vector of x for which: f(x) = 0  2) the number of solver interations  3) the inverseJacobian gradient 4) The solver error
		*/
		MultiVariateSolverResults multiVariateNewtonRaphson( std::function<std::vector<double> ( const std::vector<double>& )> targetFunction,
															 std::vector<double>& stateVariables,
															 const double tolerance,
															 const double gradientTolerance,
															 const size_t maxIterations,
															 const double bumpSize,
															 const bool useInverseJacobian,
															 const DoubleMatrix& initialInverseJacobian = DoubleMatrix() );
	}
}

