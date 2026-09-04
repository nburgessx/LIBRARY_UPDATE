// Curves
#include "tryAqObjects.h"

// Risk calculation
#include "tryAqObjectsSwapDelta.h"
#include "tryAqObjectsFixingTable.h"

// par rate calculation
#include "tryAqObjectsSwapPricing.h"

// Helper to extract par rates from a curve
#include "ExtractCurveCalibrationData.h"

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
    const double tolerance = 10; // Notional of test trades is 1MM and delta by bumping is inherently noisy

	// Test Input File(s) 
	const std::string TEST_DIR = "ETrading/LWObjects/TestLWOSwapDelta_ZAR/";

    //
    // curve input files
    //
	const std::string fileName_USDYC_OIS_CURVE			= TEST_DIR + "USD_OIS_CURVE@2.JSON";
    const std::string fileName_USDYC_STD_CURVE			= TEST_DIR + "USD_SWAP_3M_CURVE@4.JSON";

	const std::string fileName_ZARYC_STD_CURVE			= TEST_DIR + "ZAR_SWAP_3M_CURVE@6.JSON";
	const std::string fileName_ZARYC_XCCY_CURVE			= TEST_DIR + "ZAR_XCCYBASIS_CURVE@8.JSON";

	// Curve market data containing par-rates
	//const std::string fileName_ZAR_STD_MARKETDATA		= TEST_DIR + "ZAR_SWAP_3M_CURVE_MARKETDATA@5.JSON";

	// IRS Trades
	const std::string fileName_ZAR_IRS_3Y				= TEST_DIR + "ZAR_IRS_1@12.JSON";
	const std::string fileName_ZAR_IRS_4Y				= TEST_DIR + "ZAR_IRS_2@10.JSON";
	const std::string fileName_ZAR_IRS_5Y				= TEST_DIR + "ZAR_IRS_3@11.JSON";
	const std::string fileName_ZAR_IRS_6Y				= TEST_DIR + "ZAR_IRS_4@9.JSON";
	const std::string fileName_ZAR_IRS_7Y				= TEST_DIR + "ZAR_IRS_5@13.JSON";;
	const std::string fileName_ZAR_IRS_8Y				= TEST_DIR + "ZAR_IRS_6@14.JSON";
	const std::string fileName_ZAR_IRS_9Y				= TEST_DIR + "ZAR_IRS_7@15.JSON";
	const std::string fileName_ZAR_IRS_10Y				= TEST_DIR + "ZAR_IRS_8@16.JSON";
	const std::string fileName_ZAR_IRS_12Y				= TEST_DIR + "ZAR_IRS_9@17.JSON";
	const std::string fileName_ZAR_IRS_15Y				= TEST_DIR + "ZAR_IRS_10@18.JSON";
	const std::string fileName_ZAR_IRS_20Y				= TEST_DIR + "ZAR_IRS_11@19.JSON";
	const std::string fileName_ZAR_IRS_25Y				= TEST_DIR + "ZAR_IRS_12@20.JSON";
	const std::string fileName_ZAR_IRS_30Y				= TEST_DIR + "ZAR_IRS_13@21.JSON";

	// XCCY trade
	const std::string fileName_ZAR_XCCY_5Y				= TEST_DIR + "ZAR_XCCY_SWAP@22.JSON";

	// Par rate input prefix
	const std::string fileName_ZAR_IRS_parRate			= TEST_DIR + "tryMeLWOSwapParRate_";

	// DV01 Inputs
	const std::string fileName_IRS_DV01_Inputs			= TEST_DIR + "tryMeLWOSwap_IRS_DV01_inputs.csv";
	const std::string fileName_XCCY_DV01_Inputs			= TEST_DIR + "tryMeLWOSwap_XCCY_DV01_inputs.csv";

	// DV01 Outputs
	const std::string fileName_IRS_DV01_Outputs_32		= TEST_DIR + "tryMeLWOSwap_IRS_DV01_outputs_32.csv";
	const std::string fileName_IRS_DV01_Outputs_64		= TEST_DIR + "tryMeLWOSwap_IRS_DV01_outputs_64.csv";

	const std::string fileName_XCCY_DV01_Outputs_32		= TEST_DIR + "tryMeLWOSwap_XCCY_DV01_outputs_32.csv";
	const std::string fileName_XCCY_DV01_Outputs_64		= TEST_DIR + "tryMeLWOSwap_XCCY_DV01_outputs_64.csv";

	// Helper function

	void checkDV01ResultsAndRebaseIfRequired(const AQLStringVector& positionIDs,
											 const DoubleVector& deltas,
											 const AQLString& baseline_output_32,
											 const AQLString& baseline_output_64)
	{

		if (etrading::CreateDataFile::rebaseResultsEnabled())
		{
#ifdef GTEST32
			AQLString outputFileName = baseline_output_32;
#else
			AQLString outputFileName = baseline_output_64;
#endif

			// Record outputs and rebase test outputs
			etrading::CreateDataFile::setOutputFolder( etrading::getGoogleTestFolder(), false );
			CreateDataFile file(etrading::decorateFilename(outputFileName));
			for (size_t i = 0; i < positionIDs.size(); ++i)
			{
				// 12 is the number of decimal points required
				file.write(positionIDs[i], deltas[i], 12);
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

			for (size_t i = 0; i < positionIDs.size(); ++i)
			{
				AQLString key = positionIDs[i];

				double delta = deltas[i];
				double ref = resultFile[key];

				EXPECT_NEAR(delta, ref, tolerance)
					<< " Delta : " << key.getCString() << " is incorrect ";
			}
		}
	}

	void loadIRSTrades()
	{
		auto loadIRS1 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_3Y, etrading::JSON);
		auto loadIRS2 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_4Y, etrading::JSON);
		auto loadIRS3 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_5Y, etrading::JSON);
		auto loadIRS4 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_6Y, etrading::JSON);
		auto loadIRS5 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_7Y, etrading::JSON);
		auto loadIRS6 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_8Y, etrading::JSON);
		auto loadIRS7 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_9Y, etrading::JSON);
		auto loadIRS8 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_10Y, etrading::JSON);
		auto loadIRS9 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_12Y, etrading::JSON);
		auto loadIRS10 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_15Y, etrading::JSON);
		auto loadIRS11 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_20Y, etrading::JSON);
		auto loadIRS12 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_25Y, etrading::JSON);
		auto loadIRS13 = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_IRS_30Y, etrading::JSON);
	}
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestLWOSwapDelta_ZAR);


	// Since we have all IRS trades available, check the parRate / repricing
	// Originally, the par-rate calc failed on ZAR3MJ because it is a self-discounted curve
	TEST_F(TestLWOSwapDelta_ZAR, CONSISTENCY_IRS_ParRate_ZAR)
	{
		// Load Curve. ZAR_3MJ is self-discounting
		auto loadZARSTD = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZARYC_STD_CURVE, etrading::JSON);

		// Extract par rates from the ZAR_3MJ curve
		std::vector<double> zarStdParRates = curveCalibrationSwapParRates( loadZARSTD );

		loadIRSTrades();

		std::vector<std::string> swapTenors = { "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y" };

		// Check par rates
		const double parRateTolerance = 1e-5;
		int i = 0;
		for( auto swapTenor : swapTenors )
		{
			const ReadDataFile::Load parRateInputs( fileName_ZAR_IRS_parRate  + swapTenor + "_inputs" );
			const std::string swapName				= parRateInputs[ "swapName" ];
			AQLStringMatrix valuationSettingsLVB		= parRateInputs[ "valuationSettingsLVB" ];
			AQLStringMatrix fixingTableNames			= parRateInputs[ "fixingTableNames" ];

			const double actualResult	= validation::tryAqObjectsSwapParRate( swapName, valuationSettingsLVB, fixingTableNames );
			const double expectedResult	= zarStdParRates[ i ];
			EXPECT_NEAR( actualResult, expectedResult, parRateTolerance );
			i++;
		}
	}


	TEST_F(TestLWOSwapDelta_ZAR, RISK_IRS_DV01_ZAR )
	{
		// Load Curve. ZAR_3MJ is self-discounting
		auto loadZARSTD = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZARYC_STD_CURVE, etrading::JSON);

		loadIRSTrades();

		// Risk delta ladder config
		const ReadDataFile::Load dv01Inputs( fileName_IRS_DV01_Inputs );

		AQLStringVector swapNames			= dv01Inputs["swapNames"];
		AQLStringMatrix curveCollectionNames	= dv01Inputs["curveCollectionNames"];
		AQLStringMatrix fixingTableNames		= dv01Inputs["fixingTableNames"];
		bool bumpSpreadInstruments			= dv01Inputs["bumpSpreadInstruments"];
		double bumpSize						= dv01Inputs["bumpSize"];
		AQLString bumpMode					= dv01Inputs["bumpMode"];
		AQLString groupRiskBy				= dv01Inputs["groupRiskBy"];
		bool aggregateRisks					= dv01Inputs["aggregateRisks"];
		bool reportInLegCCY					= dv01Inputs["reportInLegCCY"];
		DoubleVector xccyFXSpotRates		= dv01Inputs["xccyFXSpotRates"];

		AQLStringVector positionIDs;
		DoubleVector deltas;
		validation::tryAqObjectsSwapDelta( positionIDs,
											deltas,
											swapNames,
											curveCollectionNames,
											fixingTableNames,
											bumpSpreadInstruments,
											bumpSize,
											bumpMode,
											groupRiskBy,
											aggregateRisks,
											reportInLegCCY,
											xccyFXSpotRates );

		checkDV01ResultsAndRebaseIfRequired( positionIDs, deltas, fileName_IRS_DV01_Outputs_32, fileName_IRS_DV01_Outputs_64 );
	}


	TEST_F(TestLWOSwapDelta_ZAR, RISK_XCCY_DV01_ZAR)
	{
		// Load Curves
		auto loadUSDOIS = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_USDYC_OIS_CURVE, etrading::JSON);
		auto loadUSDSTD = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_USDYC_STD_CURVE, etrading::JSON);

		auto loadZARSTD  = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZARYC_STD_CURVE, etrading::JSON);
		auto loadZARXCCY = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZARYC_XCCY_CURVE, etrading::JSON);

		// Load XCCY trade
		auto loadXCCY = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + fileName_ZAR_XCCY_5Y, etrading::JSON);


		// Risk delta ladder config
		const ReadDataFile::Load dv01Inputs(fileName_XCCY_DV01_Inputs);

		AQLStringVector swapNames			= dv01Inputs["swapNames"];
		AQLStringMatrix curveCollectionNames = dv01Inputs["curveCollectionNames"];
		AQLStringMatrix fixingTableNames		= dv01Inputs["fixingTableNames"];
		bool bumpSpreadInstruments			= dv01Inputs["bumpSpreadInstruments"];
		double bumpSize						= dv01Inputs["bumpSize"];
		AQLString bumpMode					= dv01Inputs["bumpMode"];
		AQLString groupRiskBy				= dv01Inputs["groupRiskBy"];
		bool aggregateRisks					= dv01Inputs["aggregateRisks"];
		bool reportInLegCCY					= dv01Inputs["reportInLegCCY"];
		DoubleVector xccyFXSpotRates		= dv01Inputs["xccyFXSpotRates"];

		AQLStringVector positionIDs;
		DoubleVector deltas;
		validation::tryAqObjectsSwapDelta(  positionIDs,
											deltas,
											swapNames,
											curveCollectionNames,
											fixingTableNames,
											bumpSpreadInstruments,
											bumpSize,
											bumpMode,
											groupRiskBy,
											aggregateRisks,
											reportInLegCCY,
											xccyFXSpotRates);

		checkDV01ResultsAndRebaseIfRequired(positionIDs, deltas, fileName_XCCY_DV01_Outputs_32, fileName_XCCY_DV01_Outputs_64);

	}


}
