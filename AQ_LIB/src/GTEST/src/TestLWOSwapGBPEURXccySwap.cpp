// TestLWOSwapGBPEURXccySwap.cpp

// Curves
#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"
#include "TryAqCurvesXccyBasis.h"
#include "TryAqCurvesFwdFxConst.h"

// Swap Creation and Pricing
#include "tryAqObjectsSwapCreation.h"
#include "tryAqObjectsSwapPricing.h"

// Test Infrastructure
#include "Dependency.h"   // IMPORTANT: Curve Macros are Here !!!
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"
#include <gTest/gTest.h>

using etrading::ReadDataFile;
using etrading::CreateDataFile;

// Define the Test Input Folder Here
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapXccySwapsGBPEUR/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double priceTolerance = 1e-2;
    const double basisSpreadTolerance = 1e-8;
    const int minTests = 6;

    //
    // curve input files
    //
    
    extern const char USDYC_OIS[]			    = TEST_DIR "USDYC_OIS_tryAqCurvesCalibrateOIS_inputs.csv";
    extern const char USDYC_STD[]			    = TEST_DIR "USDYC_STD_tryAqCurvesCalibrateSwap_inputs.csv";
    extern const char EURYC_OIS[]			    = TEST_DIR "EURYC_OIS_tryAqCurvesCalibrateOIS_inputs.csv";
    extern const char EURYC_STD[]			    = TEST_DIR "EURYC_STD_tryAqCurvesCalibrateSwap_inputs.csv";
    extern const char EURYC_3M6M[]			    = TEST_DIR "EURYC_3M6M_tryAqCurvesCalibrateBasis_inputs.csv";
    extern const char EURYC_XCCY[]			    = TEST_DIR "EURYC_XCCY_tryAqCurvesCalibrateBasis_inputs.csv";
    extern const char GBPYC_OIS[]			    = TEST_DIR "GBPYC_OIS_tryAqCurvesCalibrateOIS_inputs.csv";
    extern const char GBPYC_STD[]			    = TEST_DIR "GBPYC_STD_tryAqCurvesCalibrateSwap_inputs.csv";
    extern const char GBPYC_3M6M[]			    = TEST_DIR "GBPYC_3M6M_tryAqCurvesCalibrateBasis_inputs.csv";
    extern const char GBPYC_XCCY[]			    = TEST_DIR "GBPYC_XCCY_tryAqCurvesCalibrateBasis_inputs.csv";
    extern const char GBPYC_FWDEUR[]		    = TEST_DIR "GBPYC_FWDEUR_tryAqCurvesCalibrateFXForwards_inputs.csv";

    //
    // test call input and reference files
    //
    
    extern const char xccySwapInputs[]	        = TEST_DIR "XCCY3@2_tryAqObjectsSwapCreate_inputs";
    extern const char parSpreadInputs[]	        = TEST_DIR "XCCY3@2_tryAqObjectsSwapParSpread_inputs";
    extern const char parSpreadOutputs[]	    = TEST_DIR "XCCY3@2_tryAqObjectsSwapParSpread_outputs";
    extern const char parSpreadOutputs64[]	    = TEST_DIR "XCCY3@2_tryAqObjectsSwapParSpread_outputs64_";


}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the AQL_BUILD_EURUSD_XCCY_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    AQL_BUILD_GBPEUR_FWD_CURVE( TestLWOSwapGBPEURXccySwap, USDYC_OIS, USDYC_STD, EURYC_OIS, EURYC_STD, EURYC_3M6M, EURYC_XCCY, GBPYC_OIS, GBPYC_STD, GBPYC_3M6M, GBPYC_XCCY, GBPYC_FWDEUR  );

    //
    // Call Test Fixture
    //
  
    TEST_F( TestLWOSwapGBPEURXccySwap, SNAPSHOT_HardCodedParSpreadCheck )
    {
        try
        {
          
                // 1. Create the Input File Names and Append the TestCase Index + ".csv"
                
                #if defined(GTEST32)
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs;
                #else
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs64;
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( xccySwapInputs );
                const ReadDataFile::Load parSpreadInputFile( parSpreadInputs );
                const ReadDataFile::Load parSpreadOutputFile( parSpreadOutputsFilename );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the Par Spread Inputs & the Basis Spreads
                std::string swapName            = parSpreadInputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = parSpreadInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = parSpreadInputFile.getOptional("fixingTableNames", AQLStringMatrix() );
                auto spreadLegName              = parSpreadInputFile["spreadLegName"];
                
                double actualBasisSpread        = validation::tryAqObjectsSwapParSpread( swapName, curveCollectionLVB, fixingTableLVB, spreadLegName );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualBasisSpread, TEST_DIR, parSpreadOutputsFilename, basisSpreadTolerance );
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
