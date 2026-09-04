// Curves
#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"

// Swap Creation and Pricing
#include "tryAqObjectsSwapCreation.h"
#include "tryAqObjectsSwapPricing.h"
#include "tryAqObjectsLeg.h"

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
#define TEST_DIR "ETrading/AQObjects/TestAQOSwapZeroCouponSwapFixedFloat/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1e-8;

    //
    // curve input files
    //
    extern const char USDYC_OIS[]			    = TEST_DIR "USDYC_OIS_tryAqCurvesCalibrateOIS_inputs.csv";
    extern const char USDYC_STD[]			    = TEST_DIR "USDYC_STD_tryAqCurvesCalibrateSwap_inputs.csv";
    extern const char USDYC_1M[]			    = "";
    extern const char USDYC_6M[]			    = "";
    extern const char USDYC_12M[]			    = "";

    //
    // test call input and reference files
    //
    extern const char zeroCouponSwapInputs[]	= TEST_DIR "FIXEDFLOATZEROCOUPONSWAP@1_tryAqObjectsSwapCreate_inputs";
    extern const char parRateInputs[]	        = TEST_DIR "FIXEDFLOATZEROCOUPONSWAP@1_tryAqObjectsSwapParRate_inputs";
    extern const char parRateOutputs[]	    = TEST_DIR "FIXEDFLOATZEROCOUPONSWAP@1_tryAqObjectsSwapParRate_outputs";
    extern const char parRateOutputs64[]	    = TEST_DIR "FIXEDFLOATZEROCOUPONSWAP@1_tryAqObjectsSwapParRate_outputs64_";
}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    // All curves name MUST be defined for the AQL_BUILD_EURUSD_XCCY_CURVE macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    AQL_BUILD_USD_CURVE( TestAQOSwapZeroCouponSwapFixedFloat, USDYC_OIS, USDYC_1M, USDYC_STD, USDYC_6M, USDYC_12M);
    
    //
    // Call Test Fixture
    //

    TEST_F( TestAQOSwapZeroCouponSwapFixedFloat, SNAPSHOT_SwapHardCodedParRateCheck )
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
                
                std::string createSwap          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParRate Inputs & Calculate the parRate
                std::string swapName            = parRateInputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", AQLStringMatrix() );
                
                double actualSwapParRate          = validation::tryAqObjectsSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );
                
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


    TEST_F( TestAQOSwapZeroCouponSwapFixedFloat, SNAPSHOT_SwapPVZeroCheck )
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

                std::string createSwap          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
                
                // 4. Get the ParRate Inputs & Calculate the parRate
                std::string swapName            = parRateInputFile["swapName"];
                AQLStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", AQLStringMatrix() );

                double parRate = validation::tryAqObjectsSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );

				//Update the swap's fixedRate by parRate
				std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapLVB);
				std::stringstream parRateStr;
				parRateStr << std::fixed << std::setprecision(20) << parRate;

                // Create new LVB to update fixed rate
                legsLVB[0] = LabelValueBlock( legsLVB[0], etrading::IRS_KEY::FIXED_RATE, parRateStr.str() );

				std::string newSwapTradeName = swapTradeName + "_1";
				validation::tryAqObjectsSwapCreateFromLegLVBs(newSwapTradeName, legsLVB[0], legsLVB[1], swapPropertiesLVB, isXccySwap, validateKeys);
                
                double actualPV = validation::tryAqObjectsSwapPV(newSwapTradeName, curveCollectionLVB, "", fixingTableLVB);
                
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

	  TEST_F( TestAQOSwapZeroCouponSwapFixedFloat, CONSISTENCY_IsFwdInterCheck )
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
                std::string swapName          = validation::tryAqObjectsSwapCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
				auto swap = etrading::getSwap(swapName);

                AQLStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", AQLStringMatrix() );

				// Call any pricing function so that the fwdInter flag will be retrieved from a specific curveCollection and staticTable
                validation::tryAqObjectsSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );

				EXPECT_EQ(etrading::TRUE_BOOL, swap->getLeg(1)->getStaticData()->getFwdInter());

				//2) Check User specified value taking priority
				std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapLVB);
				auto firstLegLVB = legsLVB[0];
				auto secondLegLVB = LabelValueBlock( legsLVB[1], etrading::IRS_KEY::IS_FWD_INTER, etrading::toString(etrading::FALSE_BOOL) );

				std::string swapName2          = validation::tryAqObjectsSwapCreateFromLegLVBs( swapTradeName, firstLegLVB, secondLegLVB, swapPropertiesLVB, isXccySwap, validateKeys );
           		auto swap2 = etrading::getSwap(swapName2);

				// Call any pricing function so that the fwdInter flag will be retrieved from a specific curveCollection and staticTable
                validation::tryAqObjectsSwapParRate( swapName2, curveCollectionLVB, fixingTableLVB );

				EXPECT_EQ(etrading::FALSE_BOOL, swap2->getLeg(1)->getStaticData()->getFwdInter());
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

	TEST_F( TestAQOSwapZeroCouponSwapFixedFloat, CONSISTENCY_IsFwdInterCheck_MultiFloatLegs )
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

				std::string swapName          = validation::tryAqObjectsSwapCreateFromLegLVBs( swapTradeName, firstLegLVB, secondLegLVB, swapPropertiesLVB, isXccySwap, validateKeys );

                AQLStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                AQLStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", AQLStringMatrix() );
                validation::tryAqObjectsSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );

				std::string legName3          = validation::tryAqObjectsLegCreate("thirdLeg", thirdLegLVB);
				std::string legName4          = validation::tryAqObjectsLegCreate("fourLeg", fourthLegLVB);
				validation::tryAqObjectsSwapAddLeg(swapName, legName3);
				validation::tryAqObjectsSwapAddLeg(swapName, legName4);

				auto swap = etrading::getSwap(swapName);

				// Call any pricing function so that the fwdInter flag will be retrieved from a specific curveCollection and staticTable
                //AQLStringMatrix curveCollectionLVB = parRateInputFile["curveCollections"];
                //AQLStringMatrix fixingTableLVB     = parRateInputFile.getOptional("fixingTableNames", AQLStringMatrix() );

				validation::tryAqObjectsSwapParRate( swapName, curveCollectionLVB, fixingTableLVB );

				EXPECT_EQ(etrading::TRUE_BOOL, swap->getLeg(1)->getStaticData()->getFwdInter());
				EXPECT_EQ(etrading::FALSE_BOOL, swap->getLeg(2)->getStaticData()->getFwdInter()); 
				EXPECT_EQ(etrading::FALSE_BOOL, swap->getLeg(3)->getStaticData()->getFwdInter());

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
