// Include: Google Test Library
#include <gTest/gTest.h>
#include "InitializeGoogleTest.h"

#include "BondYields.h"
#include "BondFactory.h"
#include "ObjectUtilities.h"
#include "CreateFixedBond.h"
#include "tryAqObjBonds.h"

#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR1 "ETrading/Bonds/USTreasuries1/"
#define TEST_DIR2 "ETrading/Bonds/USTreasuries2/"
#define TEST_DIR3 "ETrading/Bonds/USTreasuries3/"
#define TEST_DIR4 "ETrading/Bonds/USTreasuries4/"
#define TEST_DIR5 "ETrading/Bonds/USTreasuries5/"
#define TEST_DIR6 "ETrading/Bonds/USTreasuries6/"
#define TEST_DIR7 "ETrading/Bonds/USTreasuries7/"
#define TEST_DIR8 "ETrading/Bonds/USTreasuries8/"
#define TEST_DIR9 "ETrading/Bonds/USTreasuries9/"
#define TEST_DIR10 "ETrading/Bonds/USTreasuries10/"
#define TEST_DIR11 "ETrading/Bonds/USTreasuries11/"
#define TEST_DIR12 "ETrading/Bonds/USTreasuries12/"

namespace
{
    // Number of Test Bonds
    const int TEST_COUNT1 = 25;
    const int TEST_COUNT2 = 25;
    const int TEST_COUNT3 = 25;
    const int TEST_COUNT4 = 25;
    const int TEST_COUNT5 = 25;
    const int TEST_COUNT6 = 25;
    const int TEST_COUNT7 = 25;
    const int TEST_COUNT8 = 25;
    const int TEST_COUNT9 = 25;
    const int TEST_COUNT10 = 25;
    const int TEST_COUNT11 = 25;
    const int TEST_COUNT12 = 21;

    const double priceTolerance = 1e-7;  // low priceTolerance because the yield recorded to file is 12 decimals by default, the actual yield in excel is more than 12 decimals
    const double yieldTolerance = 1e-10;

    // Test Bond Input File(s)
    extern const char createBondInputFile1[]         ="ETrading/Bonds/USTreasuries1/tryAqObjBondsCreate_inputs";
    extern const char createBondInputFile2[]         ="ETrading/Bonds/USTreasuries2/tryAqObjBondsCreate_inputs";
    extern const char createBondInputFile3[]         ="ETrading/Bonds/USTreasuries3/tryAqObjBondsCreate_inputs";
    extern const char createBondInputFile4[]         ="ETrading/Bonds/USTreasuries4/tryAqObjBondsCreate_inputs";
    extern const char createBondInputFile5[]         ="ETrading/Bonds/USTreasuries5/tryAqObjBondsCreate_inputs";
    extern const char createBondInputFile6[]         ="ETrading/Bonds/USTreasuries6/tryAqObjBondsCreate_inputs";
    extern const char createBondInputFile7[]         ="ETrading/Bonds/USTreasuries7/tryAqObjBondsCreate_inputs";
    extern const char createBondInputFile8[]         ="ETrading/Bonds/USTreasuries8/tryAqObjBondsCreate_inputs";
    extern const char createBondInputFile9[]         ="ETrading/Bonds/USTreasuries9/tryAqObjBondsCreate_inputs";
    extern const char createBondInputFile10[]        ="ETrading/Bonds/USTreasuries10/tryAqObjBondsCreate_inputs";
    extern const char createBondInputFile11[]        ="ETrading/Bonds/USTreasuries11/tryAqObjBondsCreate_inputs";
    extern const char createBondInputFile12[]        ="ETrading/Bonds/USTreasuries12/tryAqObjBondsCreate_inputs";

    extern const char priceBondInputFile1[]          ="ETrading/Bonds/USTreasuries1/tryAqObjBondsCleanPrice_inputs";
    extern const char priceBondInputFile2[]          ="ETrading/Bonds/USTreasuries2/tryAqObjBondsCleanPrice_inputs";
    extern const char priceBondInputFile3[]          ="ETrading/Bonds/USTreasuries3/tryAqObjBondsCleanPrice_inputs";
    extern const char priceBondInputFile4[]          ="ETrading/Bonds/USTreasuries4/tryAqObjBondsCleanPrice_inputs";
    extern const char priceBondInputFile5[]          ="ETrading/Bonds/USTreasuries5/tryAqObjBondsCleanPrice_inputs";
    extern const char priceBondInputFile6[]          ="ETrading/Bonds/USTreasuries6/tryAqObjBondsCleanPrice_inputs";
    extern const char priceBondInputFile7[]          ="ETrading/Bonds/USTreasuries7/tryAqObjBondsCleanPrice_inputs";
    extern const char priceBondInputFile8[]          ="ETrading/Bonds/USTreasuries8/tryAqObjBondsCleanPrice_inputs";
    extern const char priceBondInputFile9[]          ="ETrading/Bonds/USTreasuries9/tryAqObjBondsCleanPrice_inputs";
    extern const char priceBondInputFile10[]         ="ETrading/Bonds/USTreasuries10/tryAqObjBondsCleanPrice_inputs";
    extern const char priceBondInputFile11[]         ="ETrading/Bonds/USTreasuries11/tryAqObjBondsCleanPrice_inputs";
    extern const char priceBondInputFile12[]         ="ETrading/Bonds/USTreasuries12/tryAqObjBondsCleanPrice_inputs";

    extern const char priceBondOutputFile1[]         ="ETrading/Bonds/USTreasuries1/tryAqObjBondsCleanPrice_outputs";
    extern const char priceBondOutputFile2[]         ="ETrading/Bonds/USTreasuries2/tryAqObjBondsCleanPrice_outputs";
    extern const char priceBondOutputFile3[]         ="ETrading/Bonds/USTreasuries3/tryAqObjBondsCleanPrice_outputs";
    extern const char priceBondOutputFile4[]         ="ETrading/Bonds/USTreasuries4/tryAqObjBondsCleanPrice_outputs";
    extern const char priceBondOutputFile5[]         ="ETrading/Bonds/USTreasuries5/tryAqObjBondsCleanPrice_outputs";
    extern const char priceBondOutputFile6[]         ="ETrading/Bonds/USTreasuries6/tryAqObjBondsCleanPrice_outputs";
    extern const char priceBondOutputFile7[]         ="ETrading/Bonds/USTreasuries7/tryAqObjBondsCleanPrice_outputs";
    extern const char priceBondOutputFile8[]         ="ETrading/Bonds/USTreasuries8/tryAqObjBondsCleanPrice_outputs";
    extern const char priceBondOutputFile9[]         ="ETrading/Bonds/USTreasuries9/tryAqObjBondsCleanPrice_outputs";
    extern const char priceBondOutputFile10[]        ="ETrading/Bonds/USTreasuries10/tryAqObjBondsCleanPrice_outputs";
    extern const char priceBondOutputFile11[]        ="ETrading/Bonds/USTreasuries11/tryAqObjBondsCleanPrice_outputs";
    extern const char priceBondOutputFile12[]        ="ETrading/Bonds/USTreasuries12/tryAqObjBondsCleanPrice_outputs";
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
    class TestBondPricesUSTreasuries : public TestBondFactory {};


    TEST_F( TestBondPricesUSTreasuries, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData1 )
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

                // Register the Bond in the AQObj object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjBondsCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR1, priceBondOutputFile1, priceTolerance, i );

                // Check Yield Matching
                auto actualYields = validation::tryAqObjBondsYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjBondsCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
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


    TEST_F( TestBondPricesUSTreasuries, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData2 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT2; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile2, i+25 ); // File starts at 25
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile2, i+25 ); // File starts at 25
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQObj object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields  = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjBondsCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR2, priceBondOutputFile2, priceTolerance, i+25 ); // File starts at 25

                // Check Yield Matching
                auto actualYields = validation::tryAqObjBondsYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjBondsCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
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

    TEST_F( TestBondPricesUSTreasuries, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData3 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT3; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile3, i+50 ); // File Starts at 50
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile3, i+50 ); // File Starts at 50
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQObj object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjBondsCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR3, priceBondOutputFile3, priceTolerance, i+50 ); // File Starts at 50

                // Check Yield Matching
                auto actualYields = validation::tryAqObjBondsYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjBondsCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                    EXPECT_NEAR( yields[i], compoundYields[i], yieldTolerance );
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

    TEST_F( TestBondPricesUSTreasuries, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData4 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT4; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile4, i+75 ); // File Starts at 75
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile4, i+75 ); // File Starts at 75
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQObj object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjBondsCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR4, priceBondOutputFile4, priceTolerance, i+75 ); // File Starts at 75

                // Check Yield Matching
                auto actualYields = validation::tryAqObjBondsYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjBondsCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                    EXPECT_NEAR( yields[i], compoundYields[i], yieldTolerance );
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

    TEST_F( TestBondPricesUSTreasuries, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData5 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT5; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile5, i+100 ); // File Starts at 100
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile5, i+100 ); // File Starts at 100
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQObj object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjBondsCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR5, priceBondOutputFile5, priceTolerance, i+100 ); // File Starts at 100

                // Check Yield Matching
                auto actualYields = validation::tryAqObjBondsYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjBondsCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                    EXPECT_NEAR( yields[i], compoundYields[i], yieldTolerance );
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

    TEST_F( TestBondPricesUSTreasuries, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData6 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT6; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile6, i+125 ); // File Starts at 125
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile6, i+125 ); // File Starts at 125
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQObj object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjBondsCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR6, priceBondOutputFile6, priceTolerance, i+125 ); // File Starts at 125

                // Check Yield Matching
                auto actualYields = validation::tryAqObjBondsYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjBondsCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                    EXPECT_NEAR( yields[i], compoundYields[i], yieldTolerance );
                }

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT6 );
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

    TEST_F( TestBondPricesUSTreasuries, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData7 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT7; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile7, i+150 ); // File Starts at 150
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile7, i+150 ); // File Starts at 150
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQObj object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjBondsCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR7, priceBondOutputFile7, priceTolerance, i+150 ); // File Starts at 150

                // Check Yield Matching
                auto actualYields = validation::tryAqObjBondsYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjBondsCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                    EXPECT_NEAR( yields[i], compoundYields[i], yieldTolerance );
                }
             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT7 );
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

    TEST_F( TestBondPricesUSTreasuries, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData8 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT8; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile8, i+175 ); // File Starts at 175
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile8, i+175 ); // File Starts at 175
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQObj object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjBondsCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR8, priceBondOutputFile8, priceTolerance, i+175 ); // File Starts at 175

                // Check Yield Matching
                auto actualYields = validation::tryAqObjBondsYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjBondsCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                    EXPECT_NEAR( yields[i], compoundYields[i], yieldTolerance );
                }

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT8 );
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

    TEST_F( TestBondPricesUSTreasuries, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData9 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT9; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile9, i+200 ); // File Starts at 200
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile9, i+200 ); // File Starts at 200
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQObj object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjBondsCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR9, priceBondOutputFile9, priceTolerance, i+200 ); // File Starts at 200

                // Check Yield Matching
                auto actualYields = validation::tryAqObjBondsYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjBondsCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                    EXPECT_NEAR( yields[i], compoundYields[i], yieldTolerance );
                }

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT9 );
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

    TEST_F( TestBondPricesUSTreasuries, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData10 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT10; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile10, i+225 ); // File Starts at 225
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile10, i+225 ); // File Starts at 225
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQObj object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjBondsCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR10, priceBondOutputFile10, priceTolerance, i+225 ); // File Starts at 225

                // Check Yield Matching
                auto actualYields = validation::tryAqObjBondsYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjBondsCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                    EXPECT_NEAR( yields[i], compoundYields[i], yieldTolerance );
                }

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT10 );
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

    TEST_F( TestBondPricesUSTreasuries, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData11 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT11; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile11, i+250 ); // File Starts at 250
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile11, i+250 ); // File Starts at 250
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQObj object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjBondsCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR11, priceBondOutputFile11, priceTolerance, i+250 ); // File Starts at 250

                // Check Yield Matching
                auto actualYields = validation::tryAqObjBondsYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjBondsCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                    EXPECT_NEAR( yields[i], compoundYields[i], yieldTolerance );
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

    TEST_F( TestBondPricesUSTreasuries, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData12 )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT12; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile12, i+275 ); // File Starts at 275
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile12, i+275 ); // File Starts at 275
                
                // Create the Bond
                etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputFilename.getCString() );

                // Register the Bond in the AQObj object cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
                const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];
                
                // Price the Bond
                auto actualCleanPrice = validation::tryAqObjBondsCleanPrice( bondObjectName, settlementDates, yields );

                // Run the Test - Compare Results
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR12, priceBondOutputFile12, priceTolerance, i+275 ); // File Starts at 275

                // Check Yield Matching
                auto actualYields = validation::tryAqObjBondsYield( bondObjectName, settlementDates, actualCleanPrice );
                auto compoundYields = validation::tryAqObjBondsCompoundYields( bondObjectName, settlementDates, actualCleanPrice );
                for ( size_t i = 0; i != yields.size(); ++i )
                {
                    EXPECT_NEAR( yields[i], actualYields[i], yieldTolerance );
                    EXPECT_NEAR( yields[i], compoundYields[i], yieldTolerance );
                }
             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT12 );
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