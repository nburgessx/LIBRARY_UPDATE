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
#define TEST_DIR "ETrading/AQObjects/TestAQOSwapAmortization/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1e-8;

    //
    // curve input files
    //
    extern const char EURYC_OIS[]			    = TEST_DIR "EURYC_OIS_tryAqCurvesCalibrateOIS_inputs";
    extern const char EURYC_1M[]			    = "";
    extern const char EURYC_3M[]			    = "";
    extern const char EURYC_6M[]			    = TEST_DIR "EURYC_STD_tryAqCurvesCalibrateSwap_inputs";
    extern const char EURYC_12M[]			    = "";



    //
    // test call input and reference files
    //
    extern const char swapInputs[]	= TEST_DIR "EURSWAP@6_tryAqObjectsSwapCreate_inputs";

    extern const char pvInputs[]	        = TEST_DIR "EURSWAP@6_tryAqObjectsSwapPV_inputs";
    extern const char pvOutputs[]	    = TEST_DIR "EURSWAP@6_tryAqObjectsSwapPV_outputs";
    extern const char pvOutputs64[]	    = TEST_DIR "EURSWAP@6_tryAqObjectsSwapPV_outputs64_";

    extern const char parRateInputs[]	        = TEST_DIR "EURSWAP@6_tryAqObjectsSwapParRate_inputs";
    extern const char parRateOutputs[]	    = TEST_DIR "EURSWAP@6_tryAqObjectsSwapParRate_outputs";
    extern const char parRateOutputs64[]	    = TEST_DIR "EURSWAP@6_tryAqObjectsSwapParRate_outputs_outputs64_";

    extern const char pv01Inputs[]	        = TEST_DIR "EURSWAP@6_tryAqObjectsSwapPV01_inputs";
    extern const char pv01Outputs[]	    = TEST_DIR "EURSWAP@6_tryAqObjectsSwapPV01_outputs";
    extern const char pv01Outputs64[]	    = TEST_DIR "EURSWAP@6_tryAqObjectsSwapPV01_outputs_outputs64_";

}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    //
    AQL_BUILD_EUR_CURVE( TestAQOSwapAmortization, EURYC_OIS, EURYC_1M, EURYC_3M, EURYC_6M, EURYC_12M);
    
    //
    // Call Test Fixture
    //

    TEST_F( TestAQOSwapAmortization, SNAPSHOT_SwapHardCodedPVCheck )
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString pvOutputsFilename   = pvOutputs; 
                #else
                    AQLString pvOutputsFilename   = pvOutputs64; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs );
                const ReadDataFile::Load pvInputFile( pvInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParRate Inputs & Calculate the parRate
                std::string swapName            = pvInputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = pvInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = pvInputFile.getOptional("fixingTableNames", AQLStringMatrix() );

           		auto swap = etrading::getSwap(swapName);

                double actualPV = validation::tryAqObjectsSwapPV( swapName, curveCollectionLVB, "", fixingTableLVB);
                
                // 5. Check the Test Results
				const double tolerancePV = 1e-5;	// Notional of test trade is 1MM
                CheckTestResultsAndRebaseOnRequest( actualPV, TEST_DIR, pvOutputsFilename, tolerancePV );
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

    TEST_F( TestAQOSwapAmortization, SNAPSHOT_SwapHardCodedParRateCheck )
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString parRateOutputsFilename   = parRateOutputs; 
                #else
                    AQLString parRateOutputsFilename   = parRateOutputs64; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs );
                const ReadDataFile::Load parRateInputFile( parRateInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParRate Inputs & Calculate the parRate
                std::string swapName            = parRateInputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", AQLStringMatrix() );

           		auto swap = etrading::getSwap(swapName);

                double actualParRate = validation::tryAqObjectsSwapParRate( swapName, curveCollectionLVB, fixingTableLVB);
                
                // 5. Check the Test Results
                CheckTestResultsAndRebaseOnRequest( actualParRate, TEST_DIR, parRateOutputsFilename, tolerance );
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

    TEST_F( TestAQOSwapAmortization, SNAPSHOT_SwapHardCodedPV01Check )
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString pv01OutputsFilename   = pv01Outputs; 
                #else
                    AQLString pv01OutputsFilename   = pv01Outputs64; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs );
                const ReadDataFile::Load pv01InputFile( pv01Inputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParRate Inputs & Calculate the parRate
                std::string swapName            = pv01InputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = pv01InputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = pv01InputFile.getOptional("fixingTableNames", AQLStringMatrix() );

           		auto swap = etrading::getSwap(swapName);

                double actualPV01 = validation::tryAqObjectsSwapPV01( swapName, curveCollectionLVB, fixingTableLVB);
                
                // 5. Check the Test Results
                CheckTestResultsAndRebaseOnRequest( actualPV01, TEST_DIR, pv01OutputsFilename, tolerance );
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
