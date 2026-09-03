// TestSolvers.cpp

/*
 * @brief			Test the Solver Functions
 * @Created:		29th July 2020
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

// Include: Google Test Library
#include <gTest/gTest.h>

#include "Solvers.h"

namespace google_test
{
    TEST( TestNewtonRaphsonSolver, UNIT_SimpleFunction1 )
    {
		// One-dimensional objective function used by the solver:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Functiony =  x^2 + 5x ( Solution: x = 0.0, Minimum at x = -2.5 )
			return x*x + 5*x;
		};

		// Solver Inputs
		double target			= 0.0;
		double initialGuess		= 100.0;
		double eps				= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;

		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphson( function, target, initialGuess, eps, maxIterations, shiftSize );
		
		double actualSolution		= result.solution;
		double expectedSolution		= 0.0;

		const double tolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, tolerance );
    }

	TEST( TestNewtonRaphsonSolver, UNIT_SimpleFunction2 )
    {
		// One-dimensional objective function used by the solver:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = 10x^2 ( Solution: x = 0.000191 )
			return 10.0*x*x;
		};

		// Solver Inputs
		double target			= 0.0;
		double initialGuess		= 100.0;
		double eps				= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;

		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphson( function, target, initialGuess, eps, maxIterations, shiftSize );
		
		double actualSolution		= result.solution;
		double expectedSolution		= 0.000191;

		const double tolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, tolerance );
    }

	TEST( TestNewtonRaphsonSolver, UNIT_SimpleFunction3 )
    {
		// One-dimensional objective function used by the solver:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = -10.0*x^2 ( Solution: x = 0.000191 )
			return -10.0*x*x;
		};

		// Solver Inputs
		double target			= 0.0;
		double initialGuess		= 100.0;
		double eps				= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;

		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphson( function, target, initialGuess, eps, maxIterations, shiftSize );
		
		double actualSolution		= result.solution;
		double expectedSolution		= 0.000191;

		const double tolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, tolerance );
    }

	TEST( TestNewtonRaphsonSolver, UNIT_SimpleFunction4 )
    {
		// One-dimensional objective function used by the solver:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = -x^2 + 10x + 25 ( Solution: x = 12.0710678, Minimum at x = 5.0 )
			return -x*x + 10.0*x + 25;
		};

		// Solver Inputs
		double target			= 0.0;
		double initialGuess		= 100.0;
		double eps				= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;

		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphson( function, target, initialGuess, eps, maxIterations, shiftSize );
		
		double actualSolution		= result.solution;
		double expectedSolution		= 12.0710678;

		const double tolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, tolerance );
    }

	TEST( TestNewtonRaphsonSolver, UNIT_SimpleFunction5 )
    {
		// One-dimensional objective function used by the solver:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = 5x^2 + 10x ( Solution: x = 0.0, Minimum at x = -1.0 )
			return 5.0*x*x + 10.0*x;
		};

		// Solver Inputs
		double target			= 0.0;
		double initialGuess		= 100.0;
		double eps				= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;

		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphson( function, target, initialGuess, eps, maxIterations, shiftSize );
		
		double actualSolution		= result.solution;
		double expectedSolution		= 0.0;

		const double tolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, tolerance );
    }


    TEST( TestNewtonRaphsonMinimizer, UNIT_SimpleFunction1 )
    {
		// One-dimensional objective function used by the mimimzer:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = x^2 + 5x ( Solution: x = 0.0, Minimum at x = -2.5 )
			return x*x + 5.0*x;
		};

		// Solver Inputs
		double initialGuess		= 100.0;
		double tolerance		= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;

		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphsonMinimizer( function, initialGuess, tolerance, maxIterations, shiftSize );
		
		double actualSolution		= result.solution;
		double expectedSolution		= -2.5;

		const double testTolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, testTolerance );
    }


    TEST( TestNewtonRaphsonMinimizer, UNIT_SimpleFunction2 )
    {
		// One-dimensional objective function used by the mimimzer:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = x^2 - 5x + 10 ( Minimum at x = 2.5 )
			return x*x - 5.0*x + 10.0;
		};

		// Solver Inputs
		double initialGuess		= 100.0;
		double tolerance		= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;

		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphsonMinimizer( function, initialGuess, tolerance, maxIterations, shiftSize );
		
		double actualSolution		= result.solution;
		double expectedSolution		= 2.5;

		const double testTolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, testTolerance );
    }


    TEST( TestNewtonRaphsonMinimizer, UNIT_SimpleFunction3 )
    {
		// One-dimensional objective function used by the mimimzer:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = 2x^2 + 5x + 10 ( Minimum at x = -1.25 )
			return 2.0*x*x + 5.0*x + 10.0;
		};

		// Solver Inputs
		double initialGuess		= 100.0;
		double tolerance		= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;

		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphsonMinimizer( function, initialGuess, tolerance, maxIterations, shiftSize );
		
		double actualSolution		= result.solution;
		double expectedSolution		= -1.25;

		const double testTolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, testTolerance );
    }


    TEST( TestNewtonRaphsonMinimizer, UNIT_SimpleFunction4 )
    {
		// One-dimensional objective function used by the mimimzer:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = 5x^2 - 10x + 10 ( Minimum at x = 1.0 )
			return 5.0*x*x - 10.0*x + 10.0;
		};

		// Solver Inputs
		double initialGuess		= 100.0;
		double tolerance		= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;

		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphsonMinimizer( function, initialGuess, tolerance, maxIterations, shiftSize );
		
		double actualSolution		= result.solution;
		double expectedSolution		= 1.0000;

		const double testTolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, testTolerance );
    }


    TEST( TestNewtonRaphsonMinimizer, UNIT_SimpleFunction5 )
    {
		// One-dimensional objective function used by the mimimzer:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = -x^2 + 10x + 25 ( Solution: x = 12.0710678, Minimum at x = 5.0 )
			return -x*x + 10.0*x + 25.0;
		};

		// Solver Inputs
		double initialGuess		= 100.0;
		double tolerance		= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;

		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphsonMinimizer( function, initialGuess, tolerance, maxIterations, shiftSize );
		
		double actualSolution		= result.solution;
		double expectedSolution		= 5.0000;

		const double testTolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, testTolerance );
    }


    TEST( TestNewtonRaphsonMinimizer, UNIT_Minimize_With_IntervalBounds1 )
    {
		// One-dimensional objective function used by the mimimzer:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = x^2 - 100x (Solution: x = 100, Minimum at x = 50 )
			return x*x - 100.*x;
		};

		// Solver Inputs
		double initialGuess		= 10.0;
		double tolerance		= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;
		
		const double intervalLowerBound = -10.0;
		const double intervalUpperBound = 10.0;
		
		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphsonMinimizer( function, initialGuess, tolerance, maxIterations, shiftSize, intervalLowerBound, intervalUpperBound );
		
		double actualSolution		= result.solution;
		double expectedSolution		= intervalUpperBound;

		const double testTolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, testTolerance );
    }


    TEST( TestNewtonRaphsonMinimizer, UNIT_Minimize_With_IntervalBounds2 )
    {
		// One-dimensional objective function used by the mimimzer:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = x^2 + 100x (Solution: x = 0, Minimum at x = -50 )
			return x*x + 100.*x;
		};

		// Solver Inputs
		double initialGuess		= -10.0;
		double tolerance		= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;
		
		const double intervalLowerBound = -10.0;
		const double intervalUpperBound = 10.0;
		
		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphsonMinimizer( function, initialGuess, tolerance, maxIterations, shiftSize, intervalLowerBound, intervalUpperBound );
		
		double actualSolution		= result.solution;
		double expectedSolution		= intervalLowerBound;

		const double testTolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, testTolerance );
    }


    TEST( TestNewtonRaphsonMinimizer, UNIT_Minimize_Without_IntervalBounds1 )
    {
		// One-dimensional objective function used by the mimimzer:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = x^2 - 100x (Solution: x = 100, Minimum at x = 50 )
			return x*x - 100.*x;
		};

		// Solver Inputs
		double initialGuess		= 10.0;
		double tolerance		= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;
		
		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphsonMinimizer( function, initialGuess, tolerance, maxIterations, shiftSize );
		
		double actualSolution		= result.solution;
		double expectedSolution		= 50.0;

		const double testTolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, testTolerance );
    }


    TEST( TestNewtonRaphsonMinimizer, UNIT_Minimize_Without_IntervalBounds2 )
    {
		// One-dimensional objective function used by the mimimzer:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = x^2 + 100x (Solution: x = 0, Minimum at x = -50 )
			return x*x + 100.*x;
		};

		// Solver Inputs
		double initialGuess		= -10.0;
		double tolerance		= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;
		
		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphsonMinimizer( function, initialGuess, tolerance, maxIterations, shiftSize );
		
		double actualSolution		= result.solution;
		double expectedSolution		= -50.0;

		const double testTolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, testTolerance );
    }

	TEST( TestNewtonRaphsonMinimizer, UNIT_Minimize_With_UpperBoundOnly )
    {
		// One-dimensional objective function used by the mimimzer:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = x^2 - 100x (Solution: x = 100, Minimum at x = 50 )
			return x*x - 100.*x;
		};

		// Solver Inputs
		double initialGuess		= 10.0;
		double tolerance		= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;
		
		const double intervalLowerBound = std::numeric_limits<double>::quiet_NaN();
		const double intervalUpperBound = 10.0;
		
		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphsonMinimizer( function, initialGuess, tolerance, maxIterations, shiftSize, intervalLowerBound, intervalUpperBound );
		
		double actualSolution		= result.solution;
		double expectedSolution		= intervalUpperBound;

		const double testTolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, testTolerance );
    }


    TEST( TestNewtonRaphsonMinimizer, UNIT_Minimize_With_LowerBoundOnly )
    {
		// One-dimensional objective function used by the mimimzer:
		// Must list parameters requiring access to within the this lambda function [] parentheses, next to this
		// The x value is the variable which the solver will adjust in order to obtain the solution.
		auto function = [this] ( const double x )
		{
			// Test Function y = x^2 + 100x (Solution: x = 0, Minimum at x = -50 )
			return x*x + 100.*x;
		};

		// Solver Inputs
		double initialGuess		= -10.0;
		double tolerance		= 1.0e-6;
		size_t maxIterations	= 100;
		double shiftSize		= 1.0e-6;
		
		const double intervalLowerBound = -10.0;
		const double intervalUpperBound = std::numeric_limits<double>::quiet_NaN();
		
		// Solver Results Contain: Solution, nInterations and Jacobian
		auto result = etrading::solvers::newtonRaphsonMinimizer( function, initialGuess, tolerance, maxIterations, shiftSize, intervalLowerBound, intervalUpperBound );
		
		double actualSolution		= result.solution;
		double expectedSolution		= intervalLowerBound;

		const double testTolerance = 1e-4;
        EXPECT_NEAR( actualSolution, expectedSolution, testTolerance );
    }

}