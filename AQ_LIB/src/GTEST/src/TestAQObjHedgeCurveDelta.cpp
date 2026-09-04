// Curves
#include "tryAqObjCurvesMarketData.h"
#include "tryAqObjCurvesCalibrate.h"
#include "tryAqObjCurvesCalibrateHedge.h"
#include "tryAqObjCurvesGenerator.h"

// Swap Creation and Pricing
#include "tryAqObjSwapsCreation.h"
#include "tryAqObjSwapsPricing.h"

// Risk calculation
#include "tryAqObjSwapsDelta.h"
#include "tryAqObjRatesFixingTable.h"

// Test Infrastructure
#include "Dependency.h"
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"
#include <gTest/gTest.h>

#include "TestHelperUtilities.h"
#include "InitializeETrading.h"
#include "InitializeGoogleTest.h"
#include "tryAqToolsSetup.h"
#include "BuildMarketDataObjectFromFile.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

// Define the Test Input Folder Here
#define TEST_DIR "ETrading/AQObjects/TestAQObjHedgeCurveDelta/"

namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 1e-1;  // Notional of test trades is 1MM; delta by bumping is inherently noisy 


    //
    // curve input files
    //
    extern const char pricing_USDYC_OIS_marketData[]			= TEST_DIR "USD_OIS_CURVE_MARKETDATA@4_tryAqObjCurvesMarketDataCreate_inputs.csv";
    extern const char pricing_USDYC_STD_marketData[]			= TEST_DIR "MARKET_DATA_USD3ML_USDYC@3_tryAqObjCurvesMarketDataCreate_inputs.csv";
	extern const char hedge_USDYC_HEDGE_STD_marketData[]		= TEST_DIR "MARKET_DATA_USD3ML_USDYC_HEDGE@7_tryAqObjCurvesMarketDataCreate_inputs.csv";

	// pricing curve build
	extern const char calibrate_pricing_USDYC_OIS[]				= TEST_DIR "tryAqObjCurvesCalibrate_USDOIS_inputs.csv";
	extern const char calibrate_pricing_USDYC_STD[]				= TEST_DIR "tryAqObjCurvesCalibrate_USD3ML_inputs.csv";

	// Hedge curve
	extern const char curve_generator_modify_interpolation[]	= TEST_DIR "tryAqObjCurvesGeneratorModify_inputs.csv";
	extern const char calibrate_hedge_curve_inputs[]			= TEST_DIR "tryAqObjCurvesCalibrateHedge_inputs.csv";

    // Trades
	extern const char irs_spot_4Y_SwapInputs[]			= TEST_DIR "USD_3ML_spot_4Y_tryAqObjSwapsCreateFromGenerator_inputs.csv";
	extern const char irs_3Yfwdstart_4Y_SwapInputs[]	= TEST_DIR "USD_3ML_3Yfwdst_4Y_tryAqObjSwapsCreateFromGenerator_inputs.csv";
	extern const char irs_3Y9Mfwdstart_3M_SwapInputs[]	= TEST_DIR "USD_3ML_3Y9MFwdSt_3M_tryAqObjSwapsCreateFromGenerator_inputs.csv";

	//extern const char fixingTableInputs[]				= TEST_DIR "FIXING@1_tryAqObjRatesFixingTableCreate_inputs.csv";


	// Delta Ladder Inputs
	extern const char deltaLadder_spot_4Y_swap_pricingCurveCollection_input[]		= TEST_DIR "tryAqObjSwapsDeltaLadderHorizontally_pricingCurveCollection_inputs.csv";
	extern const char deltaLadder_spot_4Y_swap_hedgeCurveCollection_input[]			= TEST_DIR "tryAqObjSwapsDeltaLadderHorizontally_hedgeCurveCollection_inputs.csv";
	extern const char deltaLadder_3Yfwdst_4Y_swap_hedgeCurveCollection_input[]		= TEST_DIR "tryAqObjSwapsDeltaLadderHorizontally_3Yfwdst_4Y_hedgeCurveCollection_inputs.csv";
	extern const char deltaLadder_3Y9Mfwdst_3M_swap_hedgeCurveCollection_input[]	= TEST_DIR "tryAqObjSwapsDeltaLadderHorizontally_3Y9Mfwdst_3M_hedgeCurveCollection_inputs.csv";

	// Baseline results
	extern const char raw_delta_ladder_outputs_spot_4Y_pricingCurveCollection_32[]	= "tryAqObjSwapsDeltaLadder_spot_4Y_pricingCurveCollection_outputs.csv";
	extern const char raw_delta_ladder_outputs_spot_4Y_pricingCurveCollection_64[]	= "tryAqObjSwapsDeltaLadder_spot_4Y_pricingCurveCollection_outputs_64bit.csv"; 
	extern const char delta_ladder_outputs_spot_4Y_pricingCurveCollection_32[]		= TEST_DIR "tryAqObjSwapsDeltaLadder_spot_4Y_pricingCurveCollection_outputs.csv";
	extern const char delta_ladder_outputs_spot_4Y_pricingCurveCollection_64[]		= TEST_DIR "tryAqObjSwapsDeltaLadder_spot_4Y_pricingCurveCollection_outputs_64bit.csv";

	extern const char raw_delta_ladder_outputs_spot_4Y_hedgeCurveCollection_32[]	= "tryAqObjSwapsDeltaLadder_spot_4Y_hedgeCurveCollection_outputs.csv";
	extern const char raw_delta_ladder_outputs_spot_4Y_hedgeCurveCollection_64[]	= "tryAqObjSwapsDeltaLadder_spot_4Y_hedgeCurveCollection_outputs_64bit.csv"; 
	extern const char delta_ladder_outputs_spot_4Y_hedgeCurveCollection_32[]		= TEST_DIR "tryAqObjSwapsDeltaLadder_spot_4Y_hedgeCurveCollection_outputs.csv";
	extern const char delta_ladder_outputs_spot_4Y_hedgeCurveCollection_64[]		= TEST_DIR "tryAqObjSwapsDeltaLadder_spot_4Y_hedgeCurveCollection_outputs_64bit.csv";

	extern const char raw_delta_ladder_outputs_3Yfwdst_4Y_hedgeCurveCollection_32[]	= "tryAqObjSwapsDeltaLadder_3Yfwdst_4Y_hedgeCurveCollection_outputs.csv";
	extern const char raw_delta_ladder_outputs_3Yfwdst_4Y_hedgeCurveCollection_64[]	= "tryAqObjSwapsDeltaLadder_3Yfwdst_4Y_hedgeCurveCollection_outputs_64bit.csv"; 
	extern const char delta_ladder_outputs_3Yfwdst_4Y_hedgeCurveCollection_32[]		= TEST_DIR "tryAqObjSwapsDeltaLadder_3Yfwdst_4Y_hedgeCurveCollection_outputs.csv";
	extern const char delta_ladder_outputs_3Yfwdst_4Y_hedgeCurveCollection_64[]		= TEST_DIR "tryAqObjSwapsDeltaLadder_3Yfwdst_4Y_hedgeCurveCollection_outputs_64bit.csv";

	extern const char raw_delta_ladder_outputs_3Y9Mfwdst_3M_hedgeCurveCollection_32[]	= "tryAqObjSwapsDeltaLadder_3Y9Mfwdst_3M_hedgeCurveCollection_outputs.csv";
	extern const char raw_delta_ladder_outputs_3Y9Mfwdst_3M_hedgeCurveCollection_64[]	= "tryAqObjSwapsDeltaLadder_3Y9Mfwdst_3M_hedgeCurveCollection_outputs_64bit.csv"; 
	extern const char delta_ladder_outputs_3Y9Mfwdst_3M_hedgeCurveCollection_32[]		= TEST_DIR "tryAqObjSwapsDeltaLadder_3Y9Mfwdst_3M_hedgeCurveCollection_outputs.csv";
	extern const char delta_ladder_outputs_3Y9Mfwdst_3M_hedgeCurveCollection_64[]		= TEST_DIR "tryAqObjSwapsDeltaLadder_3Y9Mfwdst_3M_hedgeCurveCollection_outputs_64bit.csv";

	// 3Y6M_6M  i.e. single cashflow

	extern const char deltaLadder_spot_4Y_Inputs[]			= TEST_DIR "tryAqObjSwapsDeltaLadder_spot_4Y_inputs.csv";

	std::string helperCreateSwapFromSwapGenerator(const std::string& swapGeneratorName, const char* swapInputs)
	{
		// Swap
		const ReadDataFile::Load swapInputFile( swapInputs );
		std::string swapName			= swapInputFile[ "swapName" ];
		AQLStringMatrix expressionLVB		= swapInputFile[ "expressionLVB" ];
		AQLStringMatrix swapPropertiesLVB	= swapInputFile[ "swapPropertiesLVB" ];
		bool isXccySwap					= swapInputFile[ "isXccySwap" ];
		bool validateKeys				= swapInputFile[ "validateKeys" ];
		return validation::tryAqObjSwapsCreateFromGenerator( swapName, swapGeneratorName, expressionLVB, swapPropertiesLVB, isXccySwap, validateKeys );
	}

	/* @brief			Builds Generator curve by invoking the tryAqObjCurvesCalibration() API.
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/	
	void createAQObjCurveFromFileName( const AQLString& curveCalibrationFileName )
	{
		etrading::ReadDataFile::Load curveCalibrationFileObj = etrading::ReadDataFile::Load( curveCalibrationFileName );
		
		std::string aqObjCurveGeneratorName	= curveCalibrationFileObj[ "aqObjCurveGeneratorName" ];
		std::string aqObjCurveMarketDataName	= curveCalibrationFileObj[ "aqObjCurveMarketDataName" ];
		std::string domesticCurveCollection	= curveCalibrationFileObj[ "domesticCurveCollection" ];
		std::string foreignCurveCollection	= curveCalibrationFileObj[ "foreignCurveCollection" ];
		
		std::string objectName = aqObjCurveGeneratorName;

		validation::tryAqObjCurvesCalibrate(	objectName, aqObjCurveGeneratorName, aqObjCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );
	}

	/* @brief			Builds and Generator curve using the specified marketData and calibration filename
	*  @param [in]		marketDataFileName			The filename specifying generator curve data
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/
	void setUpGeneratorCurve( const AQLString& marketDataFileName, const AQLString& curveCalibrationFileName )
	{
		google_test::createAQObjMarketDataObjectFromFileName( marketDataFileName );
		createAQObjCurveFromFileName( curveCalibrationFileName );
	}

	void setupGeneratorHedgeCurve( const AQLString& marketDataFileName, const AQLString& curveGeneratorModifyInterpolation, const AQLString& curveCalibrationFileName )
	{
		// a. Load Hedge Curve Market Data
		google_test::createAQObjMarketDataObjectFromFileName( marketDataFileName );

		// b. Modify Hedge Curve Generator to use Linear Intepolation
		const ReadDataFile::Load hedgeCurveGeneratorInputFile( curveGeneratorModifyInterpolation );
		std::string newObjectName  = hedgeCurveGeneratorInputFile[ "newObjectName" ];
		std::string baseObjectName = hedgeCurveGeneratorInputFile[ "baseObjectName" ];
		AQLStringMatrix modifiedValues = hedgeCurveGeneratorInputFile[ "modifiedValues" ];
		validation::tryAqObjCurvesGeneratorModify( newObjectName, baseObjectName, modifiedValues );

		// c. Construct Hedge Curve
		const ReadDataFile::Load hedgeCurveInputFile( curveCalibrationFileName );
		std::string oisCurveObjectName		= hedgeCurveInputFile[ "oisCurveObjectName" ];
		std::string swapCurveObjectName		= hedgeCurveInputFile[ "swapCurveObjectName" ];
		std::string pricingCurveCollection	= hedgeCurveInputFile[ "pricingCurveCollection" ];
		std::string hedgeCurveCollection	= hedgeCurveInputFile[ "hedgeCurveCollection" ];
		std::string oisCurveGeneratorName	= hedgeCurveInputFile[ "oisCurveGeneratorName" ];
		std::string oisCurveMarketDataName	= hedgeCurveInputFile[ "oisCurveMarketDataName" ];
		std::string swapCurveGeneratorName	= hedgeCurveInputFile[ "swapCurveGeneratorName" ];
		std::string swapCurveMarketDataName	= hedgeCurveInputFile[ "swapCurveMarketDataName" ];
		std::string swapGeneratorName		= hedgeCurveInputFile[ "swapGeneratorName" ]; // return this as an output parameter

		validation::tryAqObjCurvesCalibrateHedge( oisCurveObjectName,
													swapCurveObjectName,
													pricingCurveCollection,
													hedgeCurveCollection,
													oisCurveGeneratorName,
													oisCurveMarketDataName,
													swapCurveGeneratorName,
													swapCurveMarketDataName,
													swapGeneratorName );
	}


	void runDeltaLadderCalculation( const ReadDataFile::Load& deltaLadder, AQLStringVector& pillarNames, AQLStringVector& headers, DoubleMatrix& deltas )
	{
		AQLStringVector swapNames					= deltaLadder[ "swapNames" ];
		AQLStringMatrix curveCollectionNames		= deltaLadder[ "curveCollectionNames" ];
		AQLStringMatrix fixingTableNames			= deltaLadder[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= deltaLadder[ "bumpSpreadInstruments" ];
		double bumpSize							= deltaLadder[ "bumpSize" ];
		AQLString bumpMode						= deltaLadder[ "bumpMode" ];
		bool aggregateRisks						= deltaLadder[ "aggregateRisks" ];
		bool reportInLegCCY						= deltaLadder[ "reportInLegCCY" ];
		std::string riskCutOffTenor				= deltaLadder[ "riskCutOffTenor" ];

        // Dummy Xccy FX Spot Rates
        DoubleVector dummyXccyFXSpotRates( swapNames.size(), 1.0 );

		validation::tryAqObjSwapsDeltaLadder(headers,
												pillarNames,
												deltas,
												swapNames,
												curveCollectionNames,
												fixingTableNames,
												bumpSpreadInstruments,
												bumpSize,
												bumpMode,
												aggregateRisks,
												reportInLegCCY,
												riskCutOffTenor,
                                                dummyXccyFXSpotRates );
	}

//	void verifyDeltaBucketAmounts( const AQLStringVector& pillarNames, const AQLStringVector& headers, const DoubleMatrix& deltas,
//									const std::string& raw_output_32,
//									const std::string& raw_output_64,
//									const std::string& output_32,
//									const std::string& output_64 )
//	{
//		// Verify delta bucket amounts
//        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
//        {
//#ifdef GTEST32
//            AQLString outputFileName = raw_output_32.c_str();
//#else
//            AQLString outputFileName = raw_output_64.c_str();
//#endif
//
//            // Record outputs and rebase test outputs
//            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
//            CreateDataFile file( etrading::decorateFilename( outputFileName ) );
//
//			file.write( "headers", headers );
//
//            for ( size_t i = 0; i < pillarNames.size(); ++i )
//            {
//                // 12 is the number of decimal points required
//                file.write( pillarNames[i], deltas[i], 12 );
//            }
//        }
//        else
//        {
//            // Carry out actual test and peform result comparison
//#ifdef GTEST32
//            const ReadDataFile::Load resultFile( output_32.c_str() );
//#else
//            const ReadDataFile::Load resultFile( output_64.c_str() );
//#endif
//
//			if ( ! resultFile.hasItem("headers") )
//			{
//				FAIL() << "Reference baseline does not contain delta column headers" << std::endl;
//			}
//
//			AQLStringVector refHeaders = resultFile["headers"];
//			if ( headers.size() != refHeaders.size() )
//			{
//				FAIL() << "Calculated delta has different number of column headers compared to reference baseline : " << headers.size() << " vs " << refHeaders.size() << std::endl;
//			}
//			for ( size_t i = 0; i < headers.size(); ++i )
//			{
//				ASSERT_EQ( headers[i], refHeaders[i] );
//			}
//
//            for ( size_t i = 0; i < pillarNames.size(); ++i )
//            {
//                AQLString key	= pillarNames[i];
//
//				if ( ! resultFile.hasItem(key) )
//				{
//					FAIL() << "Calculated results contains a pillarName: " << key << " which is missing in reference baseline" << std::endl;
//				}
//
//                DoubleVector delta	= deltas[i];
//                DoubleVector ref = resultFile[key];
//
//				if ( delta.size() != ref.size() )
//				{
//					FAIL() << "Calculated delta has different number of columns compared to reference baseline: " << delta.size() << " vs " << ref.size() << std::endl;
//				}
//
//                for( size_t j = 0; j < delta.size(); ++j )
//                {
//                    EXPECT_NEAR( delta[j], ref[j], tolerance )
//                            << " Delta for pillar point " << key.getCString() << " is incorrect ";
//                }
//            }
//		}
//
//	}

	std::string helperCreateSwapFromSwapGenerator(const char* swapGeneratorInputs, const char* swapInputs)
	{
		// Swap Generator
        const ReadDataFile::Load swapGeneratorInputFile( swapGeneratorInputs );
		AQLStringMatrix swapGeneratorLVB = swapGeneratorInputFile[ "swapGeneratorLVB" ];
		std::string swapGeneratorName = validation::tryAqObjSwapsGeneratorCreate( "EUR_6ML", swapGeneratorLVB );

		// Swap
		const ReadDataFile::Load swapInputFile( swapInputs );
		std::string swapName			= swapInputFile[ "swapName" ];
		AQLStringMatrix expressionLVB		= swapInputFile[ "expressionLVB" ];
		AQLStringMatrix swapPropertiesLVB	= swapInputFile[ "swapPropertiesLVB" ];
		bool isXccySwap					= swapInputFile[ "isXccySwap" ];
		bool validateKeys				= swapInputFile[ "validateKeys" ];
		return validation::tryAqObjSwapsCreateFromGenerator( swapName, swapGeneratorName, expressionLVB, swapPropertiesLVB, isXccySwap, validateKeys );
	}


	std::string createSwapFromDataFile( const char* swapInputs )
	{
		const ReadDataFile::Load swapInputFile( swapInputs );
		std::string swapName		= swapInputFile[ "swapName" ];
		AQLStringMatrix swapLvb		= swapInputFile[ "swapLVB" ];
		AQLStringMatrix swapProperties	= swapInputFile[ "swapPropertiesLVB" ];
		bool isXccySwap				= swapInputFile[ "isXccySwap" ];
		bool validateKeys			= swapInputFile[ "validateKeys" ];
		return validation::tryAqObjSwapsCreate( swapName, swapLvb, swapProperties, isXccySwap, validateKeys );
	};
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestAQObjHedgeCurveDelta);

	TEST_F( TestAQObjHedgeCurveDelta, RISK_DeltaLadder_Spot4Y_PricingCurve )
    {
		// 1. Build Pricing Curves
		setUpGeneratorCurve( pricing_USDYC_OIS_marketData, calibrate_pricing_USDYC_OIS );
		setUpGeneratorCurve( pricing_USDYC_STD_marketData, calibrate_pricing_USDYC_STD );

		// 2. Load trade: 4Y Interest Rate Swap
		std::string swapGeneratorName = "USD_3ML";
		std::string irsSwap = helperCreateSwapFromSwapGenerator( swapGeneratorName, irs_spot_4Y_SwapInputs);

		// 3. Run Delta Ladder calculation using pricing curves
		const ReadDataFile::Load deltaLadder_spot_4Y_swap ( deltaLadder_spot_4Y_swap_pricingCurveCollection_input );
		AQLStringVector pillarNames;
		AQLStringVector headers;
		DoubleMatrix deltas;
		runDeltaLadderCalculation( deltaLadder_spot_4Y_swap, pillarNames, headers, deltas );	

		// 4. Expect swap risk to fall entirely in the 4Y bucket.
		verifyDeltaBucketAmounts( pillarNames, headers, deltas, tolerance, TEST_DIR,
									raw_delta_ladder_outputs_spot_4Y_pricingCurveCollection_32, 
									raw_delta_ladder_outputs_spot_4Y_pricingCurveCollection_64);
	}

	TEST_F( TestAQObjHedgeCurveDelta, RISK_DeltaLadder_Spot4Y_HedgeCurve )
    {

		// 1. Build Pricing Curves
		setUpGeneratorCurve( pricing_USDYC_OIS_marketData, calibrate_pricing_USDYC_OIS );
		setUpGeneratorCurve( pricing_USDYC_STD_marketData, calibrate_pricing_USDYC_STD );

		// 2. Build Hedge Curve
		setupGeneratorHedgeCurve( hedge_USDYC_HEDGE_STD_marketData, curve_generator_modify_interpolation, calibrate_hedge_curve_inputs );

		// 3. Load trade: 4Y Interest Rate Swap
		std::string swapGeneratorName = "USD_3ML";
		std::string irsSwap = helperCreateSwapFromSwapGenerator( swapGeneratorName, irs_spot_4Y_SwapInputs);

		// 4. Run Delta Ladder calculation using hedge  curves (with 4Y point missing)
		const ReadDataFile::Load deltaLadder_spot_4Y_swap ( deltaLadder_spot_4Y_swap_hedgeCurveCollection_input );

		AQLStringVector pillarNames;
		AQLStringVector headers;
		DoubleMatrix deltas;
		runDeltaLadderCalculation( deltaLadder_spot_4Y_swap, pillarNames, headers, deltas );

		// 5. Expect risk to be spread between the 3Y and 5Y buckets, 50% of each bucket (once the DV01 is scaled by the DV01 of each calibration instrument)
		verifyDeltaBucketAmounts( pillarNames, headers, deltas, tolerance, TEST_DIR,
									raw_delta_ladder_outputs_spot_4Y_hedgeCurveCollection_32, 
									raw_delta_ladder_outputs_spot_4Y_hedgeCurveCollection_64);

	}

	TEST_F( TestAQObjHedgeCurveDelta, RISK_DeltaLadder_3YFwdStart_4Y_HedgeCurve )
    {
		// 1. Build Pricing Curves
		setUpGeneratorCurve( pricing_USDYC_OIS_marketData, calibrate_pricing_USDYC_OIS );
		setUpGeneratorCurve( pricing_USDYC_STD_marketData, calibrate_pricing_USDYC_STD );

		// 2. Build Hedge Curve
		setupGeneratorHedgeCurve( hedge_USDYC_HEDGE_STD_marketData, curve_generator_modify_interpolation, calibrate_hedge_curve_inputs );

		// 3. Load trade: 3Y forward starting 4Y Interest Rate Swap
		std::string swapGeneratorName = "USD_3ML";
		std::string irsSwap = helperCreateSwapFromSwapGenerator( swapGeneratorName, irs_3Yfwdstart_4Y_SwapInputs);

		// 4. Run Delta Ladder calculation
		const ReadDataFile::Load deltaLadder_spot_4Y_swap ( deltaLadder_3Yfwdst_4Y_swap_hedgeCurveCollection_input );

		AQLStringVector pillarNames;
		AQLStringVector headers;
		DoubleMatrix deltas;
		runDeltaLadderCalculation( deltaLadder_spot_4Y_swap, pillarNames, headers, deltas );

		// 5. Expect risk to show: short 3Y swap, long 7Y swap
		verifyDeltaBucketAmounts( pillarNames, headers, deltas, tolerance, TEST_DIR,
									raw_delta_ladder_outputs_3Yfwdst_4Y_hedgeCurveCollection_32, 
									raw_delta_ladder_outputs_3Yfwdst_4Y_hedgeCurveCollection_64);
	}

	TEST_F( TestAQObjHedgeCurveDelta, RISK_DeltaLadder_3Y9MFwdStart_3M_HedgeCurve )
    {
		// 1. Build Pricing Curves
		setUpGeneratorCurve( pricing_USDYC_OIS_marketData, calibrate_pricing_USDYC_OIS );
		setUpGeneratorCurve( pricing_USDYC_STD_marketData, calibrate_pricing_USDYC_STD );

		// 2. Build Hedge Curve
		setupGeneratorHedgeCurve( hedge_USDYC_HEDGE_STD_marketData, curve_generator_modify_interpolation, calibrate_hedge_curve_inputs );

		// 3. Load trade: 3Y9M forward starting, 3M Interest Rate Swap
		// i.e. just a single cashflow
		std::string swapGeneratorName = "USD_3ML";
		std::string irsSwap = helperCreateSwapFromSwapGenerator( swapGeneratorName, irs_3Y9Mfwdstart_3M_SwapInputs);

		// 4. Run Delta Ladder calculation
		const ReadDataFile::Load deltaLadder_spot_4Y_swap ( deltaLadder_3Y9Mfwdst_3M_swap_hedgeCurveCollection_input );

		AQLStringVector pillarNames;
		AQLStringVector headers;
		DoubleMatrix deltas;
		runDeltaLadderCalculation( deltaLadder_spot_4Y_swap, pillarNames, headers, deltas );

		// 5. Expect risk to show: short 3Y, long 4Y, with total risk coming to ( 4Y DV01 / ( 4Y * 4 cashflows per year) )
		verifyDeltaBucketAmounts( pillarNames, headers, deltas, tolerance, TEST_DIR,
									raw_delta_ladder_outputs_3Y9Mfwdst_3M_hedgeCurveCollection_32, 
									raw_delta_ladder_outputs_3Y9Mfwdst_3M_hedgeCurveCollection_64);
	}

}
