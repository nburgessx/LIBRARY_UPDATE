// TestCurveLinearInterpolation.cpp

/*
 * @brief			Curve Interpolation Framework Tests
 * @Created:		28th February 2020
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

// Include: Google Test Library
#include <gTest/gTest.h>

#include "CurveInterpolation.h"

namespace google_test
{
	// Test Tolerance
	const double tolerance = 1e-12;

    TEST( TestCurveLinearInterpolation, UNIT_Interpolate_Using_ForwardRates )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		const std::vector<double> interpolationPoints = xValues;
		const std::vector<double> expectedResults = yValues;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;
		
		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Interpolate
		// ---------------------------
		const std::vector<double> actualResults = curveInterpolationObj.interpolate( interpolationPoints );

		ASSERT_EQ( actualResults.size(), expectedResults.size() ) << "Dimension Error: Actual Results Size != Expected Results Size";
        for ( size_t i = 0 ; i < actualResults.size(); ++i )
		{
			EXPECT_NEAR( actualResults[i], expectedResults[i], tolerance ) << "Test Index: " << i << " Interpolate at x equals " << xValues[i];
		}
	}

	TEST( TestCurveLinearInterpolation, UNIT_Differentiate_Using_ForwardRates )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		const std::vector<double> differentiationPoints = xValues;
		const std::vector<double> expectedResults = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;
		
		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Differentiate
		// ---------------------------
		const std::vector<double> actualResults = curveInterpolationObj.differentiate( differentiationPoints );

		ASSERT_EQ( actualResults.size(), expectedResults.size() ) << "Dimension Error: Actual Results Size != Expected Results Size";
        for ( size_t i = 0 ; i < actualResults.size(); ++i )
		{
			EXPECT_NEAR( actualResults[i], expectedResults[i], tolerance ) << "Test Index: " << i << " Differentiate at x equals " << xValues[i];
		}
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		const std::vector<double> lowerBounds = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		const std::vector<double> upperBounds = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		const std::vector<double> expectedResults = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02};

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;
		
		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const std::vector<double> actualResults = curveInterpolationObj.integrate( lowerBounds, upperBounds );

		ASSERT_EQ( actualResults.size(), expectedResults.size() ) << "Dimension Error: Actual Results Size != Expected Results Size";
        for ( size_t i = 0 ; i < actualResults.size(); ++i )
		{
			EXPECT_NEAR( actualResults[i], expectedResults[i], tolerance ) << "Test Index: " << i << " Integral from " << lowerBounds[i] << " to " << upperBounds[i];
		}
	}


	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_BoundaryConditions )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation, Null Integral, Partial Integral, Full Integral, x2 Abscissae Integral, Upper Extrapolation
		const std::vector<double> lowerBounds		= { -1,  -0.5,  0,    0,    0,    0,    0.5,  10.5, 11.0 };
		const std::vector<double> upperBounds		= {  0,   0,    0,    0.5,  1.0,  2.0,  1.0,  11.0, 12.0 };
		const std::vector<double> expectedResults	= { 0.02, 0.01, 0.00, 0.01, 0.02, 0.04, 0.01, 0.01, 0.02 };

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;
		
		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const std::vector<double> actualResults = curveInterpolationObj.integrate( lowerBounds, upperBounds );

		ASSERT_EQ( actualResults.size(), expectedResults.size() ) << "Dimension Error: Actual Results Size != Expected Results Size";
        for ( size_t i = 0 ; i < actualResults.size(); ++i )
		{
			EXPECT_NEAR( actualResults[i], expectedResults[i], tolerance ) << "Test Index: " << i << " Integral from " << lowerBounds[i] << " to " << upperBounds[i];
		}
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_LowerExtrapolation_Type1 )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= -2;
		const double upperBound		= 0;
		const double expectedResult	= 0.04;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;
		
		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_LowerExtrapolation_Type2 )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= -0.5;
		const double upperBound		= 0;
		const double expectedResult	= 0.01;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;

		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_LowerExtrapolation_Type3 )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= -2.5;
		const double upperBound		= -0.5;
		const double expectedResult	= 0.04;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;

		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_PartialIntegral_Type1 )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= 1.0;
		const double upperBound		= 1.5;
		const double expectedResult	= 0.01;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;

		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_PartialIntegral_Type2 )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= 1.5;
		const double upperBound		= 2.0;
		const double expectedResult	= 0.01;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;

		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_PartialIntegral_Type3 )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= 0.75;
		const double upperBound		= 1.25;
		const double expectedResult	= 0.01;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;

		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_FullIntegral )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= 1.0;
		const double upperBound		= 2.0;
		const double expectedResult	= 0.02;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;

		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}


	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_FullIntegral_Front )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= 0.0;
		const double upperBound		= 1.0;
		const double expectedResult	= 0.02;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;

		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_FullIntegral_End )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= 9.0;
		const double upperBound		= 10.0;
		const double expectedResult	= 0.02;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;

		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_FullIntegralx2 )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= 1;
		const double upperBound		= 3;
		const double expectedResult	= 0.04;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;

		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_UpperExtrapolation_Type1 )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= -2;
		const double upperBound		= 0;
		const double expectedResult	= 0.04;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;

		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 10.0;
		const size_t polynomialOrder = 10.5;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_UpperExtrapolation_Type2 )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= -2;
		const double upperBound		= 0;
		const double expectedResult	= 0.04;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;

		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 10.0;
		const size_t polynomialOrder = 12.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}

	TEST( TestCurveLinearInterpolation, UNIT_Integrate_Using_ForwardRates_UpperExtrapolation_Type3 )
    {
        // 1.	Interpolation xValues and yValues  i.e. a 10 year curve with constant forwards of 2.0%
		// ---------------------------
		const std::vector<double> xValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector<double> yValues = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };

		// Tests: Lower Extrapolation
		const double lowerBound		= -2;
		const double upperBound		= 0;
		const double expectedResult	= 0.04;

		// 2.	Interpolation Data
		// ---------------------------
		const LADate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount = etrading::ACT_365_DAYCOUNT;
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;

		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		const double joinDateAsDouble = 12.0;
		const size_t polynomialOrder = 14.0;

		// Forward Adjustment Table for Turn-of-Year (ToY) adjustments
		StandardStringMatrix forwardRateAdjustmentTable;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::LINEAR_INTERPOLATION, stateVariable, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::make_shared<etrading::ForwardAdjustments>( asOfDate, forwardRateAdjustmentTable );

		// 3.	Curve Object
		// ---------------------------
		etrading::CurveInterpolation curveInterpolationObj( interpolator, forwardAdjustments );
		
		// 4. Integrate
		// ---------------------------
		const double actualResult = curveInterpolationObj.integrate( lowerBound, upperBound );

		EXPECT_NEAR( actualResult, expectedResult, tolerance ) << "Integral from " << lowerBound << " to " << upperBound;
	}

}