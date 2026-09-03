/*
 * @brief			Test File for BondPrices.cpp
 * @Created:		18th April 2017
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

// Include: Google Test Library
#include <gTest/gTest.h>
#include "InitializeMLibGoogleTest.h"

#include "BondYields.h"
#include "BondFactory.h"
#include "ObjectUtilities.h"
#include "CreateFixedBond.h"
#include "tryMeLWOBond.h"

#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR1 "ETrading/Bonds/FrenchOATsBTANsZeroCoupon1/"
#define TEST_DIR2 "ETrading/Bonds/FrenchOATsBTANsZeroCoupon2/"

namespace
{
    // Number of Test Bonds
    const int TEST_COUNT1 = 100;
    const int TEST_COUNT2 = 103;

    const double priceTolerance = 1e-7;  // low priceTolerance because the yield recorded to file is 12 decimals by default, the actual yield in excel is more than 12 decimals
    const double yieldTolerance = 1e-10;

    // Test Bond Input File(s)
    extern const char createBondInputFile1[]         =TEST_DIR1 "tryMeLWOBondCreate_inputs";
    extern const char priceBondInputFile1[]          =TEST_DIR1 "tryMeLWOBondCleanPrice_inputs";
    extern const char priceBondOutputFile1[]         =TEST_DIR1 "tryMeLWOBondCleanPrice_outputs";

    extern const char createBondInputFile2[]         =TEST_DIR2 "tryMeLWOBondCreate_inputs";
    extern const char priceBondInputFile2[]          =TEST_DIR2 "tryMeLWOBondCleanPrice_inputs";
    extern const char priceBondOutputFile2[]         =TEST_DIR2 "tryMeLWOBondCleanPrice_outputs";
}

namespace google_test
{
    class TestBondFactory : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
        public:

        static etrading::BondPtr createBond( const std::string& inputFile )
        {
            // Build the Bond Object
            CreateFixedBond fixedBond;
            etrading::BondPtr myBondPtr = fixedBond.createFixedBondFromInputFile( inputFile );
            return myBondPtr;
        }
    };

    // Inherit the Create Bond Object Framework from TestBondFactory
    class TestBondPricesFrenchOATsBTANsZeropCoupon : public TestBondFactory {};


    TEST_F( TestBondPricesFrenchOATsBTANsZeropCoupon, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData1 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT1; ++i )
             {
                // Create the Bond and Price filenames
                LAString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile1, i );
                LAString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile1, i );
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the LWO Cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< LADate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation_api::tryMeLWOBondCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR1, priceBondOutputFile1, priceTolerance, i );
                
                // Check Yield Matching
                auto actualYields = validation_api::tryMeLWOBondYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation_api::tryMeLWOBondCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                    EXPECT_NEAR( yields[i], compoundYields[i], yieldTolerance );
                }

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT1 );
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

    TEST_F( TestBondPricesFrenchOATsBTANsZeropCoupon, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData2 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT2; ++i )
             {
                // Create the Bond and Price filenames
                LAString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile2, i + 100 ); // File Starts at 100
                LAString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile2, i + 100 ); // File Starts at 100
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the LWO Cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< LADate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation_api::tryMeLWOBondCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR2, priceBondOutputFile2, priceTolerance, i + 100 ); // File Starts at 100
                
                // Check Yield Matching
                auto actualYields = validation_api::tryMeLWOBondYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation_api::tryMeLWOBondCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                    EXPECT_NEAR( yields[i], compoundYields[i], yieldTolerance );
                }

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT2 );
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

};