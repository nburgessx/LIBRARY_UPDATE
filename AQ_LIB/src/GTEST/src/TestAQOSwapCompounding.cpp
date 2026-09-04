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
#define TEST_DIR "ETrading/AQObjects/TestAQOSwapCompounding/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1e-6;

    //
    // curve input files
    //
    extern const char USDYC_OIS[]			    = TEST_DIR "USDYC_OIS_tryAqCurvesCalibrateOIS_inputs.csv";
    extern const char USDYC_STD[]			    = TEST_DIR "USDYC_STD_tryAqCurvesCalibrateSwap_inputs.csv";
    extern const char USDYC_1M[]			    = "";
    extern const char USDYC_6M[]			    = TEST_DIR "USDYC_3M6M_tryAqCurvesCalibrateBasis_inputs.csv";
    extern const char USDYC_12M[]			    = "";

    //
    // test call input and reference files
    //

    // 1) Float float swap with 'AT_MATURITY' as payment freq enum (i.e. ZeroCouponSwap), and compound method as 'FLAT, 'NORMAL', and 'SIMPLE'

    extern const char swapInputs_AtMaturity_Flat[]	            = TEST_DIR "FLOATFLOATSWAP_ATMATURITY_FLAT@1_tryAqObjectsSwapCreate_inputs";
    extern const char parSpreadInputs_AtMaturity_Flat[]	        = TEST_DIR "FLOATFLOATSWAP_ATMATURITY_FLAT@1_tryAqObjectsSwapParSpread_inputs";
    extern const char parSpreadOutputs_AtMaturity_Flat[]	    = TEST_DIR "FLOATFLOATSWAP_ATMATURITY_FLAT@1_tryAqObjectsSwapParSpread_outputs";
    extern const char parSpreadOutputs64_AtMaturity_Flat[]	    = TEST_DIR "FLOATFLOATSWAP_ATMATURITY_FLAT@1_tryAqObjectsSwapParSpread_outputs64_";

    extern const char swapInputs_AtMaturity_Normal[]	            = TEST_DIR "FLOATFLOATSWAP_ATMATURITY_NORMAL@1_tryAqObjectsSwapCreate_inputs";
    extern const char parSpreadInputs_AtMaturity_Normal[]	        = TEST_DIR "FLOATFLOATSWAP_ATMATURITY_NORMAL@1_tryAqObjectsSwapParSpread_inputs";
    extern const char parSpreadOutputs_AtMaturity_Normal[]	        = TEST_DIR "FLOATFLOATSWAP_ATMATURITY_NORMAL@1_tryAqObjectsSwapParSpread_outputs";
    extern const char parSpreadOutputs64_AtMaturity_Normal[]	    = TEST_DIR "FLOATFLOATSWAP_ATMATURITY_NORMAL@1_tryAqObjectsSwapParSpread_outputs64_";

    extern const char swapInputs_AtMaturity_Simple[]	            = TEST_DIR "FLOATFLOATSWAP_ATMATURITY_SIMPLE@1_tryAqObjectsSwapCreate_inputs";
    extern const char parSpreadInputs_AtMaturity_Simple[]	        = TEST_DIR "FLOATFLOATSWAP_ATMATURITY_SIMPLE@1_tryAqObjectsSwapParSpread_inputs";
    extern const char parSpreadOutputs_AtMaturity_Simple[]	        = TEST_DIR "FLOATFLOATSWAP_ATMATURITY_SIMPLE@1_tryAqObjectsSwapParSpread_outputs";
    extern const char parSpreadOutputs64_AtMaturity_Simple[]	    = TEST_DIR "FLOATFLOATSWAP_ATMATURITY_SIMPLE@1_tryAqObjectsSwapParSpread_outputs64_";

    // 2) Float float swap with semiannual as payment freq enum(i.e. TenorBasisSwap), and compound method as 'FLAT, 'NORMAL', and 'SIMPLE'

    extern const char swapInputs_SemiAnnual_Flat[]	            = TEST_DIR "FLOATFLOATSWAP_SEMIANNUAL_FLAT@1_tryAqObjectsSwapCreate_inputs";
    extern const char parSpreadInputs_SemiAnnual_Flat[]	        = TEST_DIR "FLOATFLOATSWAP_SEMIANNUAL_FLAT@1_tryAqObjectsSwapParSpread_inputs";
    extern const char parSpreadOutputs_SemiAnnual_Flat[]	    = TEST_DIR "FLOATFLOATSWAP_SEMIANNUAL_FLAT@1_tryAqObjectsSwapParSpread_outputs";
    extern const char parSpreadOutputs64_SemiAnnual_Flat[]	    = TEST_DIR "FLOATFLOATSWAP_SEMIANNUAL_FLAT@1_tryAqObjectsSwapParSpread_outputs64_";

    extern const char swapInputs_SemiAnnual_Normal[]	            = TEST_DIR "FLOATFLOATSWAP_SEMIANNUAL_NORMAL@1_tryAqObjectsSwapCreate_inputs";
    extern const char parSpreadInputs_SemiAnnual_Normal[]	        = TEST_DIR "FLOATFLOATSWAP_SEMIANNUAL_NORMAL@1_tryAqObjectsSwapParSpread_inputs";
    extern const char parSpreadOutputs_SemiAnnual_Normal[]	        = TEST_DIR "FLOATFLOATSWAP_SEMIANNUAL_NORMAL@1_tryAqObjectsSwapParSpread_outputs";
    extern const char parSpreadOutputs64_SemiAnnual_Normal[]	    = TEST_DIR "FLOATFLOATSWAP_SEMIANNUAL_NORMAL@1_tryAqObjectsSwapParSpread_outputs64_";

    extern const char swapInputs_SemiAnnual_Simple[]	            = TEST_DIR "FLOATFLOATSWAP_SEMIANNUAL_SIMPLE@1_tryAqObjectsSwapCreate_inputs";
    extern const char parSpreadInputs_SemiAnnual_Simple[]	        = TEST_DIR "FLOATFLOATSWAP_SEMIANNUAL_SIMPLE@1_tryAqObjectsSwapParSpread_inputs";
    extern const char parSpreadOutputs_SemiAnnual_Simple[]	        = TEST_DIR "FLOATFLOATSWAP_SEMIANNUAL_SIMPLE@1_tryAqObjectsSwapParSpread_outputs";
    extern const char parSpreadOutputs64_SemiAnnual_Simple[]	    = TEST_DIR "FLOATFLOATSWAP_SEMIANNUAL_SIMPLE@1_tryAqObjectsSwapParSpread_outputs64_";


}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the AQL_BUILD_USD_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    AQL_BUILD_USD_CURVE( TestAQOSwapCompounding, USDYC_OIS, USDYC_1M, USDYC_STD, USDYC_6M, USDYC_12M);
    
    //
    // Call Test Fixture
    //

    TEST_F( TestAQOSwapCompounding, SNAPSHOT_HardCodedParSpreadCheck_AtMaturityFlat)
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs_AtMaturity_Flat; 
                #else
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs64_AtMaturity_Flat; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs_AtMaturity_Flat );
                const ReadDataFile::Load parRateInputFile( parSpreadInputs_AtMaturity_Flat );
                const ReadDataFile::Load parRateOutputFile( parSpreadOutputsFilename );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = parRateInputFile["swapName"];
                AQLString curveCollection        = parRateInputFile["curveCollections"];
                std::string fixingTable         = parRateInputFile["fixingTableNames"];
                
                double actualSwapParSpread          = validation::tryAqObjectsSwapParSpread( swapName, etrading::fromStringToLVB(curveCollection.getCString()), etrading::fromStringToLVB(fixingTable) );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapParSpread, TEST_DIR, parSpreadOutputsFilename, tolerance );
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

   TEST_F( TestAQOSwapCompounding, SNAPSHOT_HardCodedParSpreadCheck_AtMaturityNormal)
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs_AtMaturity_Normal; 
                #else
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs64_AtMaturity_Normal; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs_AtMaturity_Normal);
                const ReadDataFile::Load parRateInputFile( parSpreadInputs_AtMaturity_Normal );
                const ReadDataFile::Load parRateOutputFile( parSpreadOutputsFilename );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = parRateInputFile["swapName"];
                AQLString curveCollection        = parRateInputFile["curveCollections"];
                std::string fixingTable         = parRateInputFile["fixingTableNames"];
                
                double actualSwapParSpread          = validation::tryAqObjectsSwapParSpread( swapName, etrading::fromStringToLVB(curveCollection.getCString()), etrading::fromStringToLVB(fixingTable) );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapParSpread, TEST_DIR, parSpreadOutputsFilename, tolerance );
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


    TEST_F( TestAQOSwapCompounding, SNAPSHOT_HardCodedParSpreadCheck_AtMaturitySimple)
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs_AtMaturity_Simple; 
                #else
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs64_AtMaturity_Simple; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs_AtMaturity_Simple );
                const ReadDataFile::Load parRateInputFile( parSpreadInputs_AtMaturity_Simple );
                const ReadDataFile::Load parRateOutputFile( parSpreadOutputsFilename );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = parRateInputFile["swapName"];
                AQLString curveCollection        = parRateInputFile["curveCollections"];
                std::string fixingTable         = parRateInputFile["fixingTableNames"];
                
                double actualSwapParSpread          = validation::tryAqObjectsSwapParSpread( swapName, etrading::fromStringToLVB(curveCollection.getCString()), etrading::fromStringToLVB(fixingTable) );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapParSpread, TEST_DIR, parSpreadOutputsFilename, tolerance );
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

        TEST_F( TestAQOSwapCompounding, SNAPSHOT_HardCodedParSpreadCheck_SemiAnnualFlat)
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs_SemiAnnual_Flat; 
                #else
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs64_SemiAnnual_Flat; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs_SemiAnnual_Flat );
                const ReadDataFile::Load parRateInputFile( parSpreadInputs_SemiAnnual_Flat );
                const ReadDataFile::Load parRateOutputFile( parSpreadOutputsFilename );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = parRateInputFile["swapName"];
                AQLString curveCollection        = parRateInputFile["curveCollections"];
                std::string fixingTable         = parRateInputFile["fixingTableNames"];
                
                double actualSwapParSpread          = validation::tryAqObjectsSwapParSpread( swapName, etrading::fromStringToLVB(curveCollection.getCString()), etrading::fromStringToLVB(fixingTable) );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapParSpread, TEST_DIR, parSpreadOutputsFilename, tolerance );
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

   TEST_F( TestAQOSwapCompounding, SNAPSHOT_HardCodedParSpreadCheck_SemiAnnualNormal)
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs_SemiAnnual_Normal; 
                #else
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs64_SemiAnnual_Normal; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs_SemiAnnual_Normal);
                const ReadDataFile::Load parRateInputFile( parSpreadInputs_SemiAnnual_Normal );
                const ReadDataFile::Load parRateOutputFile( parSpreadOutputsFilename );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = parRateInputFile["swapName"];
                AQLString curveCollection        = parRateInputFile["curveCollections"];
                std::string fixingTable         = parRateInputFile["fixingTableNames"];
                
                double actualSwapParSpread          = validation::tryAqObjectsSwapParSpread( swapName, etrading::fromStringToLVB(curveCollection.getCString()), etrading::fromStringToLVB(fixingTable) );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapParSpread, TEST_DIR, parSpreadOutputsFilename, tolerance );
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


    TEST_F( TestAQOSwapCompounding, SNAPSHOT_HardCodedParSpreadCheck_SemiAnnualSimple)
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs_SemiAnnual_Simple; 
                #else
                    AQLString parSpreadOutputsFilename   = parSpreadOutputs64_SemiAnnual_Simple; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs_SemiAnnual_Simple );
                const ReadDataFile::Load parRateInputFile( parSpreadInputs_SemiAnnual_Simple );
                const ReadDataFile::Load parRateOutputFile( parSpreadOutputsFilename );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = parRateInputFile["swapName"];
                AQLString curveCollection        = parRateInputFile["curveCollections"];
                std::string fixingTable         = parRateInputFile["fixingTableNames"];
                
                double actualSwapParSpread          = validation::tryAqObjectsSwapParSpread( swapName, etrading::fromStringToLVB(curveCollection.getCString()), etrading::fromStringToLVB(fixingTable) );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapParSpread, TEST_DIR, parSpreadOutputsFilename, tolerance );
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
