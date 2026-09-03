/*
 * @brief			Snapshot tests on the swap delta ladder - These test ARR SOFR risk
 * @Created:		9 Apr 2020
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


// Curves
#include "tryMeLWO.h"

// Risk calculation
#include "tryMeLWOSwapDelta.h"
#include "tryMeLWOFixingTable.h"

// Test Infrastructure
#include "Dependency.h"   // Curve Macros are Here !!!
#include "GetGoogleTestFolder.h"
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"
#include <gTest/gTest.h>

using etrading::ReadDataFile;
using etrading::CreateDataFile;


namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 2e-2; // Notional of test trades is 1MM and delta by bumping is inherently noisy

	// Test Input File(s) 
	const std::string TEST_DIR = "ETrading/LWObjects/TestLWOSwapDelta_ARR/";

    //
    // curve input files
    //
	const std::string fileName_USDYC_SOFR				= TEST_DIR + "CURVE_USDSOFR@3.JSON";
    const std::string fileName_USDYC_OIS				= TEST_DIR + "CURVE_USDOIS@6.JSON";

	// Swap Generators
	const std::string fileName_USD_SOFR_SWAP			= TEST_DIR + "USD_SOFR_SWAP@9.JSON";
	const std::string fileName_USD_OIS_SWAP				= TEST_DIR + "USD_OIS_SWAP@8.JSON";

	// Swap Trades
	const std::string fileName_USD_SOFR_SWAP_5Y			= TEST_DIR + "USD_SOFR_SWAP_5Y@15.JSON";
	const std::string fileName_USD_OIS_SWAP_5Y			= TEST_DIR + "USD_OIS_SWAP_5Y@19.JSON";

	// Risk Inputs
	const std::string fileName_deltaLadder_SOFR_Inputs	= TEST_DIR + "tryMeLWOSwapDeltaLadder_SOFR_inputs.csv";
	const std::string fileName_deltaLadder_OIS_Inputs	= TEST_DIR + "tryMeLWOSwapDeltaLadder_OIS_inputs.csv";

	// Risk Outputs
	const std::string fileName_raw_delta_ladder_SOFR_outputs_32	= TEST_DIR + "tryMeLWOSwapDeltaLadder_SOFR_outputs_32.csv";
	const std::string fileName_raw_delta_ladder_SOFR_outputs_64 = TEST_DIR + "tryMeLWOSwapDeltaLadder_SOFR_outputs_64.csv";

	const std::string fileName_raw_delta_ladder_OIS_outputs_32	= TEST_DIR + "tryMeLWOSwapDeltaLadder_OIS_outputs_32.csv";
	const std::string fileName_raw_delta_ladder_OIS_outputs_64  = TEST_DIR + "tryMeLWOSwapDeltaLadder_OIS_outputs_64.csv";

	// Helper function

	void checkDeltaResultsAndRebaseIfRequired(  const LAStringVector& pillarNames,
												const LAStringVector& headers,
												const DoubleMatrix& deltas,
												const LAString& baseline_output_32,
												const LAString& baseline_output_64 )
	{
		if (etrading::CreateDataFile::rebaseResultsEnabled())
		{
#ifdef GTEST32
			LAString outputFileName = baseline_output_32;
#else
			LAString outputFileName = baseline_output_64;
#endif

			// Record outputs and rebase test outputs
			etrading::CreateDataFile::setOutputFolder( etrading::getGoogleTestFolder(), false );
			CreateDataFile file(etrading::decorateFilename(outputFileName));

			file.write("headers", headers);

			for (size_t i = 0; i < pillarNames.size(); ++i)
			{
				// 12 is the number of decimal points required
				file.write(pillarNames[i], deltas[i], 12);
			}
		}
		else
		{
			// Carry out actual test and peform result comparison
#ifdef GTEST32
			const ReadDataFile::Load resultFile(baseline_output_32);
#else
			const ReadDataFile::Load resultFile(baseline_output_64);
#endif

			if (!resultFile.hasItem("headers"))
			{
				FAIL() << "Reference baseline does not contain delta column headers" << std::endl;
			}

			LAStringVector refHeaders = resultFile["headers"];
			if (headers.size() != refHeaders.size())
			{
				FAIL() << "Calculated delta has different number of column headers compared to reference baseline : " << headers.size() << " vs " << refHeaders.size() << std::endl;
			}
			for (size_t i = 0; i < headers.size(); ++i)
			{
				ASSERT_EQ(headers[i], refHeaders[i]);
			}

			for (size_t i = 0; i < pillarNames.size(); ++i)
			{
				LAString key = pillarNames[i];

				if (!resultFile.hasItem(key))
				{
					FAIL() << "Calculated results contains a pillarName: " << key << " which is missing in reference baseline" << std::endl;
				}

				DoubleVector delta = deltas[i];
				DoubleVector ref = resultFile[key];

				if (delta.size() != ref.size())
				{
					FAIL() << "Calculated delta has different number of columns compared to reference baseline: " << delta.size() << " vs " << ref.size() << std::endl;
				}

				for (size_t j = 0; j < delta.size(); ++j)
				{
					EXPECT_NEAR(delta[j], ref[j], tolerance)
						<< " Delta for pillar point " << key.getCString() << " is incorrect ";
				}
			}
		}
	}
}

namespace google_test
{

	DECLARE_TEST_FIXTURE(TestLWOSwapARRDelta);
	
	TEST_F(TestLWOSwapARRDelta, RISK_Swap_DeltaLadder_SOFR )
	{
		// Load Curves
		auto loadSOFR = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USDYC_SOFR, etrading::JSON);
		auto loadOIS  = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USDYC_OIS, etrading::JSON);

		// load swap generator
		auto loadSOFRSwapGenerator = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USD_SOFR_SWAP, etrading::JSON);

		// Load swap trade
		auto loadSOFRSwap = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USD_SOFR_SWAP_5Y, etrading::JSON);

		// Risk delta ladder config
		const ReadDataFile::Load deltaLadder( fileName_deltaLadder_SOFR_Inputs );

		LAStringVector swapNames			= deltaLadder["swapNames"];
		LAStringMatrix curveCollectionNames	= deltaLadder["curveCollectionNames"];
		LAStringMatrix fixingTableNames		= deltaLadder["fixingTableNames"];
		bool bumpSpreadInstruments			= deltaLadder["bumpSpreadInstruments"];
		double bumpSize						= deltaLadder["bumpSize"];
		LAString bumpMode					= deltaLadder["bumpMode"];
		bool aggregateRisks					= deltaLadder["aggregateRisks"];
		bool reportInLegCCY					= deltaLadder["reportInLegCCY"];
		std::string riskCutOffTenor			= deltaLadder["riskCutOffTenor"];
		DoubleVector xccyFXSpotRates		= deltaLadder["xccyFXSpotRates"];

		LAStringVector pillarNames;
		LAStringVector headers;
		DoubleMatrix deltas;
		validation_api::tryMeLWOSwapDeltaLadder(headers,
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
												xccyFXSpotRates);

		checkDeltaResultsAndRebaseIfRequired( pillarNames, headers, deltas, fileName_raw_delta_ladder_SOFR_outputs_32, fileName_raw_delta_ladder_SOFR_outputs_64);

	}

	TEST_F(TestLWOSwapARRDelta, RISK_Swap_DeltaLadder_SOFR_OIS)
	{
		// Load Curves
		auto loadSOFR = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USDYC_SOFR, etrading::JSON);
		auto loadOIS = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USDYC_OIS, etrading::JSON);

		// load swap generator
		auto loadOISSwapGenerator = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USD_OIS_SWAP, etrading::JSON);

		// Load swap trade
		auto loadSOFRSwap = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USD_OIS_SWAP_5Y, etrading::JSON);

		// Risk delta ladder config
		const ReadDataFile::Load deltaLadder(fileName_deltaLadder_OIS_Inputs);

		LAStringVector swapNames			= deltaLadder["swapNames"];
		LAStringMatrix curveCollectionNames = deltaLadder["curveCollectionNames"];
		LAStringMatrix fixingTableNames		= deltaLadder["fixingTableNames"];
		bool bumpSpreadInstruments			= deltaLadder["bumpSpreadInstruments"];
		double bumpSize						= deltaLadder["bumpSize"];
		LAString bumpMode					= deltaLadder["bumpMode"];
		bool aggregateRisks					= deltaLadder["aggregateRisks"];
		bool reportInLegCCY					= deltaLadder["reportInLegCCY"];
		std::string riskCutOffTenor			= deltaLadder["riskCutOffTenor"];
		DoubleVector xccyFXSpotRates		= deltaLadder["xccyFXSpotRates"];

		LAStringVector pillarNames;
		LAStringVector headers;
		DoubleMatrix deltas;
		validation_api::tryMeLWOSwapDeltaLadder(headers,
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
												xccyFXSpotRates);

		checkDeltaResultsAndRebaseIfRequired(pillarNames, headers, deltas, fileName_raw_delta_ladder_OIS_outputs_32, fileName_raw_delta_ladder_OIS_outputs_64); 
	}


}
