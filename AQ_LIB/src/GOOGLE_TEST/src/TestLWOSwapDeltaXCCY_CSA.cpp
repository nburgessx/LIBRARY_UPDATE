/*
 * @brief			Regression tests on the xccy swap DV01. This version tests using different CSA. i.e. it uses FXFWDCONST curves
 * @Created:		24 May 2017
 * @Author:			Ian Castleton
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

// Curves
#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "TryMeCurveTenorBasis.h"
#include "TryMeCurveXccyBasis.h"
#include "TryMeCurveFwdFxConst.h"

// Swap Creation and Pricing
#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"

// Risk calculation
#include "tryMeLWOSwapDelta.h"
#include "tryMeLWOFixingTable.h"

// Test Infrastructure
#include "Dependency.h"   // Curve Macros are Here !!!
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"
#include <gTest/gTest.h>

// #include "TestHelperUtilities.h"


using etrading::ReadDataFile;
using etrading::CreateDataFile;

// Define the Test Input Folder Here
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapDeltaXCCY_CSA/"


namespace
{    
	// test tolerance
    const double tolerance = 2e-2; // Notional of test trades is 1MM and delta by bumping is inherently noisy

	//
    // curve input files
    //
    extern const char USDYC_OIS[]					= TEST_DIR "USDYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    extern const char USDYC_STD[]					= TEST_DIR "USDYC_STD_tryMeCurveCalibrateSwap_inputs.csv";

    extern const char EURYC_OIS[]					= TEST_DIR "EURYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    extern const char EURYC_STD[]					= TEST_DIR "EURYC_STD_tryMeCurveCalibrateSwap_inputs.csv";
    extern const char EURYC_3M6M[]					= TEST_DIR "EURYC_3M6M_tryMeCurveCalibrateBasis_inputs.csv";
    extern const char EURYC_XCCY[]					= TEST_DIR "EURYC_XCCY_tryMeCurveCalibrateBasis_inputs.csv";

    extern const char GBPYC_OIS[]					= TEST_DIR "GBPYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    extern const char GBPYC_STD[]					= TEST_DIR "GBPYC_STD_tryMeCurveCalibrateSwap_inputs.csv";
    extern const char GBPYC_XCCY[]					= TEST_DIR "GBPYC_XCCY_tryMeCurveCalibrateBasis_inputs.csv";

	extern const char JPYYC_OIS[]					= TEST_DIR "JPYYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    extern const char JPYYC_JSCC_STD[]				= TEST_DIR "JPYYC_STD_tryMeCurveCalibrateSwap_inputs.csv";
    extern const char JPYYC_LCH_6M[]				= TEST_DIR "JPYYC_LCH6M_tryMeCurveCalibrateSwap_inputs.csv";
	extern const char JPYYC_LCH_3M[]				= TEST_DIR "JPYYC_LCH3M_tryMeCurveCalibrateBasis_inputs.csv";
    extern const char JPYYC_XCCY[]					= TEST_DIR "JPYYC_XCCY_tryMeCurveCalibrateBasis_inputs.csv";

	extern const char EURYC_FXFWDGBP[]				= TEST_DIR "EURYC_FWDGBP_tryMeCurveCalibrateFXForwards_inputs.csv";
	extern const char EURYC_FXFWDJPY[]				= TEST_DIR "EURYC_FWDJPY_tryMeCurveCalibrateFXForwards_inputs.csv";

	extern const char USDYC_FXFWDGBP[]				= TEST_DIR "USDYC_FWDGBPCSA_tryMeCurveCalibrateFXForwards_inputs.csv";
	extern const char USDYC_FXFWDEUR[]				= TEST_DIR "USDYC_FWDEURCSA_tryMeCurveCalibrateFXForwards_inputs.csv";
	extern const char USDYC_FXFWDJPY[]				= TEST_DIR "USDYC_FWDJPYCSA_tryMeCurveCalibrateFXForwards_inputs.csv";

	//
    // test call input and reference files
    //
	extern const char xccySwap_USDCSA[]				= TEST_DIR "XCCY_tryMeLWOSwapCreate_USDCSA_inputs.csv";    // EUR / USD XCCY swap, USD CSA i.e. regular XCCY swap
	extern const char xccySwap_GBPCSA[]				= TEST_DIR "XCCY_tryMeLWOSwapCreate_GBPCSA_inputs.csv";    // EUR / USD XCCY swap, GBP CSA
	extern const char xccySwap_EURCSA[]				= TEST_DIR "XCCY_tryMeLWOSwapCreate_EURCSA_inputs.csv";    // EUR / USD XCCY swap, EUR CSA
	extern const char xccySwap_JPYCSA[]				= TEST_DIR "XCCY_tryMeLWOSwapCreate_JPYCSA_inputs.csv";    // EUR / USD XCCY swap, JPY CSA

	// DV01 tests
	extern const char swapDV01Inputs[]				= TEST_DIR "tryMeLWOSwapDV01_inputs_";  // set up the DV01 calculation

	// Reference DV01 outputs base filename. A currency csa suffix is appended by the test in order to get the actual filename
	extern const char raw_dv01_outputs_32[]			= "tryMeLWOSwapDV01_outputs_";
	extern const char raw_dv01_outputs_64[]			= "tryMeLWOSwapDV01_outputs_64bit_";
	extern const char dv01_outputs_32[]				= TEST_DIR "tryMeLWOSwapDV01_outputs_";
	extern const char dv01_outputs_64[]				= TEST_DIR "tryMeLWOSwapDV01_outputs_64bit_";

	// DeltaLadder tests
	extern const char deltaLadderInputs[]			= TEST_DIR "tryMeLWOSwapDeltaLadder_inputs_";  // set up the DeltaLadder calculation

	// References DeltaLadder outputs base filename. A currency csa suffix is appended by the test in order to get the actual filename
	extern const char raw_delta_ladder_outputs_32[]	= "tryMeLWOSwapDeltaLadder_outputs_";
	extern const char raw_delta_ladder_outputs_64[]	= "tryMeLWOSwapDeltaLadder_outputs_64bit_";
	extern const char delta_ladder_outputs_32[]		= TEST_DIR "tryMeLWOSwapDeltaLadder_outputs_";
	extern const char delta_ladder_outputs_64[]		= TEST_DIR "tryMeLWOSwapDeltaLadder_outputs_64bit_";

	std::string createSwapFromDataFile( const char* swapInputs )
	{
		const ReadDataFile::Load swapInputFile( swapInputs );
		std::string swapName		= swapInputFile[ "swapName" ];
		LAStringMatrix swapLvb		= swapInputFile[ "swapLVB" ];
		LAStringMatrix swapProperties	= swapInputFile[ "swapPropertiesLVB" ];
		bool isXccySwap				= swapInputFile[ "isXccySwap" ];
		bool validateKeys			= swapInputFile[ "validateKeys" ];
		return validation_api::tryMeLWOSwapCreate( swapName, swapLvb, swapProperties, isXccySwap, validateKeys );
	};
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestLWOSwapDeltaXCCY_CSA);

	/* @brief	Run consistency test on all available swap curves in all ccys
    */
    void buildCurves_USDCSA()
	{        
		// Set up USD curves
		setUpMeOISCurve( USDYC_OIS );
		setUpMeSTDCurve( USDYC_STD );

		// Set up EUR XCCY curve
		setUpMeOISCurve( EURYC_OIS );
		setUpMeSTDCurve( EURYC_STD );
		setUpMeTenorBasisCurve( EURYC_3M6M );
		setUpMeXccyBasisCurve( EURYC_XCCY );
    }

    void buildCurves_EURCSA()
	{
		// Set up USD curves
		setUpMeOISCurve( USDYC_OIS );
		setUpMeSTDCurve( USDYC_STD );

		// Set up EUR XCCY curve
		setUpMeOISCurve( EURYC_OIS );
		setUpMeSTDCurve( EURYC_STD );
		setUpMeTenorBasisCurve( EURYC_3M6M );
		setUpMeXccyBasisCurve( EURYC_XCCY );

		// Set up FXFWDCONST curves
		setUpMeFwdFxConstCurveCurve( USDYC_FXFWDEUR );  // USD discounting with EUR CSA
	}

	void buildCurves_JPYCSA()
	{
		// Set up USD curves
		setUpMeOISCurve( USDYC_OIS );
		setUpMeSTDCurve( USDYC_STD );

		// Set up EUR XCCY curve
		setUpMeOISCurve( EURYC_OIS );
		setUpMeSTDCurve( EURYC_STD );
		setUpMeTenorBasisCurve( EURYC_3M6M );
		setUpMeXccyBasisCurve( EURYC_XCCY );

		// Set up JPY XCCY curve
		setUpMeOISCurve( JPYYC_OIS );
		setUpMeSTDCurve( JPYYC_JSCC_STD );
		setUpMeSTDCurve( JPYYC_LCH_6M );
		setUpMeTenorBasisCurve( JPYYC_LCH_3M );
		setUpMeXccyBasisCurve( JPYYC_XCCY );

		// Set up FXFWDCONST curves
		setUpMeFwdFxConstCurveCurve( EURYC_FXFWDJPY );  // EUR discounting with JPY CSA
		setUpMeFwdFxConstCurveCurve( USDYC_FXFWDJPY );  // USD discounting with JPY CSA
	}

	void buildCurves_GBPCSA()
	{
		// Set up USD curves
		setUpMeOISCurve( USDYC_OIS );
		setUpMeSTDCurve( USDYC_STD );

		// Set up EUR XCCY curve
		setUpMeOISCurve( EURYC_OIS );
		setUpMeSTDCurve( EURYC_STD );
		setUpMeTenorBasisCurve( EURYC_3M6M );
		setUpMeXccyBasisCurve( EURYC_XCCY );

		// Set up GBP XCCY curve
		setUpMeOISCurve( GBPYC_OIS );
		setUpMeSTDCurve( GBPYC_STD );
		setUpMeXccyBasisCurve( GBPYC_XCCY );

		// Set up FXFWDCONST curves
		setUpMeFwdFxConstCurveCurve( EURYC_FXFWDGBP );  // EUR discounting with GBP CSA
		setUpMeFwdFxConstCurveCurve( USDYC_FXFWDGBP );  // USD discounting with GBP CSA
	}

	void calcDV01AndCompareToReference( const char* csaSuffix )
	{
		const ReadDataFile::Load flatShiftDelta( swapDV01Inputs + LAString( csaSuffix ) );
		LAStringVector swapNames					= flatShiftDelta[ "swapNames" ];
		LAStringMatrix curveCollectionNames		= flatShiftDelta[ "curveCollectionNames" ];
		LAStringMatrix fixingTableNames			= flatShiftDelta[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= flatShiftDelta[ "bumpSpreadInstruments" ];
		double bumpSize							= flatShiftDelta[ "bumpSize" ];
		LAString bumpMode						= flatShiftDelta[ "bumpMode" ];
		LAString groupRiskBy					= flatShiftDelta[ "groupRiskBy" ];
		bool aggregateRisks						= flatShiftDelta[ "aggregateRisks" ];
		bool reportInLegCCY						= flatShiftDelta[ "reportInLegCCY" ];
        const DoubleVector xccyFXSpotRates   	= flatShiftDelta[ "xccyFXSpotRates" ];

		LAStringVector positionIDs;
		DoubleVector deltas;
		// The DV01 is actually calculated by the tryMeLWOSwapDelta function
		validation_api::tryMeLWOSwapDelta( positionIDs,
										   deltas,
										   swapNames,
										   curveCollectionNames,
										   fixingTableNames,
										   bumpSpreadInstruments,
										   bumpSize,
										   bumpMode,
										   groupRiskBy,
										   aggregateRisks,
										   reportInLegCCY,
                                           xccyFXSpotRates );

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
#ifdef GTEST32
            LAString outputFileName = raw_dv01_outputs_32 + LAString( csaSuffix );
#else
            LAString outputFileName = raw_dv01_outputs_64 + LAString( csaSuffix );
#endif

            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( outputFileName ) );
            for ( size_t i = 0; i < positionIDs.size(); ++i )
            {
                // 12 is the number of decimal points required
                file.write( positionIDs[i], deltas[i], 12 );
            }
        }
        else
        {
            // Carry out actual test and peform result comparison
#ifdef GTEST32
			LAString refFileName = dv01_outputs_32 + LAString( csaSuffix );
            const ReadDataFile::Load resultFile( refFileName );
#else
			LAString refFileName = dv01_outputs_64 + LAString( csaSuffix );
            const ReadDataFile::Load resultFile( refFileName );
#endif

            for ( size_t i = 0; i < positionIDs.size(); ++i )
            {
                LAString key	= positionIDs[i];

                double delta	= deltas[i];
                double ref		= resultFile[key];

				EXPECT_NEAR( delta, ref, tolerance )
					<< " Delta : " << key.getCString() << " is incorrect ";

            }
        }
	}

	void calcDeltaLadderAndCompareToReference( const char* csaSuffix )
	{
		const ReadDataFile::Load deltaLadder ( deltaLadderInputs + LAString( csaSuffix ) );
		LAStringVector swapNames			    = deltaLadder[ "swapNames" ];
		LAStringMatrix curveCollectionNames   = deltaLadder[ "curveCollectionNames" ];
		LAStringMatrix fixingTableNames	    = deltaLadder[ "fixingTableNames" ];
		bool bumpSpreadInstruments		    = deltaLadder[ "bumpSpreadInstruments" ];
		double bumpSize					    = deltaLadder[ "bumpSize" ];
		LAString bumpMode				    = deltaLadder[ "bumpMode" ];
		bool aggregateRisks				    = deltaLadder[ "aggregateRisks" ];
		bool reportInLegCCY				    = deltaLadder[ "reportInLegCCY" ];
        const DoubleVector xccyFXSpotRates      = deltaLadder[ "xccyFXSpotRates" ];
		std::string riskCutOffTenor         = "";

		LAStringVector pillarNames;
		LAStringVector headers;
		DoubleMatrix deltas;
		validation_api::tryMeLWOSwapDeltaLadder(headers,
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
                                                xccyFXSpotRates );

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
#ifdef GTEST32
            LAString outputFileName = raw_delta_ladder_outputs_32 + LAString( csaSuffix );;
#else
            LAString outputFileName = raw_delta_ladder_outputs_64 + LAString( csaSuffix );;
#endif

            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( outputFileName ) );

			file.write( "headers", headers );

            for ( size_t i = 0; i < pillarNames.size(); ++i )
            {
                // 12 is the number of decimal points required
                file.write( pillarNames[i], deltas[i], 12 );
            }
        }
        else
        {
            // Carry out actual test and peform result comparison
#ifdef GTEST32
			LAString refFileName = delta_ladder_outputs_32 + LAString( csaSuffix );
            const ReadDataFile::Load resultFile( refFileName );
#else
			LAString refFileName = delta_ladder_outputs_64 + LAString( csaSuffix );
            const ReadDataFile::Load resultFile( refFileName );
#endif

			if ( ! resultFile.hasItem("headers") )
			{
				FAIL() << "Reference baseline does not contain delta column headers" << std::endl;
			}

			LAStringVector refHeaders = resultFile["headers"];
			if ( headers.size() != refHeaders.size() )
			{
				FAIL() << "Calculated delta has different number of column headers compared to reference baseline : " << headers.size() << " vs " << refHeaders.size() << std::endl;
			}
			for ( size_t i = 0; i < headers.size(); ++i )
			{
				ASSERT_EQ( headers[i], refHeaders[i] );
			}

            for ( size_t i = 0; i < pillarNames.size(); ++i )
            {
                LAString key	= pillarNames[i];

				if ( ! resultFile.hasItem(key) )
				{
					FAIL() << "Calculated results contains a pillarName: " << key << " which is missing in reference baseline" << std::endl;
				}

                DoubleVector delta	= deltas[i];
                DoubleVector ref = resultFile[key];

				if ( delta.size() != ref.size() )
				{
					FAIL() << "Calculated delta has different number of columns compared to reference baseline: " << delta.size() << " vs " << ref.size() << std::endl;
				}

                for( size_t j = 0; j < delta.size(); ++j )
                {
                    EXPECT_NEAR( delta[j], ref[j], tolerance )
                            << " Delta for pillar point " << key.getCString() << " is incorrect ";
                }
            }
        }
	}

	// DV01 tests
	TEST_F( TestLWOSwapDeltaXCCY_CSA, RISK_swapDV01_USDCSA)
    {
        buildCurves_USDCSA();
		createSwapFromDataFile( xccySwap_USDCSA );
		calcDV01AndCompareToReference( "usdcsa.csv" );
	}

	TEST_F( TestLWOSwapDeltaXCCY_CSA, RISK_swapDV01_EURCSA)
    {
        buildCurves_EURCSA();
		createSwapFromDataFile( xccySwap_EURCSA );
		calcDV01AndCompareToReference( "eurcsa.csv" );
	}

	TEST_F( TestLWOSwapDeltaXCCY_CSA, RISK_swapDV01_JPYCSA)
    {
        buildCurves_JPYCSA();
		createSwapFromDataFile( xccySwap_JPYCSA );
		calcDV01AndCompareToReference( "jpycsa.csv" );
	}

	TEST_F( TestLWOSwapDeltaXCCY_CSA, RISK_swapDV01_GBPCSA)
    {
        buildCurves_GBPCSA();
		createSwapFromDataFile( xccySwap_GBPCSA );
		calcDV01AndCompareToReference( "gbpcsa.csv" );
	}

	// DeltaLadder tests
	TEST_F( TestLWOSwapDeltaXCCY_CSA, RISK_swapDeltaLadder_USDCSA)
    {
        buildCurves_USDCSA();
		createSwapFromDataFile( xccySwap_USDCSA );
		calcDeltaLadderAndCompareToReference( "usdcsa.csv" );
	}

	TEST_F( TestLWOSwapDeltaXCCY_CSA, RISK_swapDeltaLadder_EURCSA)
    {
        buildCurves_EURCSA();
		createSwapFromDataFile( xccySwap_EURCSA );
		calcDeltaLadderAndCompareToReference( "eurcsa.csv" );
	}

	TEST_F( TestLWOSwapDeltaXCCY_CSA, RISK_swapDeltaLadder_JPYCSA)
    {
        buildCurves_JPYCSA();
		createSwapFromDataFile( xccySwap_JPYCSA );
		calcDeltaLadderAndCompareToReference( "jpycsa.csv" );
	}

	TEST_F( TestLWOSwapDeltaXCCY_CSA, RISK_swapDeltaLadder_GBPCSA)
    {
        buildCurves_GBPCSA();
		createSwapFromDataFile( xccySwap_GBPCSA );
		calcDeltaLadderAndCompareToReference( "gbpcsa.csv" );
	}
}
