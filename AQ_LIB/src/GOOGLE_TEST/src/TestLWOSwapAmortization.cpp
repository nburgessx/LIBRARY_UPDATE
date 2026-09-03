// Curves
#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "TryMeCurveTenorBasis.h"

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
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapAmortization/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1e-8;

    //
    // curve input files
    //
    extern const char EURYC_OIS[]			    = TEST_DIR "EURYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char EURYC_1M[]			    = "";
    extern const char EURYC_3M[]			    = "";
    extern const char EURYC_6M[]			    = TEST_DIR "EURYC_STD_tryMeCurveCalibrateSwap_inputs";
    extern const char EURYC_12M[]			    = "";



    //
    // test call input and reference files
    //
    extern const char swapInputs[]	= TEST_DIR "EURSWAP@6_tryMeLWOSwapCreate_inputs";

    extern const char pvInputs[]	        = TEST_DIR "EURSWAP@6_tryMeLWOSwapPV_inputs";
    extern const char pvOutputs[]	    = TEST_DIR "EURSWAP@6_tryMeLWOSwapPV_outputs";
    extern const char pvOutputs64[]	    = TEST_DIR "EURSWAP@6_tryMeLWOSwapPV_outputs64_";

    extern const char parRateInputs[]	        = TEST_DIR "EURSWAP@6_tryMeLWOSwapParRate_inputs";
    extern const char parRateOutputs[]	    = TEST_DIR "EURSWAP@6_tryMeLWOSwapParRate_outputs";
    extern const char parRateOutputs64[]	    = TEST_DIR "EURSWAP@6_tryMeLWOSwapParRate_outputs_outputs64_";

    extern const char pv01Inputs[]	        = TEST_DIR "EURSWAP@6_tryMeLWOSwapPV01_inputs";
    extern const char pv01Outputs[]	    = TEST_DIR "EURSWAP@6_tryMeLWOSwapPV01_outputs";
    extern const char pv01Outputs64[]	    = TEST_DIR "EURSWAP@6_tryMeLWOSwapPV01_outputs_outputs64_";

}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    //
    ME_BUILD_EUR_CURVE( TestLWOSwapAmortization, EURYC_OIS, EURYC_1M, EURYC_3M, EURYC_6M, EURYC_12M);
    
    //
    // Call Test Fixture
    //

    TEST_F( TestLWOSwapAmortization, SNAPSHOT_SwapHardCodedPVCheck )
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    LAString pvOutputsFilename   = pvOutputs; 
                #else
                    LAString pvOutputsFilename   = pvOutputs64; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs );
                const ReadDataFile::Load pvInputFile( pvInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                LAStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                LAStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation_api::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParRate Inputs & Calculate the parRate
                std::string swapName            = pvInputFile["swapName"];
                LAStringMatrix curveCollectionLVB = pvInputFile["curveCollections"];
                LAStringMatrix fixingTableLVB     = pvInputFile.getOptional("fixingTableNames", LAStringMatrix() );

           		auto swap = etrading::getSwap(swapName);

                double actualPV = validation_api::tryMeLWOSwapPV( swapName, curveCollectionLVB, "", fixingTableLVB);
                
                // 5. Check the Test Results
				const double tolerancePV = 1e-5;	// Notional of test trade is 1MM
                CheckTestResultsAndRebaseOnRequest( actualPV, TEST_DIR, pvOutputsFilename, tolerancePV );
        }
        catch( const LACoreError& m )
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

    TEST_F( TestLWOSwapAmortization, SNAPSHOT_SwapHardCodedParRateCheck )
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    LAString parRateOutputsFilename   = parRateOutputs; 
                #else
                    LAString parRateOutputsFilename   = parRateOutputs64; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs );
                const ReadDataFile::Load parRateInputFile( parRateInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                LAStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                LAStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation_api::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParRate Inputs & Calculate the parRate
                std::string swapName            = parRateInputFile["swapName"];
                LAStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                LAStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", LAStringMatrix() );

           		auto swap = etrading::getSwap(swapName);

                double actualParRate = validation_api::tryMeLWOSwapParRate( swapName, curveCollectionLVB, fixingTableLVB);
                
                // 5. Check the Test Results
                CheckTestResultsAndRebaseOnRequest( actualParRate, TEST_DIR, parRateOutputsFilename, tolerance );
        }
        catch( const LACoreError& m )
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

    TEST_F( TestLWOSwapAmortization, SNAPSHOT_SwapHardCodedPV01Check )
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    LAString pv01OutputsFilename   = pv01Outputs; 
                #else
                    LAString pv01OutputsFilename   = pv01Outputs64; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs );
                const ReadDataFile::Load pv01InputFile( pv01Inputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                LAStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                LAStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation_api::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParRate Inputs & Calculate the parRate
                std::string swapName            = pv01InputFile["swapName"];
                LAStringMatrix curveCollectionLVB = pv01InputFile["curveCollections"];
                LAStringMatrix fixingTableLVB     = pv01InputFile.getOptional("fixingTableNames", LAStringMatrix() );

           		auto swap = etrading::getSwap(swapName);

                double actualPV01 = validation_api::tryMeLWOSwapPV01( swapName, curveCollectionLVB, fixingTableLVB);
                
                // 5. Check the Test Results
                CheckTestResultsAndRebaseOnRequest( actualPV01, TEST_DIR, pv01OutputsFilename, tolerance );
        }
        catch( const LACoreError& m )
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
