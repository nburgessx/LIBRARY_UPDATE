 // TestCurveInterpolation.cpp

/*
 * @brief			Tests to validate the Curve Interpolation Class
 * @Created:		3rd January 2020
 * @Author:			Arthur Wu
 * @Department:		AlgoQuantHub London Quant Research
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

// Include: Google Test Library
#include <gTest/gTest.h>

#include "InitializeAQGoogleTest.h"	// DECLARE_TEST_FIXTURE
#include "CurveInterpolation.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLLinearSplineInterpolation.h"

namespace google_test
{
	// Test Tolerance
	const double tolerance = 1e-12;
	const double backwardsCompatibilityTolerance = 1e-8;

	// Test Fixture Initialization
	DECLARE_TEST_FIXTURE( TestCurveInterpolation );
	DECLARE_TEST_FIXTURE( TestCurveInterpolation_BackwardsCompatibility );

	TEST_F(TestCurveInterpolation, UNIT_TestStateVariableFwdRate_Polynomial_DegreeZero )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;
		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		std::vector<double> terms = { 0, 1, 2, 3, 4, 5};
		std::vector<double> values = { 0.01, 0.011, 0.012, 0.003, 0.005, 0.007};
		
		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 0;
		
		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::POLYNOMIAL_INTERPOLATION, stateVariable, terms, values, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		
		// expected results
		std::vector<double> discountFactors = { 1.0, 0.99203191483706066, 0.98412732005528514, 0.97628570975790929, 0.9685065820791976, 0.96078943915232318 };
		std::vector<double> forwardRates = { 0.0080000000000000019, 0.0080000000000000019, 0.0080000000000000019, 0.0080000000000000019, 0.0080000000000000019, 0.0080000000000000019 };

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolator );
		std::vector<double> results = curve.discountFactors(terms, etrading::CONTINUOUS_COMPOUNDING);

		// Assert if dimensions don't match
		ASSERT_EQ(results.size(), discountFactors.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(discountFactors[i], results[i], tolerance);
		}

		results = curve.forwardRates(terms);

		// Assert if dimensions don't match
		ASSERT_EQ(results.size(), forwardRates.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(forwardRates[i], results[i], tolerance);
		}
	}

	TEST_F( TestCurveInterpolation, UNIT_TestStateVariableFwdRate_LinearInterpolation )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;
		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		std::vector<double> terms = { 0, 1, 2, 3, 4, 5};
		std::vector<double> values = { 0.01, 0.011, 0.012, 0.003, 0.005, 0.007 };
		
		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 1;

		etrading::InterpolationPtr interpolator =  std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::POLYNOMIAL_INTERPOLATION, stateVariable, terms, values, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );

		// expected results
		std::vector<double> discountFactors = { 1.0, 0.98965389300909556, 0.9805908312024284, 0.97277739994940993, 0.96618495335106291, 0.96078943915232318 };
		std::vector<double> forwardRates = { 0.010999999999999999, 0.0097999999999999997, 0.0086, 0.0074000000000000003, 0.0062000000000000006, 0.005000000000000001 };

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolator );
		std::vector<double> results = curve.discountFactors(terms, etrading::CONTINUOUS_COMPOUNDING);

		//Assert if dimensions don't match
		ASSERT_EQ(results.size(), discountFactors.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(discountFactors[i], results[i], tolerance);
		}

		results = curve.forwardRates(terms);

		 // Assert if dimensions don't match
		ASSERT_EQ(results.size(), forwardRates.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(forwardRates[i], results[i], tolerance);
		}
	}

	TEST_F(TestCurveInterpolation, UNIT_TestStateVariableFwdRate_Polynomial_DegreeOne )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;
		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		std::vector<double> terms = { 0, 1, 2, 3, 4, 5};
		std::vector<double> values = { 0.01, 0.011, 0.012, 0.003, 0.005, 0.007 };
		
		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 1;

		etrading::InterpolationPtr interpolator =  std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::POLYNOMIAL_INTERPOLATION, stateVariable, terms, values, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );

		// expected results
		std::vector<double> discountFactors = { 1.0, 0.98965389300909556, 0.9805908312024284, 0.97277739994940993, 0.96618495335106291, 0.96078943915232318 };
		std::vector<double> forwardRates = { 0.010999999999999999, 0.0097999999999999997, 0.0086, 0.0074000000000000003, 0.0062000000000000006, 0.005000000000000001 };

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolator );
		std::vector<double> results = curve.discountFactors(terms, etrading::CONTINUOUS_COMPOUNDING);

		// Assert if dimensions don't match
		ASSERT_EQ(results.size(), discountFactors.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(discountFactors[i], results[i], tolerance);
		}

		results = curve.forwardRates(terms);

		// Assert if dimensions don't match
		ASSERT_EQ(results.size(), forwardRates.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(forwardRates[i], results[i], tolerance);
		}
	}

	TEST_F(TestCurveInterpolation, UNIT_TestStateVariableFwdRate_PolynomialDegreeTwo )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;
		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		std::vector<double> terms = { 0, 1, 2, 3, 4, 5 };
		std::vector<double> values = { 0.01, 0.011, 0.012, 0.003, 0.005, 0.007 };
		
		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 2;

		etrading::InterpolationPtr interpolator =  std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::POLYNOMIAL_INTERPOLATION, stateVariable, terms, values, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		
		// expected results
		std::vector<double> discountFactors = { 1.0, 0.989468350299329, 0.98069590013436381, 0.97332474116216572, 0.96701346690902057, 0.96143304040491029 };
		std::vector<double> forwardRates = { 0.011535714285714285, 0.0096928571428571419, 0.0081714285714285725, 0.006971428571428572, 0.0060928571428571438, 0.0055357142857142862 };

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolator );
		std::vector<double> results = curve.discountFactors(terms, etrading::CONTINUOUS_COMPOUNDING);

		// Assert if dimensions don't match
		ASSERT_EQ(results.size(), discountFactors.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(discountFactors[i], results[i], tolerance);
		}

		results = curve.forwardRates(terms);

		// Assert if dimensions don't match
		ASSERT_EQ(results.size(), forwardRates.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(forwardRates[i], results[i], tolerance);
		}
	}

	TEST_F(TestCurveInterpolation, UNIT_TestStateVariableFwdRate_Polynomial_DegreeThree )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;
		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		std::vector<double> terms = { 0, 1, 2, 3, 4, 5 };
		std::vector<double> values = { 0.01, 0.011, 0.012, 0.003, 0.005, 0.007 };
		
		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 3;

		etrading::InterpolationPtr interpolator =  std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::POLYNOMIAL_INTERPOLATION, stateVariable, terms, values, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );

		// expected results
		std::vector<double> discountFactors = { 1.0, 0.98854527734010555, 0.97761156842174424, 0.97026359206851465, 0.96611134206365423, 0.96143304040491029 };
		std::vector<double> forwardRates = { 0.0097857142857142882, 0.012142857142857141, 0.0095714285714285727, 0.0055714285714285744, 0.0036428571428571491, 0.0072857142857142981 };

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolator );
		std::vector<double> results = curve.discountFactors(terms, etrading::CONTINUOUS_COMPOUNDING);

		// Assert if dimensions don't match
		ASSERT_EQ(results.size(), discountFactors.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(discountFactors[i], results[i], tolerance);
		}

		results = curve.forwardRates(terms);

		// Assert if dimensions don't match
		ASSERT_EQ(results.size(), forwardRates.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(forwardRates[i], results[i], tolerance);
		}
	}

	TEST_F(TestCurveInterpolation, UNIT_TestStateVariableFwdRate_Polynomial_DegreeOne_WithToY )
	{
		AQLDate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_FORWARD_RATE;
		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		std::vector<double> terms = { 0, 1, 2, 3, 4, 5 };
		std::vector<double> values = { 0.01, 0.011, 0.012, 0.003, 0.005, 0.007 };
		
		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 1;

		// expected results
		std::vector<double> discountFactors = { 1.0, 0.98965253731975977, 0.98058680138537513, 0.97277633389256946, 0.96617912978807274, 0.96078206874105998 };
		std::vector<double> forwardRates = { 0.010999999999999999,0.0103, 0.0080999999999999996, 0.0078000000000000005, 0.0065000000000000006, 0.0048000000000000013 };

		// turn-of-years
		StandardStringMatrix forwardRateAdjustmentTable = {
			{"RELATIVE", "20201230", "20210103", "0.0005"},
			{"RELATIVE", "20211230", "20220103", "-0.0005"},
			{"RELATIVE", "20221230", "20230103", "0.0004"},
			{"RELATIVE", "20231229", "20240102", "0.0003"},
			{"RELATIVE", "20241230", "20250102", "-0.0002"}
		};

		etrading::InterpolationPtr interpolator =  std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::POLYNOMIAL_INTERPOLATION, stateVariable, terms, values, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );
		etrading::ForwardAdjustmentsPtr forwardAdjustments = std::shared_ptr<etrading::ForwardAdjustments>( new etrading::ForwardAdjustments( asOfDate, forwardRateAdjustmentTable ) );

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolator, forwardAdjustments );
		std::vector<double> results = curve.discountFactors(terms, etrading::CONTINUOUS_COMPOUNDING);

		// Assert if dimensions don't match
		ASSERT_EQ(results.size(), discountFactors.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(discountFactors[i], results[i], tolerance);
		}

		results = curve.forwardRates(terms);

		// Assert if dimensions don't match
		ASSERT_EQ(results.size(), forwardRates.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(forwardRates[i], results[i], tolerance);
		}
	}

	TEST_F(TestCurveInterpolation, UNIT_TestStateVariableDiscountFactor_Polynomial_DegreeThree)
	{
		AQLDate asOfDate("20200101");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::StateVariableEnum stateVariable = etrading::STATE_VARIABLE_DF;
		const std::string curveFrequencyTenor = "3M";
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj = etrading::MOD_FOLLOWING;
		const std::string fixingCalendar = "NYB";

		std::vector<double> terms = { 0, 1, 2, 3, 4, 5 };
		std::vector<double> values = { 1.0, 0.98, 0.97,  0.96, 0.95, 0.94 };
		
		const double joinDateAsDouble = 0.0;
		const size_t polynomialOrder = 3;

		etrading::InterpolationPtr interpolator = std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate, etrading::POLYNOMIAL_INTERPOLATION, stateVariable, terms, values, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder ) );

		// expected results
		std::vector<double> discountFactors = { 1.0, 0.98126984126984118, 0.96888888888888869,  0.95968253968253958, 0.95087301587301598,  0.93968253968254012 };
		std::vector<double> forwardRates = { 0.021296833915773881, 0.014511033795309698, 0.010313663979411318, 0.0089018062811840604, 0.010390252520387763, 0.014931390824315117 };

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolator );
		std::vector<double> results = curve.discountFactors(terms);

		// Assert if dimensions don't match
		ASSERT_EQ(results.size(), discountFactors.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(discountFactors[i], results[i], tolerance);
		}

		results = curve.forwardRates(terms);

		// Assert if dimensions don't match
		ASSERT_EQ(results.size(), forwardRates.size());
		for (size_t i = 0; i < results.size(); ++i)
		{
			EXPECT_NEAR(forwardRates[i], results[i], tolerance);
		}
	}

	// Test Fixture Required Here
	TEST_F(TestCurveInterpolation_BackwardsCompatibility, UNIT_ForwardRates_FromDiscFactors_UsingSplineInterpolation )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::InterpolationEnum interpolationEnum = etrading::SPLINE_INTERPOLATION;
		const etrading::StateVariableEnum stateVariableEnum = etrading::STATE_VARIABLE_DF;
		
		std::vector<double> terms = { 0, 1, 2, 3, 4, 5};
		std::vector<double> values = { 1.0, 0.980392156862745, 0.961168781237985, 0.942322334547044, 0.923845426026514, 0.905730809829916 };
		
		std::shared_ptr<AQLInterpolationBase> laInterpolationTable( new AQLSplineInterpolation() );
		laInterpolationTable->set( terms, values );

		etrading::InterpolationPtr interpolationConfig
			= std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate,
																				   interpolationEnum,
																				   stateVariableEnum,
																				   laInterpolationTable,
																				   accrualDaycount,
																				   "12M",
																				   etrading::MOD_FOLLOWING,
																				   "TGT" ));

		// expected results
		std::vector<double> expectedForwardRates = { 0.02000126232021918, 0.020000515097708005, 0.02000000000000024, 0.019999999999999574, 0.020000862306252042, 0.020409783389029802 };

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolationConfig );
		std::vector<double> actualForwardRates = curve.forwardRates(terms);

		// Assert if dimensions don't match
		ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size());
		for (size_t i = 0; i < actualForwardRates.size(); ++i)
		{
			EXPECT_NEAR(expectedForwardRates[i], actualForwardRates[i], tolerance) << "Forward Rate: " << i;
		}
	}

	// Test Fixture Required Here
	TEST_F(TestCurveInterpolation_BackwardsCompatibility, UNIT_ForwardRates_FromDiscFactors_UsingLinearSplineInterpolation )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::InterpolationEnum interpolationEnum = etrading::LINEARSPLINE_INTERPOLATION;
		const etrading::StateVariableEnum stateVariableEnum = etrading::STATE_VARIABLE_DF;
		
		double joinDateAsDouble = 3.0;
		std::vector<double> terms = { 0, 1, 2, 3, 4, 5};
		std::vector<double> values = { 1.0, 0.980392156862745, 0.961168781237985, 0.942322334547044, 0.923845426026514, 0.905730809829916 };
		
		// Hybrid Set Method requires the joinDate
		std::shared_ptr<AQLInterpolationBase> laInterpolationTable( new AQLLinearSplineInterpolation() );
		laInterpolationTable->set( terms, values, joinDateAsDouble ); 

		etrading::InterpolationPtr interpolationConfig
			= std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate,
																				   interpolationEnum,
																				   stateVariableEnum,
																				   laInterpolationTable,
																				   accrualDaycount,
																				   "12M",
																				   etrading::MOD_FOLLOWING,
																				   "TGT" ));

		// expected results
		std::vector<double> expectedForwardRates = { 0.020000011711365954, 0.020000002935683495, 0.02000000000000024, 0.019999999999999574, 0.020000817233562947, 0.020409875790478028 };

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolationConfig );
		std::vector<double> actualForwardRates = curve.forwardRates(terms);

		// Assert if dimensions don't match
		ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size());
		for (size_t i = 0; i < actualForwardRates.size(); ++i)
		{
			EXPECT_NEAR(expectedForwardRates[i], actualForwardRates[i], tolerance) << "Forward Rate: " << i;
		}
	}

	// Test Fixture Required Here
	TEST_F(TestCurveInterpolation_BackwardsCompatibility, UNIT_ForwardRates_FromDiscFactors_UsingLinearInterpolation )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::InterpolationEnum interpolationEnum = etrading::LINEAR_INTERPOLATION;
		const etrading::StateVariableEnum stateVariableEnum = etrading::STATE_VARIABLE_DF;
		
		std::vector<double> terms = { 0, 1, 2, 3, 4, 5};
		std::vector<double> values = { 1.0, 0.980392156862745, 0.961168781237985, 0.942322334547044, 0.923845426026514, 0.905730809829916 };
		
		std::shared_ptr<AQLInterpolationBase> laInterpolationTable( new AQLSplineInterpolation() );
		laInterpolationTable->set( terms, values );

		etrading::InterpolationPtr interpolationConfig
			= std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate,
																				   interpolationEnum,
																				   stateVariableEnum,
																				   laInterpolationTable,
																				   accrualDaycount,
																				   "12M",
																				   etrading::MOD_FOLLOWING,
																				   "TGT" ));

		// expected results
		std::vector<double> expectedForwardRates = { 0.02000126232021918, 0.020000515097708005, 0.02000000000000024, 0.019999999999999574, 0.020000862306252042, 0.020409783389029802 };

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolationConfig );
		std::vector<double> actualForwardRates = curve.forwardRates(terms);

		// Assert if dimensions don't match
		ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size());
		for (size_t i = 0; i < actualForwardRates.size(); ++i)
		{
			EXPECT_NEAR(expectedForwardRates[i], actualForwardRates[i], tolerance) << "Forward Rate: " << i;
		}
	}

	// Test Fixture Required Here
	TEST_F(TestCurveInterpolation_BackwardsCompatibility, CONSISTENCY_DiscountFactors_FromDiscFactors_UsingSplineInterpolation )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::InterpolationEnum interpolationEnum = etrading::SPLINE_INTERPOLATION;
		const etrading::StateVariableEnum stateVariableEnum = etrading::STATE_VARIABLE_DF;
		
		std::vector<double> terms = { 0, 1, 2, 3, 4, 5};
		std::vector<double> values = { 1.0, 0.980392156862745, 0.961168781237985, 0.942322334547044, 0.923845426026514, 0.905730809829916 };
		
		std::shared_ptr<AQLInterpolationBase> laInterpolationTable( new AQLSplineInterpolation() );
		laInterpolationTable->set( terms, values );

		etrading::InterpolationPtr interpolationConfig
			= std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate,
																				   interpolationEnum,
																				   stateVariableEnum,
																				   laInterpolationTable,
																				   accrualDaycount,
																				   "12M",
																				   etrading::MOD_FOLLOWING,
																				   "TGT" ));

		// expected results
		std::vector<double> expectedDiscountFactors = values;

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolationConfig );
		std::vector<double> actualDiscountFactors = curve.discountFactors(terms);

		// Assert if dimensions don't match
		ASSERT_EQ( actualDiscountFactors.size(), expectedDiscountFactors.size());
		for (size_t i = 0; i < actualDiscountFactors.size(); ++i)
		{
			EXPECT_NEAR(expectedDiscountFactors[i], actualDiscountFactors[i], tolerance) << "Discount Factor: " << i;
		}
	}

	// Test Fixture Required Here
	TEST_F(TestCurveInterpolation_BackwardsCompatibility, CONSISTENCY_DiscountFactors_FromDiscFactors_UsingLinearSplineInterpolation )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::InterpolationEnum interpolationEnum = etrading::LINEARSPLINE_INTERPOLATION;
		const etrading::StateVariableEnum stateVariableEnum = etrading::STATE_VARIABLE_DF;
		
		double joinDateAsDouble = 3.0;
		std::vector<double> terms = { 0, 1, 2, 3, 4, 5};
		std::vector<double> values = { 1.0, 0.980392156862745, 0.961168781237985, 0.942322334547044, 0.923845426026514, 0.905730809829916 };
		
		// Hybrid Set Method requires the joinDate
		std::shared_ptr<AQLInterpolationBase> laInterpolationTable( new AQLLinearSplineInterpolation() );
		laInterpolationTable->set( terms, values, joinDateAsDouble ); 

		etrading::InterpolationPtr interpolationConfig
			= std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate,
																				   interpolationEnum,
																				   stateVariableEnum,
																				   laInterpolationTable,
																				   accrualDaycount,
																				   "12M",
																				   etrading::MOD_FOLLOWING,
																				   "TGT" ));

		// expected results
		std::vector<double> expectedDiscountFactors = values;

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolationConfig );
		std::vector<double> actualDiscountFactors = curve.discountFactors(terms);

		// Assert if dimensions don't match
		ASSERT_EQ( actualDiscountFactors.size(), expectedDiscountFactors.size());
		for (size_t i = 0; i < actualDiscountFactors.size(); ++i)
		{
			EXPECT_NEAR(expectedDiscountFactors[i], actualDiscountFactors[i], tolerance) << "Discount Factor: " << i;
		}
	}

	// Test Fixture Required Here
	TEST_F(TestCurveInterpolation_BackwardsCompatibility, CONSISTENCY_DiscountFactors_FromDiscFactors_UsingLinearInterpolation )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::InterpolationEnum interpolationEnum = etrading::LINEAR_INTERPOLATION;
		const etrading::StateVariableEnum stateVariableEnum = etrading::STATE_VARIABLE_DF;
		
		std::vector<double> terms = { 0, 1, 2, 3, 4, 5};
		std::vector<double> values = { 1.0, 0.980392156862745, 0.961168781237985, 0.942322334547044, 0.923845426026514, 0.905730809829916 };
		
		std::shared_ptr<AQLInterpolationBase> laInterpolationTable( new AQLSplineInterpolation() );
		laInterpolationTable->set( terms, values );

		etrading::InterpolationPtr interpolationConfig
			= std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate,
																				   interpolationEnum,
																				   stateVariableEnum,
																				   laInterpolationTable,
																				   accrualDaycount,
																				   "12M",
																				   etrading::MOD_FOLLOWING,
																				   "TGT" ));
		// expected results
		std::vector<double> expectedDiscountFactors = values;

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolationConfig );
		std::vector<double> actualDiscountFactors = curve.discountFactors(terms);

		// Assert if dimensions don't match
		ASSERT_EQ( actualDiscountFactors.size(), expectedDiscountFactors.size());
		for (size_t i = 0; i < actualDiscountFactors.size(); ++i)
		{
			EXPECT_NEAR(expectedDiscountFactors[i], actualDiscountFactors[i], tolerance) << "Discount Factor: " << i;
		}
	}

	// Test Fixture Required Here
	TEST_F(TestCurveInterpolation_BackwardsCompatibility, CONSISTENCY_ForwardRates_FromForwardRates_UsingSplineInterpolation )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::InterpolationEnum interpolationEnum = etrading::SPLINE_INTERPOLATION;
		const etrading::StateVariableEnum stateVariableEnum = etrading::STATE_VARIABLE_FORWARD_RATE;
		
		std::vector<double> terms = { 0, 1, 2, 3, 4, 5};
		std::vector<double> values = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };
		
		std::shared_ptr<AQLInterpolationBase> laInterpolationTable( new AQLSplineInterpolation() );
		laInterpolationTable->set( terms, values );

		etrading::InterpolationPtr interpolationConfig
			= std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate,
																				   interpolationEnum,
																				   stateVariableEnum,
																				   laInterpolationTable,
																				   accrualDaycount,
																				   "12M",
																				   etrading::MOD_FOLLOWING,
																				   "TGT" ));

		// expected results
		std::vector<double> expectedForwardRates = values;

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolationConfig );
		std::vector<double> actualForwardRates = curve.forwardRates(terms);

		// Assert if dimensions don't match
		ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size());
		for (size_t i = 0; i < actualForwardRates.size(); ++i)
		{
			EXPECT_NEAR(expectedForwardRates[i], actualForwardRates[i], tolerance) << "Forward Rate: " << i;
		}
	}

	// Test Fixture Required Here
	TEST_F(TestCurveInterpolation_BackwardsCompatibility, CONSISTENCY_ForwardRates_FromForwardRates_UsingLinearSplineInterpolation )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::InterpolationEnum interpolationEnum = etrading::LINEARSPLINE_INTERPOLATION;
		const etrading::StateVariableEnum stateVariableEnum = etrading::STATE_VARIABLE_FORWARD_RATE;
		
		double joinDateAsDouble = 3.0;
		std::vector<double> terms = { 0, 1, 2, 3, 4, 5};
		std::vector<double> values = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };
		
		// Hybrid Set Method requires the joinDate
		std::shared_ptr<AQLInterpolationBase> laInterpolationTable( new AQLLinearSplineInterpolation() );
		laInterpolationTable->set( terms, values, joinDateAsDouble ); 

		etrading::InterpolationPtr interpolationConfig
			= std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate,
																				   interpolationEnum,
																				   stateVariableEnum,
																				   laInterpolationTable,
																				   accrualDaycount,
																				   "12M",
																				   etrading::MOD_FOLLOWING,
																				   "TGT" ));
		
		// expected results
		std::vector<double> expectedForwardRates = values;

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolationConfig );
		std::vector<double> actualForwardRates = curve.forwardRates(terms);

		// Assert if dimensions don't match
		ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size());
		for (size_t i = 0; i < actualForwardRates.size(); ++i)
		{
			EXPECT_NEAR(expectedForwardRates[i], actualForwardRates[i], tolerance) << "Forward Rate: " << i;
		}
	}

	// Test Fixture Required Here
	TEST_F(TestCurveInterpolation_BackwardsCompatibility, CONSISTENCY_ForwardRates_FromForwardRates_UsingLinearInterpolation )
	{
		const AQLDate asOfDate("20200103");
		const etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		const etrading::InterpolationEnum interpolationEnum = etrading::LINEAR_INTERPOLATION;
		const etrading::StateVariableEnum stateVariableEnum = etrading::STATE_VARIABLE_FORWARD_RATE;
		
		std::vector<double> terms = { 0, 1, 2, 3, 4, 5};
		std::vector<double> values = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };
		
		std::shared_ptr<AQLInterpolationBase> laInterpolationTable( new AQLSplineInterpolation() );
		laInterpolationTable->set( terms, values );

		etrading::InterpolationPtr interpolationConfig
			= std::shared_ptr<etrading::Interpolator>( new etrading::Interpolator( asOfDate,
																				   interpolationEnum,
																				   stateVariableEnum,
																				   laInterpolationTable,
																				   accrualDaycount,
																				   "12M",
																				   etrading::MOD_FOLLOWING,
																				   "TGT" ));

		// expected results
		std::vector<double> expectedForwardRates = values;

		// Curve Interpolation
		etrading::CurveInterpolation curve( interpolationConfig );
		std::vector<double> actualForwardRates = curve.forwardRates(terms);

		// Assert if dimensions don't match
		ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size());
		for (size_t i = 0; i < actualForwardRates.size(); ++i)
		{
			EXPECT_NEAR(expectedForwardRates[i], actualForwardRates[i], tolerance) << "Forward Rate: " << i;
		}
	}
}