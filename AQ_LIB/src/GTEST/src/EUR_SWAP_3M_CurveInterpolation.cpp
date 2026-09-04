 // EUR_SWAP_3M_CurveInterpolation.cpp

/*
 * @brief			Tests to validate the Curve Interpolation Class
 * @Created:		6th May 2020
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

// Include: Google Test Library
#include <gTest/gTest.h>
#include "InitializeGoogleTest.h"	// DECLARE_TEST_FIXTURE

// Infrastructure
#include "Dependency.h"
#include "ReadDataFile.h"
#include "GetGoogleTestFolder.h"
#include "ResultsProcessor.h"
#include "CoreEnumerations.h"
#include "DataUtilities.h"		// AQ_TO_STRING macros

// Curves and Swap Pricing
#include "tryMeLWO.h"
#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"
#include "BuildSwapTradeFromGenerator.h"
#include "ExtractCurveCalibrationData.h"
#include "RepriceCalibrationInstruments.h"

// Curve Results
#include "CurveResultsContainer.h"
#include "tryMeCurveResults.h"
#include "CurveUtilities.h"						// Contains Legcacy Curve Discount Factor and Forward Rate Methods
#include "AQLCurveForwardRateHelpers.h"			// Also contains Legcacy Curve Discount Factor and Forward Rate Methods
#include "ParameterValidation.h"				// Contains etrading::getDataInstance() method

// Interpolators
#include "CurveInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLLinearSplineInterpolation.h"

// Forward Rate Math Methods
#include "tryMeMathForwardRate.h"


// Define Test Folder Path(s)
// *** The Google Test DataInstance folder is defined within etrading project, GoogleTest Folder, GetGoogleTestFolder.h ***

const std::string TEST_FOLDER = "ETrading/Curves/TestCurveInterpolation/EUR/";

namespace google_test
{
	namespace
	{
		// Interpolation can be: LINEAR, LINEARSPLINE, SPLINE, STEP, LEFTCONTINUOUS, RIGHTCONTINUOUS
		void loadCurves( const std::string interpolation )
		{
			const std::string fileNameEUROIS = TEST_FOLDER + "EUR_OIS_CURVE_" + interpolation + ".JSON";
			const std::string fileNameEUR3ML = TEST_FOLDER + "EUR_SWAP_3M_CURVE_" + interpolation + ".JSON";

			// Load Curves
		    const std::string curveObjectEUROIS_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUROIS, etrading::JSON );
		    const std::string curveObjectEUR3ML_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR3ML, etrading::JSON );
		}
	}

	// Declare Test Fixture Class
    DECLARE_TEST_FIXTURE( EUR_SWAP_3M_CurveInterpolation )

    // Call Test Fixture Class
    TEST_F( EUR_SWAP_3M_CurveInterpolation, CONSISTENCY_ForwardRates_LinearInterpolationOnDiscountFactors )
    {
		// 1. Load the Curves
		const etrading::InterpolationEnum interpolation				= etrading::LINEAR_INTERPOLATION;
		loadCurves( toString(interpolation) );

		// 2. Static Data
		const std::string curveCollection							= "EURYC";
		const std::string curveIndex								= "EUR3ML";
		const AQLDate asOfDate										= etrading::getCurveAsOfDate( curveCollection );
		const AQLDate hybridInterpolationJoinDate					= etrading::getHybridInterpolationJoinDate( curveCollection, curveIndex );
		const bool useForwardInterpolation							= false;
		const std::string fixingCalendar							= "TGT";
		
		const StandardStringMatrix forwardAjustmentTable = StandardStringMatrix();

		const etrading::DayCountEnum accrualDaycount				= etrading::ACT_360_DAYCOUNT;
		const etrading::CurveTenorEnum	curveTenor					= etrading::CURVE_TENOR_3M;
		const etrading::CompoundingFrequencyEnum compoundFreq		= etrading::SIMPLE_COMPOUNDING;
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj	= etrading::MOD_FOLLOWING;
		const etrading::StateVariableEnum stateVariable				= etrading::STATE_VARIABLE_DF;
		
		// 3. Extract the Curve Results: Matrix with 2 Columns - PaymentDatesAsTerms and DiscountFactors
        const etrading::VariantMatrix curveResults = validation::tryMeCurveResultsDiscountFactorsDisplayAll( curveCollection, curveIndex );
		
		// 4. Extract Curve Payment Dates and Discount Factors from the Curve Results Table
		ASSERT_GT( curveResults.size(), size_t(0) );		// we expect more than 1 row of data
        ASSERT_EQ( curveResults[0].size(), size_t(2) );		// we expect exactly 2 columns of data

		std::vector< AQLDate > curveDates( curveResults.size() );
		std::vector< double > discountFactors( curveResults.size(), 1.0 );

		for ( size_t i = 0; i < curveResults.size(); ++i )
        {
            curveDates[i]			= curveResults[i][0];	// Column 1: Curve Dates
			discountFactors[i]		= curveResults[i][1];	// Column 2: Discount Factors
        }

		// 5. Imply the Forward Rates from the Curve Results Table
		const std::vector< double> expectedForwardRates = validation::tryMeCurveResultsForwardRatesDisplay( curveCollection, curveIndex, curveDates, useForwardInterpolation, fixingBusDayAdj, fixingCalendar );

		// 6. Get the Forward Rates from the Curve Interpolation Class, using isFwdInter = false (undesirable legacy feature)
		const std::vector< double > actualForwardRates = validation::tryMeMathForwardRates( curveDates, asOfDate, curveCollection, curveIndex, false, interpolation, stateVariable, curveDates, discountFactors, accrualDaycount, toString( curveTenor), fixingBusDayAdj, fixingCalendar, hybridInterpolationJoinDate, forwardAjustmentTable, compoundFreq );

		// 7. Test the Results Match
		ASSERT_EQ( expectedForwardRates.size(), actualForwardRates.size() );
		const double forwardRateTolerance = 1.0e-10;
		
		for( size_t j = 0; j < expectedForwardRates.size(); ++j )
		{
			EXPECT_NEAR( actualForwardRates[j], expectedForwardRates[j], forwardRateTolerance ) << "Inconsistent Forward Rates, See forward rate number : " << AQ_TO_STRING_FROM_SIZE_T( j );
		}
    }
    
	// Call Test Fixture Class
    TEST_F( EUR_SWAP_3M_CurveInterpolation, CONSISTENCY_ForwardRates_SplineInterpolationOnDiscountFactors )
    {
		// 1. Load the Curves
		const etrading::InterpolationEnum interpolation				= etrading::SPLINE_INTERPOLATION;
		loadCurves( toString(interpolation) );

		// 2. Static Data
		const std::string curveCollection							= "EURYC";
		const std::string curveIndex								= "EUR3ML";
		const AQLDate asOfDate										= etrading::getCurveAsOfDate( curveCollection );
		const AQLDate hybridInterpolationJoinDate					= etrading::getHybridInterpolationJoinDate( curveCollection, curveIndex );
		const bool useForwardInterpolation							= false;
		const std::string fixingCalendar							= "TGT";
		
		const StandardStringMatrix forwardAjustmentTable = StandardStringMatrix();

		const etrading::DayCountEnum accrualDaycount				= etrading::ACT_360_DAYCOUNT;
		const etrading::CurveTenorEnum	curveTenor					= etrading::CURVE_TENOR_3M;
		const etrading::CompoundingFrequencyEnum compoundFreq		= etrading::SIMPLE_COMPOUNDING;
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj	= etrading::MOD_FOLLOWING;
		const etrading::StateVariableEnum stateVariable				= etrading::STATE_VARIABLE_DF;
		
		// 3. Extract the Curve Results: Matrix with 2 Columns - PaymentDatesAsTerms and DiscountFactors
        const etrading::VariantMatrix curveResults = validation::tryMeCurveResultsDiscountFactorsDisplayAll( curveCollection, curveIndex );
		
		// 4. Extract Curve Payment Dates and Discount Factors from the Curve Results Table
		ASSERT_GT( curveResults.size(), size_t(0) );		// we expect more than 1 row of data
        ASSERT_EQ( curveResults[0].size(), size_t(2) );		// we expect exactly 2 columns of data

		std::vector< AQLDate > curveDates( curveResults.size() );
		std::vector< double > discountFactors( curveResults.size(), 1.0 );

		for ( size_t i = 0; i < curveResults.size(); ++i )
        {
            curveDates[i]			= curveResults[i][0];	// Column 1: Curve Dates
			discountFactors[i]		= curveResults[i][1];	// Column 2: Discount Factors
        }

		// 5. Imply the Forward Rates from the Curve Results Table
		const std::vector< double> expectedForwardRates = validation::tryMeCurveResultsForwardRatesDisplay( curveCollection, curveIndex, curveDates, useForwardInterpolation, fixingBusDayAdj, fixingCalendar );

		// 6. Get the Forward Rates from the Curve Interpolation Class using isFwdInter = false
		const std::vector< double > actualForwardRates = validation::tryMeMathForwardRates( curveDates, asOfDate, curveCollection, curveIndex, false, interpolation, stateVariable, curveDates, discountFactors, accrualDaycount, toString( curveTenor), fixingBusDayAdj, fixingCalendar, hybridInterpolationJoinDate, forwardAjustmentTable, compoundFreq );

		// 7. Test the Results Match
		ASSERT_EQ( expectedForwardRates.size(), actualForwardRates.size() );
		const double forwardRateTolerance = 1.0e-10;
		
		for( size_t j = 0; j < expectedForwardRates.size(); ++j )
		{
			EXPECT_NEAR( actualForwardRates[j], expectedForwardRates[j], forwardRateTolerance ) << "Inconsistent Forward Rates, See forward rate number : " << AQ_TO_STRING_FROM_SIZE_T( j );
		}
    }

	// Call Test Fixture Class
    TEST_F( EUR_SWAP_3M_CurveInterpolation, CONSISTENCY_ForwardRates_LinearSplineInterpolationOnDiscountFactors )
    {
		// 1. Load the Curves
		const etrading::InterpolationEnum interpolation				= etrading::LINEARSPLINE_INTERPOLATION;
		loadCurves( toString(interpolation) );

		// 2. Static Data
		const std::string curveCollection							= "EURYC";
		const std::string curveIndex								= "EUR3ML";
		const AQLDate asOfDate										= etrading::getCurveAsOfDate( curveCollection );
		const AQLDate hybridInterpolationJoinDate					= etrading::getHybridInterpolationJoinDate( curveCollection, curveIndex );
		const bool useForwardInterpolation							= false;
		const std::string fixingCalendar							= "TGT";
		
		const StandardStringMatrix forwardAjustmentTable = StandardStringMatrix();

		const etrading::DayCountEnum accrualDaycount				= etrading::ACT_360_DAYCOUNT;
		const etrading::CurveTenorEnum	curveTenor					= etrading::CURVE_TENOR_3M;
		const etrading::CompoundingFrequencyEnum compoundFreq		= etrading::SIMPLE_COMPOUNDING;
		const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj	= etrading::MOD_FOLLOWING;
		const etrading::StateVariableEnum stateVariable				= etrading::STATE_VARIABLE_DF;
		
		// 3. Extract the Curve Results: Matrix with 2 Columns - PaymentDatesAsTerms and DiscountFactors
        const etrading::VariantMatrix curveResults = validation::tryMeCurveResultsDiscountFactorsDisplayAll( curveCollection, curveIndex );
		
		// 4. Extract Curve Payment Dates and Discount Factors from the Curve Results Table
		ASSERT_GT( curveResults.size(), size_t(0) );		// we expect more than 1 row of data
        ASSERT_EQ( curveResults[0].size(), size_t(2) );		// we expect exactly 2 columns of data

		std::vector< AQLDate > curveDates( curveResults.size() );
		std::vector< double > discountFactors( curveResults.size(), 1.0 );

		for ( size_t i = 0; i < curveResults.size(); ++i )
        {
            curveDates[i]			= curveResults[i][0];	// Column 1: Curve Dates
			discountFactors[i]		= curveResults[i][1];	// Column 2: Discount Factors
        }

		// 5. Imply the Forward Rates from the Curve Results Table
		const std::vector< double> expectedForwardRates = validation::tryMeCurveResultsForwardRatesDisplay( curveCollection, curveIndex, curveDates, useForwardInterpolation, fixingBusDayAdj, fixingCalendar );

		// 6. Get the Forward Rates from the Curve Interpolation Class - using isFwdInter = false
		const std::vector< double > actualForwardRates = validation::tryMeMathForwardRates( curveDates, asOfDate, curveCollection, curveIndex, false, interpolation, stateVariable, curveDates, discountFactors, accrualDaycount, toString( curveTenor), fixingBusDayAdj, fixingCalendar, hybridInterpolationJoinDate, forwardAjustmentTable, compoundFreq );

		// 7. Test the Results Match
		ASSERT_EQ( expectedForwardRates.size(), actualForwardRates.size() );
		const double forwardRateTolerance = 1.0e-10;
		
		for( size_t j = 0; j < expectedForwardRates.size(); ++j )
		{
			EXPECT_NEAR( actualForwardRates[j], expectedForwardRates[j], forwardRateTolerance ) << "Inconsistent Forward Rates, See forward rate number : " << AQ_TO_STRING_FROM_SIZE_T( j );
		}
    }
	
	// // Call Test Fixture Class
    // TEST_F( EUR_SWAP_3M_CurveInterpolation, CONSISTENCY_ForwardRates_StepInterpolationOnDiscountFactors )
    // {
	// 	// 1. Load the Curves
	// 	const etrading::InterpolationEnum interpolation				= etrading::STEP_INTERPOLATION;
	// 	loadCurves( toString(interpolation) );
	// 
	// 	// 2. Static Data
	// 	const std::string curveCollection							= "EURYC";
	// 	const std::string curveIndex								= "EUR3ML";
	// 	const AQLDate asOfDate										= etrading::getCurveAsOfDate( curveCollection );
	// 	const AQLDate hybridInterpolationJoinDate					= etrading::getHybridInterpolationJoinDate( curveCollection, curveIndex );
	// 	const bool useForwardInterpolation							= false;
	// 	const std::string fixingCalendar							= "TGT";
	// 	
	// 	const StandardStringMatrix forwardAjustmentTable = StandardStringMatrix();
	// 
	// 	const etrading::DayCountEnum accrualDaycount				= etrading::ACT_360_DAYCOUNT;
	// 	const etrading::CurveTenorEnum	curveTenor					= etrading::CURVE_TENOR_3M;
	// 	const etrading::CompoundingFrequencyEnum compoundFreq		= etrading::SIMPLE_COMPOUNDING;
	// 	const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj	= etrading::MOD_FOLLOWING;
	// 	const etrading::StateVariableEnum stateVariable				= etrading::STATE_VARIABLE_DF;
	// 	
	// 	// 3. Extract the Curve Results: Matrix with 2 Columns - PaymentDatesAsTerms and DiscountFactors
    //     const etrading::VariantMatrix curveResults = validation::tryMeCurveResultsDiscountFactorsDisplayAll( curveCollection, curveIndex );
	// 	
	// 	// 4. Extract Curve Payment Dates and Discount Factors from the Curve Results Table
	// 	ASSERT_GT( curveResults.size(), size_t(0) );		// we expect more than 1 row of data
    //     ASSERT_EQ( curveResults[0].size(), size_t(2) );		// we expect exactly 2 columns of data
	// 
	// 	std::vector< AQLDate > curveDates( curveResults.size() );
	// 	std::vector< double > discountFactors( curveResults.size(), 1.0 );
	// 
	// 	for ( size_t i = 0; i < curveResults.size(); ++i )
    //     {
    //         curveDates[i]			= curveResults[i][0];	// Column 1: Curve Dates
	// 		discountFactors[i]		= curveResults[i][1];	// Column 2: Discount Factors
    //     }
	// 
	// 	// 5. Imply the Forward Rates from the Curve Results Table
	// 	const std::vector< double> expectedForwardRates = validation::tryMeCurveResultsForwardRatesDisplay( curveCollection, curveIndex, curveDates, useForwardInterpolation, fixingBusDayAdj, fixingCalendar );
	// 
	// 	// 6. Get the Forward Rates from the Curve Interpolation Class
	// 	const std::vector< double > actualForwardRates = validation::tryMeMathForwardRates( curveDates, asOfDate, interpolation, stateVariable, curveDates, discountFactors, accrualDaycount, toString( curveTenor), fixingBusDayAdj, fixingCalendar, hybridInterpolationJoinDate, forwardAjustmentTable, compoundFreq );
	// 
	// 	// 7. Test the Results Match
	// 	ASSERT_EQ( expectedForwardRates.size(), actualForwardRates.size() );
	// 	const double forwardRateTolerance = 1.0e-10;
	// 	
	// 	for( size_t j = 0; j < expectedForwardRates.size(); ++j )
	// 	{
	// 		EXPECT_NEAR( actualForwardRates[j], expectedForwardRates[j], forwardRateTolerance ) << "Inconsistent Forward Rates, See forward rate number : " << AQ_TO_STRING_FROM_SIZE_T( j );
	// 	}
    // }

}