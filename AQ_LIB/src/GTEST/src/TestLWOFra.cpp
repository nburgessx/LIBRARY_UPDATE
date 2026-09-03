// Curves
#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "TryMeCurveTenorBasis.h"

// Leg Creation and Pricing
#include "tryMeLWOLeg.h"

// Test Infrastructure
#include "Dependency.h"   // IMPORTANT: Curve Macros are Here !!!
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"
#include <gTest/gTest.h>

#include "LWOUtilities.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

// Define the Test Input Folder Here
#define TEST_DIR "ETrading/LWObjects/TestLWOFra/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1e-4;

    //
    // curve input files
    //
    extern const char USDYC_OIS[]			    = TEST_DIR "USDYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char USDYC_1M[]			    = "";
    extern const char USDYC_STD[]			    = TEST_DIR "USDYC_STD_tryMeCurveCalibrateSwap_inputs";
    extern const char USDYC_6M[]			    = TEST_DIR "USDYC_3M6M_tryMeCurveCalibrateBasis_inputs";
    extern const char USDYC_12M[]			    = "";

    //
    // test call input and reference files
    //
    extern const char fraInput1[]	= TEST_DIR "FRA_USD@1_tryMeLWOLegCreate_inputs";
	extern const char pvInputs1[] = TEST_DIR "FRA_USD@1_tryMeLWOLegPV_inputs";
	extern const char pvOutputs1[] = TEST_DIR "FRA_USD@1_tryMeLWOLegPV_outputs";
	extern const char pvOutputs1_64[] = TEST_DIR "FRA_USD@1_tryMeLWOLegPV_outputs64_";


	extern const char fraInput2[] = TEST_DIR "FRA_USD_STUB@2_tryMeLWOLegCreate_inputs";
	extern const char pvInputs2[] = TEST_DIR "FRA_USD_STUB@2_tryMeLWOLegPV_inputs";
	extern const char pvOutputs2[] = TEST_DIR "FRA_USD_STUB@2_tryMeLWOLegPV_outputs";
	extern const char pvOutputs2_64[] = TEST_DIR "FRA_USD_STUB@2_tryMeLWOLegPV_outputs64_";

}

namespace google_test
{

	//
	// Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
	//
	// All curves name MUST be defined for the AQL_BUILD_EURUSD_XCCY_CURVE macro to work.
	// If any curve is not in use and not defined, simply assign a "" to the curve name.
	//
	AQL_BUILD_USD_CURVE(TestLWOFra, USDYC_OIS, USDYC_1M, USDYC_STD, USDYC_6M, USDYC_12M);

    //
    // Call Test Fixture
    //

	//Testing a FRA without Stub 
	TEST_F(TestLWOFra, SNAPSHOT_CheckFraPV)
	{
		try
		{
			// 1. Create the Input File Names 
			#if defined(GTEST32)
				AQLString pvOutputsFilename = pvOutputs1;
			#else
				AQLString pvOutputsFilename = pvOutputs1_64;
			#endif

			// 2. Load the Input Files
			const ReadDataFile::Load tradeInputFile(fraInput1);
			const ReadDataFile::Load pvInputFile(pvInputs1);
			const ReadDataFile::Load pvOutputFile(pvOutputsFilename);

			// Get the Trade Inputs & Create the Swap
			std::string legObjectName = tradeInputFile["legObjectName"];
			AQLStringMatrix legLVB = tradeInputFile["legLVB"];
			bool validateKeys = tradeInputFile["validateKeys"];

			std::string createSwap = validation::tryMeLWOLegCreate(legObjectName, legLVB, validateKeys);

			// 4. Get the ParRate Inputs & Calculate the parRate
			etrading::LabelValueBlock curveCollection = etrading::fromStringToLVB(pvInputFile["curveCollection"]);

			double actualPV = validation::tryMeLWOLegPV(legObjectName, curveCollection, "");

			// 5. Check the Test Results or Rebase
			CheckTestResultsAndRebaseOnRequest(actualPV, TEST_DIR, pvOutputsFilename, tolerance);

		}
		catch (const AQLCoreError& m)
		{
			std::cout << m.getMsg();
			ASSERT_FALSE(true);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
			ASSERT_FALSE(true);
		}
	}

	//Testing a FRA with Stub 
	TEST_F(TestLWOFra, SNAPSHOT_CheckStubFraPV_withFirstStubCurveIndex)
	{
		try
		{
			// 1. Create the Input File Names 
			#if defined(GTEST32)
				AQLString pvOutputsFilename = pvOutputs2;
			#else
				AQLString pvOutputsFilename = pvOutputs2_64;
			#endif

			// 2. Load the Input Files
			const ReadDataFile::Load tradeInputFile(fraInput2);
			const ReadDataFile::Load pvInputFile(pvInputs2);
			const ReadDataFile::Load pvOutputFile(pvOutputsFilename);

			// Get the Trade Inputs & Create the Swap
			std::string legObjectName = tradeInputFile["legObjectName"];
			AQLStringMatrix legLVB = tradeInputFile["legLVB"];
			bool validateKeys = tradeInputFile["validateKeys"];

			std::string createSwap = validation::tryMeLWOLegCreate(legObjectName, legLVB, validateKeys);

			// 4. Get the ParRate Inputs & Calculate the parRate
			etrading::LabelValueBlock curveCollection = etrading::fromStringToLVB(pvInputFile["curveCollection"]);

			double actualPV = validation::tryMeLWOLegPV(legObjectName, curveCollection, "");

			// 5. Check the Test Results or Rebase
			CheckTestResultsAndRebaseOnRequest(actualPV, TEST_DIR, pvOutputsFilename, tolerance);

		}
		catch (const AQLCoreError& m)
		{
			std::cout << m.getMsg();
			ASSERT_FALSE(true);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
			ASSERT_FALSE(true);
		}
	}

}
