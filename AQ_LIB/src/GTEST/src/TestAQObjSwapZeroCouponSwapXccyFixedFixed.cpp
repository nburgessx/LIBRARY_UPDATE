// Curves
#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"
#include "TryAqCurvesXccyBasis.h"

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
#define TEST_DIR "ETrading/AQObjects/TestAQObjSwapZeroCouponSwapXccyFixedFixed/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1.5e-4; // Notional of test trade is 10MM

    //
    // curve input files
    //
    extern const char USDYC_OIS[]			    = TEST_DIR "USDYC_OIS_tryAqCurveCalibrateOIS_inputs.csv";
    extern const char USDYC_STD[]			    = TEST_DIR "USDYC_STD_tryAqCurveCalibrateSwap_inputs.csv";
    extern const char EURYC_OIS[]			    = TEST_DIR "EURYC_OIS_tryAqCurveCalibrateOIS_inputs.csv";
    extern const char EURYC_STD[]			    = TEST_DIR "EURYC_STD_tryAqCurveCalibrateSwap_inputs.csv";
    extern const char EURYC_3M6M[]			    = TEST_DIR "EURYC_3M6M_tryAqCurveCalibrateBasis_inputs.csv";
    extern const char EURYC_XCCY[]			    = TEST_DIR "EURYC_XCCY_tryAqCurveCalibrateBasis_inputs.csv";

    //
    // test call input and reference files
    //
    extern const char zeroCouponSwapInputs[]	= TEST_DIR "XCCYFIXEDFIXEDZEROCOUPONSWAP@1_tryAqSwapObjectCreate_inputs";
    extern const char pvInputs[]	        = TEST_DIR "XCCYFIXEDFIXEDZEROCOUPONSWAP@1_tryAqSwapObjectPV_inputs";
    extern const char pvOutputs[]	    = TEST_DIR "XCCYFIXEDFIXEDZEROCOUPONSWAP@1_tryAqSwapObjectPV_outputs";
    extern const char pvOutputs64[]	    = TEST_DIR "XCCYFIXEDFIXEDZEROCOUPONSWAP@1_tryAqSwapObjectPV_outputs64_";

}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the AQL_BUILD_EURUSD_XCCY_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    AQL_BUILD_EURUSD_XCCY_CURVE( TestAQObjSwapZeroCouponSwapXccyFixedFixed, USDYC_OIS, USDYC_STD, EURYC_OIS, EURYC_STD, EURYC_3M6M, EURYC_XCCY );
    
    //
    // Call Test Fixture
    //

    TEST_F( TestAQObjSwapZeroCouponSwapXccyFixedFixed, SNAPSHOT_SwapHardCodedPVCheck )
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
