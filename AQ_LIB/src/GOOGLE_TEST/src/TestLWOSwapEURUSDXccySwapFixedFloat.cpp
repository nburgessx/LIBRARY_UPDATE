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
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapXccySwapsEURUSDFixedFloat/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1e-5;  // Spread is reported in BPs

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
    extern const char fixedLegSpreadSwapInputs[]				= TEST_DIR "FIXEDLEGSPREAD@15_tryMeLWOSwapCreate_inputs";
    extern const char fixedLegSpreadSwapParSpreadInputs[]	    = TEST_DIR "FIXEDLEGSPREAD@15_tryMeLWOSwapParSpread_inputs";
    extern const char fixedLegSpreadSwapParSpreadOutputs[]	    = TEST_DIR "FIXEDLEGSPREAD@15_tryMeLWOSwapParSpread_outputs";
    extern const char fixedLegSpreadSwapParSpreadOutputs64[]	= TEST_DIR "FIXEDLEGSPREAD@15_tryMeLWOSwapParSpread_outputs64_";
    extern const char fixedLegSpreadSwapSpreadInputs[]	        = TEST_DIR "FIXEDLEGSPREAD@15_tryMeLWOSwapSpread_inputs";
    extern const char fixedLegSpreadSwapSpreadOutputs[]			= TEST_DIR "FIXEDLEGSPREAD@15_tryMeLWOSwapSpread_outputs";
    extern const char fixedLegSpreadSwapSpreadOutputs64[]	    = TEST_DIR "FIXEDLEGSPREAD@15_tryMeLWOSwapSpread_outputs64_";

    extern const char floatLegSpreadSwapInputs[]				= TEST_DIR "FLOATLEGSPREAD@15_tryMeLWOSwapCreate_inputs";
    extern const char floatLegSpreadSwapParSpreadInputs[]	    = TEST_DIR "FLOATLEGSPREAD@15_tryMeLWOSwapParSpread_inputs";
    extern const char floatLegSpreadSwapParSpreadOutputs[]	    = TEST_DIR "FLOATLEGSPREAD@15_tryMeLWOSwapParSpread_outputs";
    extern const char floatLegSpreadSwapParSpreadOutputs64[]	= TEST_DIR "FLOATLEGSPREAD@15_tryMeLWOSwapParSpread_outputs64_";
    extern const char floatLegSpreadSwapSpreadInputs[]	        = TEST_DIR "FLOATLEGSPREAD@15_tryMeLWOSwapSpread_inputs";
    extern const char floatLegSpreadSwapSpreadOutputs[]			= TEST_DIR "FLOATLEGSPREAD@15_tryMeLWOSwapSpread_outputs";
    extern const char floatLegSpreadSwapSpreadOutputs64[]	    = TEST_DIR "FLOATLEGSPREAD@15_tryMeLWOSwapSpread_outputs64_";

}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the ME_BUILD_EURUSD_XCCY_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ME_BUILD_EURUSD_XCCY_CURVE( TestLWOSwapEURUSDXccySwapFixedFloat, USDYC_OIS, USDYC_STD, EURYC_OIS, EURYC_STD, EURYC_3M6M, EURYC_XCCY );


    TEST_F( TestLWOSwapEURUSDXccySwapFixedFloat, SNAPSHOT_HardCodedParSpreadCheck_FixedLegSpread )
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString outputsFilename   = fixedLegSpreadSwapParSpreadOutputs; 
                #else
                    AQLString outputsFilename   = fixedLegSpreadSwapParSpreadOutputs64; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( fixedLegSpreadSwapInputs );
                const ReadDataFile::Load inputFile( fixedLegSpreadSwapParSpreadInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = inputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = inputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = inputFile.getOptional("fixingTableNames", AQLStringMatrix() );
                std::string spreadLegName       = inputFile["spreadLegName"];
                
                double actualSwapParSpread          = validation::tryMeLWOSwapParSpread( swapName, curveCollectionLVB, fixingTableLVB, spreadLegName.c_str() );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapParSpread, TEST_DIR, outputsFilename, tolerance );
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


   TEST_F( TestLWOSwapEURUSDXccySwapFixedFloat, SNAPSHOT_HardCodedSpreadCheck_FixedLegSpread )
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString outputsFilename   = fixedLegSpreadSwapSpreadOutputs; 
                #else
                    AQLString outputsFilename   = fixedLegSpreadSwapSpreadOutputs64; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( fixedLegSpreadSwapInputs );
                const ReadDataFile::Load inputFile( fixedLegSpreadSwapSpreadInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = inputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = inputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = inputFile.getOptional("fixingTableNames", AQLStringMatrix() );
                std::string spreadLegName       = inputFile["spreadLegName"];
                
                double actualSwapSpread          = validation::tryMeLWOSwapSpread( swapName, curveCollectionLVB, fixingTableLVB, spreadLegName.c_str() );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapSpread, TEST_DIR, outputsFilename, tolerance );
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

    TEST_F( TestLWOSwapEURUSDXccySwapFixedFloat, SNAPSHOT_HardCodedParSpreadCheck_FloatLegSpread )
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString outputsFilename   = floatLegSpreadSwapParSpreadOutputs; 
                #else
                    AQLString outputsFilename   = floatLegSpreadSwapParSpreadOutputs64; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( floatLegSpreadSwapInputs );
                const ReadDataFile::Load inputFile( floatLegSpreadSwapParSpreadInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = inputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = inputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = inputFile.getOptional("fixingTableNames", AQLStringMatrix() );
                std::string spreadLegName       = inputFile["spreadLegName"];
                
                double actualSwapParSpread          = validation::tryMeLWOSwapParSpread( swapName, curveCollectionLVB, fixingTableLVB, spreadLegName.c_str() );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapParSpread, TEST_DIR, outputsFilename, tolerance );
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


   TEST_F( TestLWOSwapEURUSDXccySwapFixedFloat, SNAPSHOT_HardCodedSpreadCheck_FloatLegSpread )
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString outputsFilename   = floatLegSpreadSwapSpreadOutputs; 
                #else
                    AQLString outputsFilename   = floatLegSpreadSwapSpreadOutputs64; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( floatLegSpreadSwapInputs );
                const ReadDataFile::Load inputFile( floatLegSpreadSwapSpreadInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = inputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = inputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = inputFile.getOptional("fixingTableNames", AQLStringMatrix() );
                std::string spreadLegName       = inputFile["spreadLegName"];
                
                double actualSwapSpread          = validation::tryMeLWOSwapSpread( swapName, curveCollectionLVB, fixingTableLVB, spreadLegName.c_str() );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapSpread, TEST_DIR, outputsFilename, tolerance );
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
