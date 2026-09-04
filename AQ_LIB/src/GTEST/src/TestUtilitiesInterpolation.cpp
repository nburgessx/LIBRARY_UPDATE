// TestUtilitiesInterpolation.cpp

#include "tryAqToolsInterpolation.h"
#include <algorithm>					// std::for_each


// Include: Google Test Library
#include <gTest/gTest.h>


TEST(TestUtilitiesInterpolation, UNIT_LinearInterpolation )
{
	const std::string interpolation = "Linear";
	const double tolerance = 1e-10;

	// Interpolation Table - Node Points and Values
	const std::vector<double> xValues = { 2, 3, 4, 5 };
	const std::vector<double> yValues = { 20, 30, 40, 50};
	
	// Interpolation Points
	const std::vector<double> interpValues		= { 1, 2, 3, 4, 5, 6 };
	const std::vector<double> expectedResults	= { 10, 20, 30, 40, 50, 60 };
	
	ASSERT_EQ( interpValues.size(), expectedResults.size() );
	for ( size_t i = 0; i < expectedResults.size(); ++i )
	{
		const double actualResult = validation::tryAqToolsInterpolation(xValues, yValues, interpValues[i], interpolation );
		EXPECT_NEAR( expectedResults[i], actualResult, tolerance );
	}
}

TEST(TestUtilitiesInterpolation, UNIT_LinearInterpolation2)
{
	const std::string interpolation = "Linear";
	const double tolerance = 1e-10;

	// Interpolation Table - Node Points and Values
	const std::vector<double> xValues = { 2, 3, 4, 5 };
	const std::vector<double> yValues = { 20, 30, 40, 50 };

	// Interpolation Points
	const std::vector<double> interpValues		= { 1.5, 2.5, 3.5, 4.5, 5.5, 6.5 };
	const std::vector<double> expectedResults	= { 15, 25, 35, 45, 55, 65 };

	ASSERT_EQ(interpValues.size(), expectedResults.size());
	for (size_t i = 0; i < expectedResults.size(); ++i)
	{
		const double actualResult = validation::tryAqToolsInterpolation(xValues, yValues, interpValues[i], interpolation);
		EXPECT_NEAR(expectedResults[i], actualResult, tolerance);
	}
}

TEST(TestUtilitiesInterpolation, UNIT_LinearWithFlatExtrapolation)
{
	const std::string interpolation = "LinearWithFlatExtrapolation";
	const double tolerance = 1e-10;

	// Interpolation Table - Node Points and Values
	const std::vector<double> xValues = { 2, 3, 4, 5 };
	const std::vector<double> yValues = { 20, 30, 40, 50 };

	// Interpolation Points
	const std::vector<double> interpValues		= { 1, 2, 3, 4, 5, 6 };
	const std::vector<double> expectedResults	= { 20, 20, 30, 40, 50, 50 }; // Flat Extrapolation

	ASSERT_EQ(interpValues.size(), expectedResults.size());
	for (size_t i = 0; i < expectedResults.size(); ++i)
	{
		const double actualResult = validation::tryAqToolsInterpolation(xValues, yValues, interpValues[i], interpolation);
		EXPECT_NEAR(expectedResults[i], actualResult, tolerance);
	}
}

TEST(TestUtilitiesInterpolation, UNIT_SplineInterpolation)
{
	const std::string interpolation = "Spline";
	const double tolerance = 1e-10;

	// Interpolation Table - Node Points and Values
	const std::vector<double> xValues = { 2, 3, 4, 5 };
	const std::vector<double> yValues = { 20, 30, 40, 50 };

	// Interpolation Points
	const std::vector<double> interpValues		= { 1, 2, 3, 4, 5, 6 };
	const std::vector<double> expectedResults	= { 10, 20, 30, 40, 50, 60 };

	ASSERT_EQ(interpValues.size(), expectedResults.size());
	for (size_t i = 0; i < expectedResults.size(); ++i)
	{
		const double actualResult = validation::tryAqToolsInterpolation(xValues, yValues, interpValues[i], interpolation);
		EXPECT_NEAR(expectedResults[i], actualResult, tolerance);
	}
}

TEST(TestUtilitiesInterpolation, UNIT_SplineInterpolation2)
{
	const std::string interpolation = "Spline";
	const double tolerance = 1e-10;

	// Interpolation Table - Node Points and Values
	const std::vector<double> xValues = { 2, 3, 4, 5 };
	const std::vector<double> yValues = { 14, 19, 26, 35 }; // = X^2 + 10

	// Interpolation Points
	const std::vector<double> interpValues		= { 1.5, 2.5, 3.5, 4.5, 5.5, 6.5 };
	const std::vector<double> expectedResults	= { 11.65, 16.35, 22.2, 30.35, 39.65, 47.75  };

	ASSERT_EQ(interpValues.size(), expectedResults.size());
	for (size_t i = 0; i < expectedResults.size(); ++i)
	{
		const double actualResult = validation::tryAqToolsInterpolation(xValues, yValues, interpValues[i], interpolation);
		EXPECT_NEAR(expectedResults[i], actualResult, tolerance);
	}
}

TEST(TestUtilitiesInterpolation, UNIT_LeftContinuousInterpolation)
{
	const std::string interpolation = "Left_Continuous";
	const double tolerance = 1e-10;

	// Interpolation Table - Node Points and Values
	const std::vector<double> xValues = { 2, 3, 4, 5 };
	const std::vector<double> yValues = { 20, 30, 40, 50 };

	// Interpolation Points
	const std::vector<double> interpValues		= { 1, 2, 3, 4, 5, 6 };
	const std::vector<double> expectedResults	= { 20, 20, 30, 40, 50, 50 };

	ASSERT_EQ(interpValues.size(), expectedResults.size());
	for (size_t i = 0; i < expectedResults.size(); ++i)
	{
		const double actualResult = validation::tryAqToolsInterpolation(xValues, yValues, interpValues[i], interpolation);
		EXPECT_NEAR(expectedResults[i], actualResult, tolerance);
	}
}

TEST(TestUtilitiesInterpolation, UNIT_LeftContinuousInterpolation2)
{
	const std::string interpolation = "Left_Continuous";
	const double tolerance = 1e-10;

	// Interpolation Table - Node Points and Values
	const std::vector<double> xValues = { 2, 3, 4, 5 };
	const std::vector<double> yValues = { 20, 30, 40, 50 };

	// Interpolation Points
	const std::vector<double> interpValues		= { 1.5, 2.5, 3.5, 4.5, 5.5, 6.5 };
	const std::vector<double> expectedResults	= { 20, 30, 40, 50, 50, 50 };

	ASSERT_EQ(interpValues.size(), expectedResults.size());
	for (size_t i = 0; i < expectedResults.size(); ++i)
	{
		const double actualResult = validation::tryAqToolsInterpolation(xValues, yValues, interpValues[i], interpolation);
		EXPECT_NEAR(expectedResults[i], actualResult, tolerance);
	}
}

TEST(TestUtilitiesInterpolation, UNIT_RightContinuousInterpolation)
{
	const std::string interpolation = "Right_Continuous";
	const double tolerance = 1e-10;

	// Interpolation Table - Node Points and Values
	const std::vector<double> xValues = { 2, 3, 4, 5 };
	const std::vector<double> yValues = { 20, 30, 40, 50 };

	// Interpolation Points
	const std::vector<double> interpValues		= { 1, 2, 3, 4, 5, 6 };
	const std::vector<double> expectedResults	= { 20, 20, 30, 40, 50, 50 };

	ASSERT_EQ(interpValues.size(), expectedResults.size());
	for (size_t i = 0; i < expectedResults.size(); ++i)
	{
		const double actualResult = validation::tryAqToolsInterpolation(xValues, yValues, interpValues[i], interpolation);
		EXPECT_NEAR(expectedResults[i], actualResult, tolerance);
	}
}

TEST(TestUtilitiesInterpolation, UNIT_RightContinuousInterpolation2)
{
	const std::string interpolation = "Right_Continuous";
	const double tolerance = 1e-10;

	// Interpolation Table - Node Points and Values
	const std::vector<double> xValues = { 2, 3, 4, 5 };
	const std::vector<double> yValues = { 20, 30, 40, 50 };

	// Interpolation Points
	const std::vector<double> interpValues		= { 1.5, 2.5, 3.5, 4.5, 5.5, 6.5 };
	const std::vector<double> expectedResults	= { 20, 20, 30, 40, 50, 50 };

	ASSERT_EQ(interpValues.size(), expectedResults.size());
	for (size_t i = 0; i < expectedResults.size(); ++i)
	{
		const double actualResult = validation::tryAqToolsInterpolation(xValues, yValues, interpValues[i], interpolation);
		EXPECT_NEAR(expectedResults[i], actualResult, tolerance);
	}
}

TEST(TestUtilitiesInterpolation, UNIT_MonotoneCubic)
{
	const std::string interpolation = "MonotoneSpline";
	const double tolerance = 1e-10;

	// Interpolation Table - Node Points and Values
	// Note that the yValues are chosen to be deliberately non-monotonic.
	// Here we are checking that the interpolator is *piecewise monotonic*
	// i.e. that the interpolator is monotonic BETWEEN node points
	const std::vector<double> xValues = { 1, 2, 3, 4, 5 };
	const std::vector<double> yValues = { 1, 2, 3, 3, 2 };

	// Interpolation Points
	const std::vector<double> interpValues		= { 1, 1.5, 2, 2.5,   3, 3.3, 3.6, 4, 4.5,    5 };  // Check that interpolation is monotonic between 3 and 4
	const std::vector<double> expectedResults	= { 1, 1.5, 2, 2.625, 3, 3,   3,   3, 2.6875, 2 };

	ASSERT_EQ(interpValues.size(), expectedResults.size());
	for (size_t i = 0; i < expectedResults.size(); ++i)
	{
		const double actualResult = validation::tryAqToolsInterpolation(xValues, yValues, interpValues[i], interpolation);
		EXPECT_NEAR(expectedResults[i], actualResult, tolerance);
	}
}

TEST(TestUtilitiesInterpolation, UNIT_MonotoneParabolic)
{
	const std::string interpolation = "MonotoneParabolic";
	const double tolerance = 1e-10;

	// Interpolation Table - Node Points and Values
	// Note that the yValues are chosen to be deliberately non-monotonic.
	
	// The MonotoneParabolic does not have as high tension as the MonotoneCubic (which guarantees monotonicity between points)
	// however it has more tension than regular cubic spline.
	const std::vector<double> xValues = { 1, 2, 3, 4, 5 };
	const std::vector<double> yValues = { 1, 2, 3, 3, 2.6 };

	// Interpolation Points
	const std::vector<double> interpValues		= { 1, 1.5, 2, 2.5,    3, 3.3,      3.6,      4, 4.5,    5 };
	const std::vector<double> expectedResults	= { 1, 1.5, 2, 2.5875, 3, 3.0567,   3.0576,   3, 2.85, 2.6 };

	ASSERT_EQ(interpValues.size(), expectedResults.size());
	for (size_t i = 0; i < expectedResults.size(); ++i)
	{
		const double actualResult = validation::tryAqToolsInterpolation(xValues, yValues, interpValues[i], interpolation);
		EXPECT_NEAR(expectedResults[i], actualResult, tolerance);
	}
}
