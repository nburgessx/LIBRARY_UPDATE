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
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapZeroCouponSwapFloatFloat/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1e-6;  // Spread is reported in BPs

    //
    // curve input files
    //
    extern const char USDYC_OIS[]			    = TEST_DIR "USDYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    extern const char USDYC_STD[]			    = TEST_DIR "USDYC_STD_tryMeCurveCalibrateSwap_inputs.csv";
    extern const char USDYC_1M[]			    = "";
    extern const char USDYC_6M[]			    = TEST_DIR "USDYC_3M6M_tryMeCurveCalibrateBasis_inputs.csv";
    extern const char USDYC_12M[]			    = "";

    //
    // test call input and reference files
    //
    extern const char zeroCouponSwapInputs[]	= TEST_DIR "FLOATFLOATZEROCOUPONSWAP@2_tryMeLWOSwapCreate_inputs";
    extern const char parSpreadInputs[]	        = TEST_DIR "FLOATFLOATZEROCOUPONSWAP@2_tryMeLWOSwapParSpread_inputs";
    extern const char parSpreadOutputs[]	    = TEST_DIR "FLOATFLOATZEROCOUPONSWAP@2_tryMeLWOSwapParSpread_outputs";
    extern const char parSpreadOutputs64[]	    = TEST_DIR "FLOATFLOATZEROCOUPONSWAP@2_tryMeLWOSwapParSpread_outputs64_";

}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the ME_BUILD_EURUSD_XCCY_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ME_BUILD_USD_CURVE( TestLWOSwapZeroCouponSwapFloatFloat, USDYC_OIS, USDYC_1M, USDYC_STD, USDYC_6M, USDYC_12M);
    
    //
    // Call Test Fixture
    //

    TEST_F( TestLWOSwapZeroCouponSwapFloatFloat, SNAPSHOT_HardCodedParSpreadCheck )
    {
        try
        {
                // 1. Create the Input File Names 
                #if defined(GTEST32)
                    LAString parSpreadOutputsFilename   = parSpreadOutputs; 
                #else
                    LAString parSpreadOutputsFilename   = parSpreadOutputs64; 
                #endif

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( zeroCouponSwapInputs );
                const ReadDataFile::Load parRateInputFile( parSpreadInputs );
                const ReadDataFile::Load parRateOutputFile( parSpreadOutputsFilename );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                LAStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                LAStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation_api::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = parRateInputFile["swapName"];
                LAString curveCollection        = parRateInputFile["curveCollections"];
                std::string fixingTable         = parRateInputFile["fixingTableNames"];

                double actualSwapParSpread          = validation_api::tryMeLWOSwapParSpread( swapName, etrading::fromStringToLVB(curveCollection.getCString()), etrading::fromStringToLVB(fixingTable) );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapParSpread, TEST_DIR, parSpreadOutputsFilename, tolerance );
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


    TEST_F( TestLWOSwapZeroCouponSwapFloatFloat, SNAPSHOT_SwapPVZeroCheck )
    {
        try
        {
                const double zeroCouponSwapTolerance = 1e-5;  // Spread is reported in BPs

                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( zeroCouponSwapInputs );
                const ReadDataFile::Load parRateInputFile( parSpreadInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                LAStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                LAStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation_api::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = parRateInputFile["swapName"];
				etrading::LabelValueBlock curveCollection        = etrading::fromStringToLVB(parRateInputFile["curveCollections"]);
				etrading::LabelValueBlock fixingTable         = etrading::fromStringToLVB(parRateInputFile["fixingTableNames"]);
                
                double actualSwapParSpread = validation_api::tryMeLWOSwapParSpread( swapName, curveCollection, fixingTable );

				//Update the swap's spread by actualSwapParSpread
				std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapLVB);

				std::stringstream actualSwapParSpreadStr;
				actualSwapParSpreadStr << std::fixed << std::setprecision(20) << actualSwapParSpread;
				// Create new LVB to update the float spread
                legsLVB[1] = LabelValueBlock( legsLVB[1], etrading::IRS_KEY::FLOAT_SPREAD, actualSwapParSpreadStr.str() );

				std::string newSwapTradeName = swapTradeName + "_1";
				validation_api::tryMeLWOSwapCreateFromLegLVBs(newSwapTradeName, legsLVB[0], legsLVB[1], swapPropertiesLVB, isXccySwap, validateKeys);

                double actualPV = validation_api::tryMeLWOSwapPV(newSwapTradeName, curveCollection, "", fixingTable);
                
                // 5. Check the Test Results
                EXPECT_NEAR( 0, actualPV, zeroCouponSwapTolerance );
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

	TEST_F( TestLWOSwapZeroCouponSwapFloatFloat, CONSISTENCY_IsFwdInterCheck )
    {
        try
        {
                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( zeroCouponSwapInputs );
                const ReadDataFile::Load parRateInputFile( parSpreadInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                LAStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                LAStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

				//1) Check Default Value
                std::string swapName          = validation_api::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
           		auto swap = etrading::getSwap(swapName);

				etrading::LabelValueBlock curveCollection        = etrading::fromStringToLVB(parRateInputFile["curveCollections"]);
				etrading::LabelValueBlock fixingTable         = etrading::fromStringToLVB(parRateInputFile["fixingTableNames"]);

				// Call any pricing function so that the fwdInter flag will be retrieved from a specific curveCollection and staticTable
                validation_api::tryMeLWOSwapPV( swapName, curveCollection, "", fixingTable);

				EXPECT_EQ(etrading::FALSE_BOOL, swap->getLeg(0)->getStaticData()->getFwdInter()); 
				EXPECT_EQ(etrading::FALSE_BOOL, swap->getLeg(1)->getStaticData()->getFwdInter());

				//2) Check User specified value taking priority
				std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapLVB);
				auto firstLegLVB = legsLVB[0];
				auto secondLegLVB = LabelValueBlock( legsLVB[1], etrading::IRS_KEY::IS_FWD_INTER, etrading::toString(etrading::TRUE_BOOL) );

				std::string swapName2 = validation_api::tryMeLWOSwapCreateFromLegLVBs( swapTradeName, firstLegLVB, secondLegLVB, swapPropertiesLVB, isXccySwap, validateKeys );
           		auto swap2 = etrading::getSwap(swapName2);

				// Call any pricing function so that the fwdInter flag will be retrieved from a specific curveCollection and staticTable
				validation_api::tryMeLWOSwapPV( swapName, curveCollection, "", fixingTable);

				EXPECT_EQ(etrading::FALSE_BOOL, swap2->getLeg(0)->getStaticData()->getFwdInter()); 
				EXPECT_EQ(etrading::TRUE_BOOL, swap2->getLeg(1)->getStaticData()->getFwdInter());
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
