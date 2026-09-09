// Curves
#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"

// Swap Creation and Pricing
#include "tryAqSwapObjectCreation.h"
#include "tryAqSwapObjectPricing.h"

// Test Infrastructure
#include "Dependency.h"   // IMPORTANT: Curve Macros are Here !!!
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"
#include <gTest/gTest.h>

#include "AQObjUtilities.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

// Define the Test Input Folder Here
#define TEST_DIR "ETrading/AQObjects/TestAQObjSwapZeroCouponSwapFixedFixed/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 4e-2;  // Notional of test trades is 10MM.

    //
    // curve input files
    //
    extern const char USDYC_OIS[]			    = TEST_DIR "USDYC_OIS_tryAqCurveCalibrateOIS_inputs.csv";
    extern const char USDYC_STD[]			    = "";
    extern const char USDYC_1M[]			    = "";
    extern const char USDYC_6M[]			    = "";
    extern const char USDYC_12M[]			    = "";

    //
    // test call input and reference files
    //
    extern const char zeroCouponSwapInputs[]	= TEST_DIR "FIXEDFIXEDZEROCOUPONSWAP@1_tryAqSwapObjectCreate_inputs";
    extern const char pvInputs[]	            = TEST_DIR "FIXEDFIXEDZEROCOUPONSWAP@1_tryAqSwapObjectPV_inputs";
    extern const char pvOutputs[]	            = TEST_DIR "FIXEDFIXEDZEROCOUPONSWAP@1_tryAqSwapObjectPV_outputs";
    extern const char pvOutputs64[]	            = TEST_DIR "FIXEDFIXEDZEROCOUPONSWAP@1_tryAqSwapObjectPV_outputs64_";

}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the AQL_BUILD_EURUSD_XCCY_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    AQL_BUILD_USD_CURVE(TestAQObjSwapZeroCouponSwapFixedFixed, USDYC_OIS, USDYC_1M, USDYC_STD, USDYC_6M, USDYC_12M);
    
    //
    // Call Test Fixture
    //

    TEST_F( TestAQObjSwapZeroCouponSwapFixedFixed, SNAPSHOT_SwapHardCodedPVCheck )
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString pvOutputFileName   = pvOutputs; 
                #else
                    AQLString pvOutputFileName   = pvOutputs64; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( zeroCouponSwapInputs );
                const ReadDataFile::Load pvInputFile( pvInputs );
                const ReadDataFile::Load pvOutputFile( pvOutputFileName );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqSwapObjectCreate( swapName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                AQLStringMatrix curveCollectionLVB = pvInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = pvInputFile.getOptional("fixingTableNames", AQLStringMatrix() );
                
                double actualPV = validation::tryAqSwapObjectPV( swapName, curveCollectionLVB, "", fixingTableLVB);
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualPV, TEST_DIR, pvOutputFileName, tolerance );
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



}
