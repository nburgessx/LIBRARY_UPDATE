// Curves
#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "TryMeCurveTenorBasis.h"

// Swap Creation and Pricing
#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"
#include "tryMeLWOLeg.h"

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
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapZeroCouponSwapFixedFloat/"

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
    extern const char USDYC_1M[]			    = "";
    extern const char USDYC_6M[]			    = "";
    extern const char USDYC_12M[]			    = "";

    //
    // test call input and reference files
    //
    extern const char zeroCouponSwapInputs[]	= TEST_DIR "FIXEDFLOATZEROCOUPONSWAP@1_tryMeLWOSwapCreate_inputs";
    extern const char parRateInputs[]	        = TEST_DIR "FIXEDFLOATZEROCOUPONSWAP@1_tryMeLWOSwapParRate_inputs";
    extern const char parRateOutputs[]	    = TEST_DIR "FIXEDFLOATZEROCOUPONSWAP@1_tryMeLWOSwapParRate_outputs";
    extern const char parRateOutputs64[]	    = TEST_DIR "FIXEDFLOATZEROCOUPONSWAP@1_tryMeLWOSwapParRate_outputs64_";
}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the ME_BUILD_EURUSD_XCCY_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ME_BUILD_USD_CURVE( TestLWOSwapZeroCouponSwapFixedFloat, USDYC_OIS, USDYC_1M, USDYC_STD, USDYC_6M, USDYC_12M);
    
    //
    // Call Test Fixture
    //

    TEST_F( TestLWOSwapZeroCouponSwapFixedFloat, SNAPSHOT_SwapHardCodedParRateCheck )
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
                const ReadDataFile::Load tradeInputFile( zeroCouponSwapInputs );
                const ReadDataFile::Load parRateInputFile( parRateInputs );
                const ReadDataFile::Load parRateOutputFile( parRateOutputsFilename );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                LAStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                LAStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];
                
                std::string createSwap          = validation::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParRate Inputs & Calculate the parRate
                std::string swapName            = parRateInputFile["swapName"];
                LAStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                LAStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", LAStringMatrix() );
                
                double actualSwapParRate          = validation::tryMeLWOSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );
                
                // 5. Check the Test Results or Rebase
                CheckTestResultsAndRebaseOnRequest( actualSwapParRate, TEST_DIR, parRateOutputsFilename, tolerance );
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


    TEST_F( TestLWOSwapZeroCouponSwapFixedFloat, SNAPSHOT_SwapPVZeroCheck )
    {
        try
        {
                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( zeroCouponSwapInputs );
                const ReadDataFile::Load parRateInputFile( parRateInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                LAStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                LAStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryMeLWOSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParRate Inputs & Calculate the parRate
                std::string swapName            = parRateInputFile["swapName"];
                LAStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                LAStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", LAStringMatrix() );

                double parRate = validation::tryMeLWOSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );

				//Update the swap's fixedRate by parRate
				std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapLVB);
				std::stringstream parRateStr;
				parRateStr << std::fixed << std::setprecision(20) << parRate;

                // Create new LVB to update fixed rate
                legsLVB[0] = LabelValueBlock( legsLVB[0], etrading::IRS_KEY::FIXED_RATE, parRateStr.str() );

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

	  TEST_F( TestLWOSwapZeroCouponSwapFixedFloat, CONSISTENCY_IsFwdInterCheck )
    {
        try
        {
                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( zeroCouponSwapInputs );
                const ReadDataFile::Load parRateInputFile( parRateInputs );
        
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
                validation::tryMeLWOSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );

				EXPECT_EQ(etrading::TRUE_BOOL, swap->getLeg(1)->getStaticData()->getFwdInter());

				//2) Check User specified value taking priority
				std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapLVB);
				auto firstLegLVB = legsLVB[0];
				auto secondLegLVB = LabelValueBlock( legsLVB[1], etrading::IRS_KEY::IS_FWD_INTER, etrading::toString(etrading::FALSE_BOOL) );

				std::string swapName2          = validation::tryMeLWOSwapCreateFromLegLVBs( swapTradeName, firstLegLVB, secondLegLVB, swapPropertiesLVB, isXccySwap, validateKeys );
           		auto swap2 = etrading::getSwap(swapName2);

				// Call any pricing function so that the fwdInter flag will be retrieved from a specific curveCollection and staticTable
                validation::tryMeLWOSwapParRate( swapName2, curveCollectionLVB, fixingTableLVB );

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

	TEST_F( TestLWOSwapZeroCouponSwapFixedFloat, CONSISTENCY_IsFwdInterCheck_MultiFloatLegs )
    {
        try
        {
                // 2. Load the Input Files
                const ReadDataFile::Load tradeInputFile( zeroCouponSwapInputs );
                const ReadDataFile::Load parRateInputFile( parRateInputs );
        
                // 3. Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                LAStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                LAStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

				std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapLVB);
				auto firstLegLVB = legsLVB[0];
				auto secondLegLVB = legsLVB[1];

				const std::string defaultNotional = boost::lexical_cast<std::string>(1e6);

                // Leg 3 Property Update
                StandardStringVector leg3Keys(2);
                StandardStringVector leg3Values(2);

                leg3Keys[0] = etrading::IRS_KEY::LEG_TYPE;
                leg3Keys[1] = etrading::IRS_KEY::NOTIONAL;
                
                leg3Values[0] = "leg3:float";
                leg3Values[1] = defaultNotional;

				auto thirdLegLVB = LabelValueBlock( legsLVB[1], leg3Keys, leg3Values );

                // Leg 3 Property Update
                StandardStringVector leg4Keys(2);
                StandardStringVector leg4Values(2);

                leg4Keys[0] = etrading::IRS_KEY::LEG_TYPE;
                leg4Keys[1] = etrading::IRS_KEY::NOTIONAL;
                
                leg4Values[0] = "leg4:float";
                leg4Values[1] = defaultNotional;

				auto fourthLegLVB = LabelValueBlock( legsLVB[1], leg4Keys, leg4Values );

				std::string swapName          = validation::tryMeLWOSwapCreateFromLegLVBs( swapTradeName, firstLegLVB, secondLegLVB, swapPropertiesLVB, isXccySwap, validateKeys );

                LAStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                LAStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", LAStringMatrix() );
                validation::tryMeLWOSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );

				std::string legName3          = validation::tryMeLWOLegCreate("thirdLeg", thirdLegLVB);
				std::string legName4          = validation::tryMeLWOLegCreate("fourLeg", fourthLegLVB);
				validation::tryMeLWOSwapAddLeg(swapName, legName3);
				validation::tryMeLWOSwapAddLeg(swapName, legName4);

				auto swap = etrading::getSwap(swapName);

				// Call any pricing function so that the fwdInter flag will be retrieved from a specific curveCollection and staticTable
                //LAStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                //LAStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", LAStringMatrix() );

				validation::tryMeLWOSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );

				EXPECT_EQ(etrading::TRUE_BOOL, swap->getLeg(1)->getStaticData()->getFwdInter());
				EXPECT_EQ(etrading::FALSE_BOOL, swap->getLeg(2)->getStaticData()->getFwdInter()); 
				EXPECT_EQ(etrading::FALSE_BOOL, swap->getLeg(3)->getStaticData()->getFwdInter());

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
