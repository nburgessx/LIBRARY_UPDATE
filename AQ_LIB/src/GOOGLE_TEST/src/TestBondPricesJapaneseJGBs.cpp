// Include: Google Test Library
#include <gTest/gTest.h>
#include "InitializeAQGoogleTest.h"

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

#define TEST_DIR1 "ETrading/Bonds/JapaneseJGBs1/"
#define TEST_DIR2 "ETrading/Bonds/JapaneseJGBs2/"
#define TEST_DIR3 "ETrading/Bonds/JapaneseJGBs3/"

namespace
{
    // Number of Test Bonds
    const int TEST_COUNT1 = 100;
    const int TEST_COUNT2 = 100;
    const int TEST_COUNT3 = 65;

    const double priceTolerance = 1e-7; 
    const double yieldTolerance = 1e-10;

    // Test Bond Input File(s)
    extern const char createBondInputFile1[]         ="ETrading/Bonds/JapaneseJGBs1/tryMeLWOBondCreate_inputs";
    extern const char createBondInputFile2[]         ="ETrading/Bonds/JapaneseJGBs2/tryMeLWOBondCreate_inputs";
    extern const char createBondInputFile3[]         ="ETrading/Bonds/JapaneseJGBs3/tryMeLWOBondCreate_inputs";

    extern const char priceBondInputFile1[]          ="ETrading/Bonds/JapaneseJGBs1/tryMeLWOBondCleanPrice_inputs";
    extern const char priceBondInputFile2[]          ="ETrading/Bonds/JapaneseJGBs2/tryMeLWOBondCleanPrice_inputs";
    extern const char priceBondInputFile3[]          ="ETrading/Bonds/JapaneseJGBs3/tryMeLWOBondCleanPrice_inputs";

    extern const char priceBondOutputFile1[]         ="ETrading/Bonds/JapaneseJGBs1/tryMeLWOBondCleanPrice_outputs";
    extern const char priceBondOutputFile2[]         ="ETrading/Bonds/JapaneseJGBs2/tryMeLWOBondCleanPrice_outputs";
    extern const char priceBondOutputFile3[]         ="ETrading/Bonds/JapaneseJGBs3/tryMeLWOBondCleanPrice_outputs";

}

namespace google_test
{
    class TestBondFactory : public virtual testing::Test, public virtual google_test::InitializeAQGoogleTest
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
    class TestBondPricesJapaneseJGBs : public TestBondFactory {};


    TEST_F( TestBondPricesJapaneseJGBs, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData1 )
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
                auto actualCleanPrice = validation::tryMeLWOBondCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR1, priceBondOutputFile1, priceTolerance, i );

                // Check Yield Matching
                auto actualYields = validation::tryMeLWOBondYield( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
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

    TEST_F( TestBondPricesJapaneseJGBs, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData2 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT2; ++i )
             {
                // Create the Bond and Price filenames
                LAString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile2, i+100 ); // File Starts at 100
                LAString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile2, i+100 ); // File Starts at 100
                
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
                auto actualCleanPrice = validation::tryMeLWOBondCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR2, priceBondOutputFile2, priceTolerance, i+100 ); // File Starts at 100

                // Check Yield Matching
                auto actualYields = validation::tryMeLWOBondYield( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
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

    TEST_F( TestBondPricesJapaneseJGBs, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData3 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT3; ++i )
             {
                // Create the Bond and Price filenames
                LAString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile3, i+200 ); // File Starts at 200
                LAString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile3, i+200 ); // File Starts at 200
                
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
                auto actualCleanPrice = validation::tryMeLWOBondCleanPrice( bondObjectName, settlementDates, yields );
                
                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR3, priceBondOutputFile3, priceTolerance, i+200 ); // File Starts at 200

                // Check Yield Matching
                auto actualYields = validation::tryMeLWOBondYield( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                }

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT3 );
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