// TestAQObjSwapEURUSDXccySwap.cpp

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

using etrading::ReadDataFile;
using etrading::CreateDataFile;

// Define the Test Input Folder Here
#define TEST_DIR "ETrading/AQObjects/TestAQObjSwapXccySwaps/"

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
    
    extern const char USDYC_OIS[]			    = TEST_DIR "USDYC_OIS_tryAqCurveCalibrateOIS_inputs.csv";
    extern const char USDYC_STD[]			    = TEST_DIR "USDYC_STD_tryAqCurveCalibrateSwap_inputs.csv";
    extern const char EURYC_OIS[]			    = TEST_DIR "EURYC_OIS_tryAqCurveCalibrateOIS_inputs.csv";
    extern const char EURYC_STD[]			    = TEST_DIR "EURYC_STD_tryAqCurveCalibrateSwap_inputs.csv";
    extern const char EURYC_3M6M[]			    = TEST_DIR "EURYC_3M6M_tryAqCurveCalibrateBasis_inputs.csv";
    extern const char EURYC_XCCY[]			    = TEST_DIR "EURYC_XCCY_tryAqCurveCalibrateBasis_inputs.csv";

    //
    // test call input and reference files
    //
    
    extern const char xccySwapInputs[]	        = TEST_DIR "XCCY1@182_tryAqSwapObjectCreate_inputs";
    extern const char priceInputs[]		        = TEST_DIR "XCCY1@182_tryAqSwapObjectPV_inputs";
    extern const char priceOutputs[]		    = TEST_DIR "XCCY1@182_tryAqSwapObjectPV_outputs";
    extern const char priceOutputs64[]		    = TEST_DIR "XCCY1@182_tryAqSwapObjectPV_outputs64_";
    extern const char parSpreadInputs[]	        = TEST_DIR "XCCY1@182_tryAqSwapObjectParSpread_inputs";
    extern const char parSpreadOutputs[]	    = TEST_DIR "XCCY1@182_tryAqSwapObjectParSpread_outputs";
    extern const char parSpreadOutputs64[]	    = TEST_DIR "XCCY1@182_tryAqSwapObjectParSpread_outputs64_";


}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the AQL_BUILD_EURUSD_XCCY_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    AQL_BUILD_EURUSD_XCCY_CURVE( TestAQObjSwapEURUSDXccySwap, USDYC_OIS, USDYC_STD, EURYC_OIS, EURYC_STD, EURYC_3M6M, EURYC_XCCY );

    //
    // Call Test Fixture
    //

    TEST_F( TestAQObjSwapEURUSDXccySwap, SNAPSHOT_HardCodedPVCheck )
    {
        int i = 0;
        try
        {
            for ( i = 0; ; ++i )
            {
                // 1. Create the Input File Names and Append the TestCase Index + ".csv"
                AQLString xccyInputsFilename     = CreateDataFile::makeFilename( xccySwapInputs, i );
                AQLString priceInputsFilename    = CreateDataFile::makeFilename( priceInputs, i );
                
#if defined(GTEST32)
                AQLString priceOutputsFilename   = CreateDataFile::makeFilename( priceOutputs, i );
#else
                AQLString priceOutputsFilename   = CreateDataFile::makeFilename( priceOutputs64, i );
#endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( xccyInputsFilename );
                const ReadDataFile::Load priceInputFile( priceInputsFilename );
                const ReadDataFile::Load priceOutputFile( priceOutputsFilename );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryAqSwapObjectCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the Price Inputs & Price the Swap
                std::string swapName            = priceInputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = priceInputFile["curveCollections"];
                AQLString legName                = priceInputFile.getOptional("legName", AQLString() );
                AQLStringMatrix fixingTableLVB     = priceInputFile.getOptional("fixingTableNames", AQLStringMatrix() );
                
                double actualSwapPrice          = validation::tryAqSwapObjectPV( swapName, curveCollectionLVB, legName, fixingTableLVB );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapPrice, TEST_DIR, priceOutputsFilename, priceTolerance );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GT( i, minTests );
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

    TEST_F( TestAQObjSwapEURUSDXccySwap, SNAPSHOT_HardCodedParSpreadCheck )
    {
        int i = 0;
        try
        {
            for ( i = 0; ; ++i )
            {
                // 1. Create the Input File Names and Append the TestCase Index + ".csv"
                AQLString xccyInputsFilename     = CreateDataFile::makeFilename( xccySwapInputs, i );
                AQLString parSpreadInputsFilename    = CreateDataFile::makeFilename( parSpreadInputs, i );
                
#if defined(GTEST32)
                AQLString parSpreadOutputsFilename   = CreateDataFile::makeFilename( parSpreadOutputs, i );
#else
                AQLString parSpreadOutputsFilename   = CreateDataFile::makeFilename( parSpreadOutputs64, i );
#endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( xccyInputsFilename );
                const ReadDataFile::Load priceInputFile( parSpreadInputsFilename );
                const ReadDataFile::Load priceOutputFile( parSpreadOutputsFilename );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryAqSwapObjectCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the Par Spread Inputs & the Basis Spreads
                std::string swapName            = priceInputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = priceInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = priceInputFile.getOptional("fixingTableNames", AQLStringMatrix() );
                
                double actualBasisSpread        = validation::tryAqSwapObjectParSpread( swapName, curveCollectionLVB, fixingTableLVB );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualBasisSpread, TEST_DIR, parSpreadOutputsFilename, basisSpreadTolerance );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GT( i, minTests );
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
