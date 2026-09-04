// Include: Google Test Library
#include <gTest/gTest.h>

// Test Includes
#include "InitializeGoogleTest.h"
#include "GetGoogleTestFolder.h"
#include "RecordMacros.h"						// Logfile Macros
#include "ResultsProcessor.h"					// Test Rebase Methods
#include "tryAqObjects.h"							// AQO Curve Loading et al.
#include "tryAqCurvesResults.h"					// Curve Results Jacobian 


// Test Folders
const std::string TEST_FOLDER						= "ETrading/Curves/TestCurveResultsJacobian/";
const std::string TEST_FOLDER_CURVE_GENERATORS		= "ETrading/Curves/TestCurveResultsJacobian/CurveGenerators/";
const std::string TEST_FOLDER_NO_BUMP				= "ETrading/Curves/TestCurveResultsJacobian/NoBump/";
const std::string TEST_FOLDER_BUMPED				= "ETrading/Curves/TestCurveResultsJacobian/Bumped/";

// Test Files: Function Inputs / Outputs
const std::string discountFactorDelta_USDOIS_inputs		= TEST_FOLDER + "tryAqCurvesResultsJacobianDiscountFactorDelta_USDOIS_inputs.csv";
const std::string discountFactorDelta_USD6ML_inputs		= TEST_FOLDER + "tryAqCurvesResultsJacobianDiscountFactorDelta_USD6ML_inputs.csv";
const std::string discountFactorDelta_USD3ML_inputs		= TEST_FOLDER + "tryAqCurvesResultsJacobianDiscountFactorDelta_USD3ML_inputs.csv";

const std::string discountFactorDelta_USDOIS_outputs_X86	= TEST_FOLDER + "tryAqCurvesResultsJacobianDiscountFactorDelta_USDOIS_outputs_X86.csv";
const std::string discountFactorDelta_USD3ML_outputs_X86	= TEST_FOLDER + "tryAqCurvesResultsJacobianDiscountFactorDelta_USD3ML_outputs_X86.csv";
const std::string discountFactorDelta_USD6ML_outputs_X86	= TEST_FOLDER + "tryAqCurvesResultsJacobianDiscountFactorDelta_USD6ML_outputs_X86.csv";

const std::string discountFactorDelta_USDOIS_outputs_X64	= TEST_FOLDER + "tryAqCurvesResultsJacobianDiscountFactorDelta_USDOIS_outputs_X64.csv";
const std::string discountFactorDelta_USD3ML_outputs_X64	= TEST_FOLDER + "tryAqCurvesResultsJacobianDiscountFactorDelta_USD3ML_outputs_X64.csv";
const std::string discountFactorDelta_USD6ML_outputs_X64	= TEST_FOLDER + "tryAqCurvesResultsJacobianDiscountFactorDelta_USD6ML_outputs_X64.csv";

// Test Files: USD CURVES NO BUMP
const std::string fileNameUSDOIS_CurveGenerator			= TEST_FOLDER_CURVE_GENERATORS + "USD_OIS_WITH_RISK.JSON";
const std::string fileNameUSD3ML_CurveGenerator			= TEST_FOLDER_CURVE_GENERATORS + "USD_SWAP_3M_WITH_RISK.JSON";
const std::string fileNameUSD6ML_CurveGenerator			= TEST_FOLDER_CURVE_GENERATORS + "USD_BASIS_6M_WITH_RISK.JSON";

// Test Files: USD CURVES NO BUMP
const std::string fileNameUSDOIS_NoBump					= TEST_FOLDER_NO_BUMP + "USD_OIS_CURVE.JSON";
const std::string fileNameUSD3ML_NoBump					= TEST_FOLDER_NO_BUMP + "USD_SWAP_3M_CURVE.JSON";
const std::string fileNameUSD6ML_NoBump					= TEST_FOLDER_NO_BUMP + "USD_BASIS_6M_CURVE.JSON";

// Test Files: USD CURVES BUMPED
const std::string fileNameUSDOIS_Bumped					= TEST_FOLDER_BUMPED + "USD_OIS_CURVE_FLAT_SHIFT.JSON";
const std::string fileNameUSD3ML_Bumped					= TEST_FOLDER_BUMPED + "USD_SWAP_3M_CURVE_FLAT_SHIFT.JSON";
const std::string fileNameUSD6ML_Bumped					= TEST_FOLDER_BUMPED + "USD_BASIS_6M_CURVE_FLAT_SHIFT.JSON";


namespace google_test
{
    // Tolerance Settings
	// Note Curve calibration tolerance is 1e-10, we can't test with higher precision than the curve solver tolerance.
	const double jacobianTolerance				= 1e-6; // Build Server Architecture is different to local box Architecture	- reduced the tolerance from 1e-9 to 1e-6 to resolve differences
    const double discountFactorDeltaTolerance	= 1e-6; // Build Server Architecture is different to local box Architecture	- reduced the tolerance from 1e-9 to 1e-6 to resolve differences

    // TEST FIXTURE CONSTRUCTOR
    DECLARE_TEST_FIXTURE( TestCurveResultsJacobian )

	TEST_F( TestCurveResultsJacobian, SNAPSHOT_DiscountFactorDelta_FlatShift_USDOIS )
	{
		// Load Curve Generators and Build Curve With Flat Shift Risk Enabled
		// Note Curves Don't Need to be Bumped - Use the Original Curves
		const std::string loadUSDOISCurveGenerator		= validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_CurveGenerator, etrading::JSON );
		const std::string loadUSDOISCurve		        = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_NoBump, etrading::JSON );

		// Jacobian Discount Factor Delta
		etrading::VariantMatrix discountFactorDeltas = validation::tryAqCurvesResultsJacobianDiscountFactorDelta( "USDYC", "USDOIS" );

		// Validation: Check if results are empty and if present FlatShift Discount Factor Deltas should have two columns
		ASSERT_GT( discountFactorDeltas.size(), (unsigned int)(0) );
		ASSERT_EQ( discountFactorDeltas[0].size(), (unsigned int)(2) );

		// Run Test
		#if defined(GTEST32)
			CheckTestResultsAndRebaseOnRequest( discountFactorDeltas, TEST_FOLDER, discountFactorDelta_USDOIS_outputs_X86, discountFactorDeltaTolerance );
		#else
			CheckTestResultsAndRebaseOnRequest( discountFactorDeltas, TEST_FOLDER, discountFactorDelta_USDOIS_outputs_X64, discountFactorDeltaTolerance );
		#endif
	}

	TEST_F( TestCurveResultsJacobian, SNAPSHOT_DiscountFactorDelta_FlatShift_USD3ML )
	{
		// Load Curve Generators and Build Curve With Flat Shift Risk Enabled
		// Note Curves Don't Need to be Bumped - Use the Original Curves
		const std::string loadUSDOISCurveGenerator		= validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_CurveGenerator, etrading::JSON );
		const std::string loadUSD3MLCurveGenerator		= validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML_CurveGenerator, etrading::JSON );
        const std::string loadUSDOISCurve		        = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_NoBump, etrading::JSON );
        const std::string loadUSD3MLCurve		        = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML_NoBump, etrading::JSON );
        
		// Jacobian Discount Factor Delta
		etrading::VariantMatrix discountFactorDeltas = validation::tryAqCurvesResultsJacobianDiscountFactorDelta( "USDYC", "USD3ML" );

		// Validation: Check if results are empty and if present FlatShift Discount Factor Deltas should have two columns
		ASSERT_GT( discountFactorDeltas.size(), (unsigned int)(0) );
		ASSERT_EQ( discountFactorDeltas[0].size(), (unsigned int)(2) );

		// Run Test
		#if defined(GTEST32)
			CheckTestResultsAndRebaseOnRequest( discountFactorDeltas, TEST_FOLDER, discountFactorDelta_USD3ML_outputs_X86, discountFactorDeltaTolerance );
		#else
			CheckTestResultsAndRebaseOnRequest( discountFactorDeltas, TEST_FOLDER, discountFactorDelta_USD3ML_outputs_X64, discountFactorDeltaTolerance );
		#endif

	}

	TEST_F( TestCurveResultsJacobian, SNAPSHOT_DiscountFactorDelta_FlatShift_USD6ML )
	{
		// Load Curve Generators and Build Curve With Flat Shift Risk Enabled
		// Note Curves Don't Need to be Bumped - Use the Original Curves
		const std::string loadUSDOISCurveGenerator		= validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_CurveGenerator, etrading::JSON );
		const std::string loadUSD3MLCurveGenerator		= validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML_CurveGenerator, etrading::JSON );
		const std::string loadUSD6MLCurveGenerator		= validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML_CurveGenerator, etrading::JSON );
		const std::string loadUSDOISCurve		        = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_NoBump, etrading::JSON );
        const std::string loadUSD3MLCurve		        = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML_NoBump, etrading::JSON );
        const std::string loadUSD6MLCurve		        = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML_NoBump, etrading::JSON );

		// Jacobian Discount Factor Delta
		etrading::VariantMatrix discountFactorDeltas = validation::tryAqCurvesResultsJacobianDiscountFactorDelta( "USDYC", "USD6ML" );

		// Validation: Check if results are empty and if present FlatShift Discount Factor Deltas should have two columns
		ASSERT_GT( discountFactorDeltas.size(), (unsigned int)(0) );
		ASSERT_EQ( discountFactorDeltas[0].size(), (unsigned int)(2) );

		// Run Test
		#if defined(GTEST32)
			CheckTestResultsAndRebaseOnRequest( discountFactorDeltas, TEST_FOLDER, discountFactorDelta_USD6ML_outputs_X86, discountFactorDeltaTolerance );
		#else
			CheckTestResultsAndRebaseOnRequest( discountFactorDeltas, TEST_FOLDER, discountFactorDelta_USD6ML_outputs_X64, discountFactorDeltaTolerance );
		#endif
	}

	TEST_F( TestCurveResultsJacobian, CONSISTENCY_DiscountFactorDelta_FlatShift_USDOIS )
	{
		// Load Curve Generators and Build Curve With Flat Shift Risk Enabled
		// Note Curves Don't Need to be Bumped - Use the Original Curves
		const std::string loadUSDOISCurveGenerator = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_CurveGenerator, etrading::JSON );
		const std::string originalCurveUSDOIS = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_NoBump, etrading::JSON );
		
		const double oneBasisPointShiftSize = 0.0001;

		// ========================================================================================================================================================
		// ACTUAL RESULTS - Implied from the Jacobian Discount Factor Delta
		// ========================================================================================================================================================

		// 1.	Original Discount Factors
		etrading::VariantMatrix originalDiscountFactors	= validation::tryAqCurvesResultsDiscountFactorsDisplayAll("USDYC","USDOIS");

		// 2.	Jacobian Discount Factor Delta
		etrading::VariantMatrix discountFactorDelta	= validation::tryAqCurvesResultsJacobianDiscountFactorDelta( "USDYC", "USDOIS" );
		
		// 3.	Discount Factor Delta01 i.e. the Delta scaled by 1 basis point
		etrading::VariantMatrix discountFactorDelta01 = discountFactorDelta;
		for( size_t i = 0; i < discountFactorDelta.size(); ++i )
		{
			// Discount Factors are stored in column 2
			discountFactorDelta01[i][1] = ( discountFactorDelta[i][1].getValue<double>() ) * oneBasisPointShiftSize;
		}

		// Dimension Validation: Check if results are empty and if present FlatShift Discount Factor Deltas should have two columns
		ASSERT_EQ( originalDiscountFactors.size(), discountFactorDelta01.size() );
		ASSERT_GT( discountFactorDelta01.size(), (unsigned int)( 0 ) );
		ASSERT_EQ( discountFactorDelta01[0].size(), (unsigned int)( 2 ) );
		
		// 4. Implied Discount Factors when the Curve is Shifted by 1 Bps
		etrading::VariantMatrix actualShiftedDiscountFactors = originalDiscountFactors;
		for( size_t i = 0; i < actualShiftedDiscountFactors.size(); ++i )
		{
			// Discount Factors are stored in column 2
			// Shifted Discount Factor = Original Discount Factor + DiscountFactorDelta01
			actualShiftedDiscountFactors[i][1] = ( originalDiscountFactors[i][1].getValue<double>() ) + ( discountFactorDelta01[i][1].getValue<double>() );
		}

		// ========================================================================================================================================================
		// EXPECTED RESULTS - Calculated by Shifting the Curve Market Data by 1 Basis Point Manually
		// ========================================================================================================================================================

		// 5. Load Shifted Curve with Market Data Bumped by 1 Bps
		const std::string shiftedCurveUSDOIS = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_Bumped, etrading::JSON );

		// 6. Shifted Discount Factors
		etrading::VariantMatrix expectedShiftedDiscountFactors	= validation::tryAqCurvesResultsDiscountFactorsDisplayAll("USDYC","USDOIS");

		// Dimension Validation
		ASSERT_EQ( actualShiftedDiscountFactors.size(), expectedShiftedDiscountFactors.size() );
		
		// ========================================================================================================================================================
		// COMPARE RESULTS
		// ========================================================================================================================================================

		// 7. Compare the Actual Results with the Expected Results
		for( size_t j = 0; j < expectedShiftedDiscountFactors.size(); ++j )
		{
			EXPECT_NEAR( actualShiftedDiscountFactors[j][1].getValue<double>(), expectedShiftedDiscountFactors[j][1].getValue<double>(), discountFactorDeltaTolerance );
		}
	}

	TEST_F( TestCurveResultsJacobian, CONSISTENCY_DiscountFactorDelta_FlatShift_USD3ML )
	{
		// Load Curve Generators and Build Curve With Flat Shift Risk Enabled
		// Note Curves Don't Need to be Bumped - Use the Original Curves
		const std::string loadUSDOISCurveGenerator = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_CurveGenerator, etrading::JSON );
		const std::string loadUSD3MLCurveGenerator = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML_CurveGenerator, etrading::JSON );

		const std::string originalCurveUSDOIS = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_NoBump, etrading::JSON );
		const std::string originalCurveUSD3ML = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML_NoBump, etrading::JSON );
		
		const double oneBasisPointShiftSize = 0.0001;

		// ========================================================================================================================================================
		// ACTUAL RESULTS - Implied from the Jacobian Discount Factor Delta
		// ========================================================================================================================================================

		// 1.	Original Discount Factors
		etrading::VariantMatrix originalDiscountFactors	= validation::tryAqCurvesResultsDiscountFactorsDisplayAll("USDYC","USD3ML");

		// 2.	Jacobian Discount Factor Delta
		etrading::VariantMatrix discountFactorDelta	= validation::tryAqCurvesResultsJacobianDiscountFactorDelta( "USDYC", "USD3ML" );
		
		// 3.	Discount Factor Delta01 i.e. the Delta scaled by 1 basis point
		etrading::VariantMatrix discountFactorDelta01 = discountFactorDelta;
		for( size_t i = 0; i < discountFactorDelta.size(); ++i )
		{
			// Discount Factors are stored in column 2
			discountFactorDelta01[i][1] = ( discountFactorDelta[i][1].getValue<double>() ) * oneBasisPointShiftSize;
		}

		// Dimension Validation: Check if results are empty and if present FlatShift Discount Factor Deltas should have two columns
		ASSERT_EQ( originalDiscountFactors.size(), discountFactorDelta01.size() );
		ASSERT_GT( discountFactorDelta01.size(), (unsigned int)( 0 ) );
		ASSERT_EQ( discountFactorDelta01[0].size(), (unsigned int)( 2 ) );
		
		// 4. Implied Discount Factors when the Curve is Shifted by 1 Bps
		etrading::VariantMatrix actualShiftedDiscountFactors = originalDiscountFactors;
		for( size_t i = 0; i < actualShiftedDiscountFactors.size(); ++i )
		{
			// Discount Factors are stored in column 2
			// Shifted Discount Factor = Original Discount Factor + DiscountFactorDelta01
			actualShiftedDiscountFactors[i][1] = ( originalDiscountFactors[i][1].getValue<double>() ) + ( discountFactorDelta01[i][1].getValue<double>() );
		}

		// ========================================================================================================================================================
		// EXPECTED RESULTS - Calculated by Shifting the Curve Market Data by 1 Basis Point Manually
		// ========================================================================================================================================================

		// 5. Load Shifted Curve with Market Data Bumped by 1 Bps
		const std::string shiftedCurveUSD3ML = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML_Bumped, etrading::JSON );

		// 6. Shifted Discount Factors
		etrading::VariantMatrix expectedShiftedDiscountFactors	= validation::tryAqCurvesResultsDiscountFactorsDisplayAll("USDYC","USD3ML");

		// Dimension Validation
		ASSERT_EQ( actualShiftedDiscountFactors.size(), expectedShiftedDiscountFactors.size() );
		
		// ========================================================================================================================================================
		// COMPARE RESULTS
		// ========================================================================================================================================================

		// 7. Compare the Actual Results with the Expected Results
		for( size_t j = 0; j < expectedShiftedDiscountFactors.size(); ++j )
		{
			EXPECT_NEAR( actualShiftedDiscountFactors[j][1].getValue<double>(), expectedShiftedDiscountFactors[j][1].getValue<double>(), discountFactorDeltaTolerance );
		}
	}

	TEST_F( TestCurveResultsJacobian, CONSISTENCY_DiscountFactorDelta_FlatShift_USD6ML )
	{
		// Load Curve Generators and Build Curve With Flat Shift Risk Enabled
		// Note Curves Don't Need to be Bumped - Use the Original Curves
		const std::string loadUSDOISCurveGenerator = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_CurveGenerator, etrading::JSON );
		const std::string loadUSD3MLCurveGenerator = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML_CurveGenerator, etrading::JSON );
		const std::string loadUSD6MLCurveGenerator = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML_CurveGenerator, etrading::JSON );

		const std::string originalCurveUSDOIS = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS_NoBump, etrading::JSON );
		const std::string originalCurveUSD3ML = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML_NoBump, etrading::JSON );
		const std::string originalCurveUSD6ML = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML_NoBump, etrading::JSON );
		
		const double oneBasisPointShiftSize = 0.0001;

		// ========================================================================================================================================================
		// ACTUAL RESULTS - Implied from the Jacobian Discount Factor Delta
		// ========================================================================================================================================================

		// 1.	Original Discount Factors
		etrading::VariantMatrix originalDiscountFactors	= validation::tryAqCurvesResultsDiscountFactorsDisplayAll("USDYC","USD6ML");

		// 2.	Jacobian Discount Factor Delta
		etrading::VariantMatrix discountFactorDelta	= validation::tryAqCurvesResultsJacobianDiscountFactorDelta( "USDYC", "USD6ML" );
		
		// 3.	Discount Factor Delta01 i.e. the Delta scaled by 1 basis point
		etrading::VariantMatrix discountFactorDelta01 = discountFactorDelta;
		for( size_t i = 0; i < discountFactorDelta.size(); ++i )
		{
			// Discount Factors are stored in column 2
			discountFactorDelta01[i][1] = ( discountFactorDelta[i][1].getValue<double>() ) * oneBasisPointShiftSize;
		}

		// Dimension Validation: Check if results are empty and if present FlatShift Discount Factor Deltas should have two columns
		ASSERT_EQ( originalDiscountFactors.size(), discountFactorDelta01.size() );
		ASSERT_GT( discountFactorDelta01.size(), (unsigned int)( 0 ) );
		ASSERT_EQ( discountFactorDelta01[0].size(), (unsigned int)( 2 ) );
		
		// 4. Implied Discount Factors when the Curve is Shifted by 1 Bps
		etrading::VariantMatrix actualShiftedDiscountFactors = originalDiscountFactors;
		for( size_t i = 0; i < actualShiftedDiscountFactors.size(); ++i )
		{
			// Discount Factors are stored in column 2
			// Shifted Discount Factor = Original Discount Factor + DiscountFactorDelta01
			actualShiftedDiscountFactors[i][1] = ( originalDiscountFactors[i][1].getValue<double>() ) + ( discountFactorDelta01[i][1].getValue<double>() );
		}

		// ========================================================================================================================================================
		// EXPECTED RESULTS - Calculated by Shifting the Curve Market Data by 1 Basis Point Manually
		// ========================================================================================================================================================

		// 5. Load Shifted Curve with Market Data Bumped by 1 Bps
		const std::string shiftedCurveUSD6ML = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML_Bumped, etrading::JSON );

		// 6. Shifted Discount Factors
		etrading::VariantMatrix expectedShiftedDiscountFactors	= validation::tryAqCurvesResultsDiscountFactorsDisplayAll("USDYC","USD6ML");

		// Dimension Validation
		ASSERT_EQ( actualShiftedDiscountFactors.size(), expectedShiftedDiscountFactors.size() );
		
		// ========================================================================================================================================================
		// COMPARE RESULTS
		// ========================================================================================================================================================

		// 7. Compare the Actual Results with the Expected Results
		for( size_t j = 0; j < expectedShiftedDiscountFactors.size(); ++j )
		{
			EXPECT_NEAR( actualShiftedDiscountFactors[j][1].getValue<double>(), expectedShiftedDiscountFactors[j][1].getValue<double>(), discountFactorDeltaTolerance );
		}
	}

}