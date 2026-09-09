// Include: Google Test Library
#include <gTest/gTest.h>
#include "InitializeGoogleTest.h"

#include "Dependency.h"
#include "ReadDataFile.h"
#include "GetGoogleTestFolder.h"
#include "ResultsProcessor.h"

#include "BondYields.h"
#include "BondFactory.h"
#include "ObjectUtilities.h"
#include "CreateFixedBond.h"

#include "tryAqObject.h"
#include "tryAqAssetSwapObject.h"
#include "tryAqBondObject.h"

#include "CreateDataFile.h"
#include "TestHelperUtilities.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

namespace
{
	// Test Input File(s) 

	const std::string TEST_DIR = "ETrading/Bonds/Spreads/";

	// --- Curves---
	const std::string fileName_AUD_OIS = TEST_DIR + "AUD_OIS_CURVE.JSON";
	const std::string fileName_AUD_3ML = TEST_DIR + "AUD_SWAP_3M_CURVE.JSON";

	const std::string fileName_USD_OIS = TEST_DIR + "USD_OIS_CURVE.JSON";
	const std::string fileName_USD_3ML = TEST_DIR + "USD_SWAP_3M_CURVE.JSON";

	const std::string fileName_EUR_OIS = TEST_DIR + "EUR_OIS_CURVE.JSON";
	const std::string fileName_EUR_3ML = TEST_DIR + "EUR_SWAP_3M_CURVE.JSON";
	const std::string fileName_EUR_6ML = TEST_DIR + "EUR_SWAP_6M_CURVE.JSON";

	const std::string fileName_JPY_OIS = TEST_DIR + "JPY_JSCC_OIS_CURVE.JSON";
	const std::string fileName_JPY_6ML = TEST_DIR + "JPY_JSCC_SWAP_6M_CURVE.JSON";

	// --- AUD Bond---
    const std::string fileName_Bond_AUD	= TEST_DIR + "AU000XCLWAR9.JSON";
	const std::string fileName_AssetSwap_AUD	= TEST_DIR + "AU000XCLWAR9_ASSET_SWAP.JSON";

	const std::string assetSwapSpreadInputFile_AUD = TEST_DIR + "AU000XCLWAR9_ASSET_SWAP_tryAqAssetSwapObjectSpread_inputs";
	const std::string assetSwapSpreadOutputFile_AUD = TEST_DIR + "AU000XCLWAR9_ASSET_SWAP_tryAqAssetSwapObjectSpread_outputs";

	const std::string zSpreadInputFile_AUD = TEST_DIR + "AU000XCLWAR9_tryAqBondObjectZSpread_inputs";
	const std::string zSpreadOutputFile_AUD = TEST_DIR + "AU000XCLWAR9_tryAqBondObjectZSpread_outputs";

	// --- USD Bond---
	const std::string fileName_Bond_USD = TEST_DIR + "US912810QT88.JSON";
	const std::string fileName_AssetSwap_USD = TEST_DIR + "US912810QT88_ASSET_SWAP.JSON";

	const std::string assetSwapSpreadInputFile_USD = TEST_DIR + "US912810QT88_ASSET_SWAP_tryAqAssetSwapObjectSpread_inputs";
	const std::string assetSwapSpreadOutputFile_USD = TEST_DIR + "US912810QT88_ASSET_SWAP_tryAqAssetSwapObjectSpread_outputs";

	const std::string zSpreadInputFile_USD = TEST_DIR + "US912810QT88_tryAqBondObjectZSpread_inputs";
	const std::string zSpreadOutputFile_USD = TEST_DIR + "US912810QT88_tryAqBondObjectZSpread_outputs";

	// --- German Bond ---
	const std::string fileName_Bond_German = TEST_DIR + "DE0001135424.JSON";
	const std::string fileName_AssetSwap_German = TEST_DIR + "DE0001135424_ASSET_SWAP.JSON";

	const std::string assetSwapSpreadInputFile_German = TEST_DIR + "DE0001135424_ASSET_SWAP_tryAqAssetSwapObjectSpread_inputs";
	const std::string assetSwapSpreadOutputFile_German = TEST_DIR + "DE0001135424_ASSET_SWAP_tryAqAssetSwapObjectSpread_outputs";

	const std::string zSpreadInputFile_German = TEST_DIR + "DE0001135424_tryAqBondObjectZSpread_inputs";
	const std::string zSpreadOutputFile_German = TEST_DIR + "DE0001135424_tryAqBondObjectZSpread_outputs";

	// --- Dutch Bond ---
	const std::string fileName_Bond_Dutch = TEST_DIR + "NL0012818504.JSON";
	const std::string fileName_AssetSwap_Dutch = TEST_DIR + "NL0012818504_ASSET_SWAP.JSON";

	const std::string assetSwapSpreadInputFile_Dutch = TEST_DIR + "NL0012818504_ASSET_SWAP_tryAqAssetSwapObjectSpread_inputs";
	const std::string assetSwapSpreadOutputFile_Dutch = TEST_DIR + "NL0012818504_ASSET_SWAP_tryAqAssetSwapObjectSpread_outputs";

	const std::string zSpreadInputFile_Dutch = TEST_DIR + "NL0012818504_tryAqBondObjectZSpread_inputs";
	const std::string zSpreadOutputFile_Dutch = TEST_DIR + "NL0012818504_tryAqBondObjectZSpread_outputs";

	// --- French Bond ---
	const std::string fileName_Bond_French = TEST_DIR + "FR0013154028.JSON";
	const std::string fileName_AssetSwap_French = TEST_DIR + "FR0013154028_ASSET_SWAP.JSON";

	const std::string assetSwapSpreadInputFile_French = TEST_DIR + "FR0013154028_ASSET_SWAP_tryAqAssetSwapObjectSpread_inputs";
	const std::string assetSwapSpreadOutputFile_French = TEST_DIR + "FR0013154028_ASSET_SWAP_tryAqAssetSwapObjectSpread_outputs";

	const std::string zSpreadInputFile_French = TEST_DIR + "FR0013154028_tryAqBondObjectZSpread_inputs";
	const std::string zSpreadOutputFile_French = TEST_DIR + "FR0013154028_tryAqBondObjectZSpread_outputs";

	// --- Italian Bond ---
	const std::string fileName_Bond_Italian = TEST_DIR + "IT0005240830.JSON";
	const std::string fileName_AssetSwap_Italian = TEST_DIR + "IT0005240830_ASSET_SWAP.JSON";

	const std::string assetSwapSpreadInputFile_Italian = TEST_DIR + "IT0005240830_ASSET_SWAP_tryAqAssetSwapObjectSpread_inputs";
	const std::string assetSwapSpreadOutputFile_Italian = TEST_DIR + "IT0005240830_ASSET_SWAP_tryAqAssetSwapObjectSpread_outputs";

	const std::string zSpreadInputFile_Italian = TEST_DIR + "IT0005240830_tryAqBondObjectZSpread_inputs";
	const std::string zSpreadOutputFile_Italian = TEST_DIR + "IT0005240830_tryAqBondObjectZSpread_outputs";


	// --- JGB Bond---
	const std::string fileName_Bond_JGB = TEST_DIR + "JP1200491106.JSON";
	const std::string fileName_AssetSwap_JGB = TEST_DIR + "JP1200491106_ASSET_SWAP.JSON";

	const std::string assetSwapSpreadInputFile_JGB = TEST_DIR + "JP1200491106_ASSET_SWAP_tryAqAssetSwapObjectSpread_inputs";
	const std::string assetSwapSpreadOutputFile_JGB = TEST_DIR + "JP1200491106_ASSET_SWAP_tryAqAssetSwapObjectSpread_outputs";

	const std::string zSpreadInputFile_JGB = TEST_DIR + "JP1200491106_tryAqBondObjectZSpread_inputs";
	const std::string zSpreadOutputFile_JGB = TEST_DIR + "JP1200491106_tryAqBondObjectZSpread_outputs";

}

namespace google_test
{

	// Declare Test Fixture Class
	DECLARE_TEST_FIXTURE(TestBondSpreads)

	// --- AUD Bond---

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_AssetSwapSpread_AUD)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AUD_OIS, etrading::JSON);
		auto load3ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AUD_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_AUD, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_AUD, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadInputFile_AUD.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Price Matching
		auto actualSpread = validation::tryAqAssetSwapObjectSpread(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);


		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadOutputFile_AUD.c_str(), getBondSpreadTolerance());

	}

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_ZSpread_AUD)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AUD_OIS, etrading::JSON);
		auto load3ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AUD_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_AUD, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_AUD, etrading::JSON);

		const ReadDataFile::Load inputFile(zSpreadInputFile_AUD.c_str());

		//1) Check Price Matching
		auto actual_zSpread = validation::tryAqBondObjectZSpread(inputFile["bondObjectName"], inputFile["settlementDate"], inputFile["bondPrice"], inputFile["curveCollection"], inputFile["forecastCurve"], true);

		CheckTestResultsAndRebaseOnRequest(actual_zSpread, TEST_DIR.c_str(), zSpreadOutputFile_AUD.c_str(), getBondSpreadTolerance());

	}

	// --- USD Bond---

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_AssetSwapSpread_USD)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_USD_OIS, etrading::JSON);
		auto load3ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_USD_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_USD, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_USD, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadInputFile_USD.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Price Matching
		auto actualSpread = validation::tryAqAssetSwapObjectSpread(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadOutputFile_USD.c_str(), getBondSpreadTolerance());

	}

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_ZSpread_USD)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_USD_OIS, etrading::JSON);
		auto load3ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_USD_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_USD, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_USD, etrading::JSON);

		const ReadDataFile::Load inputFile(zSpreadInputFile_USD.c_str());

		//1) Check Price Matching
		auto actual_zSpread = validation::tryAqBondObjectZSpread(inputFile["bondObjectName"], inputFile["settlementDate"], inputFile["bondPrice"], inputFile["curveCollection"], inputFile["forecastCurve"], true);

		CheckTestResultsAndRebaseOnRequest(actual_zSpread, TEST_DIR.c_str(), zSpreadOutputFile_USD.c_str(), getBondSpreadTolerance());

	}


	// --- German Bond---

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_AssetSwapSpread_German)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);
		auto load6ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_6ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_German, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_German, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadInputFile_German.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Price Matching
		auto actualSpread = validation::tryAqAssetSwapObjectSpread(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadOutputFile_German.c_str(), getBondSpreadTolerance());

	}

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_ZSpread_German)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);
		auto load6ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_6ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_German, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_German, etrading::JSON);

		const ReadDataFile::Load inputFile(zSpreadInputFile_German.c_str());

		//1) Check Price Matching
		auto actual_zSpread = validation::tryAqBondObjectZSpread(inputFile["bondObjectName"], inputFile["settlementDate"], inputFile["bondPrice"], inputFile["curveCollection"], inputFile["forecastCurve"], true);

		CheckTestResultsAndRebaseOnRequest(actual_zSpread, TEST_DIR.c_str(), zSpreadOutputFile_German.c_str(), getBondSpreadTolerance());

	}

	// --- Dutch Bond---

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_AssetSwapSpread_Dutch)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);
		auto load6ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_6ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_Dutch, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_Dutch, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadInputFile_Dutch.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Price Matching
		auto actualSpread = validation::tryAqAssetSwapObjectSpread(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadOutputFile_Dutch.c_str(), getBondSpreadTolerance());

	}

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_ZSpread_Dutch)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);
		auto load6ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_6ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_Dutch, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_Dutch, etrading::JSON);

		const ReadDataFile::Load inputFile(zSpreadInputFile_Dutch.c_str());

		//1) Check Price Matching
		auto actual_zSpread = validation::tryAqBondObjectZSpread(inputFile["bondObjectName"], inputFile["settlementDate"], inputFile["bondPrice"], inputFile["curveCollection"], inputFile["forecastCurve"], true);

		CheckTestResultsAndRebaseOnRequest(actual_zSpread, TEST_DIR.c_str(), zSpreadOutputFile_Dutch.c_str(), getBondSpreadTolerance());

	}

	// --- French Bond---

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_AssetSwapSpread_French)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);
		auto load6ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_6ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_French, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_French, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadInputFile_French.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Price Matching
		auto actualSpread = validation::tryAqAssetSwapObjectSpread(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadOutputFile_French.c_str(), getBondSpreadTolerance());

	}

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_ZSpread_French)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);
		auto load6ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_6ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_French, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_French, etrading::JSON);

		const ReadDataFile::Load inputFile(zSpreadInputFile_French.c_str());

		//1) Check Price Matching
		auto actual_zSpread = validation::tryAqBondObjectZSpread(inputFile["bondObjectName"], inputFile["settlementDate"], inputFile["bondPrice"], inputFile["curveCollection"], inputFile["forecastCurve"], true);

		CheckTestResultsAndRebaseOnRequest(actual_zSpread, TEST_DIR.c_str(), zSpreadOutputFile_French.c_str(), getBondSpreadTolerance());

	}

	// --- Italian Bond---

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_AssetSwapSpread_Italian)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);
		auto load6ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_6ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_Italian, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_Italian, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadInputFile_Italian.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Price Matching
		auto actualSpread = validation::tryAqAssetSwapObjectSpread(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadOutputFile_Italian.c_str(), getBondSpreadTolerance());

	}

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_ZSpread_Italian)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);
		auto load6ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_EUR_6ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_Italian, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_Italian, etrading::JSON);

		const ReadDataFile::Load inputFile(zSpreadInputFile_Italian.c_str());

		//1) Check Price Matching
		auto actual_zSpread = validation::tryAqBondObjectZSpread(inputFile["bondObjectName"], inputFile["settlementDate"], inputFile["bondPrice"], inputFile["curveCollection"], inputFile["forecastCurve"], true);

		CheckTestResultsAndRebaseOnRequest(actual_zSpread, TEST_DIR.c_str(), zSpreadOutputFile_Italian.c_str(), getBondSpreadTolerance());

	}


	// --- JGB Bond---

	TEST_F(TestBondSpreads, SNAPSHOT_AssetSwapSpread_JGB)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_JPY_OIS, etrading::JSON);
		auto load6ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_JPY_6ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_JGB, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_JGB, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadInputFile_JGB.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Price Matching
		auto actualSpread = validation::tryAqAssetSwapObjectSpread(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadOutputFile_JGB.c_str(), getBondSpreadTolerance());

	}

	// Call Test Fixture Class
	TEST_F(TestBondSpreads, SNAPSHOT_ZSpread_JGB)
	{
		// Load Curves
		auto loadOIS = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_JPY_OIS, etrading::JSON);
		auto load6ML = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_JPY_6ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_Bond_JGB, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryAqObjectLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_JGB, etrading::JSON);

		const ReadDataFile::Load inputFile(zSpreadInputFile_JGB.c_str());

		//1) Check Price Matching
		auto actual_zSpread = validation::tryAqBondObjectZSpread(inputFile["bondObjectName"], inputFile["settlementDate"], inputFile["bondPrice"], inputFile["curveCollection"], inputFile["forecastCurve"], true);

		CheckTestResultsAndRebaseOnRequest(actual_zSpread, TEST_DIR.c_str(), zSpreadOutputFile_JGB.c_str(), getBondSpreadTolerance());

	}



};