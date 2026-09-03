// Include: Google Test Library
#include <gTest/gTest.h>
#include "InitializeAQGoogleTest.h"

#include "Dependency.h"
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"
#include "GetGoogleTestFolder.h"

#include "tryMeLWOBond.h"
#include "tryMeLWO.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

namespace
{
    // Number of Test Bonds
	const int TEST_COUNT = 43;

    const double priceTolerance = 1e-7;  // low priceTolerance because the yield recorded to file is 12 decimals by default, the actual yield in excel is more than 12 decimals
    const double yieldTolerance = 1e-10;

    // Test Bond Input File(s)

	const std::string TEST_DIR = "ETrading/Bonds/UKGilt/";

	const std::string bondFile = TEST_DIR + "BOND";

	const std::string priceFunctionInput     = "tryMeLWOBondPrice_inputs";
    const std::string priceFunctionOutput    = "tryMeLWOBondPrice_outputs";
	const std::string yieldFunctionInput	 = "tryMeLWOBondYield_inputs";
	const std::string yieldfunctionOuput	 = "tryMeLWOBondYield_outputs";

}

namespace google_test
{

	// Declare Test Fixture Class
	DECLARE_TEST_FIXTURE(TestBondPricesUKGilt)

    TEST_F(TestBondPricesUKGilt, SCENARIO_CheckBondPricesMatchLiveMarketData)
    {
        int i = 1;
        try
        {
             for ( i = 1; i <= TEST_COUNT; ++i )
             {
				 // Create the Bond and Price filenames
				 const std::string bondFileName = bondFile + std::to_string(static_cast<long long> (i) );
				 const std::string bondInputFilename = bondFileName + ".JSON";

				 const std::string priceInputFilename = bondFileName + "_" + priceFunctionInput + ".csv";
				 const std::string priceOutputFilename = bondFileName + "_" + priceFunctionOutput + ".csv";

				// Load Bond
				auto loadBond = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondInputFilename, etrading::JSON);

				//1) Check Price Matching
				const ReadDataFile::Load priceInputFile(priceInputFilename.c_str());

				auto actualPrices = validation_api::tryMeLWOBondPrice(priceInputFile["bondObjectName"], priceInputFile["settlementDates"], priceInputFile["yields"]);
				CheckTestResultsAndRebaseOnRequest(actualPrices, TEST_DIR.c_str(), priceOutputFilename.c_str(), priceTolerance);

             }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT );
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

	TEST_F(TestBondPricesUKGilt, SCENARIO_CheckBondYieldsMatchLiveMarketData)
	{
		int i = 1;
		try
		{
			for (i = 1; i <= TEST_COUNT; ++i)
			{
				// Create the Bond and Price filenames
				const std::string bondFileName = bondFile + std::to_string(static_cast<long long>(i) );
				const std::string bondInputFilename = bondFileName + ".JSON";

				const std::string yieldInputFilename = bondFileName + "_" + yieldFunctionInput + ".csv";
				const std::string yieldOutputFilename = bondFileName + "_" + yieldfunctionOuput + ".csv";

				// Load Bond
				auto loadBond = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondInputFilename, etrading::JSON);

				//2) Check Yield Matching
				const ReadDataFile::Load yieldInputFile(yieldInputFilename.c_str());
				const std::string bondObjectName = yieldInputFile["bondObjectName"];
				std::vector< LADate > settlementDates = yieldInputFile["settlementDates"];
				std::vector< double > prices = yieldInputFile["prices"];

				auto actualYields = validation_api::tryMeLWOBondYield(bondObjectName, settlementDates, prices);
				auto compoundYields = validation_api::tryMeLWOBondCompoundYields(bondObjectName, settlementDates, prices);

				CheckTestResultsAndRebaseOnRequest(actualYields, TEST_DIR.c_str(), yieldOutputFilename.c_str(), yieldTolerance);
				CheckTestResultsAndRebaseOnRequest(compoundYields, TEST_DIR.c_str(), yieldOutputFilename.c_str(), yieldTolerance);

			}
		}
		catch (const ReadDataFile::LoadError&)
		{
			EXPECT_GE(i, TEST_COUNT);
		}
		catch (const LACoreError& m)
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