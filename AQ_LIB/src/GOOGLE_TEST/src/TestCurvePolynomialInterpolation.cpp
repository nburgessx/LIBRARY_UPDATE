// TestCurvePolynomialInterpolation.cpp

/*
 * @brief			Tests to validate the Polynomial Interpolation Class
 * @Created:		2nd January 2020
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

// Include: Google Test Library
#include <gTest/gTest.h>

#include "PolynomialInterpolation.h"

namespace google_test
{
	// Test Tolerance
	const double tolerance = 1e-12;

    TEST( TestCurvePolynomialInterpolation, UNIT_TestInterpolation_PolynomialDegreeOne_OnNodePoints )
    {
        // Target Function: Constant value of 1.0 bounded from 0.0 to 10.0
        std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        std::vector<double> values = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
        
		// Polynomial Interpolation
		size_t degreeOne = 1;
		etrading::PolynomialInterpolation polynomial( terms, values, degreeOne );
		std::vector<double> results = polynomial.interpolate( terms );

		// Assert if dimensions don't match
		ASSERT_EQ( results.size(), values.size() );
		for( size_t i = 0; i < results.size(); ++i )
		{
			EXPECT_NEAR( values[i], results[i], tolerance );
		}

		// test differentiate
		auto dydx_results = polynomial.differentiate(terms);
		std::vector<double> dydx_values = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  };

		// Assert if dimensions don't match
		ASSERT_EQ(dydx_results.size(), dydx_values.size());
		for (size_t i = 0; i < dydx_results.size(); ++i)
		{
			EXPECT_NEAR(dydx_values[i], dydx_results[i], tolerance);
		}
    }

	TEST( TestCurvePolynomialInterpolation, UNIT_TestInterpolation_PolynomialDegreeOne_BetweenNodePoints )
    {
        // Target Function: Constant value of 1.0 bounded from 0.0 to 10.0
        std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        std::vector<double> values = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
        
		// Polynomial Interpolation
		size_t degreeOne = 1;
		etrading::PolynomialInterpolation polynomial( terms, values, degreeOne );
		std::vector<double> testPoints = { 0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5 };
		std::vector<double> results = polynomial.interpolate( testPoints );

		// Assert if dimensions don't match
		ASSERT_EQ( results.size(), testPoints.size() );
		for( size_t i = 0; i < results.size(); ++i )
		{
			EXPECT_NEAR( 1.0, results[i], tolerance );
		}
    }

    TEST( TestCurvePolynomialInterpolation, UNIT_TestIntegration_PolynomialDegreeOne_SingleIntegration )
    {
        // Target Function: Constant value of 1.0 bounded from 0.0 to 10.0
        std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        std::vector<double> values = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
        
		// Polynomial Interpolation
		size_t degreeOne = 1;
		etrading::PolynomialInterpolation polynomial( terms, values, degreeOne );
		double result = polynomial.integrate( 0.0, 10.0 );

		// Expect Result to be 10
		EXPECT_NEAR( 10.0, result, tolerance );
    }

    TEST( TestCurvePolynomialInterpolation, UNIT_TestIntegration_PolynomialDegreeOne_MultipleIntegration )
    {
        // Target Function: Constant value of 1.0 bounded from 0.0 to 10.0
        std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        std::vector<double> values = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
        
		// Polynomial Interpolation
		size_t degreeOne = 1;
		etrading::PolynomialInterpolation polynomial( terms, values, degreeOne );
		std::vector<double> lowerBounds = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		std::vector<double> upperBounds = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		std::vector<double> results = polynomial.integrate( lowerBounds, upperBounds );

		// Assert if dimensions don't match
		ASSERT_EQ( results.size(), lowerBounds.size() );
		for( size_t i = 0; i < results.size(); ++i )
		{
			// Expect Area of Each Unit Abscissae to be 1.0
			EXPECT_NEAR( 1.0, results[i], tolerance );
		}
    }

    TEST( TestCurvePolynomialInterpolation, UNIT_TestInterpolation_PolynomialDegreeTwo_OnNodePoints )
    {
        // Target Function: y = x^2
        std::vector<double> terms = { -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5 };
        std::vector<double> values = {  25, 16, 9, 4, 1, 0, 1, 4, 9, 16, 25 };
        
		// Polynomial Interpolation
		size_t degreeTwo = 2;
		etrading::PolynomialInterpolation polynomial( terms, values, degreeTwo );
		std::vector<double> results = polynomial.interpolate( terms );

		// Assert if dimensions don't match
		ASSERT_EQ( results.size(), values.size() );
		for( size_t i = 0; i < results.size(); ++i )
		{
			EXPECT_NEAR( values[i], results[i], tolerance );
		}

		// test differentiate
		auto dydx_results = polynomial.differentiate(terms);
		std::vector<double> dydx_values = { -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10 };

		// Assert if dimensions don't match
		ASSERT_EQ(dydx_results.size(), dydx_values.size());
		for (size_t i = 0; i < dydx_results.size(); ++i)
		{
			EXPECT_NEAR(dydx_values[i], dydx_results[i], tolerance);
		}
    }

	TEST( TestCurvePolynomialInterpolation, UNIT_TestIntegration_PolynomialDegreeZero )
    {
        // Target Function: Constant value of 1.0 bounded from 0.0 to 10.0
        const std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> values = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
        
		// Integrands size = terms.size() - 1
		const std::vector<double> expectedIntegrands = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

		// Polynomial Interpolation
		const size_t degreeZero = 0;
		etrading::PolynomialInterpolation polynomial( terms, values, degreeZero );
		
		const size_t nIntegrands = terms.size() -1 ;
		std::vector<double> actualIntegrands( nIntegrands, 0.0 );

		// Assert if dimensions don't match
		ASSERT_EQ( actualIntegrands.size(), expectedIntegrands.size() );

		// Start from index 1
		for( size_t i = 1; i < actualIntegrands.size(); ++i )
		{
			const double lowerLimit = terms[i-1];
			const double upperLimit = terms[i];

			// index base 1, therefore i-1
			actualIntegrands[i-1] = polynomial.integrate( lowerLimit, upperLimit );
			EXPECT_NEAR( expectedIntegrands[i-1], actualIntegrands[i-1], tolerance ) << "Index i: " << i-1 << std::endl;
		}
    }

	TEST( TestCurvePolynomialInterpolation, UNIT_TestIntegration_PolynomialDegreeOne )
    {
        // Target Function: y = x, bounded from 0.0 to 10.0
        const std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> values = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        
		// Integrands size = terms.size()-1
		// When Integrating from 0 to terms[i]
		const std::vector<double> expectedIntegrands = { 0.5, 2.0, 4.5, 8, 12.5, 18, 24.5, 32, 40.5, 50 };

		// Polynomial Interpolation
		const size_t degreeOne = 1;
		etrading::PolynomialInterpolation polynomial( terms, values, degreeOne );
		
		const size_t nIntegrands = terms.size() -1 ;
		std::vector<double> actualIntegrands( nIntegrands, 0.0 );

		// Assert if dimensions don't match
		ASSERT_EQ( actualIntegrands.size(), expectedIntegrands.size() );

		// Integrate from 0 to terms[i], Start from index 1
		for( size_t i = 1; i < actualIntegrands.size(); ++i )
		{
			const double lowerLimit = terms[0];
			const double upperLimit = terms[i];

			// index base 1, therefore i-1
			actualIntegrands[i-1] = polynomial.integrate( lowerLimit, upperLimit );
			EXPECT_NEAR( expectedIntegrands[i-1], actualIntegrands[i-1], tolerance ) << "Index i: " << i-1 << std::endl;
		}
    }

	TEST( TestCurvePolynomialInterpolation, UNIT_TestDifferentiation_PolynomialDegreeZero )
    {
        // Target Function: Constant value of 1.0 bounded from 0.0 to 10.0
        const std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> values = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

		const std::vector<double> expectedDifferentiations = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		// Polynomial Interpolation
		const size_t degreeZero = 0;
		etrading::PolynomialInterpolation polynomial( terms, values, degreeZero );
		
		const size_t nDifferentiations = terms.size();
		std::vector<double> actualDifferentiations( nDifferentiations, 0.0 );

		// Assert if dimensions don't match
		ASSERT_EQ( actualDifferentiations.size(), expectedDifferentiations.size() );
		for( size_t i = 0; i < actualDifferentiations.size(); ++i )
		{
			actualDifferentiations[i] = polynomial.differentiate( terms[i] );
			EXPECT_NEAR( expectedDifferentiations[i], actualDifferentiations[i], tolerance ) << "Index i: " << i << std::endl;
		}
    }

	TEST( TestCurvePolynomialInterpolation, UNIT_TestDifferentiation_PolynomialDegreeOne )
    {
        // Target Function: y = x, bounded from 0.0 to 10.0
        const std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> values = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

		const std::vector<double> expectedDifferentiations = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

		// Polynomial Interpolation
		const size_t degreeOne = 1;
		etrading::PolynomialInterpolation polynomial( terms, values, degreeOne );
		
		const size_t nDifferentiations = terms.size();
		std::vector<double> actualDifferentiations( nDifferentiations, 0.0 );

		// Assert if dimensions don't match
		ASSERT_EQ( actualDifferentiations.size(), expectedDifferentiations.size() );
		for( size_t i = 0; i < actualDifferentiations.size(); ++i )
		{
			actualDifferentiations[i] = polynomial.differentiate( terms[i] );
			EXPECT_NEAR( expectedDifferentiations[i], actualDifferentiations[i], tolerance ) << "Index i: " << i << std::endl;
		}
    }
}