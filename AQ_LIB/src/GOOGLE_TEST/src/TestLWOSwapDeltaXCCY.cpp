// Curves
#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "TryMeCurveTenorBasis.h"
#include "TryMeCurveXccyBasis.h"

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

using etrading::ReadDataFile;
using etrading::CreateDataFile;

// Define the Test Input Folder Here
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapDeltaXCCY/"

namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 1.0; // Notional of test trades is 1MM and delta by bumping is inherently noisy
								  // Also: In 64bits desktop PC (i7-4790) gives slightly different numerical results to Build Server (Xeon E7-4870)  (?!)

    //
    // curve input files
    //
    extern const char USDYC_OIS[]					= TEST_DIR "USDYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    extern const char USDYC_STD[]					= TEST_DIR "USDYC_STD_tryMeCurveCalibrateSwap_inputs.csv";

    extern const char EURYC_OIS[]					= TEST_DIR "EURYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    extern const char EURYC_STD[]					= TEST_DIR "EURYC_STD_tryMeCurveCalibrateSwap_inputs.csv";
    extern const char EURYC_3M6M[]					= TEST_DIR "EURYC_3M6M_tryMeCurveCalibrateBasis_inputs.csv";
    extern const char EURYC_XCCY[]					= TEST_DIR "EURYC_XCCY_tryMeCurveCalibrateBasis_inputs.csv";

	extern const char JPYYC_OIS[]					= TEST_DIR "JPYYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    extern const char JPYYC_JSCC_STD[]				= TEST_DIR "JPYYC_STD_tryMeCurveCalibrateSwap_inputs.csv";
    extern const char JPYYC_LCH_6M[]				= TEST_DIR "JPYYC_LCH6M_tryMeCurveCalibrateBasis_inputs.csv";
	extern const char JPYYC_LCH_3M[]				= TEST_DIR "JPYYC_LCH3M_tryMeCurveCalibrateBasis_inputs.csv";
    extern const char JPYYC_XCCY[]					= TEST_DIR "JPYYC_XCCY_tryMeCurveCalibrateBasis_inputs.csv";

    //
    // test call input and reference files
    //
	extern const char xccySwapInputs1_Swap1[]		= TEST_DIR "XCCY1@4_tryMeLWOSwapCreate_inputs_1.csv";
	extern const char xccySwapInputs1_Swap2[]		= TEST_DIR "XCCY2@1_tryMeLWOSwapCreate_inputs_1.csv";

	extern const char xccySwapInputs2_Swap1[]		= TEST_DIR "XCCY1@4_tryMeLWOSwapCreate_inputs_2.csv";
	extern const char xccySwapInputs2_Swap2[]		= TEST_DIR "XCCY2@1_tryMeLWOSwapCreate_inputs_2.csv";

	extern const char raw_delta_ladder_outputs1_32[]	= "tryMeLWOSwapDeltaLadder_outputs_1.csv";
	extern const char raw_delta_ladder_outputs1_64[]	= "tryMeLWOSwapDeltaLadder_outputs_1_64bit.csv";

	extern const char raw_delta_ladder_outputs2_32[]	= "tryMeLWOSwapDeltaLadder_outputs_2.csv";
	extern const char raw_delta_ladder_outputs2_64[]	= "tryMeLWOSwapDeltaLadder_outputs_2_64bit.csv";

	extern const char raw_delta_ladder_outputs3_32[]	= "tryMeLWOSwapDeltaLadder_outputs_3.csv";
	extern const char raw_delta_ladder_outputs3_64[]	= "tryMeLWOSwapDeltaLadder_outputs_3_64bit.csv";

	extern const char delta_ladder_outputs1_32[]		= TEST_DIR "tryMeLWOSwapDeltaLadder_outputs_1.csv";
	extern const char delta_ladder_outputs1_64[]		= TEST_DIR "tryMeLWOSwapDeltaLadder_outputs_1_64bit.csv";

	extern const char delta_ladder_outputs2_32[]		= TEST_DIR "tryMeLWOSwapDeltaLadder_outputs_2.csv";
	extern const char delta_ladder_outputs2_64[]		= TEST_DIR "tryMeLWOSwapDeltaLadder_outputs_2_64bit.csv";

	extern const char delta_ladder_outputs3_32[]		= TEST_DIR "tryMeLWOSwapDeltaLadder_outputs_3.csv";
	extern const char delta_ladder_outputs3_64[]		= TEST_DIR "tryMeLWOSwapDeltaLadder_outputs_3_64bit.csv";

	extern const char deltaLadderInputs[]			= TEST_DIR "tryMeLWOSwapDeltaLadder_inputs.csv";  // ValuationCCY is USD 
    extern const char deltaLadderInputs1[]			= TEST_DIR "tryMeLWOSwapDeltaLadder_inputs_1.csv";  // ValuationCCY is USD 


	std::string helperCreateSwapFromSwapGenerator(const char* swapGeneratorInputs, const char* swapInputs)
	{
		// Swap Generator
        const ReadDataFile::Load swapGeneratorInputFile( swapGeneratorInputs );
		AQLStringMatrix swapGeneratorLVB = swapGeneratorInputFile[ "swapGeneratorLVB" ];
		std::string swapGeneratorName = validation::tryMeLWOSwapGeneratorCreate( "EUR_6ML", swapGeneratorLVB );

		// Swap
		const ReadDataFile::Load swapInputFile( swapInputs );
		std::string swapName			= swapInputFile[ "swapName" ];
		AQLStringMatrix expressionLVB		= swapInputFile[ "expressionLVB" ];
		AQLStringMatrix swapPropertiesLVB	= swapInputFile[ "swapPropertiesLVB" ];
		bool isXccySwap					= swapInputFile[ "isXccySwap" ];
		bool validateKeys				= swapInputFile[ "validateKeys" ];
		return validation::tryMeLWOSwapCreateFromGenerator( swapName, swapGeneratorName, expressionLVB, swapPropertiesLVB, isXccySwap, validateKeys );
	}

	std::string createSwapFromDataFile( const char* swapInputs )
	{
		const ReadDataFile::Load swapInputFile( swapInputs );
		std::string swapName		= swapInputFile[ "swapName" ];
		AQLStringMatrix swapLvb		= swapInputFile[ "swapLVB" ];
		AQLStringMatrix swapProperties	= swapInputFile[ "swapPropertiesLVB" ];
		bool isXccySwap				= swapInputFile[ "isXccySwap" ];
		bool validateKeys			= swapInputFile[ "validateKeys" ];
		return validation::tryMeLWOSwapCreate( swapName, swapLvb, swapProperties, isXccySwap, validateKeys );
	};

}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the ME_BUILD_EURUSD_XCCY_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
	
	ME_BUILD_EURUSD_JPYUSD_XCCY_CURVE( TestLWOSwapDeltaXCCY, USDYC_OIS, USDYC_STD, EURYC_OIS, EURYC_STD, EURYC_3M6M, EURYC_XCCY, JPYYC_OIS, JPYYC_JSCC_STD, JPYYC_LCH_6M, JPYYC_LCH_3M, JPYYC_XCCY );


	// 1. Valuation Currency in USD for both XCCY swaps. USDCSA for both swaps.
	// We expect a single USD total for each pillar point, along with the breakdowbn by swap leg.
	TEST_F( TestLWOSwapDeltaXCCY, RISK_DeltaLadder_ValuationCCY_USD )
    {
		// 1. Load the Input Files

		// 5YR XCCY  JPY USD, USDCSA, valuationCCY=USD
		std::string xccySwap1 = createSwapFromDataFile( xccySwapInputs1_Swap1 );

		// 5YR XCCY  EUR USD, USDCSA, valuationCCY=USD
		std::string xccySwap2 = createSwapFromDataFile( xccySwapInputs1_Swap2 );

		const ReadDataFile::Load deltaLadder ( deltaLadderInputs );
		AQLStringVector swapNames					= deltaLadder[ "swapNames" ];
		AQLStringMatrix curveCollectionNames		= deltaLadder[ "curveCollectionNames" ];
		AQLStringMatrix fixingTableNames			= deltaLadder[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= deltaLadder[ "bumpSpreadInstruments" ];
		double bumpSize							= deltaLadder[ "bumpSize" ];
		AQLString bumpMode						= deltaLadder[ "bumpMode" ];
		bool aggregateRisks						= deltaLadder[ "aggregateRisks" ];
		bool reportInLegCCY						= deltaLadder[ "reportInLegCCY" ];
        const DoubleVector xccyFXSpotRates   	= deltaLadder[ "xccyFXSpotRates" ];
		std::string riskCutOffTenor             = "";

		AQLStringVector pillarNames;
		AQLStringVector headers;
		DoubleMatrix deltas;
		validation::tryMeLWOSwapDeltaLadder(headers,
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
            AQLString outputFileName = raw_delta_ladder_outputs1_32;
#else
            AQLString outputFileName = raw_delta_ladder_outputs1_64;
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
            const ReadDataFile::Load resultFile( delta_ladder_outputs1_32 );
#else
            const ReadDataFile::Load resultFile( delta_ladder_outputs1_64 );
#endif

			if ( ! resultFile.hasItem("headers") )
			{
				FAIL() << "Reference baseline does not contain delta column headers" << std::endl;
			}

			AQLStringVector refHeaders = resultFile["headers"];
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
                AQLString key	= pillarNames[i];

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

	// 2. Valuation Currency in JPY for XCCY1, and EUR for XCCY2. USDCSA for both swaps.
	// We expect a EUR and JPY total for each pillar point, along with the breakdowbn by swap leg.
	TEST_F( TestLWOSwapDeltaXCCY, RISK_DeltaLadder_ValuationCCY_JPY_EUR )
    {
		// 1. Load the Input Files

		// 5YR XCCY  JPY USD, USDCSA, valuationCCY=USD
		std::string xccySwap1 = createSwapFromDataFile( xccySwapInputs2_Swap1 );

		// 5YR XCCY  EUR USD, USDCSA, valuationCCY=USD
		std::string xccySwap2 = createSwapFromDataFile( xccySwapInputs2_Swap2 );

		const ReadDataFile::Load deltaLadder ( deltaLadderInputs1 );
		AQLStringVector swapNames					= deltaLadder[ "swapNames" ];
		AQLStringMatrix curveCollectionNames		= deltaLadder[ "curveCollectionNames" ];
		AQLStringMatrix fixingTableNames			= deltaLadder[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= deltaLadder[ "bumpSpreadInstruments" ];
		double bumpSize							= deltaLadder[ "bumpSize" ];
		AQLString bumpMode						= deltaLadder[ "bumpMode" ];
		bool aggregateRisks						= deltaLadder[ "aggregateRisks" ];
		bool reportInLegCCY						= deltaLadder[ "reportInLegCCY" ];
        const DoubleVector xccyFXSpotRates   	= deltaLadder[ "xccyFXSpotRates" ];
		std::string riskCutOffTenor             = "";

		AQLStringVector pillarNames;
		AQLStringVector headers;
		DoubleMatrix deltas;
		validation::tryMeLWOSwapDeltaLadder(headers,
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
            AQLString outputFileName = raw_delta_ladder_outputs2_32;
#else
            AQLString outputFileName = raw_delta_ladder_outputs2_64;
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
            const ReadDataFile::Load resultFile( delta_ladder_outputs2_32 );
#else
            const ReadDataFile::Load resultFile( delta_ladder_outputs2_64 );
#endif

			if ( ! resultFile.hasItem("headers") )
			{
				FAIL() << "Reference baseline does not contain delta column headers" << std::endl;
			}

			AQLStringVector refHeaders = resultFile["headers"];
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
                AQLString key	= pillarNames[i];

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

	// 3. Same as test 2, however report results in native leg CCY, not valuation CCY.
	// We expect a EUR, JPY and USD total for each pillar point, along with the breakdowbn by swap leg.
	TEST_F( TestLWOSwapDeltaXCCY, RISK_DeltaLadder_ValuationCCY_JPY_EUR_Report_LegCCY )
    {
		// 1. Load the Input Files

		// 5YR XCCY  JPY USD, USDCSA, valuationCCY=USD
		std::string xccySwap1 = createSwapFromDataFile( xccySwapInputs2_Swap1 );

		// 5YR XCCY  EUR USD, USDCSA, valuationCCY=USD
		std::string xccySwap2 = createSwapFromDataFile( xccySwapInputs2_Swap2 );

		const ReadDataFile::Load deltaLadder ( deltaLadderInputs1 );
		AQLStringVector swapNames					= deltaLadder[ "swapNames" ];
		AQLStringMatrix curveCollectionNames		= deltaLadder[ "curveCollectionNames" ];
		AQLStringMatrix fixingTableNames			= deltaLadder[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= deltaLadder[ "bumpSpreadInstruments" ];
		double bumpSize							= deltaLadder[ "bumpSize" ];
		AQLString bumpMode						= deltaLadder[ "bumpMode" ];
		bool aggregateRisks						= deltaLadder[ "aggregateRisks" ];
		bool reportInLegCCY						= true;
        const DoubleVector xccyFXSpotRates   	= deltaLadder[ "xccyFXSpotRates" ];
		std::string riskCutOffTenor             = "";

		AQLStringVector pillarNames;
		AQLStringVector headers;
		DoubleMatrix deltas;
		validation::tryMeLWOSwapDeltaLadder( headers,
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
            AQLString outputFileName = raw_delta_ladder_outputs3_32;
#else
            AQLString outputFileName = raw_delta_ladder_outputs3_64;
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
            const ReadDataFile::Load resultFile( delta_ladder_outputs3_32 );
#else
            const ReadDataFile::Load resultFile( delta_ladder_outputs3_64 );
#endif

			if ( ! resultFile.hasItem("headers") )
			{
				FAIL() << "Reference baseline does not contain delta column headers" << std::endl;
			}

			AQLStringVector refHeaders = resultFile["headers"];
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
                AQLString key	= pillarNames[i];

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



}
