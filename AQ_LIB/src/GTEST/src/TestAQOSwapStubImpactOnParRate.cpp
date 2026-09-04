// Curves
#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"

// Swap Creation and Pricing
#include "tryAqObjectsSwapCreation.h"
#include "tryAqObjectsSwapPricing.h"

// Test Infrastructure
#include "Dependency.h"   // IMPORTANT: Curve Macros are Here !!!
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"
#include <gTest/gTest.h>

#include "AQOUtilities.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

// Define the Test Input Folder Here
#define TEST_DIR "ETrading/AQObjects/TestAQOSwapStubImpactOnParRate/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1e-8;

    //
    // curve input files
    //
    extern const char JPYYC_OIS[]			    = TEST_DIR "JPYYC_OIS.csv";
    extern const char JPYYC_6M[]			    = TEST_DIR "JPYYC_6M.csv";
    extern const char JPYYC_1M[]			    = "";
    extern const char JPYYC_3M[]			    = TEST_DIR "JPYYC_3M.csv";
	extern const char JPYYC_12M[]			    = "";

    //
    // test call input and reference files
    //

	const unsigned int IMM_SWAPS_COUNT	 = 19;
	const unsigned int STUB_SWAPS_COUNT  = 7;
	const unsigned int LIBOR_SWAPS_COUNT = 10;

    extern const char swapInputs_IMM_swaps[]		= TEST_DIR "swapInputs_IMM_swaps_";
    extern const char swapInputs_Stub_swaps[]	    = TEST_DIR "swapInputs_stub_swaps_";
    extern const char swapInputs_Libor_swaps[]	    = TEST_DIR "swapInputs_Libor_swaps_";

	extern const char parRateOutputsFilename_IMMSwaps_32bit[]			= TEST_DIR "parRateOutputsFilename_IMMSwaps_32bit_";
	extern const char parRateOutputsFilename_IMMSwaps_64bit[]			= TEST_DIR "parRateOutputsFilename_IMMSwaps_64bit_";
	
	extern const char parRateOutputsFilename_StubSwaps_32bit[]		= TEST_DIR "parRateOutputsFilename_StubSwaps_32bit_";
	extern const char parRateOutputsFilename_StubSwaps_64bit[]		= TEST_DIR "parRateOutputsFilename_StubSwaps_64bit_";

	extern const char parRateOutputsFilename_LiborSwaps_32bit[]		= TEST_DIR "parRateOutputsFilename_LiborSwaps_32bit_";
	extern const char parRateOutputsFilename_LiborSwaps_64bit[]		= TEST_DIR "parRateOutputsFilename_LiborSwaps_64bit_";
}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the AQL_BUILD_USD_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    AQL_BUILD_JPY_CURVE( TestAQOSwapStubImpactOnParRate, JPYYC_OIS, JPYYC_1M, JPYYC_3M, JPYYC_6M, JPYYC_12M);
    
    //
    // Call Test Fixture
    //

	void runTest(const AQLString& testType)
	{
		AQLString input, output32bit, output64bit;
		unsigned int testCount;
		if (testType == "IMM_SWAPS")
		{
			testCount = IMM_SWAPS_COUNT;
			input = swapInputs_IMM_swaps;
			output32bit = parRateOutputsFilename_IMMSwaps_32bit;
			output64bit = parRateOutputsFilename_IMMSwaps_64bit;
		}
		else if (testType == "STUB_SWAPS")
		{
			testCount = STUB_SWAPS_COUNT;
			input = swapInputs_Stub_swaps;
			output32bit = parRateOutputsFilename_StubSwaps_32bit;
			output64bit = parRateOutputsFilename_StubSwaps_64bit;
		}
		else if (testType == "LIBOR_SWAPS")
		{
			testCount = LIBOR_SWAPS_COUNT;
			input = swapInputs_Libor_swaps;
			output32bit = parRateOutputsFilename_LiborSwaps_32bit;
			output64bit = parRateOutputsFilename_LiborSwaps_64bit;
		}
		else
		{
			AQ_THROW("Unsupported test type");
		}

		try
        {
			for (size_t i = 0; i < testCount; ++i)
			{
                // Load the Input Files
                const ReadDataFile::Load tradeInputFile( input + AQLString(static_cast<int>(i)) + AQLString(".csv") );
        
                // Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string swapObj = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // Calculate the par rate
                double swapParRate = validation::tryAqObjectsSwapParRate( swapTradeName, etrading::fromStringToLVB("JPYYC") );
                
                // Check the Test Results or Rebase
				#if defined(GTEST32)
                    AQLString parRateOutputsFilename   = output32bit + AQLString(static_cast<int>(i)) + AQLString(".csv"); 
                #else
                    AQLString parRateOutputsFilename   = output64bit + AQLString(static_cast<int>(i)) + AQLString(".csv");
                #endif

                CheckTestResultsAndRebaseOnRequest( swapParRate, TEST_DIR, parRateOutputsFilename, tolerance );
			}
        }
        catch( const AQLCoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
	}

    TEST_F( TestAQOSwapStubImpactOnParRate, SNAPSHOT_IMM_Swaps)
    {
        runTest("IMM_SWAPS");
    }

	TEST_F( TestAQOSwapStubImpactOnParRate, SNAPSHOT_Stub_Swaps)
    {
        runTest("STUB_SWAPS");
    }

	TEST_F( TestAQOSwapStubImpactOnParRate, SNAPSHOT_Libor_Swaps)
    {
        runTest("LIBOR_SWAPS");
    }
  
}
