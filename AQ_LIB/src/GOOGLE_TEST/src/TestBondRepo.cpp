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

#include "Dependency.h"
#include "ReadDataFile.h"
#include "GetGoogleTestFolder.h"
#include "ResultsProcessor.h"

#include "BondYields.h"
#include "BondFactory.h"
#include "ObjectUtilities.h"
#include "CreateFixedBond.h"

#include "tryMeLWO.h"
#include "tryMeLWOAssetSwap.h"
#include "tryMeLWOBond.h"

#include "CreateDataFile.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

namespace
{
    const double priceTolerance = 1e-10;
	const double yieldTolerance = 1e-10;

	const std::string TEST_DIR = "ETrading/Bonds/Repo/";
	
	const std::string bondObject_input1 = TEST_DIR + "DE0001102424.JSON";
	const std::string bondObject_input2 = TEST_DIR + "DE0001102440.JSON";

	const std::string forwardPrice_input = TEST_DIR + "DE0001102424_tryMeLWOBondForwardPrice_inputs";
	const std::string forwardPrice_output = TEST_DIR + "DE0001102424_tryMeLWOBondForwardPrice_outputs";

	const std::string repoRateFromForwardPrice_input = TEST_DIR + "DE0001102424_tryMeLWOBondRepoRate_inputs";
	const std::string repoRateFromForwardPrice_output = TEST_DIR + "DE0001102424_tryMeLWOBondRepoRate_outputs";

	const std::string futurePrice_input = TEST_DIR + "DE0001102424_tryMeLWOBondFuturePrice_inputs";
	const std::string futurePrice_output = TEST_DIR + "DE0001102424_tryMeLWOBondFuturePrice_outputs";

	const std::string impliedRate_input = TEST_DIR + "DE0001102424_tryMeLWOBondRepoRateFromFuture_inputs";
	const std::string impliedRate_output = TEST_DIR + "DE0001102424_tryMeLWOBondRepoRateFromFuture_outputs";

	const std::string conversionFactor_input = TEST_DIR + "DE0001102424_tryMeLWOBondConversionFactor_inputs";
	const std::string conversionFactor_output = TEST_DIR + "DE0001102424_tryMeLWOBondConversionFactor_outputs";

	const std::string grossBasis_input = TEST_DIR + "DE0001102424_tryMeLWOBondGrossBasis_inputs";
	const std::string grossBasis_output = TEST_DIR + "DE0001102424_tryMeLWOBondGrossBasis_outputs";

	const std::string netBasis_input = TEST_DIR + "DE0001102424_tryMeLWOBondNetBasis_inputs";
	const std::string netBasis_output = TEST_DIR + "DE0001102424_tryMeLWOBondNetBasis_outputs";

	const std::string cheapestToDeliver_input = TEST_DIR + "tryMeLWOBondCheapestToDeliver_inputs";
	const std::string cheapestToDeliver_output = TEST_DIR + "tryMeLWOBondCheapestToDeliver_outputs";

	const std::string cheapestToDeliverByNetBasis_input = TEST_DIR + "tryMeLWOBondCheapestToDeliverByNetBasis_inputs";
	const std::string cheapestToDeliverByNetBasis_output = TEST_DIR + "tryMeLWOBondCheapestToDeliverByNetBasis_inputs_outputs";

}

namespace google_test
{

	// This test uses German Bond Future as an example to show the functions related to repo are working. 
	// For German Bond Future "RXU8 Comdty", it has two underlying bonds:  DE0001102424, DE0001102440 . 
	// Two bond objects are created for cheapest to deliver pricing, and the first bond DE0001102424 is used to test other pricing functions 

	// Declare Test Fixture Class
	DECLARE_TEST_FIXTURE(TestBondRepo)

	// Call Test Fixture Class
	TEST_F(TestBondRepo, SNAPSHOT_ForwardPrice)
	{
		// Load Bond
		auto loadBond = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondObject_input1, etrading::JSON);

		const ReadDataFile::Load inputFile(forwardPrice_input.c_str());

		auto actualResult = validation_api::tryMeLWOBondForwardPrice(inputFile["bondObjectName"], inputFile["settleDate"], inputFile["forwardSettleDate"], inputFile["price"], inputFile["repoRate"], inputFile["repoDayCount"]);

		CheckTestResultsAndRebaseOnRequest(actualResult, TEST_DIR.c_str(), forwardPrice_output.c_str(), priceTolerance);

	}

	TEST_F(TestBondRepo, SNAPSHOT_RepoRate)
	{
		// Load Bond
		auto loadBond = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondObject_input1, etrading::JSON);

		const ReadDataFile::Load inputFile(repoRateFromForwardPrice_input.c_str());

		auto actualResult = validation_api::tryMeLWOBondRepoRate(inputFile["bondObjectName"], inputFile["settleDate"], inputFile["forwardSettleDate"], inputFile["price"], inputFile["forwardPrice"], inputFile["repoDayCount"]);

		CheckTestResultsAndRebaseOnRequest(actualResult, TEST_DIR.c_str(), repoRateFromForwardPrice_output.c_str(), yieldTolerance);

	}

	TEST_F(TestBondRepo, SNAPSHOT_FuturePrice)
	{
		// Load Bond
		auto loadBond = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondObject_input1, etrading::JSON);

		const ReadDataFile::Load inputFile(futurePrice_input.c_str());

		auto actualResult = validation_api::tryMeLWOBondFuturePrice(inputFile["bondObjectName"], inputFile["settleDate"], inputFile["deliveryDate"], inputFile["bondPrice"], inputFile["repoRate"], inputFile["repoDayCount"], inputFile["conversionFactor"]);

		CheckTestResultsAndRebaseOnRequest(actualResult, TEST_DIR.c_str(), futurePrice_output.c_str(), priceTolerance);

	}

	TEST_F(TestBondRepo, SNAPSHOT_ImpliedRepo)
	{
		// Load Bond
		auto loadBond = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondObject_input1, etrading::JSON);

		const ReadDataFile::Load inputFile(impliedRate_input.c_str());

		auto actualResult = validation_api::tryMeLWOBondRepoRateFromFuture(inputFile["bondObjectName"], inputFile["settleDate"], inputFile["deliveryDate"], inputFile["price"], inputFile["futurePrice"], inputFile["conversionFactor"], inputFile["repoDayCount"]);

		CheckTestResultsAndRebaseOnRequest(actualResult, TEST_DIR.c_str(), impliedRate_output.c_str(), yieldTolerance);

	}

	TEST_F(TestBondRepo, SNAPSHOT_ConversionFactor)
	{
		// Load Bond
		auto loadBond = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondObject_input1, etrading::JSON);

		const ReadDataFile::Load inputFile(conversionFactor_input.c_str());

		auto actualResult = validation_api::tryMeLWOBondConversionFactor(inputFile["bondObjectName"], inputFile["firstFutureSettleDate"], inputFile["notionalBondCouponRate"]);

		CheckTestResultsAndRebaseOnRequest(actualResult, TEST_DIR.c_str(), conversionFactor_output.c_str(), yieldTolerance);

	}

	TEST_F(TestBondRepo, SNAPSHOT_GrossBasis)
	{
		// Load Bond
		auto loadBond = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondObject_input1, etrading::JSON);

		const ReadDataFile::Load inputFile(grossBasis_input.c_str());

		auto actualResult = validation_api::tryMeLWOBondGrossBasis(inputFile["bondObjectName"], inputFile["settleDate"], inputFile["price"], inputFile["futurePrice"], inputFile["conversionFactor"]);

		CheckTestResultsAndRebaseOnRequest(actualResult, TEST_DIR.c_str(), grossBasis_output.c_str(), yieldTolerance);

	}

	TEST_F(TestBondRepo, SNAPSHOT_NetBasis)
	{
		// Load Bond
		auto loadBond = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondObject_input1, etrading::JSON);

		const ReadDataFile::Load inputFile(netBasis_input.c_str());

		auto actualResult = validation_api::tryMeLWOBondNetBasis(inputFile["bondObjectName"], inputFile["settleDate"], inputFile["deliveryDate"], inputFile["bondPrice"], inputFile["actualRepoRate"], inputFile["repoDayCount"], inputFile["futurePrice"], inputFile["conversionFactor"]);

		CheckTestResultsAndRebaseOnRequest(actualResult, TEST_DIR.c_str(), netBasis_output.c_str(), yieldTolerance);
	}

	TEST_F(TestBondRepo, SNAPSHOT_CheapestToDeliver)
	{
		// Load Bonds
		validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondObject_input1, etrading::JSON);
		validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondObject_input2, etrading::JSON);

		const ReadDataFile::Load inputFile(cheapestToDeliver_input.c_str());

		auto actualResult = validation_api::tryMeLWOBondCheapestToDeliver(inputFile["futurePrice"], inputFile["settleDate"], inputFile["deliveryDate"], inputFile["repoDayCount"], inputFile["bondObjectNames"], inputFile["bondPrices"], inputFile["conversionFactors"]);

		const ReadDataFile::Load outputFile(cheapestToDeliver_output.c_str());

		const std::string expectedResult = outputFile["output"];

		EXPECT_TRUE(actualResult == expectedResult);
	}

	TEST_F(TestBondRepo, SNAPSHOT_CheapestToDeliverByNetBasis)
	{
		// Load Bonds
		validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondObject_input1, etrading::JSON);
		validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + bondObject_input2, etrading::JSON);

		const ReadDataFile::Load inputFile(cheapestToDeliverByNetBasis_input.c_str());

		auto actualResult = validation_api::tryMeLWOBondCheapestToDeliverByNetBasis(inputFile["futurePrice"], inputFile["settleDate"], inputFile["deliveryDate"], inputFile["repoDayCount"], inputFile["bondObjectNames"], inputFile["bondPrices"], inputFile["conversionFactors"], inputFile["actualRepoRates"]);

		const ReadDataFile::Load outputFile(cheapestToDeliver_output.c_str());

		const std::string expectedResult = outputFile["output"];

		EXPECT_TRUE(actualResult == expectedResult);
	}

};