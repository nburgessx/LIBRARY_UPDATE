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

#define TEST_DIR1 "ETrading/Bonds/AustraliaType23Govt1/"
#define TEST_DIR2 "ETrading/Bonds/AustraliaType23Govt2/"

namespace
{
    // Number of Test Bonds
	const int TEST_COUNT1 = 37;
	const int TEST_COUNT2 = 37;

    const double priceTolerance = 1e-7;  // low priceTolerance because the yield recorded to file is 12 decimals by default, the actual yield in excel is more than 12 decimals
    const double yieldTolerance = 1e-10;

    // Test Bond Input File(s)
    extern const char createBondInputFile1[]         ="ETrading/Bonds/AustraliaType23Govt1/tryMeLWOBondCreateFromLVB_inputs";
	extern const char priceBondInputFile1[]          ="ETrading/Bonds/AustraliaType23Govt1/tryMeLWOBondPrice_inputs";
    extern const char priceBondOutputFile1[]         ="ETrading/Bonds/AustraliaType23Govt1/tryMeLWOBondPrice_outputs";
	extern const char yieldBondInputFile1[]			 = "ETrading/Bonds/AustraliaType23Govt1/tryMeLWOBondYield_inputs";
	extern const char yieldBondOutputFile1[]		 = "ETrading/Bonds/AustraliaType23Govt1/tryMeLWOBondYield_outputs";

	extern const char createBondInputFile2[]		= "ETrading/Bonds/AustraliaType23Govt2/tryMeLWOBondCreateFromLVB_inputs";
	extern const char priceBondInputFile2[]			= "ETrading/Bonds/AustraliaType23Govt2/tryMeLWOBondPrice_inputs";
	extern const char priceBondOutputFile2[]		= "ETrading/Bonds/AustraliaType23Govt2/tryMeLWOBondPrice_outputs";
	extern const char yieldBondInputFile2[]			= "ETrading/Bonds/AustraliaType23Govt2/tryMeLWOBondYield_inputs";
	extern const char yieldBondOutputFile2[]		= "ETrading/Bonds/AustraliaType23Govt2/tryMeLWOBondYield_outputs";

}

namespace google_test
{
    class TestBondPricesAustraliaType23Govt : public virtual testing::Test, public virtual google_test::InitializeGoogleTest //: public TestBondFactory 
	{
	public:
		etrading::BondPtr createBondFromSingleLVB(const std::string& inputFile)
		{
			// Build the Bond Object
			CreateFixedBond fixedBond;
			etrading::BondPtr myBondPtr = fixedBond.createFixedBondFromInputFileSingleLVB(inputFile);
			return myBondPtr;
		}
	};

    TEST_F( TestBondPricesAustraliaType23Govt, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData1)
    {
        int i = 0;
        try
        {
             for ( i = 0; i < TEST_COUNT1; ++i )
             {
                // Create the Bond and Price filenames
                AQLString bondInputFilename      = CreateDataFile::makeFilename( createBondInputFile1, i );
                
                // Create the Bond
                etrading::BondPtr myBondPtr = createBondFromSingleLVB( bondInputFilename.getCString() );

                // Register the Bond in the LWO Cache
                etrading::registerToCache< etrading::Bond >( myBondPtr );

                // Get the Bond Price Inputs
				AQLString priceInputFilename = CreateDataFile::makeFilename(priceBondInputFile1, i);
				const ReadDataFile::Load priceInputFile( priceInputFilename );
                const std::string bondObjectName = priceInputFile["bondObjectName"];
                
                std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
                std::vector< double > yields = priceInputFile["yields"];

				//1) Check Price Matching
				auto actualPrices = validation::tryAqObjectsBondPrice(bondObjectName, settlementDates, yields);
				CheckTestResultsAndRebaseOnRequest(actualPrices, TEST_DIR1, priceBondOutputFile1, priceTolerance, i );

				//2) Check Yield Matching
				AQLString yieldInputFilename = CreateDataFile::makeFilename(yieldBondInputFile1, i);
				const ReadDataFile::Load yieldInputFile(yieldInputFilename);
				std::vector< double > prices = yieldInputFile["prices"];

				auto actualYields = validation::tryAqObjectsBondYield(bondObjectName, settlementDates, prices);
				auto compoundYields = validation::tryAqObjectsBondCompoundYields(bondObjectName, settlementDates, prices);

				CheckTestResultsAndRebaseOnRequest(actualYields, TEST_DIR1, yieldBondOutputFile1, yieldTolerance, i);
				CheckTestResultsAndRebaseOnRequest(compoundYields, TEST_DIR1, yieldBondOutputFile1, yieldTolerance, i);

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

	TEST_F(TestBondPricesAustraliaType23Govt, SCENARIO_CheckBondPricesAndYieldsMatchLiveMarketData2)
	{
		int i = 0;
		try
		{
			for (i = 0; i < TEST_COUNT2; ++i)
			{
				// Create the Bond and Price filenames
				AQLString bondInputFilename = CreateDataFile::makeFilename(createBondInputFile2, i + TEST_COUNT1);

				// Create the Bond
				etrading::BondPtr myBondPtr = createBondFromSingleLVB(bondInputFilename.getCString());

				// Register the Bond in the LWO Cache
				etrading::registerToCache< etrading::Bond >(myBondPtr);

				// Get the Bond Price Inputs
				AQLString priceInputFilename = CreateDataFile::makeFilename(priceBondInputFile2, i + TEST_COUNT1);
				const ReadDataFile::Load priceInputFile(priceInputFilename);
				const std::string bondObjectName = priceInputFile["bondObjectName"];

				std::vector< AQLDate > settlementDates = priceInputFile["settlementDates"];
				std::vector< double > yields = priceInputFile["yields"];

				//1) Check Price Matching
				auto actualPrices = validation::tryAqObjectsBondPrice(bondObjectName, settlementDates, yields);
				CheckTestResultsAndRebaseOnRequest(actualPrices, TEST_DIR2, priceBondOutputFile2, priceTolerance, i + TEST_COUNT1);

				//2) Check Yield Matching
				AQLString yieldInputFilename = CreateDataFile::makeFilename(yieldBondInputFile2, i + TEST_COUNT1);
				const ReadDataFile::Load yieldInputFile(yieldInputFilename);
				std::vector< double > prices = yieldInputFile["prices"];

				auto actualYields = validation::tryAqObjectsBondYield(bondObjectName, settlementDates, prices);
				auto compoundYields = validation::tryAqObjectsBondCompoundYields(bondObjectName, settlementDates, prices);

				CheckTestResultsAndRebaseOnRequest(actualYields, TEST_DIR2, yieldBondOutputFile2, yieldTolerance, i + TEST_COUNT1);
				CheckTestResultsAndRebaseOnRequest(compoundYields, TEST_DIR2, yieldBondOutputFile2, yieldTolerance, i + TEST_COUNT1);

			}
		}
		catch (const ReadDataFile::LoadError&)
		{
			EXPECT_GE(i, TEST_COUNT2);
		}
		catch (const AQLCoreError& m)
		{
			std::cout << m.getMsg();
			ASSERT_FALSE(true);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
			ASSERT_FALSE(true);
		}
	}


};