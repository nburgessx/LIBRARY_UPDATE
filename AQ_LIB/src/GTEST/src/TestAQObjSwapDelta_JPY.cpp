#include <string>
#include <sstream>

// Curves
#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"
#include "TryAqCurvesXccyBasis.h"
#include "TryAqCurvesFwdFxConst.h"

// Swap Creation and Pricing
#include "tryAqSwapObjectCreation.h"
#include "tryAqSwapObjectPricing.h"

// Risk calculation
#include "tryAqSwapObjectDelta.h"
#include "tryAqRateFixingTable.h"

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
#define TEST_DIR "ETrading/AQObjects/TestAQObjSwapDelta_JPY_LCH6M/"


namespace
{    
	// test tolerance
    const double tolerance = 1.e-2;   // Notional of test trades is 1MM and delta by bumping is inherently noisy

	//
    // curve input files
    //
	const char JPYYC_OIS[]					= TEST_DIR "JPYYC_OIS_tryAqCurveCalibrateOIS_inputs.csv";
    const char JPYYC_JSCC_STD[]				= TEST_DIR "JPYYC_STD_tryAqCurveCalibrateSwap_inputs.csv";
    const char JPYYC_LCH_6M[]				= TEST_DIR "JPYYC_LCH6M_tryAqCurveCalibrateSwap_inputs.csv";
	//
    // test call input and reference files
    //
	int portfolioSize = 18;

	const char swapInputFileSuffix[]		= "_JPY_6ML_LCH_tryAqSwapObjectCreateFromGenerator_inputs.csv";   

	// DV01 tests
	const char swapDV01Inputs[]				= TEST_DIR "tryAqObjSwapsDV01_inputs";  // set up the DV01 calculation

	// Reference DV01 outputs base filename. A currency csa suffix is appended by the test in order to get the actual filename
	const char raw_dv01_outputs_32[]			= "tryAqObjSwapsDV01_outputs.csv";
	const char raw_dv01_outputs_64[]			= "tryAqObjSwapsDV01_outputs_64bit.csv";
	const char dv01_outputs_32[]				= TEST_DIR "tryAqObjSwapsDV01_outputs.csv";
	const char dv01_outputs_64[]				= TEST_DIR "tryAqObjSwapsDV01_outputs_64bit.csv";
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestAQObjSwapDelta_JPY);

	/* @brief	Run consistency test on all available swap curves in all ccys
    */
	void buildCurves_JPY()
	{
		// Set up minimal JPY curves required for LCH6M
		setUpAqOISCurve( JPYYC_OIS );
		setUpAqSTDCurve( JPYYC_JSCC_STD );
		setUpAqSTDCurve( JPYYC_LCH_6M );
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
		AQLStringMatrix expressionLVB		= swapInputFile[ "expressionLVB" ];
		AQLStringMatrix swapPropertiesLVB	= swapInputFile[ "swapPropertiesLVB" ];
		bool isXccySwap					= swapInputFile[ "isXccySwap" ];
		bool validateKeys				= swapInputFile[ "validateKeys" ];
		return validation::tryAqSwapObjectCreateFromGenerator( swapName, swapGeneratorName, expressionLVB, swapPropertiesLVB, isXccySwap, validateKeys );
	}

	void calcDV01AndCompareToReference()
	{
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

        // Dummy Xccy FX Spot Rates
        DoubleVector dummyXccyFXSpotRates( swapNames.size(), 1.0 );

		AQLStringVector positionIDs;
		DoubleVector deltas;
		// The DV01 is actually calculated by the tryAqSwapObjectDelta function
		validation::tryAqSwapObjectDelta( positionIDs,
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
			AQLString refFileName = dv01_outputs_32;
            const ReadDataFile::Load resultFile( refFileName );
#else
			AQLString refFileName = dv01_outputs_64;
            const ReadDataFile::Load resultFile( refFileName );
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

	// DV01 tests
	TEST_F( TestAQObjSwapDelta_JPY, SNAPSHOT_swapDV01_LCH6M)
    {
        buildCurves_JPY();

		// Deliberately build the curves twice:
		// We attempt to provoke a bug that previously existed in AQLUpdateCurveObject::setUpGenerateConfig() when setting  isSwapTenorAdjust:
		// ( bug fixed in revision 8069 ).
		// If the curves are built a 2nd time such that this curve data is reset, the code in AQLUpdateCurveObject would incorrectly set the flag to true
		// ( by casting an AQLDataBool object pointer to bool ).
		// This test is to make sure that we detect this type of coding error in future.
		buildCurves_JPY();

		for (int i=1; i <= portfolioSize; i++)
		{
			helperCreateSwapFromSwapGenerator( i, swapInputFileSuffix );
		}

		calcDV01AndCompareToReference();
	}

}
