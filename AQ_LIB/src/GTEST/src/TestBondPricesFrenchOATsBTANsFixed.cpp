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

#define TEST_DIR "ETrading/Bonds/FrenchOATsBTANsFixed/"

namespace
{
    // Number of Test Bonds
    const int TEST_COUNT = 83;

    const double priceTolerance =1e-7;  // low priceTolerance because the yield recorded to file is 12 decimals by default, the actual yield in excel is more than 12 decimals
    const double yieldTolerance = 1e-10;

    // Test Bond Input File(s)
    extern const char createBondInputFile[]         =TEST_DIR "tryAqObjBondsCreate_inputs";
    extern const char priceBondInputFile[]          =TEST_DIR "tryAqObjBondsCleanPrice_inputs";
    extern const char priceBondOutputFile[]         =TEST_DIR "tryAqObjBondsCleanPrice_outputs";
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
    class TestBondPricesFrenchOATsBTANsFixed : public TestBondFactory {};


    TEST_F( TestBondPricesFrenchOATsBTANsFixed, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData )
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile, i );
                AQLString priceInputFilename     = CreateDataFile::makeFilename( priceBondInputFile, i );
                
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
                CheckTestResultsAndRebaseOnRequest( actualCleanPrice, TEST_DIR, priceBondOutputFile, priceTolerance, i );

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
            EXPECT_GE( i, TEST_COUNT );
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