// TestCurveSplineInterpolation.cpp

/*
 * @brief			Tests to validate the LASplineInterpolation Class
 * @Created:		26th March 2020
 * @Author:			Arthur Wu
 * @Department:		AlgoQuantHub London Quant Research
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

// Include: Google Test Library
#include <gTest/gTest.h>

#include "LASplineInterpolation.h"

namespace google_test
{
	// Test Tolerance
	const double tolerance = 1e-12;

    TEST(TestCurveSplineInterpolation, UNIT_TestInterpolation_FlatFunction1 )
    {
        // Target Function: f(x) = 1.0
        std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        std::vector<double> values = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
        
		// Interpolator2 uses natural spline
		LASplineInterpolation interp1(false), interp2(true);

		interp1.set(terms, values);
		interp2.set(terms, values);
		
		double x = 3.3;
		EXPECT_NEAR(interp1.value(x), 1.0, tolerance);
		EXPECT_NEAR(interp1.differentiate(x), 0.0, tolerance);
		EXPECT_NEAR(interp1.integrate(2., 5.), 3.0, tolerance);

		EXPECT_NEAR(interp2.value(x), 1.0, tolerance);
		EXPECT_NEAR(interp2.differentiate(x), 0.0, tolerance);
		EXPECT_NEAR(interp2.integrate(2., 5.), 3.0, tolerance);
    }

	TEST(TestCurveSplineInterpolation, UNIT_TestInterpolation_FlatFunction2 )
    {
        // Target Function: f(x) = 1.0
        std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        std::vector<double> values = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
        
		// Interpolator2 uses natural spline
		LASplineInterpolation interp1(false), interp2(true);
	
		interp1.set(terms, values);
		interp2.set(terms, values);
		
		double x = 9.0;
		EXPECT_NEAR( interp1.value(x), 1.0, tolerance );
		EXPECT_NEAR(interp1.differentiate(x), 0.0, tolerance);
		EXPECT_NEAR(interp1.integrate(0.0, 10.), 10.0, tolerance);
	
		EXPECT_NEAR(interp2.value(x), 1.0, tolerance);
		EXPECT_NEAR(interp2.differentiate(x), 0.0, tolerance);
		EXPECT_NEAR(interp2.integrate(0., 10.), 10.0, tolerance);
    }

	TEST(TestCurveSplineInterpolation, UNIT_TestInterpolation_LinearFunction1)
	{
		// Target Function: Linear f(x) = x
		std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		std::vector<double> values = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

		// Interpolator2 uses natural spline
		LASplineInterpolation interp1(false), interp2(true);

		interp1.set(terms, values);
		interp2.set(terms, values);

		double x = 3.3;
		EXPECT_NEAR(interp1.value(x), x, tolerance);
		EXPECT_NEAR(interp1.differentiate(x), 1.0, tolerance);
		EXPECT_NEAR(interp1.integrate(2.0, 5.0), 10.5, tolerance);

		EXPECT_NEAR(interp2.value(x), x, tolerance);
		EXPECT_NEAR(interp2.differentiate(x), 1.0, tolerance);
		EXPECT_NEAR(interp2.integrate(2.0, 5.0), 10.5, tolerance);
	}

	TEST(TestCurveSplineInterpolation, UNIT_TestInterpolation_LinearFunction2)
	{
		// Target Function: Linear f(x) = x
		std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		std::vector<double> values = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	
		// Interpolator2 uses natural spline
		LASplineInterpolation interp1(false), interp2(true);
	
		interp1.set(terms, values);
		interp2.set(terms, values);
	
		double x = 3.3;
		EXPECT_NEAR(interp1.value(x), x, tolerance);
		EXPECT_NEAR(interp1.differentiate(x), 1.0, tolerance);
		EXPECT_NEAR(interp1.integrate(0.0, 10.0), 50.0, tolerance);
	
		EXPECT_NEAR(interp2.value(x), x, tolerance);
		EXPECT_NEAR(interp2.differentiate(x), 1.0, tolerance);
		EXPECT_NEAR(interp2.integrate(0.0, 10.0), 50.0, tolerance);
	}

	TEST(TestCurveSplineInterpolation, UNIT_TestInterpolation_ZigZag)
	{
		// Target Function: f(x) = random chosen
		std::vector<double> terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		std::vector<double> values = { 1, 3, 6, 7, 2, -6, -10, -7.2, -3.14, 0.1, 1.8 };

		// Interpolator2 uses natural spline
		LASplineInterpolation interp1(false), interp2(true);

		interp1.set(terms, values);
		interp2.set(terms, values);

		double x = 3.3;
		EXPECT_NEAR(interp1.value(x), 6.1261518217505095, tolerance);
		EXPECT_NEAR(interp1.differentiate(x), -3.9438784094213424, tolerance);
		EXPECT_NEAR(interp1.integrate(2.1, 5.3), 7.114688309028816, tolerance);

		EXPECT_NEAR(interp2.value(x), 6.1271576417563249, tolerance);
		EXPECT_NEAR(interp2.differentiate(x), -3.942872946681117, tolerance);
		EXPECT_NEAR(interp2.integrate(2.1, 5.3), 7.1128681070871833, tolerance);
	}


}