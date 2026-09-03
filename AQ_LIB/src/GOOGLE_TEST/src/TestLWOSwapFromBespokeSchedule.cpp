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

#include "CreateDataFile.h"
#include "TestHelperUtilities.h"

#include "tryMeLWOSchedule.h"
#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"
#include "tryMeLWO.h"
#include "tryMeLWOLeg.h"
#include "tryMeLWOSwapDelta.h"


using etrading::ReadDataFile;
using etrading::CreateDataFile;

namespace
{
	// This test cover: leverage, compounding (float schedule's accrual freq differnt from payment freq)

	const double pvTolerance = 250.;		// Trade notional is EUR 195,000,000
	const double tolerance = 1e-6;
	const double deltaTolerance = 25.0;


	const std::string TEST_DIR = "ETrading/LWObjects/TestLWOSwapFromBespokeSchedule/";

    //
    // curve input files
    //
	const std::string fileName_EUR_OIS = TEST_DIR + "EUR_OIS_CURVE.JSON";
	const std::string fileName_EUR_1ML = TEST_DIR + "EUR_BASIS_1M_CURVE.JSON";
	const std::string fileName_EUR_3ML = TEST_DIR + "EUR_SWAP_3M_CURVE.JSON";

	const std::string bespokeScheduleCreationInput_Fixed = TEST_DIR + "FIXED_SCHEDULE_tryMeLWOScheduleCreateBespoke_inputs";
	const std::string bespokeScheduleCreationInput_Float_1M = TEST_DIR + "FLOAT_1M_SCHEDULE_tryMeLWOScheduleCreateBespoke_inputs";
	const std::string bespokeScheduleCreationInput_Float_3M = TEST_DIR + "FLOAT_3M_SCHEDULE_tryMeLWOScheduleCreateBespoke_inputs";

	const std::string legCreationInput_Fixed = TEST_DIR + "FIXED_LEG_tryMeLWOLegCreateFromSchedule_inputs";
	const std::string legCreationInput_Float_1M = TEST_DIR + "FLOAT_1M_LEG_tryMeLWOLegCreateFromSchedule_inputs";
	const std::string legCreationInput_Float_3M = TEST_DIR + "FLOAT_3M_LEG_tryMeLWOLegCreateFromSchedule_inputs";

	const std::string swapCreationInput = TEST_DIR + "VNS_SWAP_tryMeLWOSwapCreateFromLegs_inputs";

	const std::string pvInput = TEST_DIR + "validationUtilitySwapPV_inputs";
	const std::string pvOutput = TEST_DIR + "validationUtilitySwapPV_outputs";

	const std::string parRateInput = TEST_DIR + "tryMeLWOSwapParRate_inputs";
	const std::string parRateOutput = TEST_DIR + "tryMeLWOSwapParRate_outputs";

	const std::string pv01Input = TEST_DIR + "tryMeLWOSwapPV01_inputs";
	const std::string pv01Output = TEST_DIR + "tryMeLWOSwapPV01_outputs";

	const std::string dv01Input = TEST_DIR + "tryMeLWOSwapDelta_inputs";
	const std::string dv01Output = TEST_DIR + "tryMeLWOSwapDelta_outputs";

	//With Fee Leg:

	const std::string TEST_WITH_FEE_DIR = TEST_DIR + "WithFeeLeg/";

	const std::string feeCreationInput = TEST_WITH_FEE_DIR + "FEE1_tryMeLWOFeeLegCreate_inputs";

	const std::string pvInput_withFee = TEST_WITH_FEE_DIR + "validationUtilitySwapPV_inputs";
	const std::string pvOutput_withFee = TEST_WITH_FEE_DIR + "validationUtilitySwapPV_outputs";

	const std::string parRateInput_withFee = TEST_WITH_FEE_DIR + "tryMeLWOSwapParRate_inputs";
	const std::string parRateOutput_withFee = TEST_WITH_FEE_DIR + "tryMeLWOSwapParRate_outputs";

	const std::string pv01Input_withFee = TEST_WITH_FEE_DIR + "tryMeLWOSwapPV01_inputs";
	const std::string pv01Output_withFee = TEST_WITH_FEE_DIR + "tryMeLWOSwapPV01_outputs";

	const std::string dv01Input_withFee = TEST_WITH_FEE_DIR + "tryMeLWOSwapDelta_inputs";
	const std::string dv01Output_withFee = TEST_WITH_FEE_DIR + "tryMeLWOSwapDelta_outputs";


	void loadCurves()
	{
		// Load Curves
		auto loadOIS = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);
		auto load1ML = validation_api::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_1ML, etrading::JSON);
	}

	std::string createBespokeSchedule(const std::string& fileName)
	{
		const ReadDataFile::Load inputFile(fileName.c_str());

		std::string scheduleName = inputFile["scheduleName"];
		LAStringMatrix bespokeScheduleProperties = inputFile["bespokeScheduleProperties"];
		LAStringMatrix bespokeScheduleLVB = inputFile["bespokeScheduleLVB"];

		validation_api::tryMeLWOScheduleCreateBespoke(scheduleName, bespokeScheduleProperties, bespokeScheduleLVB);

		return scheduleName;
	}

	std::string createLegFromSchedule(const std::string& fileName)
	{
		const ReadDataFile::Load inputFile(fileName.c_str());

		std::string legObjectName = inputFile["legObjectName"];
		std::string scheduleName = inputFile["scheduleName"];
		LAStringMatrix legLVB = inputFile["legLVB"];

		validation_api::tryMeLWOLegCreateFromSchedule(legObjectName, scheduleName, legLVB);

		return legObjectName;
	}

	std::string createSwapFromLegs(const std::string& fileName)
	{
		const ReadDataFile::Load inputFile(fileName.c_str());

		std::string swapName = inputFile["swapName"];
		std::vector<std::string> legObjectNames = inputFile["legObjectNames"];
		LAStringMatrix swapPropertiesLVB = inputFile["swapPropertiesLVB"];
		bool isXccySwap = inputFile["isXccySwap"];

		validation_api::tryMeLWOSwapCreateFromLegs(swapName, legObjectNames, swapPropertiesLVB, isXccySwap);

		return swapName;
	}

	std::string loadSwap()
	{
		//Create Schedules
		createBespokeSchedule(bespokeScheduleCreationInput_Fixed);
		createBespokeSchedule(bespokeScheduleCreationInput_Float_1M);
		createBespokeSchedule(bespokeScheduleCreationInput_Float_3M);

		//Create Legs
		createLegFromSchedule(legCreationInput_Fixed);
		createLegFromSchedule(legCreationInput_Float_1M);
		createLegFromSchedule(legCreationInput_Float_3M);

		//Create Swap
		std:: string swapName  = createSwapFromLegs(swapCreationInput);

		return swapName;
	}

	std::string createFeeLeg(const std::string& fileName)
	{
		const ReadDataFile::Load inputFile(fileName.c_str());

		std::string legObjectName = inputFile["legObjectName"];
		LAStringMatrix feeProperties = inputFile["feeProperties"];
		LAStringMatrix feeScheduleLVB = inputFile["feeScheduleLVB"];

		validation_api::tryMeLWOFeeLegCreate(legObjectName, feeProperties, feeScheduleLVB);

		return legObjectName;
	}

	std::string loadSwapWithFeeLeg()
	{
		std::string swapName = loadSwap();
		std::string feeName = createFeeLeg(feeCreationInput);

		validation_api::tryMeLWOSwapAddFee(swapName, feeName);
		return swapName;
	}


}

namespace google_test
{

	// Declare Test TestLWOSwapFromBespokeSchedule Class
	DECLARE_TEST_FIXTURE(TestLWOSwapFromBespokeSchedule)

	// Call Test Fixture Class
	TEST_F(TestLWOSwapFromBespokeSchedule, SNAPSHOT_Swap_PV)
	{
		loadCurves();
		loadSwap();

		const ReadDataFile::Load inputFile(pvInput.c_str());
		std::string swapName = inputFile["swapName"];
		LAStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];

		double actualValue = validation_api::tryMeLWOSwapPV(swapName, valuationSettingsLVB);

		CheckTestResultsAndRebaseOnRequest(actualValue, TEST_DIR.c_str(), pvOutput.c_str(), pvTolerance);
	}

	TEST_F(TestLWOSwapFromBespokeSchedule, SNAPSHOT_Swap_ParRate)
	{
		loadCurves();
		loadSwap();

		const ReadDataFile::Load inputFile(parRateInput.c_str());
		std::string swapName = inputFile["swapName"];
		LAStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];

		double actualValue = validation_api::tryMeLWOSwapParRate(swapName, valuationSettingsLVB);

		CheckTestResultsAndRebaseOnRequest(actualValue, TEST_DIR.c_str(), parRateOutput.c_str(), tolerance);
	}

	TEST_F(TestLWOSwapFromBespokeSchedule, SNAPSHOT_Swap_PV01)
	{
		loadCurves();
		loadSwap();

		const ReadDataFile::Load inputFile(pv01Input.c_str());
		std::string swapName = inputFile["swapName"];
		LAStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];

		double actualValue = validation_api::tryMeLWOSwapPV01(swapName, valuationSettingsLVB);

		CheckTestResultsAndRebaseOnRequest(actualValue, TEST_DIR.c_str(), pv01Output.c_str(), tolerance);

	}

	TEST_F(TestLWOSwapFromBespokeSchedule, SNAPSHOT_Swap_DV01)
	{
		loadCurves();
		loadSwap();

		const ReadDataFile::Load inputFile(dv01Input.c_str());

		LAStringVector swapNames = inputFile["swapNames"];
		LAStringMatrix curveCollectionNames = inputFile["curveCollectionNames"];
		LAStringMatrix fixingTableNames = inputFile["fixingTableNames"];
		bool bumpSpreadInstruments = inputFile["bumpSpreadInstruments"];
		double bumpSize = inputFile["bumpSize"];
		LAString bumpMode = inputFile["bumpMode"];
		LAString groupRiskBy = inputFile["groupRiskBy"];
		bool aggregateRisks = inputFile["aggregateRisks"];
		bool reportInLegCCY = inputFile["reportInLegCCY"];
		DoubleVector xccyFXSpotRates = inputFile["xccyFXSpotRates"];

		DoubleVector deltas;
		LAStringVector tempStrVector;
		validation_api::tryMeLWOSwapDelta(tempStrVector, deltas, swapNames, curveCollectionNames, fixingTableNames, bumpSpreadInstruments, bumpSize, bumpMode, groupRiskBy, aggregateRisks, reportInLegCCY, xccyFXSpotRates);

		const ReadDataFile::Load outputFile(dv01Output.c_str());
		double expectDelta = outputFile["TotalDelta"];

		EXPECT_NEAR(deltas[0], expectDelta, deltaTolerance);

	}

	// Call Test Fixture Class
	TEST_F(TestLWOSwapFromBespokeSchedule, SNAPSHOT_Swap_PV_withFee)
	{
		loadCurves();
		loadSwapWithFeeLeg();

		const ReadDataFile::Load inputFile(pvInput_withFee.c_str());
		std::string swapName = inputFile["swapName"];
		LAStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];

		double actualValue = validation_api::tryMeLWOSwapPV(swapName, valuationSettingsLVB);

		CheckTestResultsAndRebaseOnRequest(actualValue, TEST_WITH_FEE_DIR.c_str(), pvOutput_withFee.c_str(), pvTolerance);
	}

	TEST_F(TestLWOSwapFromBespokeSchedule, SNAPSHOT_Swap_ParRate_withFee)
	{
		loadCurves();
		loadSwapWithFeeLeg();

		const ReadDataFile::Load inputFile(parRateInput_withFee.c_str());
		std::string swapName = inputFile["swapName"];
		LAStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];

		double actualValue = validation_api::tryMeLWOSwapParRate(swapName, valuationSettingsLVB);

		CheckTestResultsAndRebaseOnRequest(actualValue, TEST_WITH_FEE_DIR.c_str(), parRateOutput_withFee.c_str(), tolerance);
	}

	TEST_F(TestLWOSwapFromBespokeSchedule, SNAPSHOT_Swap_PV01_withFee)
	{
		loadCurves();
		loadSwapWithFeeLeg();

		const ReadDataFile::Load inputFile(pv01Input_withFee.c_str());
		std::string swapName = inputFile["swapName"];
		LAStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];

		double actualValue = validation_api::tryMeLWOSwapPV01(swapName, valuationSettingsLVB);

		CheckTestResultsAndRebaseOnRequest(actualValue, TEST_WITH_FEE_DIR.c_str(), pv01Output_withFee.c_str(), tolerance);

	}

	TEST_F(TestLWOSwapFromBespokeSchedule, SNAPSHOT_Swap_DV01_withFee)
	{
		loadCurves();
		loadSwapWithFeeLeg();

		const ReadDataFile::Load inputFile(dv01Input_withFee.c_str());

		LAStringVector swapNames = inputFile["swapNames"];
		LAStringMatrix curveCollectionNames = inputFile["curveCollectionNames"];
		LAStringMatrix fixingTableNames = inputFile["fixingTableNames"];
		bool bumpSpreadInstruments = inputFile["bumpSpreadInstruments"];
		double bumpSize = inputFile["bumpSize"];
		LAString bumpMode = inputFile["bumpMode"];
		LAString groupRiskBy = inputFile["groupRiskBy"];
		bool aggregateRisks = inputFile["aggregateRisks"];
		bool reportInLegCCY = inputFile["reportInLegCCY"];
		DoubleVector xccyFXSpotRates = inputFile["xccyFXSpotRates"];

		DoubleVector deltas;
		LAStringVector tempStrVector;
		validation_api::tryMeLWOSwapDelta(tempStrVector, deltas, swapNames, curveCollectionNames, fixingTableNames, bumpSpreadInstruments, bumpSize, bumpMode, groupRiskBy, aggregateRisks, reportInLegCCY, xccyFXSpotRates);

		const ReadDataFile::Load outputFile(dv01Output_withFee.c_str());
		double expectDelta = outputFile["TotalDelta"];

		EXPECT_NEAR(deltas[0], expectDelta, deltaTolerance);

	}

}

