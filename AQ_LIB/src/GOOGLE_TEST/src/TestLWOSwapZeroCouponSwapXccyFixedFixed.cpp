// Curves
#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "TryMeCurveTenorBasis.h"
#include "TryMeCurveXccyBasis.h"

// Swap Creation and Pricing
#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"

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
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapZeroCouponSwapXccyFixedFixed/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1.5e-4; // Notional of test trade is 10MM

    //
    // curve input files
    //
    extern const char USDYC_OIS[]			    = TEST_DIR "USDYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    extern const char USDYC_STD[]			    = TEST_DIR "USDYC_STD_tryMeCurveCalibrateSwap_inputs.csv";
    extern const char EURYC_OIS[]			    = TEST_DIR "EURYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    extern const char EURYC_STD[]			    = TEST_DIR "EURYC_STD_tryMeCurveCalibrateSwap_inputs.csv";
    extern const char EURYC_3M6M[]			    = TEST_DIR "EURYC_3M6M_tryMeCurveCalibrateBasis_inputs.csv";
    extern const char EURYC_XCCY[]			    = TEST_DIR "EURYC_XCCY_tryMeCurveCalibrateBasis_inputs.csv";

    //
    // test call input and reference files
    //
    extern const char zeroCouponSwapInputs[]	= TEST_DIR "XCCYFIXEDFIXEDZEROCOUPONSWAP@1_tryMeLWOSwapCreate_inputs";
    extern const char pvInputs[]	        = TEST_DIR "XCCYFIXEDFIXEDZEROCOUPONSWAP@1_tryMeLWOSwapPV_inputs";
    extern const char pvOutputs[]	    = TEST_DIR "XCCYFIXEDFIXEDZEROCOUPONSWAP@1_tryMeLWOSwapPV_outputs";
    extern const char pvOutputs64[]	    = TEST_DIR "XCCYFIXEDFIXEDZEROCOUPONSWAP@1_tryMeLWOSwapPV_outputs64_";

}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the ME_BUILD_EURUSD_XCCY_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ME_BUILD_EURUSD_XCCY_CURVE( TestLWOSwapZeroCouponSwapXccyFixedFixed, USDYC_OIS, USDYC_STD, EURYC_OIS, EURYC_STD, EURYC_3M6M, EURYC_XCCY );
    
    //
    // Call Test Fixture
    //

    TEST_F( TestLWOSwapZeroCouponSwapXccyFixedFixed, SNAPSHOT_SwapHardCodedPVCheck )
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

                std::string createSwap          = validation::tryMeLWOSwapCreate( swapName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                AQLStringMatrix curveCollectionLVB = pvInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = pvInputFile.getOptional("fixingTableNames", AQLStringMatrix() );
                
                double actualPV = validation::tryMeLWOSwapPV( swapName, curveCollectionLVB, "", fixingTableLVB);
                
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
