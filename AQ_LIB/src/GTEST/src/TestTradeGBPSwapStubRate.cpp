#include <gTest/gTest.h>

#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"
#include "tryAqSwapsStubRate.h"

#include "tryAqObjects.h"
#include "AQOUtilities.h"
#include "tryAqObjSwapsPricing.h"
#include <boost/range/irange.hpp>
#include "AQLDateScheduleHelpers.h"
#include "GetGoogleTestFolder.h"


using etrading::ReadDataFile;
using etrading::CreateDataFile;

// test tolerance
const double tolerance = 1e-8;	

#define TEST_DIR "ETrading/Trades/TestTradeGBPSwapStubRate/"

namespace
{
	extern const char GBP_OIS[] = TEST_DIR "GBPLIVE_OIS_tryAqCurvesCalibrateOIS_inputs";
	extern const char GBP_1M[] = "";
	extern const char GBP_3M[] = TEST_DIR "GBPLIVE_STD_tryAqCurvesCalibrateSwap_inputs";
	extern const char GBP_6M[] = "";
	extern const char GBP_12M[] = "";

	extern const char fileStubRateInput[] = TEST_DIR "tryAqSwapsStubRate_inputs";
	extern const char fileStubRateOutput[] = TEST_DIR "tryAqSwapsStubRate_outputs";

	const std::string googleTestFolder = etrading::getGoogleTestFolder();
	const std::string FixingTableObject = googleTestFolder + TEST_DIR + "GBPFIX_3M.JSON";
	const std::string SwapObject = googleTestFolder + TEST_DIR + "MySwap.JSON";
}

namespace google_test
{

	//Note that "GBP_3M" is the standard curve in the test
	AQL_BUILD_GBP_CURVE(TestTradeGBPSwapStubRate, GBP_OIS, GBP_1M, GBP_6M, GBP_3M, GBP_12M);

	TEST_F(TestTradeGBPSwapStubRate, SNAPSHOT_tryMe_stubRateFromFixingTable )
    {
		try
		{
			const ReadDataFile::Load inputFile(fileStubRateInput);

			AQLStringVector curveIndices = inputFile["curveIndices"];
			AQLStringVector curveTenors = inputFile["curveTenors"];
			DoubleVector tenorCurveFixings = inputFile["tenorCurveFixings"];
			AQLStringMatrix swapLVB = inputFile["swapLVB"];

			double stubRate = validation::tryAqSwapsStubRate(swapLVB, curveIndices, curveTenors, tenorCurveFixings, true);

			CheckTestResultsAndRebaseOnRequest(stubRate, TEST_DIR, fileStubRateOutput, tolerance);

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


	TEST_F(TestTradeGBPSwapStubRate, CONSISTENCY_AQOSwap_subRateFromFixingTable)
	{
		try
		{
			//1) Get fixing rate from Fixing Table object
			const std::string fixingTableName = validation::tryAqObjLoad(FixingTableObject, etrading::JSON);
			etrading::DataProvider dataProvider(etrading::ValuationSettings(etrading::fromStringToLVB("GBPLIVE"), etrading::fromStringToLVB(fixingTableName), ""));
			auto fixingTable = etrading::getFixingTable(fixingTableName, false /* do not throw when missing*/);
			AQLDate fixingDate("20200309", "YYYYMMDD");
			double expectedFixingRate = fixingTable->getFixingValue(etrading::toGregorianDateFromAQLDate(fixingDate));

			//2) Get fixing rate from Swap object
			const std::string swapName = validation::tryAqObjLoad(SwapObject, etrading::JSON);

			std::vector<std::string> columnList = { "FixingDate", "FloatRate" };

			AnyTypeMatrix fixingDateAndFixingRates = validation::tryAqObjSwapsDisplayCashflows(swapName, etrading::fromStringToLVB("GBPLIVE"), AQLString("leg2:float"), etrading::fromStringToLVB(fixingTableName), false, columnList).front();

			double fixingRateFromSwap = 0.0;
			for (auto it : fixingDateAndFixingRates)
			{
				auto fixingDt = it[0];
				auto rate = it[1];

				if (etrading::AQLDateScheduleHelpers::getAQLDate(boost::get<double>(it[0])) == fixingDate)
				{
					fixingRateFromSwap = boost::get<double>(it[1]);
					break;
				}
			}

			EXPECT_NEAR(fixingRateFromSwap, expectedFixingRate, tolerance);

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


}
