// Curves
#include "tryMeLWOCurveMarketData.h"
#include "tryMeLWOCurveDualBootstrap.h"

// Swap Creation and Pricing
#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"

// Risk calculation
#include "tryMeLWOSwapDelta.h"
#include "tryMeLWOFixingTable.h"

// Test Infrastructure
#include "Dependency.h"   // Curve Macros are Here !!!
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"
#include "tryMeUtilitySetup.h"
#include <gTest/gTest.h>

#include "BuildMarketDataObjectFromFile.h"
#include "BuildSwapTradeFromGenerator.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

// Define the Test Input Folder Here
#define TEST_DIR "ETrading/LWObjects/TestLWOSwapDeltaDualBootstrap/"

namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 50.0; // Notional of test trades is 100MM and delta by bumping is inherently noisy
	
	unsigned int EUR_TEST_COUNT = 1;
	unsigned int USD_TEST_COUNT = 1;

	// Swap definitions
	extern const char EUR_FIXEDFLOATSWAP_1[] = TEST_DIR "EUR_FixedFloatSwapLVB_1.csv";
	extern const char EUR_FIXEDFLOATSWAP_2[] = TEST_DIR "EUR_FixedFloatSwapLVB_2.csv";
	extern const char EUR_FIXEDFLOATSWAP_3[] = TEST_DIR "EUR_FixedFloatSwapLVB_3.csv";

	extern const char USD_FIXEDFLOATSWAP_1[] = TEST_DIR "USD_FixedFloatSwapLVB_1.csv";
	extern const char USD_FIXEDFLOATSWAP_2[] = TEST_DIR "USD_FixedFloatSwapLVB_2.csv";
	extern const char USD_FIXEDFLOATSWAP_3[] = TEST_DIR "USD_FixedFloatSwapLVB_3.csv";

	// Result paths
	extern const char PORTFOLIO_DELTA_LADDER_INPUTS[] = "PortfolioDeltaLadder_inputs";
	extern const char PORTFOLIO_DELTA_LADDER_OUTPUTS_32BIT[] = "PortfolioDeltaLadder_outputs_32bits";
	extern const char PORTFOLIO_DELTA_LADDER_OUTPUTS_64BIT[] = "PortfolioDeltaLadder_outputs_64bits";
	
	void buildDualBootstrapCurves(const LAString& ccy, const LAString& prefix, const std::string& stdGenerator, const std::string& oisGenerator, const std::string& curveCollection)
	{
		LAString oisCurveMarketDataFile = TEST_DIR;
		oisCurveMarketDataFile += prefix + LAString("OIS_MARKETDATA");
		LAString stdCurveMarketDataFile = TEST_DIR;
		stdCurveMarketDataFile += prefix + LAString("STD_MARKETDATA");

		std::string oisCurveMarketObjectHandle = google_test::createLWOMarketDataObjectFromFileName(oisCurveMarketDataFile);
		std::string stdCurveMarketObjectHandle = google_test::createLWOMarketDataObjectFromFileName(stdCurveMarketDataFile);

		validation::tryMeLWOCurveDualBootstrap("", curveCollection, stdGenerator, oisGenerator, stdCurveMarketObjectHandle, oisCurveMarketObjectHandle);
	}

}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestLWOSwapDeltaDualBootstrap);
	

	// This test checks the delta ladder profile of 3 EUR vanilla swaps
	TEST_F(TestLWOSwapDeltaDualBootstrap, SNAPSHOT_EUR_SWAP_IR_DELTA)
	{
		// Load all the swaps 
		createSwapFromDataFile(EUR_FIXEDFLOATSWAP_1);
		createSwapFromDataFile(EUR_FIXEDFLOATSWAP_2);
		createSwapFromDataFile(EUR_FIXEDFLOATSWAP_3);

		LAString ccy = "EUR";

		for (unsigned int i = 0; i < EUR_TEST_COUNT; ++i)
		{
			//------------------------------------------------
			// 1. Build curves
			LAString prefix = ccy + LAString("_") + LAString(static_cast<int>(i + 1)) + LAString("_");
			buildDualBootstrapCurves(ccy, prefix, "EUR_SWAP_3M", "EUR_OIS", "EURYC");

			// Load inputs to meLWOSwapDDeltaLadder
			LAString deltaLadderInputDir = TEST_DIR;
			deltaLadderInputDir += prefix + PORTFOLIO_DELTA_LADDER_INPUTS;
			const ReadDataFile::Load deltaLadderInputs(deltaLadderInputDir);

			//------------------------------------------------
			// 2. Calculate deltas
			LAStringVector headers;
			LAStringVector pillarNames;
			DoubleMatrix deltas;

			LAStringVector swapNames				= deltaLadderInputs["swapNames"];
			LAStringMatrix curveCollectionNames	= deltaLadderInputs["curveCollectionNames"];
			LAStringMatrix fixingTableNames		= deltaLadderInputs["fixingTableNames"];
			bool bumpSpreadInstruments			= deltaLadderInputs["bumpSpreadInstruments"];
			double bumpSize						= deltaLadderInputs["bumpSize"];
			LAString bumpMode					= deltaLadderInputs["bumpMode"];
			bool aggregateRisks					= deltaLadderInputs["aggregateRisks"];
			bool reportInLegCCY					= deltaLadderInputs["reportInLegCCY"];
			std::string riskCutOffTenor			= deltaLadderInputs["riskCutOffTenor"];

            // Dummy Xccy FX Spot Rates
            DoubleVector dummyXccyFXSpotRates( swapNames.size(), 1.0 );

			validation::tryMeLWOSwapDeltaLadder(headers,
													pillarNames,
													deltas,
													swapNames,
													curveCollectionNames,
													fixingTableNames,
													bumpSpreadInstruments,
													bumpSize,
													bumpMode,
													aggregateRisks,
													reportInLegCCY,
													riskCutOffTenor,
                                                    dummyXccyFXSpotRates );

			//------------------------------------------------
			// 3. Check risk results
			LAString outputFile_32bit = prefix + PORTFOLIO_DELTA_LADDER_OUTPUTS_32BIT;
			LAString outputFile_64bit = prefix + PORTFOLIO_DELTA_LADDER_OUTPUTS_64BIT;
			verifyDeltaBucketAmounts(pillarNames, headers, deltas, tolerance, TEST_DIR, outputFile_32bit, outputFile_64bit);

			//------------------------------------------------
			// 4. Flush the curve curves
			validation::tryMeUtilityClearEntityPool();
		}		
	}

	// This test checks the delta ladder profile of 3 USD vanilla swaps
	TEST_F(TestLWOSwapDeltaDualBootstrap, SNAPSHOT_USD_SWAP_IR_DELTA)
	{
		// Load all the swaps 
		createSwapFromDataFile(USD_FIXEDFLOATSWAP_1);
		createSwapFromDataFile(USD_FIXEDFLOATSWAP_2);
		createSwapFromDataFile(USD_FIXEDFLOATSWAP_3);

		LAString ccy = "USD";

		for (unsigned int i = 0; i < USD_TEST_COUNT; ++i)
		{
			//------------------------------------------------
			// 1. Build curves
			LAString prefix = ccy + LAString("_") + LAString(static_cast<int>(i + 1)) + LAString("_");
			buildDualBootstrapCurves(ccy, prefix, "USD_SWAP_3M", "USD_OIS", "USDYC");

			// Load inputs to meLWOSwapDDeltaLadder
			LAString deltaLadderInputDir = TEST_DIR;
			deltaLadderInputDir += prefix + PORTFOLIO_DELTA_LADDER_INPUTS;
			const ReadDataFile::Load deltaLadderInputs(deltaLadderInputDir);

			//------------------------------------------------
			// 2. Calculate deltas
			LAStringVector headers;
			LAStringVector pillarNames;
			DoubleMatrix deltas;

			LAStringVector swapNames              = deltaLadderInputs["swapNames"];
			LAStringMatrix curveCollectionNames   = deltaLadderInputs["curveCollectionNames"];
			LAStringMatrix fixingTableNames       = deltaLadderInputs["fixingTableNames"];
			bool bumpSpreadInstruments          = deltaLadderInputs["bumpSpreadInstruments"];
			double bumpSize                     = deltaLadderInputs["bumpSize"];
			LAString bumpMode                   = deltaLadderInputs["bumpMode"];
			bool aggregateRisks                 = deltaLadderInputs["aggregateRisks"];
			bool reportInLegCCY                 = deltaLadderInputs["reportInLegCCY"];
			std::string riskCutOffTenor         = deltaLadderInputs["riskCutOffTenor"];

            // Dummy Xccy FX Spot Rates
            DoubleVector dummyXccyFXSpotRates( swapNames.size(), 1.0 );
			
            validation::tryMeLWOSwapDeltaLadder(headers,
													pillarNames,
													deltas,
													swapNames,
													curveCollectionNames,
													fixingTableNames,
													bumpSpreadInstruments,
													bumpSize,
													bumpMode,
													aggregateRisks,
													reportInLegCCY,
													riskCutOffTenor,
                                                    dummyXccyFXSpotRates );

			//------------------------------------------------
			// 3. Check risk results
			LAString outputFile_32bit = prefix + PORTFOLIO_DELTA_LADDER_OUTPUTS_32BIT;
			LAString outputFile_64bit = prefix + PORTFOLIO_DELTA_LADDER_OUTPUTS_64BIT;
			verifyDeltaBucketAmounts(pillarNames, headers, deltas, tolerance, TEST_DIR, outputFile_32bit, outputFile_64bit);

			//------------------------------------------------
			// 4. Flush the object pool cache
			validation::tryMeUtilityClearEntityPool();
		}


	}
}
