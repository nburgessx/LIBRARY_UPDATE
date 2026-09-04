// Curves
#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"
#include "TryAqCurvesXccyBasis.h"

// Swap Creation and Pricing
#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"

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
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapZeroCouponSwapXccyFixedFloat/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1e-8;

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
    extern const char zeroCouponSwapInputs[]	= TEST_DIR "XCCYFIXEDFLOATZEROCOUPONSWAP@1_tryMeLWOSwapCreate_inputs";
    extern const char parRateInputs[]	        = TEST_DIR "XCCYFIXEDFLOATZEROCOUPONSWAP@1_tryMeLWOSwapParRate_inputs";
    extern const char parRateOutputs[]	    = TEST_DIR "XCCYFIXEDFLOATZEROCOUPONSWAP@1_tryMeLWOSwapParRate_outputs";
    extern const char parRateOutputs64[]	    = TEST_DIR "XCCYFIXEDFLOATZEROCOUPONSWAP@1_tryMeLWOSwapParRate_outputs64_";
}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the AQL_BUILD_EURUSD_XCCY_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    AQL_BUILD_EURUSD_XCCY_CURVE( TestLWOSwapZeroCouponSwapXccyFixedFloat, USDYC_OIS, USDYC_STD, EURYC_OIS, EURYC_STD, EURYC_3M6M, EURYC_XCCY );
    
    //
    // Call Test Fixture
    //

    TEST_F( TestLWOSwapZeroCouponSwapXccyFixedFloat, SNAPSHOT_SwapHardCodedParRateCheck )
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
                const ReadDataFile::Load tradeInputFile( zeroCouponSwapInputs );
                const ReadDataFile::Load parRateInputFile( parRateInputs );
                const ReadDataFile::Load parRateOutputFile( parRateOutputsFilename );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParRate Inputs & Calculate the parRate
                std::string swapName            = parRateInputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", AQLStringMatrix() );
                
                double actualSwapParRate          = validation::tryMeLWOSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapParRate, TEST_DIR, parRateOutputsFilename, tolerance );
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


    TEST_F( TestLWOSwapZeroCouponSwapXccyFixedFloat, SNAPSHOT_SwapPVZeroCheck )
    {
        try
        {
                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( zeroCouponSwapInputs );
                const ReadDataFile::Load parRateInputFile( parRateInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParRate Inputs & Calculate the parRate
                std::string swapName            = parRateInputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", AQLStringMatrix() );

                double parRate = validation::tryMeLWOSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );

				//Update the swap's fixedRate by parRate
				std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapLVB);
				std::stringstream parRateStr;
				parRateStr << std::fixed << std::setprecision(20) << parRate;
				
                // Create LVB to update Fixed Rate
                legsLVB[0] = LabelValueBlock( legsLVB[0], etrading::IRS_KEY::FIXED_RATE, parRateStr.str() );

				std::string newSwapTradeName = swapTradeName + "_1";
				validation::tryMeLWOSwapCreateFromLegLVBs(newSwapTradeName, legsLVB[0], legsLVB[1], swapPropertiesLVB, isXccySwap, validateKeys);

				double actualPV = validation::tryMeLWOSwapPV(newSwapTradeName, curveCollectionLVB, "", fixingTableLVB);
                
                // 5. Check the Test Results
                EXPECT_NEAR( 0, actualPV, tolerance );
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

	TEST_F( TestLWOSwapZeroCouponSwapXccyFixedFloat, CONSISTENCY_IsFwdInterCheck )
    {
        try
        {
                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( zeroCouponSwapInputs );
                const ReadDataFile::Load parRateInputFile( parRateInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

				//1) Check Default Value
                std::string swapName          = validation::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
           		auto swap = etrading::getSwap(swapName);

                AQLStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", AQLStringMatrix() );

				// Call any pricing function so that the fwdInter flag will be retrieved from a specific curveCollection and staticTable
                validation::tryMeLWOSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );

				EXPECT_EQ(etrading::FALSE_BOOL, swap->getLeg(1)->getStaticData()->getFwdInter());

				//2) Check User specified value taking priority
				std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapLVB);
				auto firstLegLVB = legsLVB[0];
				auto secondLegLVB = LabelValueBlock( legsLVB[1], etrading::IRS_KEY::IS_FWD_INTER, etrading::toString(etrading::TRUE_BOOL) );

				std::string swapName2 = validation::tryMeLWOSwapCreateFromLegLVBs( swapTradeName, firstLegLVB, secondLegLVB, swapPropertiesLVB, isXccySwap, validateKeys );
           		auto swap2 = etrading::getSwap(swapName2);

				// Call any pricing function so that the fwdInter flag will be retrieved from a specific curveCollection and staticTable
				// Since float leg's EURYC_3M6M is a basis curve, cannot set it to true, throw error when pricing
				EXPECT_THROW(validation::tryMeLWOSwapParRate( swapName2, curveCollectionLVB, fixingTableLVB ), AQLCoreError );  // On Windows AQLCoreError inherits from std::exception; however on Linux it does not.
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
