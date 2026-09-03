#include <string>
#include <sstream>

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
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapDelta_JPY_LCH6M/"


namespace
{    
	// test tolerance
    const double tolerance = 1.e-2;   // Notional of test trades is 1MM and delta by bumping is inherently noisy

	//
    // curve input files
    //
	const char JPYYC_OIS[]					= TEST_DIR "JPYYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    const char JPYYC_JSCC_STD[]				= TEST_DIR "JPYYC_STD_tryMeCurveCalibrateSwap_inputs.csv";
    const char JPYYC_LCH_6M[]				= TEST_DIR "JPYYC_LCH6M_tryMeCurveCalibrateSwap_inputs.csv";
	//
    // test call input and reference files
    //
	int portfolioSize = 18;

	const char swapInputFileSuffix[]		= "_JPY_6ML_LCH_tryMeLWOSwapCreateFromGenerator_inputs.csv";   

	// DV01 tests
	const char swapDV01Inputs[]				= TEST_DIR "tryMeLWOSwapDV01_inputs";  // set up the DV01 calculation

	// Reference DV01 outputs base filename. A currency csa suffix is appended by the test in order to get the actual filename
	const char raw_dv01_outputs_32[]			= "tryMeLWOSwapDV01_outputs.csv";
	const char raw_dv01_outputs_64[]			= "tryMeLWOSwapDV01_outputs_64bit.csv";
	const char dv01_outputs_32[]				= TEST_DIR "tryMeLWOSwapDV01_outputs.csv";
	const char dv01_outputs_64[]				= TEST_DIR "tryMeLWOSwapDV01_outputs_64bit.csv";
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestLWOSwapDelta_JPY);

	/* @brief	Run consistency test on all available swap curves in all ccys
    */
	void buildCurves_JPY()
	{
		// Set up minimal JPY curves required for LCH6M
		setUpMeOISCurve( JPYYC_OIS );
		setUpMeSTDCurve( JPYYC_JSCC_STD );
		setUpMeSTDCurve( JPYYC_LCH_6M );
	}


	std::string helperCreateSwapFromSwapGenerator(int swapNumber, const char* swapInputFileSuffix)
	{
		// Swap Generator
		std::ostringstream oss;
		oss << TEST_DIR;
		oss << swapNumber;
		oss << swapInputFileSuffix;
		std::string swapInputsName = oss.str();

		// Swap
		const ReadDataFile::Load swapInputFile( swapInputsName.c_str() );
		std::string swapName			= swapInputFile[ "swapName" ];
		std::string swapGeneratorName	= swapInputFile[ "swapGeneratorName" ];
		LAStringMatrix expressionLVB		= swapInputFile[ "expressionLVB" ];
		LAStringMatrix swapPropertiesLVB	= swapInputFile[ "swapPropertiesLVB" ];
		bool isXccySwap					= swapInputFile[ "isXccySwap" ];
		bool validateKeys				= swapInputFile[ "validateKeys" ];
		return validation::tryMeLWOSwapCreateFromGenerator( swapName, swapGeneratorName, expressionLVB, swapPropertiesLVB, isXccySwap, validateKeys );
	}

	void calcDV01AndCompareToReference()
	{
		const ReadDataFile::Load flatShiftDelta( swapDV01Inputs );
		LAStringVector swapNames					= flatShiftDelta[ "swapNames" ];
		LAStringMatrix curveCollectionNames		= flatShiftDelta[ "curveCollectionNames" ];
		LAStringMatrix fixingTableNames			= flatShiftDelta[ "fixingTableNames" ];
		bool bumpSpreadInstruments				= flatShiftDelta[ "bumpSpreadInstruments" ];
		double bumpSize							= flatShiftDelta[ "bumpSize" ];
		LAString bumpMode						= flatShiftDelta[ "bumpMode" ];
		LAString groupRiskBy					= flatShiftDelta[ "groupRiskBy" ];
		bool aggregateRisks						= flatShiftDelta[ "aggregateRisks" ];
		bool reportInLegCCY						= flatShiftDelta[ "reportInLegCCY" ];

        // Dummy Xccy FX Spot Rates
        DoubleVector dummyXccyFXSpotRates( swapNames.size(), 1.0 );

		LAStringVector positionIDs;
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
                                           dummyXccyFXSpotRates );

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
#ifdef GTEST32
            LAString outputFileName = raw_dv01_outputs_32;
#else
            LAString outputFileName = raw_dv01_outputs_64;
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
			LAString refFileName = dv01_outputs_32;
            const ReadDataFile::Load resultFile( refFileName );
#else
			LAString refFileName = dv01_outputs_64;
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

	// DV01 tests
	TEST_F( TestLWOSwapDelta_JPY, SNAPSHOT_swapDV01_LCH6M)
    {
        buildCurves_JPY();

		// Deliberately build the curves twice:
		// We attempt to provoke a bug that previously existed in LAUpdateCurveObject::setUpGenerateConfig() when setting  isSwapTenorAdjust:
		// ( bug fixed in revision 8069 ).
		// If the curves are built a 2nd time such that this curve data is reset, the code in LAUpdateCurveObject would incorrectly set the flag to true
		// ( by casting an LADataBool object pointer to bool ).
		// This test is to make sure that we detect this type of coding error in future.
		buildCurves_JPY();

		for (int i=1; i <= portfolioSize; i++)
		{
			helperCreateSwapFromSwapGenerator( i, swapInputFileSuffix );
		}

		calcDV01AndCompareToReference();
	}

}
