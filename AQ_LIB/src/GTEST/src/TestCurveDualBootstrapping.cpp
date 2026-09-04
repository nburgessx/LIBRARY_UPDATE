#include "Variant.h"
#include "LabelValueBlock.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "TestHelperUtilities.h"
#include "InitializeETrading.h"
#include "tryAqToolsSetup.h"
#include "ContainerUtilities.h"
#include "CoreEnumerations.h"
#include "ResultsProcessor.h"
#include "BuildMarketDataObjectFromFile.h"

#include <sstream>
#include <boost/range/irange.hpp>

#include "CurveOis.h"

// "Generator" API
#include "tryAqObjCurvesMarketData.h"
#include "tryAqObjCurvesDualBootstrap.h"

// Forward Rates Table
#include "tryAqCurvesForwardRate.h"

using etrading::ReadDataFile;

// Define the Test Input Folder Here
#define TEST_DIR			  "ETrading/Curves/TestCurveDualBootstrapping/"


namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 1e-8;

	// Test count
	const size_t EUR_TEST_COUNT = 2;
	const size_t USD_TEST_COUNT = 1;

    //
    // curve input files
    //
	
    // NEW EUR Tests
    // -------------
	extern const char FORWARD_RATES_INPUTS[] = "ForwardRates_Inputs";
	extern const char FORWARD_RATES_OUTPUTS_32bit[] = "ForwardRates_Outputs_32bit";
	extern const char FORWARD_RATES_OUTPUTS_64bit[] = "ForwardRates_Outputs_64bit";
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestCurveDualBootstrapping);
		
	void testForwardRates(const AQLString& curveName, const AQLString& prefix)
	{
		AQLString oisForwardRateInputFile = TEST_DIR;
		oisForwardRateInputFile += prefix + AQLString("_") + curveName + AQLString("_") + FORWARD_RATES_INPUTS;

		const ReadDataFile::Load inputFile(oisForwardRateInputFile);
		const DateVector fromDateVector = inputFile["fromDates"];
		const DateVector toDateVector = inputFile["toDates"];
		const DoubleArray results = validation::tryAqCurvesForwardRatesFromForwardDates(fromDateVector,
			toDateVector,
			inputFile["curveCollection"],
			inputFile["curveIndex"],
			inputFile["fwdInter"],
			inputFile["businessDayAdjust"]);

		ASSERT_EQ(fromDateVector.size(), results.size())
			<< "Results size should match the number of forward rates requested" << std::endl;

#if defined(GTEST32)
		AQLString outputFile = TEST_DIR;
		outputFile += prefix + AQLString("_") + curveName + AQLString("_") + FORWARD_RATES_OUTPUTS_32bit;
		CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, outputFile, tolerance);
#else
		AQLString outputFile = TEST_DIR;
		outputFile += prefix + AQLString("_") + curveName + AQLString("_") + FORWARD_RATES_OUTPUTS_64bit;
		CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, outputFile, tolerance);
#endif
	}

	void testForwardRatesFromDualBootstrappedCurves(const AQLString& ccy, const std::string& stdGenerator, const std::string& oisGenerator, const std::string& curveCollection, unsigned int testCount)
	{
		for (size_t i = 0; i < testCount; ++i)
		{
			//----------------------------------------------------------------------------------------
			// Build yield curves of the current test case

			AQLString prefix = ccy + AQLString("_") + AQLString(static_cast<int>(i + 1));

			AQLString oisCurveMarketDataFile = TEST_DIR;
			oisCurveMarketDataFile += prefix + AQLString("_OIS_MARKETDATA");
			AQLString stdCurveMarketDataFile = TEST_DIR;
			stdCurveMarketDataFile += prefix + AQLString("_STD_MARKETDATA");

			std::string oisCurveMarketObjectHandle = google_test::createAQOMarketDataObjectFromFileName(oisCurveMarketDataFile);
			std::string stdCurveMarketObjectHandle = google_test::createAQOMarketDataObjectFromFileName(stdCurveMarketDataFile);

			validation::tryAqObjCurvesDualBootstrap("", curveCollection, stdGenerator, oisGenerator, stdCurveMarketObjectHandle, oisCurveMarketObjectHandle);

			//----------------------------------------------------------------------------------------
			// Test forward rates for the OIS curve and the STD curve
			testForwardRates("OIS", prefix);
			testForwardRates("STD", prefix);

			//----------------------------------------------------------------------------------------
			// Flush the cache in preparation for a new set of curves
			validation::tryAqToolsClearEntityPool();
		}
	}


	TEST_F(TestCurveDualBootstrapping, SNAPSHOT_EUR_FORWARD_RATE_TESTS)
	{
		unsigned int testCount = EUR_TEST_COUNT;
		std::string oisGenerator = "EUR_OIS_LOB_3Y";
		std::string stdGenerator = "EUR_SWAP_3M";
		std::string curveCollection = "EURYC";

		testForwardRatesFromDualBootstrappedCurves("EUR", stdGenerator, oisGenerator, curveCollection, testCount);
	}

	TEST_F(TestCurveDualBootstrapping, SNAPSHOT_USD_FORWARD_RATE_TESTS)
	{
		unsigned int testCount = USD_TEST_COUNT;
		std::string oisGenerator = "USD_OIS";
		std::string stdGenerator = "USD_SWAP_3M";
		std::string curveCollection = "USDYC";

		testForwardRatesFromDualBootstrappedCurves("USD", stdGenerator, oisGenerator, curveCollection, testCount);
	}
}
