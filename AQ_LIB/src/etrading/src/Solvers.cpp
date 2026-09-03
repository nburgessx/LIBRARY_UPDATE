/*
 * @brief			MLIBQ Solver Framework
 * @Created:		1st June 2017
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "Solvers.h"
#include "LACoreAppError.h"
#include "LABasic.h"
#include "ExceptionMacros.h"

#include <limits>
#include <boost/math/tools/roots.hpp> // For TOMS748 dataInstance finder algorithm


namespace etrading
{
	namespace solvers
	{

		/* @brief A helper class functor which determines if the bracket ( a, b ) is within the specified epsilon.
		*         Used by the toms478 dataInstance finder.
		* 
		*/
		class Tolerance {
		public:
			Tolerance( double eps) : eps_( eps ) {}
			bool operator()( double a, double b )
			{
				return ( fabs(b - a) <= eps_);
			}
		private:
			double eps_;
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
							   const bool functionIsRising,
							   const double bracketFactor )
		{
			// Initially our chosen max iterations, but updated with actual.
			boost::uintmax_t iterations = (boost::uintmax_t) maxIterations;

			Tolerance tol( tolerance );
			std::pair<double,double> dataInstance = boost::math::tools::bracket_and_solve_root( function, initialGuess, bracketFactor, functionIsRising, tol, iterations );
			
			// Take the solution to be the mid-point of the final bracket.
			const double solution = 0.5 * ( dataInstance.first + dataInstance.second );
			const double epsilon  = function( solution );

			// Populate the Results Struct
            SolverResults results;

            results.solution                = solution;
            results.numberOfIterations      = (unsigned int) iterations;
            results.jacobian                = std::numeric_limits<double>::quiet_NaN();
            results.epsilon                 = epsilon;

			return results;
		}


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
                                       const size_t maxIntervalExpansionAttempts,
                                       const bool throwIfMultipleSolutions,
                                       const double shiftSizeForMultipleSolutionCheck )
		{
            MLIB_REQUIRE( initialLowerBound < initialUpperBound, "No Solution Found: Invalid Problem Interval; Bisection Method LowerBound must be smaller than the UpperBound" )
            
            double lowerBound               = initialLowerBound;
            double lowerValue               = function( lowerBound );
            
            double upperBound               = initialUpperBound;
            double upperValue               = function( upperBound );

            unsigned int iterationCount     = 1;
            double midPoint                 = lowerBound;
            double midValue                 = lowerValue;
            

            // 1. Check Boundary Conditions
            // --------------------------------------------------------------------------
            const bool isLowerValueZero     = MLIB_IS_EQUAL_ZERO( lowerValue );
            const bool isUpperValueZero     = MLIB_IS_EQUAL_ZERO( upperValue );

            if ( isLowerValueZero || isUpperValueZero )
            {
                if( isLowerValueZero && isUpperValueZero )
                {
                    MLIB_THROW("No Solution Found: Multiple Solutions Found in the Problem Interval.")
                }
                
                midPoint = isLowerValueZero ? lowerBound : upperBound;
                midValue = isLowerValueZero ? lowerValue : upperValue;
            }
            else
            {
                // 2. Attempt to Expand Interval if it does not contain a solution
                // --------------------------------------------------------------------------
                
                // A solution exists in the interval when the target function changes value
                // i.e. No dataInstance found in the problem interval
                if ( MLIB_IS_GREATER_THAN_ZERO( lowerValue * upperValue ) )
                {
                    if ( maxIntervalExpansionAttempts > 0 )
                    {
                        // Expand the Problem Search Interval
                        size_t intervalExpansionAttempt = 1;
                        
                        while(  MLIB_IS_GREATER_THAN_ZERO( lowerValue * upperValue ) && intervalExpansionAttempt <= maxIntervalExpansionAttempts )
                        {
                            // Expand Bounds
                            lowerBound -= std::fabs( lowerBound );
                            upperBound += std::fabs( upperBound );
                            
                            // Update Values
                            lowerValue = function( lowerBound ) ;
                            upperValue = function( upperBound );

                            ++intervalExpansionAttempt;
                        }
                    }
                    MLIB_REQUIRE( MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO( lowerValue * upperValue ), "No Solution Found: Problem interval does not contain a solution." )
                }
            
                // 3. Biscect the Interval and Search for a Soltion
                // --------------------------------------------------------------------------

                while( ( upperBound - lowerBound ) > minimumIntervalSize && iterationCount < maxIterations )
                {
                    lowerValue  = function( lowerBound );
                    upperValue  = function( upperBound );

                    midPoint    = ( upperBound + lowerBound ) / 2.0;
                    midValue    = function( midPoint );
                    
                    // Check for a Solution in the Lower Half of the Interval
                    // i.e. we have a dataInstance in the interval when the function value changes sign
                    if ( MLIB_IS_LESS_THAN_ZERO( lowerValue * midValue ) )
                    {
                        upperBound = midPoint;
                    }

                    // Solution must be in the Upper Half of the interval if it is not in the Lower Half
                    else
                    {
                        lowerBound = midPoint;
                    }
                    
                    // Check if the MidValue is the Solution, where MidValue is the target function - target price
                    if ( std::fabs( midValue ) <= tolerance )
                    {
                        break;
                    }

                    // Update the Iteration Count
                    iterationCount++;

                }
                MLIB_REQUIRE( iterationCount <= maxIterations, "No Solution Found: The Bisection Solver ran out of iterations." )
            }


            // Check for Convergence
            // Mid Value is the target function - target price
            MLIB_REQUIRE( std::fabs( midValue ) <= tolerance, "No Solution Found: The Bisection Solver failed to converge." )
            
            // Do not Allow Underdetermined Solutions i.e. Many solutions giving the same target price 
            // This the same as ensuring the derivative of the solution is non-zero
            if ( throwIfMultipleSolutions )
            {
                const double changeInInterval   = ( shiftSizeForMultipleSolutionCheck < 1e-8 ) ? 1e-8 : shiftSizeForMultipleSolutionCheck;
                const double dMidValue          = function( midPoint + changeInInterval );
                const double solutionSlope      = ( midValue - dMidValue ) / changeInInterval;
                MLIB_REQUIRE( MLIB_IS_GREATER_THAN( std::fabs(solutionSlope), gradientTolerance ), "No Solution Found: Multiple Solutions Found in the Problem Interval." )
            }

            // Populate the Results Struct
            SolverResults results;

            results.solution                = midPoint;
            results.numberOfIterations      = iterationCount;
            results.jacobian                = std::numeric_limits<double>::quiet_NaN();  // No Jacobian when using Bisection Method
            results.epsilon                 = std::fabs( midValue );

			return results;
		}


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
							         const double bumpSize )
		{
			// Notation:	 x1 = dataInstance for (n+1)th iteration
			//				 x0 = dataInstance for nth iteration
			//				 y0 = Distance from Search Target: Function valuation for the nth iteration MINUS target
			//				dy0 = derivative of function valuation MINUS target for nth iteration
			
			MLIB_REQUIRE( MLIB_IS_GREATER_THAN_ZERO( bumpSize ), "Invalid Optimzation Set-Up: must be greater than zero" )

			double x1 = initialGuess;
			double x0 = initialGuess;
        
            double epsilon              = 0.0;  // Solver Error
			unsigned int iterationCount = 0;    // Solver Interations
            double dy0                  = 0.0;  // Solver Slope

			for( iterationCount = 0; iterationCount < maxIterations; ++iterationCount )
			{
				double functionValue  = function( x0 );

				// If the function valuation equals the target input, exit the for loop since we have found our result
				epsilon = functionValue - target;
                if ( std::abs( epsilon ) < tolerance )
				{
					break;
				}
            
				double funcValueWithUpwardBump		= function( x0 + bumpSize );

				double y0							= functionValue - target;
				dy0									= ( funcValueWithUpwardBump - functionValue ) / bumpSize; // Difference, target terms cancel
            
				// Divide by zero guard
				MLIB_THROW_IF( dy0 == 0, "No Solution Found. The Newton-Raphson derivative has zero value." )

				// Newton-Raphson Formula:
                x1 = x0 - ( y0 / dy0 );

				// Update the Initial Guess for Next Iteration
				x0 = x1;
			}
        
			// We already checked if the target value = solver value above, so if we have reached our max iterations we have not found a solution
			MLIB_THROW_IF( iterationCount == maxIterations, "No Solution Found. The Newton-Raphson solver ran out of iterations." )
			
			const double solution = x1;

            // Populate the Results Struct
            SolverResults results;

            results.solution                = solution;
            results.numberOfIterations      = iterationCount;
            results.jacobian                = dy0;
            results.epsilon                 = epsilon;

			return results;
		}
		

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
											  const double intervalLowerBound,
											  const double intervalUpperBound,
											  const double defaultValueToUseOnFailure )
		{
			// Notation:	 x1 = dataInstance for (n+1)th iteration
			//				 x0 = dataInstance for nth iteration
			//				dy1 = first derivative of function valuation MINUS target for nth iteration
			//				dy2 = second derivative of function valuation MINUS target for nth iteration
			
			MLIB_REQUIRE( MLIB_IS_GREATER_THAN_ZERO( bumpSize ), "Invalid Optimzation Set-Up: must be greater than zero" )

			// Apply Interval Bounds if the interval lower- and upper-bounds are not NaN
			const bool applyIntervalLowerBound = !std::isnan( intervalLowerBound );
			const bool applyIntervalUpperBound = !std::isnan( intervalUpperBound );
			
			// Validation of the Optional Interval Bounds
			if( applyIntervalLowerBound && applyIntervalUpperBound )
			{
				MLIB_REQUIRE( intervalLowerBound < intervalUpperBound, "Invalid Optimzation Set-Up: The Interval Lowerbound must be less than the Upperbound" )
			}
			
			// Validation of the Optional Interval Bounds
			MLIB_THROW_IF( applyIntervalUpperBound && (initialGuess > intervalUpperBound), "Invalid Optimzation Set-Up: Initial Guess must not be greater than the Interval Upperbound" )
			MLIB_THROW_IF( applyIntervalLowerBound && (initialGuess <  intervalLowerBound), "Invalid Optimzation Set-Up: Initial Guess must not be lower than the Interval Lowerbound" )

			double x1 = initialGuess;
			double x0 = initialGuess;
        
            double epsilon					= 0.0;  // Minimizer Error
			unsigned int iterationCount		= 0;    // Minimizer Interations
            double dy1						= 0.0;  // Minimizer First Derivative
			double dy2						= 0.0;	// Minimizer Second Derivative

			for( iterationCount = 0; iterationCount < maxIterations; ++iterationCount )
			{
				double functionValue  = function( x0 );

				double funcValueWithUpBump			= function( x0 + bumpSize );
				double funcValueWithDownBump		= function( x0 - bumpSize );

				// Using Central Differencing
				dy1		= ( funcValueWithUpBump - funcValueWithDownBump ) / (2.0 * bumpSize );					
				dy2		= ( funcValueWithUpBump - 2 * functionValue + funcValueWithDownBump ) / ( bumpSize * bumpSize );

				// Divide by zero guard
				MLIB_THROW_IF( dy2 == 0.0, "No Minimum Value Found. The underlying problem has no local minimum since it is constant or linear having a second derivative zero." );

				// Newton-Raphson Mimimization Formula:
                x1 = x0 - ( dy1 / dy2 );

				// Don't Allow the Minimizer to go outside the interval bounds
				if( applyIntervalLowerBound && x1 < intervalLowerBound )
				{
					x1 = intervalLowerBound;
				}
				else if( applyIntervalUpperBound && x1 > intervalUpperBound )
				{
					x1 = intervalUpperBound;
				}

				// If the function value has converged i.e. x(n+1) = x(n) then exit the for loop since we have found our result
                epsilon = abs( x1 - x0 );
				if ( epsilon < tolerance )
				{
					break;
				}
				
				// Update the Initial Guess for Next Iteration
				x0 = x1;
				
			}
        
			double solution = x1;

			// We already checked for convergence above, so if we have reached our max iterations we have not found a solution
			if ( iterationCount >= maxIterations )
			{
				MLIB_THROW_IF( std::isnan( defaultValueToUseOnFailure ), "No Minimum Found. The Newton-Raphson solver ran out of iterations." )
				solution = defaultValueToUseOnFailure;
			}

            // Populate the Results Struct
            SolverResults results;

            results.solution                = solution;
            results.numberOfIterations      = iterationCount;
            results.jacobian                = dy1;
            results.epsilon                 = epsilon;

			return results;
		}


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
															 const DoubleMatrix& initialInverseJacobian,
															 const bool fastConvergenceCheck )
		{
			// Initial evaluation of the targetFunction using the provided stateVariables.
			std::vector<double> y0 = targetFunction( stateVariables );

			// 1.	Fast Rebuild - Check if Previous Solution is Still the Current Solution
			// *********************************************************************************************

			bool usePreviousSolution = false;

            // Use previous solution if it exists and within tolerance
            if ( initialInverseJacobian.size() != 0 )
            {
                usePreviousSolution = true;
			    for (unsigned int i = 0; i < numPoints; ++i)
			    {
				    // A solution is considered acceptable only when all y values are within tolerance
				    if ( LAMath::abs( y0[i] ) >= tolerance )
				    {
					    usePreviousSolution = false;
					    break;
				    }
			    }
            }

			if( usePreviousSolution )
			{
				MultiVariateSolverResults results;
			
				results.solution                = stateVariables;
				results.numberOfIterations      = 0u;
				results.inverseJacobian         = initialInverseJacobian;
				// Jacobian Not Available - We could invert the Inverse Jacobian, but we choose not to for performance reasons
				results.jacobian				= LAMatrix(); 
				results.epsilon                 = y0;

				return results;
			}

			// 2.	Apply Initial Bump if Not Using Inverse Jacobian
			// *********************************************************************************************
			
			if ( !useInverseJacobian )
			{
				// For the initial step, apply a small parallel bump to the stateVariables in order to verify that the targetFunction changes.
				for (unsigned int i = 0; i < numPoints; ++i)
				{
					stateVariables[ offset + i ] += initialBumpSize;
				}
			}

			// Evaluate the targetFunction in preparation for the first iteration
			std::vector<double> y1 = targetFunction( stateVariables );

			// Stores the inverseJacobian matrix at each iteration
			LAMatrix inverseJacobian( numPoints, numPoints );
			LAMatrix jacobianMatrix;

			bool solutionFound = false;
			unsigned int iterationCount;
			for( iterationCount = 0; iterationCount < maxIterations; ++iterationCount )
			{
				// 3.	Calculate the Inverse Jacobian
				// *********************************************************************************************

				if ( iterationCount == 0 && useInverseJacobian )
				{
					// Retrieve the initialInverseJacobian matrix if we have been requested to use it.
					// A small change in input variables can often be solved for using the initial estimate of the gradient
					// and a single matrix-multiply, without needing to re-run the full Newton-Raphson loop.
					for (unsigned int i = 0; i < numPoints; ++i)
					{
						const DoubleVector& dataVec = initialInverseJacobian[i];
						for (unsigned int j = 0; j < numPoints; ++j)
						{
							const double value = dataVec[j];
							inverseJacobian.setValue(j, i, value);
						}
					}
				}
				else
				{
					// 4.	Check Target Function has Not Converged to the Wrong Solution
					// *********************************************************************************************
					// Sanity check: Verify that the target function is still changing and is sensitive to the updated state variables.
					bool derivativeIsZero = true;
					for ( unsigned int i = 0; i < numPoints; ++i )
					{
						if ( LAMath::abs( y0[i] - y1[i]) >= gradientTolerance )
						{
							derivativeIsZero = false;
							break;
						}
					}
					MLIB_THROW_IF( derivativeIsZero, "No Solution Found. The Newton-Raphson derivative has zero value." );

					// Compute the inverse-jacobian matrix by bumping the state variables and revaluing the target function
					jacobianMatrix.resize(numPoints, numPoints);
					for (unsigned int i = 0; i < numPoints; ++i)
					{
						std::vector<double> bumpedStateVariables = stateVariables;
						bumpedStateVariables[ offset + i] += bumpSize;
					
						std::vector<double> y0WithBump = targetFunction( bumpedStateVariables );

						for (unsigned int j = 0; j < numPoints; ++j)
						{
							const double deltaValue = ( y1[j] - y0WithBump[j] ) / bumpSize;
							jacobianMatrix.setValue(j, i, deltaValue);
						}
					}
					inverseJacobian = jacobianMatrix.inverseMatrix();
				}

				// 5.	Apply Newton-Raphson Formula
				// *********************************************************************************************

				// Compute Newton Raphson adjustment to the state variables
				
				// X(n+1) = X(n) + f(X(n)) / f'(X(n))
				// or alternatively
				// X(n+1) = X(n) + J_inverse * f(X(n)),		where J_inverse = Inverse Jacobian = 1 / f'(X(n))
				
				//	a)		Newton-Raphson Term:	f(X(n))
				LAMatrix functionValues( y1 );

				//	b)		Newton-Raphson Term:	J_inverse * f(X(n))
				LAMatrix inverseJ_times_function = inverseJacobian * functionValues;
				
				for (unsigned int i = 0; i < numPoints; ++i)
				{
				//	c)		Newton-Raphson Term:	X(n) + J_Inverse * f(X(n))
					stateVariables[offset + i] += inverseJ_times_function.getValue(i, 0);
				}
				
				//	d)		Newton-Raphson:	X(n+1) = X(n) + J_Inverse * f(X(n))
				y0 = y1;
				y1 = targetFunction( stateVariables );

				// 6.	Check if Solution Found
				// ************************************************************************************************
				if ( fastConvergenceCheck )
				{
					// When 'fastConvergenceCheck' is TRUE, we stop the search when *** ONE *** of the y values is within tolerance.
					solutionFound = false;
					for (unsigned int i = 0; i < numPoints; ++i)
					{
						// Exit as soon as ONE of the y values is within tolerance
						if ( LAMath::abs( y1[i] ) < tolerance )
						{
							solutionFound = true;
							break;
						}
					}
				}
				else
				{
					// When 'fastConvergenceCheck' is FALSE, we check that *** ALL *** y values are within the tolerance level.
					solutionFound = true;
					for (unsigned int i = 0; i < numPoints; ++i)
					{
						// A solution is considered acceptable only when all y values are within tolerance
						if ( LAMath::abs( y1[i] ) >= tolerance )
						{
							solutionFound = false;
							break;
						}
					}
				}
			    
				// Exit Newton-Raphson loop  if a solution has been found
				if ( solutionFound )
				{
					break;
				}
			}

			// 7.	Populate Results
			// ************************************************************************************************
			MLIB_REQUIRE( solutionFound, "No Solution Found. The Newton-Raphson solver ran out of iterations." );

            MultiVariateSolverResults results;
			
			results.solution                = stateVariables;
            results.numberOfIterations      = iterationCount;
            results.inverseJacobian         = inverseJacobian;
			results.jacobian				= jacobianMatrix;
			results.epsilon                 = y1;

			return results;
		}


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
															 const DoubleMatrix& initialInverseJacobian )
		{
			// Apply sensible defaults for this simpler solver
			
			// Initial solver bump before the main newton-raphson iteration
			const double initialBumpSize = bumpSize;

			// By default solve for the entire stateVariables vector
			const size_t offset = 0;
			const size_t numPoints = stateVariables.size();

			// By default, check all of the outputs y = f(x) for convergence
			const bool fastConvergenceCheck = false;

			return multiVariateNewtonRaphson( targetFunction,
											  stateVariables,
											  offset,
											  numPoints,
											  tolerance,
											  gradientTolerance,
											  maxIterations,
											  initialBumpSize,
											  bumpSize,
											  useInverseJacobian,
											  initialInverseJacobian,
											  fastConvergenceCheck );
		}
	}
}

