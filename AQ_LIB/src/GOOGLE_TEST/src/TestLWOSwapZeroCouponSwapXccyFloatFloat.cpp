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
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapZeroCouponSwapXccyFloatFloat/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1e-7;   // Par Spread is reported in BPs

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
    extern const char zeroCouponSwapInputs[]	= TEST_DIR "XCCYFLOATFLOATZEROCOUPONSWAP@1_tryMeLWOSwapCreate_inputs";
    extern const char parSpreadInputs[]	        = TEST_DIR "XCCYFLOATFLOATZEROCOUPONSWAP@1_tryMeLWOSwapParSpread_inputs";
    extern const char parSpreadOutputs[]	    = TEST_DIR "XCCYFLOATFLOATZEROCOUPONSWAP@1_tryMeLWOSwapParSpread_outputs";
    extern const char parSpreadOutputs64[]	    = TEST_DIR "XCCYFLOATFLOATZEROCOUPONSWAP@1_tryMeLWOSwapParSpread_outputs64_";

}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the ME_BUILD_EURUSD_XCCY_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ME_BUILD_EURUSD_XCCY_CURVE( TestLWOSwapZeroCouponSwapXccyFloatFloat, USDYC_OIS, USDYC_STD, EURYC_OIS, EURYC_STD, EURYC_3M6M, EURYC_XCCY );
    
    //
    // Call Test Fixture
    //

    TEST_F( TestLWOSwapZeroCouponSwapXccyFloatFloat, SNAPSHOT_HardCodedParSpreadCheck )
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
                
                std::string createSwap          = validation::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = parRateInputFile["swapName"];
                LAStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                LAStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", LAStringMatrix() );
                
                double actualSwapParSpread          = validation::tryMeLWOSwapParSpread( swapName, curveCollectionLVB, fixingTableLVB );
                
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


    TEST_F( TestLWOSwapZeroCouponSwapXccyFloatFloat, SNAPSHOT_SwapPVZeroCheck )
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

                std::string createSwap          = validation::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParSpread Inputs & Calculate the parSpread
                std::string swapName            = parRateInputFile["swapName"];
                LAStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                LAStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", LAStringMatrix() );
                
                double actualSwapParSpread          = validation::tryMeLWOSwapParSpread( swapName, curveCollectionLVB, fixingTableLVB );

				//Update the swap's spread by actualSwapParSpread
				std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapLVB);

				std::stringstream actualSwapParSpreadStr;
				actualSwapParSpreadStr << std::fixed << std::setprecision(20) << actualSwapParSpread;
				
                // Create New LVB to update Float Spread
                legsLVB[1] = LabelValueBlock( legsLVB[1], etrading::IRS_KEY::FLOAT_SPREAD, actualSwapParSpreadStr.str() );

				std::string newSwapTradeName = swapTradeName + "_1";
				validation::tryMeLWOSwapCreateFromLegLVBs(newSwapTradeName, legsLVB[0], legsLVB[1], swapPropertiesLVB, isXccySwap, validateKeys);

				double actualPV = validation::tryMeLWOSwapPV(newSwapTradeName, curveCollectionLVB, "", fixingTableLVB);
                
                // 5. Check the Test Results
                EXPECT_NEAR( 0, actualPV, tolerance );
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

	TEST_F( TestLWOSwapZeroCouponSwapXccyFloatFloat, CONSISTENCY_IsFwdInterCheck )
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
                std::string swapName          = validation::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
           		auto swap = etrading::getSwap(swapName);

                LAStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                LAStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", LAStringMatrix() );

				// Call any pricing function so that the fwdInter flag will be retrieved from a specific curveCollection and staticTable
                validation::tryMeLWOSwapPV( swapName, curveCollectionLVB, "", fixingTableLVB);

				EXPECT_EQ(etrading::FALSE_BOOL, swap->getLeg(0)->getStaticData()->getFwdInter()); 
				EXPECT_EQ(etrading::FALSE_BOOL, swap->getLeg(1)->getStaticData()->getFwdInter());

				//2) Check User specified value taking priority
				std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapLVB);
				
                auto firstLegLVB = LabelValueBlock( legsLVB[0],
                                                    etrading::IRS_KEY::IS_FWD_INTER,
                                                    etrading::toString(etrading::TRUE_BOOL) ); 
				auto secondLegLVB = legsLVB[1];

				std::string swapName2          = validation::tryMeLWOSwapCreateFromLegLVBs( swapTradeName, firstLegLVB, secondLegLVB, swapPropertiesLVB, isXccySwap, validateKeys );
           		auto swap2 = etrading::getSwap(swapName2);

				// Call any pricing function so that the fwdInter flag will be retrieved from a specific curveCollection and staticTable
                validation::tryMeLWOSwapPV( swapName, curveCollectionLVB, "", fixingTableLVB);

				EXPECT_EQ(etrading::TRUE_BOOL, swap2->getLeg(0)->getStaticData()->getFwdInter()); 
				EXPECT_EQ(etrading::FALSE_BOOL, swap2->getLeg(1)->getStaticData()->getFwdInter()); 

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
