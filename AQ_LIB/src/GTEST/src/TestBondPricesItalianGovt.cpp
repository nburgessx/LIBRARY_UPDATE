// Include: Google Test Library
#include <gTest/gTest.h>
#include "InitializeGoogleTest.h"

#include "BondYields.h"
#include "BondFactory.h"
#include "ObjectUtilities.h"
#include "CreateFixedBond.h"
#include "tryAqObjectsBond.h"

#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR1 "ETrading/Bonds/ItalianType1/"
#define TEST_DIR2 "ETrading/Bonds/ItalianType523ZeroCoupon1/"
#define TEST_DIR3 "ETrading/Bonds/ItalianType523ZeroCoupon2/"
#define TEST_DIR4 "ETrading/Bonds/ItalianType523Fixed/"
#define TEST_DIR5 "ETrading/Bonds/ItalianType527Type529/"

namespace
{
    // Number of Test Bonds
    const int TEST_COUNT1 = 50;
    const int TEST_COUNT2 = 130;
    const int TEST_COUNT3 = 128;
    const int TEST_COUNT4 = 75;
    const int TEST_COUNT5 = 20;

    const double priceTolerance = 1e-7;  
    const double yieldTolerance = 1e-10;

    // Test Bond Input File(s)
    extern const char createBondInputFile1[]         ="ETrading/Bonds/ItalianType1/tryAqObjectsBondCreate_inputs";
    extern const char createBondInputFile2[]         ="ETrading/Bonds/ItalianType523ZeroCoupon1/tryAqObjectsBondCreate_inputs";
    extern const char createBondInputFile3[]         ="ETrading/Bonds/ItalianType523ZeroCoupon2/tryAqObjectsBondCreate_inputs";
    extern const char createBondInputFile4[]         ="ETrading/Bonds/ItalianType523Fixed/tryAqObjectsBondCreate_inputs";
    extern const char createBondInputFile5[]         ="ETrading/Bonds/ItalianType527Type529/tryAqObjectsBondCreate_inputs";

    extern const char priceBondInputFile1[]          ="ETrading/Bonds/ItalianType1/tryAqObjectsBondCleanPrice_inputs";
    extern const char priceBondInputFile2[]          ="ETrading/Bonds/ItalianType523ZeroCoupon1/tryAqObjectsBondCleanPrice_inputs";
    extern const char priceBondInputFile3[]          ="ETrading/Bonds/ItalianType523ZeroCoupon2/tryAqObjectsBondCleanPrice_inputs";
    extern const char priceBondInputFile4[]          ="ETrading/Bonds/ItalianType523Fixed/tryAqObjectsBondCleanPrice_inputs";
    extern const char priceBondInputFile5[]          ="ETrading/Bonds/ItalianType527Type529/tryAqObjectsBondCleanPrice_inputs";

    extern const char priceBondOutputFile1[]         ="ETrading/Bonds/ItalianType1/tryAqObjectsBondCleanPrice_outputs";
    extern const char priceBondOutputFile2[]         ="ETrading/Bonds/ItalianType523ZeroCoupon1/tryAqObjectsBondCleanPrice_outputs";
    extern const char priceBondOutputFile3[]         ="ETrading/Bonds/ItalianType523ZeroCoupon2/tryAqObjectsBondCleanPrice_outputs";
    extern const char priceBondOutputFile4[]         ="ETrading/Bonds/ItalianType523Fixed/tryAqObjectsBondCleanPrice_outputs";
    extern const char priceBondOutputFile5[]         ="ETrading/Bonds/ItalianType527Type529/tryAqObjectsBondCleanPrice_outputs";
}

namespace google_test
{
    class TestBondFactory : public virtual testing::Test, public virtual google_test::InitializeGoogleTest
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
    class TestBondPricesItalianGovt : public TestBondFactory {};


    TEST_F( TestBondPricesItalianGovt, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData1 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT1; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile1, i );
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile1, i );
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQO object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjectsBondCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR1, priceBondOutputFile1, priceTolerance, i );
                
                // Check Yield Matching
                auto actualYields = validation::tryAqObjectsBondYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjectsBondCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );

                    auto expectedCompoundYield = myBondPtr->convertYieldFromYieldFreqToCouponFreq(yields[i]);
                    EXPECT_NEAR( expectedCompoundYield, compoundYields[i], yieldTolerance );
                }

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT1 );
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

    TEST_F( TestBondPricesItalianGovt, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData2 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT2; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile2, i ); 
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile2, i ); 
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQO object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjectsBondCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR2, priceBondOutputFile2, priceTolerance, i ); 

                // Check Yield Matching
                auto actualYields = validation::tryAqObjectsBondYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjectsBondCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );

                    auto expectedCompoundYield = myBondPtr->convertYieldFromYieldFreqToCouponFreq(yields[i]);
                    EXPECT_NEAR( expectedCompoundYield, compoundYields[i], yieldTolerance );
                }

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT2 );
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

    TEST_F( TestBondPricesItalianGovt, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData3 )
    {
        int i = 0;
        int fileStartIndex = 130; // File Starts at 130
        try
        {
             for ( i = 0; i < TEST_COUNT3; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile3, i+fileStartIndex ); 
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile3, i+fileStartIndex ); 
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQO object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjectsBondCleanPrice( bondObjectName, settlementDates, yields );
                
                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR3, priceBondOutputFile3, priceTolerance, i+fileStartIndex );

                // Check Yield Matching
                auto actualYields = validation::tryAqObjectsBondYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjectsBondCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );

                    auto expectedCompoundYield = myBondPtr->convertYieldFromYieldFreqToCouponFreq(yields[i]);
                    EXPECT_NEAR( expectedCompoundYield, compoundYields[i], yieldTolerance );
                }

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT3 );
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


     TEST_F( TestBondPricesItalianGovt, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData4 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT4; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile4, i); 
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile4, i ); 
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQO object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjectsBondCleanPrice( bondObjectName, settlementDates, yields );
                
                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR4, priceBondOutputFile4, priceTolerance, i );

                // Check Yield Matching
                auto actualYields = validation::tryAqObjectsBondYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjectsBondCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );

                    auto expectedCompoundYield = myBondPtr->convertYieldFromYieldFreqToCouponFreq(yields[i]);
                    EXPECT_NEAR( expectedCompoundYield, compoundYields[i], yieldTolerance );
                }

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT4 );
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

    TEST_F( TestBondPricesItalianGovt, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData5 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT5; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile5, i); 
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile5, i ); 
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQO object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjectsBondCleanPrice( bondObjectName, settlementDates, yields );
                
                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR5, priceBondOutputFile5, priceTolerance, i );

                // Check Yield Matching
                auto actualYields = validation::tryAqObjectsBondYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjectsBondCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );

                    auto expectedCompoundYield = myBondPtr->convertYieldFromYieldFreqToCouponFreq(yields[i]);
                    EXPECT_NEAR( expectedCompoundYield, compoundYields[i], yieldTolerance );
                }

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT5 );
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


};