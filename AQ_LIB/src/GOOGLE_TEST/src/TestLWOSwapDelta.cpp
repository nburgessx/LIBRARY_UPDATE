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
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapDelta/"

namespace
{
    // test tolerances
    // ---------------
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

    //
    // test call input and reference files
    //
	extern const char irsSwapGeneratorInputs[]		= TEST_DIR "EUR_6ML@199_tryMeLWOSwapGeneratorCreate_inputs.csv";
    extern const char irsSwapInputs[]				= TEST_DIR "EUR_6ML_tryMeLWOSwapCreateFromGenerator_inputs.csv";
	extern const char oisSwapInputs[]				= TEST_DIR "EUROIS6@63_tryMeLWOSwapCreateFromLegLVBs_inputs.csv";
	extern const char tenorBasisSwapInputs[]		= TEST_DIR "EUR3X6_2@112_tryMeLWOSwapCreateFromLegLVBs_inputs.csv";
	extern const char xccySwapInputs[]				= TEST_DIR "XCCY4@52_tryMeLWOSwapCreate_inputs.csv";

	extern const char fixingTableInputs[]			= TEST_DIR "FIXING@1_tryMeLWOFixingTableCreate_inputs.csv";

	extern const char raw_delta_outputs_ois_32[]	= "tryMeLWOSwapDelta_outputs_ois.csv";
	extern const char raw_delta_outputs_ois_64[]	= "tryMeLWOSwapDelta_outputs_ois_64bit.csv";
	extern const char delta_outputs_ois_32[]		= TEST_DIR "tryMeLWOSwapDelta_outputs_ois.csv";
	extern const char delta_outputs_ois_64[]		= TEST_DIR "tryMeLWOSwapDelta_outputs_ois_64bit.csv";

	extern const char raw_delta_outputs_irs_32[]	= "tryMeLWOSwapDelta_outputs_irs.csv";
	extern const char raw_delta_outputs_irs_64[]	= "tryMeLWOSwapDelta_outputs_irs_64bit.csv";
	extern const char delta_outputs_irs_32[]		= TEST_DIR "tryMeLWOSwapDelta_outputs_irs.csv";
	extern const char delta_outputs_irs_64[]		= TEST_DIR "tryMeLWOSwapDelta_outputs_irs_64bit.csv";

	extern const char raw_delta_outputs_tb_32[]		= "tryMeLWOSwapDelta_outputs_tb.csv";
	extern const char raw_delta_outputs_tb_64[]		= "tryMeLWOSwapDelta_outputs_tb_64bit.csv";
	extern const char delta_outputs_tb_32[]			= TEST_DIR "tryMeLWOSwapDelta_outputs_tb.csv";
	extern const char delta_outputs_tb_64[]			= TEST_DIR "tryMeLWOSwapDelta_outputs_tb_64bit.csv";

	extern const char raw_delta_outputs_xccy_32[]	= "tryMeLWOSwapDelta_outputs_xccy.csv";
	extern const char raw_delta_outputs_xccy_64[]	= "tryMeLWOSwapDelta_outputs_xccy_64bit.csv";
	extern const char delta_outputs_xccy_32[]		= TEST_DIR "tryMeLWOSwapDelta_outputs_xccy.csv";
	extern const char delta_outputs_xccy_64[]		= TEST_DIR "tryMeLWOSwapDelta_outputs_xccy_64bit.csv";

	extern const char raw_dv01_outputs_32[]			= "tryMeLWOSwapDV01_outputs.csv";
	extern const char raw_dv01_outputs_64[]			= "tryMeLWOSwapDV01_outputs_64bit.csv";
	extern const char dv01_outputs_32[]				= TEST_DIR "tryMeLWOSwapDV01_outputs.csv";
	extern const char dv01_outputs_64[]				= TEST_DIR "tryMeLWOSwapDV01_outputs_64bit.csv";

	extern const char raw_delta_ladder_outputs_32[]	= "tryMeLWOSwapDeltaLadder_outputs.csv";
	extern const char raw_delta_ladder_outputs_64[]	= "tryMeLWOSwapDeltaLadder_outputs_64bit.csv";
 
	extern const char delta_ladder_outputs_32[]		= TEST_DIR "tryMeLWOSwapDeltaLadder_outputs.csv";
	extern const char delta_ladder_outputs_64[]		= TEST_DIR "tryMeLWOSwapDeltaLadder_outputs_64bit.csv";

	extern const char flatShiftDeltaInputs_ois[]	= TEST_DIR "tryMeLWOSwapDelta_inputs_ois.csv";
	extern const char flatShiftDeltaInputs_irs[]	= TEST_DIR "tryMeLWOSwapDelta_inputs_irs.csv";
	extern const char flatShiftDeltaInputs_tb[]		= TEST_DIR "tryMeLWOSwapDelta_inputs_tb.csv";
	extern const char flatShiftDeltaInputs_xccy[]	= TEST_DIR "tryMeLWOSwapDelta_inputs_xccy.csv";
	extern const char swapDV01Inputs[]				= TEST_DIR "tryMeLWOSwapDV01_inputs.csv";
	extern const char deltaLadderInputs[]			= TEST_DIR "tryMeLWOSwapDeltaLadder_inputs.csv";

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

	std::string helperCreateSwapFromLegLVBs( const char* swapInputs )
	{
		const ReadDataFile::Load swapInputFile( swapInputs );
		std::string swapName		= swapInputFile[ "swapName" ];
		AQLStringMatrix leg1Lvb		= swapInputFile[ "leg1LVB" ];
		AQLStringMatrix leg2Lvb		= swapInputFile[ "leg2LVB" ];
		AQLStringMatrix swapProperties	= swapInputFile[ "swapPropertiesLVB" ];
		bool isXccySwap				= swapInputFile[ "isXccySwap" ];
		bool validateKeys			= swapInputFile[ "validateKeys" ];
		return validation::tryMeLWOSwapCreateFromLegLVBs( swapName, leg1Lvb, leg2Lvb, swapProperties, isXccySwap, validateKeys );
	};

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
    ME_BUILD_EURUSD_XCCY_CURVE( TestLWOSwapDelta, USDYC_OIS, USDYC_STD, EURYC_OIS, EURYC_STD, EURYC_3M6M, EURYC_XCCY );


	
	// This test checks the FlatShiftDelta for a 6Y OIS
    TEST_F( TestLWOSwapDelta, RISK_FlatShiftDeltaOIS )
    {
		// 1. Load the Input Files

		// 6YR OIS
		std::string oisSwap = helperCreateSwapFromLegLVBs( oisSwapInputs );
		
		const ReadDataFile::Load flatShiftDelta( flatShiftDeltaInputs_ois );
		AQLStringVector swapNames					= flatShiftDelta[ "swapNames" ];
		AQLStringMatrix curveCollectionNames		= flatShiftDelta[ "curveCollectionNames" ];
		AQLStringMatrix fixingTableNames			= flatShiftDelta[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= flatShiftDelta[ "bumpSpreadInstruments" ];
		double bumpSize							= flatShiftDelta[ "bumpSize" ];
		AQLString bumpMode						= flatShiftDelta[ "bumpMode" ];
		AQLString groupRiskBy					= flatShiftDelta[ "groupRiskBy" ];
		bool aggregateRisks						= flatShiftDelta[ "aggregateRisks" ];
		bool reportInLegCCY						= flatShiftDelta[ "reportInLegCCY" ];

        // Dummy Xccy FX Spot Rates
        DoubleVector dummyXccyFXSpotRates( swapNames.size(), 1.0 );

		AQLStringVector legNames;
		DoubleVector deltas;
		validation::tryMeLWOSwapDelta( legNames,
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
                                           dummyXccyFXSpotRates );

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
#ifdef GTEST32
            AQLString outputFileName = raw_delta_outputs_ois_32;
#else
            AQLString outputFileName = raw_delta_outputs_ois_64;
#endif

            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( outputFileName ) );
            for ( size_t i = 0; i < legNames.size(); ++i )
            {
                // 12 is the number of decimal points required
                file.write( legNames[i], deltas[i], 12 );
            }
        }
        else
        {
            // Carry out actual test and peform result comparison
#ifdef GTEST32
            const ReadDataFile::Load resultFile( delta_outputs_ois_32 );
#else
            const ReadDataFile::Load resultFile( delta_outputs_ois_64 );
#endif

			double flatShiftDeltaTolerance = 1.e-4; // 1MM notional
            for ( size_t i = 0; i < legNames.size(); ++i )
            {
                AQLString key	= legNames[i];

                double delta	= deltas[i];
                double ref		= resultFile[key];

				EXPECT_NEAR( delta, ref, flatShiftDeltaTolerance)
					<< " Delta : " << key.getCString() << " is incorrect ";

            }
        }
    }

	// This test checks the FlatShiftDelta for a 5Y swap
    TEST_F( TestLWOSwapDelta, RISK_FlatShiftDeltaIRS )
    {
		// 1. Load the Input Files

		// 5Y Interest Rate Swap
		std::string irsSwap = helperCreateSwapFromSwapGenerator( irsSwapGeneratorInputs, irsSwapInputs);
		
		// Fixing Tables
		const ReadDataFile::Load fixingTableFile( fixingTableInputs );
		std::string tableName	= fixingTableFile[ "tableName" ];
		std::string currency	= fixingTableFile[ "currency" ];
		std::string tenor		= fixingTableFile[ "curveTenor" ];
		std::vector< boost::gregorian::date > fixingDates = fixingTableFile[ "fixingDates" ];
		DoubleVector fixingValues = fixingTableFile[ "fixingValues" ];
		
		std::string fixingTableLeg1 = validation::tryMeLWOFixingTableCreate( tableName, currency, tenor, fixingDates, fixingValues );
		
		const ReadDataFile::Load flatShiftDelta( flatShiftDeltaInputs_irs );
		AQLStringVector swapNames					= flatShiftDelta[ "swapNames" ];
		AQLStringMatrix curveCollectionNames		= flatShiftDelta[ "curveCollectionNames" ];
		AQLStringMatrix fixingTableNames			= flatShiftDelta[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= flatShiftDelta[ "bumpSpreadInstruments" ];
		double bumpSize							= flatShiftDelta[ "bumpSize" ];
		AQLString bumpMode						= flatShiftDelta[ "bumpMode" ];
		AQLString groupRiskBy					= flatShiftDelta[ "groupRiskBy" ];
		bool aggregateRisks						= flatShiftDelta[ "aggregateRisks" ];
		bool reportInLegCCY						= flatShiftDelta[ "reportInLegCCY" ];
        
        // Dummy Xccy FX Spot Rates
        DoubleVector dummyXccyFXSpotRates( swapNames.size(), 1.0 );

		AQLStringVector legNames;
		DoubleVector deltas;
		validation::tryMeLWOSwapDelta( legNames,
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
                                           dummyXccyFXSpotRates );

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
#ifdef GTEST32
            AQLString outputFileName = raw_delta_outputs_irs_32;
#else
            AQLString outputFileName = raw_delta_outputs_irs_64;
#endif

            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( outputFileName ) );
            for ( size_t i = 0; i < legNames.size(); ++i )
            {
                // 12 is the number of decimal points required
                file.write( legNames[i], deltas[i], 12 );
            }
        }
        else
        {
            // Carry out actual test and peform result comparison
#ifdef GTEST32
            const ReadDataFile::Load resultFile( delta_outputs_irs_32 );
#else
            const ReadDataFile::Load resultFile( delta_outputs_irs_64 );
#endif

			// 1. The standard test setup calculates the risk for a single IRS by LEG
			EXPECT_EQ( "LEG", groupRiskBy );

			double sumDelta = 0.0;
            for ( size_t i = 0; i < legNames.size(); ++i )
            {
                AQLString key	= legNames[i];

                double delta	= deltas[i];
                double ref		= resultFile[key];
				sumDelta += delta;

				EXPECT_NEAR( delta, ref, tolerance )
					<< " Delta : " << key.getCString() << " is incorrect ";
            }

			// 2. Now recalculate the risk by SWAP
			DoubleVector deltaBySwap;
			AQLString groupRiskBySwap = "SWAP";
			validation::tryMeLWOSwapDelta( legNames,
											   deltaBySwap,
											   swapNames,
											   curveCollectionNames,
											   fixingTableNames,
											   bumpSpreadInstruments,
											   bumpSize,
											   bumpMode,
											   groupRiskBySwap,
											   aggregateRisks,
											   reportInLegCCY,
                                               dummyXccyFXSpotRates );

			// Expecting a single delta number
			EXPECT_EQ( 1, deltaBySwap.size() );
			EXPECT_NEAR( sumDelta, deltaBySwap[0], tolerance )
				<< "Delta aggregated by SWAP is incorrect ";

			// 3. Finally recalculate the TOTAL delta of the portfolio 
			// As this is a single trade, this will match the SWAP level delta
			DoubleVector deltaByTotal;
			AQLString groupRiskByTotal = "TOTAL";
			validation::tryMeLWOSwapDelta( legNames,
											   deltaByTotal,
											   swapNames,
											   curveCollectionNames,
											   fixingTableNames,
											   bumpSpreadInstruments,
											   bumpSize,
											   bumpMode,
											   groupRiskByTotal,
											   aggregateRisks,
											   reportInLegCCY,
                                               dummyXccyFXSpotRates );

			// Expecting a single delta number
			EXPECT_EQ( 1, deltaByTotal.size() );
			EXPECT_NEAR( sumDelta, deltaByTotal[0], tolerance )
				<< "Delta aggregated by TOTAL is incorrect ";

        }
    }

	// This test checks the FlatShiftDelta for a 2Y Tenor Basis
    TEST_F( TestLWOSwapDelta, RISK_FlatShiftDeltaTenorBasis )
    {
		// 1. Load the Input Files

		// 2Y Tenor Basis
		std::string tenorBasisSwap = helperCreateSwapFromLegLVBs( tenorBasisSwapInputs );
		
		const ReadDataFile::Load flatShiftDelta( flatShiftDeltaInputs_tb );
		AQLStringVector swapNames					= flatShiftDelta[ "swapNames" ];
		AQLStringMatrix curveCollectionNames		= flatShiftDelta[ "curveCollectionNames" ];
		AQLStringMatrix fixingTableNames			= flatShiftDelta[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= flatShiftDelta[ "bumpSpreadInstruments" ];
		double bumpSize							= flatShiftDelta[ "bumpSize" ];
		AQLString bumpMode						= flatShiftDelta[ "bumpMode" ];
		AQLString groupRiskBy					= flatShiftDelta[ "groupRiskBy" ];
		bool aggregateRisks						= flatShiftDelta[ "aggregateRisks" ];
		bool reportInLegCCY						= flatShiftDelta[ "reportInLegCCY" ];;

        // Dummy Xccy FX Spot Rates
        DoubleVector dummyXccyFXSpotRates( swapNames.size(), 1.0 );

		AQLStringVector legNames;
		DoubleVector deltas;
		validation::tryMeLWOSwapDelta( legNames,
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
                                           dummyXccyFXSpotRates );

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
#ifdef GTEST32
            AQLString outputFileName = raw_delta_outputs_tb_32;
#else
            AQLString outputFileName = raw_delta_outputs_tb_64;
#endif

            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( outputFileName ) );
            for ( size_t i = 0; i < legNames.size(); ++i )
            {
                // 12 is the number of decimal points required
                file.write( legNames[i], deltas[i], 12 );
            }
        }
        else
        {
            // Carry out actual test and peform result comparison
#ifdef GTEST32
            const ReadDataFile::Load resultFile( delta_outputs_tb_32 );
#else
            const ReadDataFile::Load resultFile( delta_outputs_tb_64 );
#endif

            for ( size_t i = 0; i < legNames.size(); ++i )
            {
                AQLString key	= legNames[i];

                double delta	= deltas[i];
                double ref		= resultFile[key];

				EXPECT_NEAR( delta, ref, tolerance )
					<< " Delta : " << key.getCString() << " is incorrect ";
            }
        }
    }

	// This test checks the FlatShiftDelta for a 4Y XCCY
    TEST_F( TestLWOSwapDelta, RISK_FlatShiftDeltaXCCY )
    {
		// 1. Load the Input Files
				
		// 4YR XCCY
		std::string xccySwap = createSwapFromDataFile( xccySwapInputs );
		
		const ReadDataFile::Load flatShiftDelta( flatShiftDeltaInputs_xccy );
		AQLStringVector swapNames					= flatShiftDelta[ "swapNames" ];
		AQLStringMatrix curveCollectionNames		= flatShiftDelta[ "curveCollectionNames" ];
		AQLStringMatrix fixingTableNames			= flatShiftDelta[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= flatShiftDelta[ "bumpSpreadInstruments" ];
		double bumpSize							= flatShiftDelta[ "bumpSize" ];
		AQLString bumpMode						= flatShiftDelta[ "bumpMode" ];
		AQLString groupRiskBy					= flatShiftDelta[ "groupRiskBy" ];
		bool aggregateRisks						= flatShiftDelta[ "aggregateRisks" ];
		bool reportInLegCCY						= flatShiftDelta[ "reportInLegCCY" ];
        DoubleVector xccyFXSpotRates            = flatShiftDelta[ "xccyFXSpotRates" ];

		AQLStringVector legNames;
		DoubleVector deltas;
		validation::tryMeLWOSwapDelta( legNames,
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
            AQLString outputFileName = raw_delta_outputs_xccy_32;
#else
            AQLString outputFileName = raw_delta_outputs_xccy_64;
#endif

            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( outputFileName ) );
            for ( size_t i = 0; i < legNames.size(); ++i )
            {
                // 12 is the number of decimal points required
                file.write( legNames[i], deltas[i], 12 );
            }
        }
        else
        {
            // Carry out actual test and peform result comparison
#ifdef GTEST32
            const ReadDataFile::Load resultFile( delta_outputs_xccy_32 );
#else
            const ReadDataFile::Load resultFile( delta_outputs_xccy_64 );
#endif

            for ( size_t i = 0; i < legNames.size(); ++i )
            {
                AQLString key	= legNames[i];

                double delta	= deltas[i];
                double ref		= resultFile[key];

				EXPECT_NEAR( delta, ref, tolerance )
					<< " Delta : " << key.getCString() << " is incorrect ";

            }
        }
    }

	// This test checks the FlatShiftDelta for a 4Y XCCY
	// It checks there is no curve interference by rebuilding the EUR TenorBasis curve out of sequence,
	// after all other curves are built. Types of interference seen in the past occurred
	// through dataValues such as isRenotionalAdjust, curveType.
	// Note: Same test inputs as for test FlatShiftDeltaXCCY
    TEST_F( TestLWOSwapDelta, RISK_FlatShiftDeltaXCCY_RebuildEURTenorBasis )
    {
		// 0. Rebuild the EUR TenorBasis
		TryMeCurveTenorBasis tenorBasis( EURYC_3M6M );

		// 1. Load the Input Files
				
		// 4YR XCCY
		std::string xccySwap = createSwapFromDataFile( xccySwapInputs );
		
		const ReadDataFile::Load flatShiftDelta( flatShiftDeltaInputs_xccy );
		AQLStringVector swapNames					= flatShiftDelta[ "swapNames" ];
		AQLStringMatrix curveCollectionNames		= flatShiftDelta[ "curveCollectionNames" ];
		AQLStringMatrix fixingTableNames			= flatShiftDelta[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= flatShiftDelta[ "bumpSpreadInstruments" ];
		double bumpSize							= flatShiftDelta[ "bumpSize" ];
		AQLString bumpMode						= flatShiftDelta[ "bumpMode" ];
		AQLString groupRiskBy					= flatShiftDelta[ "groupRiskBy" ];
		bool aggregateRisks						= flatShiftDelta[ "aggregateRisks" ];
		bool reportInLegCCY						= flatShiftDelta[ "reportInLegCCY" ];
        DoubleVector xccyFXSpotRates            = flatShiftDelta[ "xccyFXSpotRates" ];

		AQLStringVector legNames;
		DoubleVector deltas;
		validation::tryMeLWOSwapDelta( legNames,
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
            AQLString outputFileName = raw_delta_outputs_xccy_32;
#else
            AQLString outputFileName = raw_delta_outputs_xccy_64;
#endif

            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( outputFileName ) );
            for ( size_t i = 0; i < legNames.size(); ++i )
            {
                // 12 is the number of decimal points required
                file.write( legNames[i], deltas[i], 12 );
            }
        }
        else
        {
            // Carry out actual test and peform result comparison
#ifdef GTEST32
            const ReadDataFile::Load resultFile( delta_outputs_xccy_32 );
#else
            const ReadDataFile::Load resultFile( delta_outputs_xccy_64 );
#endif

            for ( size_t i = 0; i < legNames.size(); ++i )
            {
                AQLString key	= legNames[i];

                double delta	= deltas[i];
                double ref		= resultFile[key];

				EXPECT_NEAR( delta, ref, tolerance )
					<< " Delta : " << key.getCString() << " is incorrect ";

            }
        }
    }

	// This test checks the DV01 for a portfolio of swaps, and reports the DV01 at trade level
    TEST_F( TestLWOSwapDelta, RISK_swapDV01 )
    {
		// 1. Load the Input Files
				
		// 6YR OIS
		std::string oisSwap = helperCreateSwapFromLegLVBs( oisSwapInputs );
			
		// 5Y Interest Rate Swap
		std::string irsSwap = helperCreateSwapFromSwapGenerator( irsSwapGeneratorInputs, irsSwapInputs);
		
		// 2YR TenorBasis
		std::string tenorBasisSwap = helperCreateSwapFromLegLVBs( tenorBasisSwapInputs );

		// 4YR XCCY
		std::string xccySwap = createSwapFromDataFile( xccySwapInputs );
		
		const ReadDataFile::Load flatShiftDelta( swapDV01Inputs );
		AQLStringVector swapNames					= flatShiftDelta[ "swapNames" ];
		AQLStringMatrix curveCollectionNames		= flatShiftDelta[ "curveCollectionNames" ];
		AQLStringMatrix fixingTableNames			= flatShiftDelta[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= flatShiftDelta[ "bumpSpreadInstruments" ];
		double bumpSize							= flatShiftDelta[ "bumpSize" ];
		AQLString bumpMode						= flatShiftDelta[ "bumpMode" ];
		AQLString groupRiskBy					= flatShiftDelta[ "groupRiskBy" ];
		bool aggregateRisks						= flatShiftDelta[ "aggregateRisks" ];
		bool reportInLegCCY						= flatShiftDelta[ "reportInLegCCY" ];
        DoubleVector xccyFXSpotRates            = flatShiftDelta[ "xccyFXSpotRates" ];

		AQLStringVector positionIDs;
		DoubleVector deltas;
		// The DV01 is actually calculated by the tryMeLWOSwapDelta function
		validation::tryMeLWOSwapDelta( positionIDs,
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
            AQLString outputFileName = raw_dv01_outputs_32;
#else
            AQLString outputFileName = raw_dv01_outputs_64;
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
            const ReadDataFile::Load resultFile( dv01_outputs_32 );
#else
            const ReadDataFile::Load resultFile( dv01_outputs_64 );
#endif

            for ( size_t i = 0; i < positionIDs.size(); ++i )
            {
                AQLString key	= positionIDs[i];

                double delta	= deltas[i];
                double ref		= resultFile[key];

				EXPECT_NEAR( delta, ref, tolerance )
					<< " Delta : " << key.getCString() << " is incorrect ";

            }
        }
    }

	TEST_F( TestLWOSwapDelta, RISK_DeltaLadderPortfolio )
    {

		// 1. Load the Input Files

		// 6YR OIS
		std::string oisSwap = helperCreateSwapFromLegLVBs( oisSwapInputs );
			
		// 5Y Interest Rate Swap
		std::string irsSwap = helperCreateSwapFromSwapGenerator( irsSwapGeneratorInputs, irsSwapInputs);
		
		// 2YR TenorBasis
		std::string tenorBasisSwap = helperCreateSwapFromLegLVBs( tenorBasisSwapInputs );

		// 4YR XCCY
		std::string xccySwap = createSwapFromDataFile( xccySwapInputs );

		const ReadDataFile::Load deltaLadder ( deltaLadderInputs );
		AQLStringVector swapNames					= deltaLadder[ "swapNames" ];
		AQLStringMatrix curveCollectionNames		= deltaLadder[ "curveCollectionNames" ];
		AQLStringMatrix fixingTableNames			= deltaLadder[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= deltaLadder[ "bumpSpreadInstruments" ];
		double bumpSize							= deltaLadder[ "bumpSize" ];
		AQLString bumpMode						= deltaLadder[ "bumpMode" ];
		bool aggregateRisks						= deltaLadder[ "aggregateRisks" ];
		bool reportInLegCCY						= deltaLadder[ "reportInLegCCY" ];
		DoubleVector xccyFXSpotRates            = deltaLadder[ "xccyFXSpotRates" ];
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
            AQLString outputFileName = raw_delta_ladder_outputs_32;
#else
            AQLString outputFileName = raw_delta_ladder_outputs_64;
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
            const ReadDataFile::Load resultFile( delta_ladder_outputs_32 );
#else
            const ReadDataFile::Load resultFile( delta_ladder_outputs_64 );
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
