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

#include "tryMeLWO.h"
#include "tryMeLWOAssetSwap.h"
//#include "tryMeLWOBond.h"

#include "CreateDataFile.h"
#include "TestHelperUtilities.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

namespace
{
	// Test Input File(s) 
	const std::string TEST_DIR = "ETrading/Bonds/FRNSpreads/";

	// --- Curves---

	const std::string fileName_EUR_OIS = TEST_DIR + "EUR_OIS_CURVE.JSON";
	const std::string fileName_EUR_3ML = TEST_DIR + "EUR_SWAP_3M_CURVE.JSON";

	const std::string fileName_GBP_OIS = TEST_DIR + "GBP_OIS_CURVE.JSON";
	const std::string fileName_GBP_3ML = TEST_DIR + "GBP_SWAP_3M_CURVE.JSON";

	const std::string fileName_USD_OIS = TEST_DIR + "USD_OIS_CURVE.JSON";
	const std::string fileName_USD_3ML = TEST_DIR + "USD_SWAP_3M_CURVE.JSON";

	// --- EUR Floater ---
    const std::string fileName_Bond_EUR_Floater	= TEST_DIR + "XS1781813511.JSON";
	const std::string fileName_AssetSwap_EUR_Floater = TEST_DIR + "XS1781813511_ASSET_SWAP.JSON";

	const std::string assetSwapSpreadInputFile_EUR_Floater = TEST_DIR + "XS1781813511_ASSET_SWAP_tryMeLWOAssetSwapSpread_inputs";
	const std::string assetSwapSpreadOutputFile_EUR_Floater = TEST_DIR + "XS1781813511_ASSET_SWAP_tryMeLWOAssetSwapSpread_outputs";

	const std::string assetSwapSpreadFixedEqvCouponInputFile_EUR_Floater = TEST_DIR + "XS1781813511_ASSET_SWAP_tryMeLWOAssetSwapFixedEqvCoupon_inputs";
	const std::string assetSwapSpreadFixedEqvCouponOutputFile_EUR_Floater = TEST_DIR + "XS1781813511_ASSET_SWAP_tryMeLWOAssetSwapFixedEqvCoupon_outputs";

	const std::string assetSwapSpreadFromFixedEqvCouponInputFile_EUR_Floater = TEST_DIR + "XS1781813511_ASSET_SWAP_tryMeLWOAssetSwapSpreadFromFixedEqvCoupon_inputs";
	const std::string assetSwapSpreadFromFixedEqvCouponOutputFile_EUR_Floater = TEST_DIR + "XS1781813511_ASSET_SWAP_tryMeLWOAssetSwapSpreadFromFixedEqvCoupon_outputs";

	// --- EUR Floater PaymentDate UnAdjusted---
	const std::string fileName_Bond_EUR_Floater_UNADJUSTED = TEST_DIR + "DE000A19X793.JSON";
	const std::string fileName_AssetSwap_EUR_Floater_UNADJUSTED = TEST_DIR + "DE000A19X793_ASSET_SWAP.JSON";

	const std::string assetSwapSpreadInputFile_EUR_Floater_UNADJUSTED = TEST_DIR + "DE000A19X793_ASSET_SWAP_tryMeLWOAssetSwapSpread_inputs";
	const std::string assetSwapSpreadOutputFile_EUR_Floater_UNADJUSTED = TEST_DIR + "DE000A19X793_ASSET_SWAP_tryMeLWOAssetSwapSpread_outputs";

	const std::string assetSwapSpreadFixedEqvCouponInputFile_EUR_Floater_UNADJUSTED = TEST_DIR + "DE000A19X793_ASSET_SWAP_tryMeLWOAssetSwapFixedEqvCoupon_inputs";
	const std::string assetSwapSpreadFixedEqvCouponOutputFile_EUR_Floater_UNADJUSTED = TEST_DIR + "DE000A19X793_ASSET_SWAP_tryMeLWOAssetSwapFixedEqvCoupon_outputs";

	const std::string assetSwapSpreadFromFixedEqvCouponInputFile_EUR_Floater_UNADJUSTED = TEST_DIR + "DE000A19X793_ASSET_SWAP_tryMeLWOAssetSwapSpreadFromFixedEqvCoupon_inputs";
	const std::string assetSwapSpreadFromFixedEqvCouponOutputFile_EUR_Floater_UNADJUSTED = TEST_DIR + "DE000A19X793_ASSET_SWAP_tryMeLWOAssetSwapSpreadFromFixedEqvCoupon_outputs";

	// --- EUR Floater PaymentDate UnAdjusted ACTACT---
	const std::string fileName_Bond_EUR_Floater_UNADJUSTED_ACTACT = TEST_DIR + "XS1794195724.JSON";
	const std::string fileName_AssetSwap_EUR_Floater_UNADJUSTED_ACTACT = TEST_DIR + "XS1794195724_ASSET_SWAP.JSON";

	const std::string assetSwapSpreadInputFile_EUR_Floater_UNADJUSTED_ACTACT = TEST_DIR + "XS1794195724_ASSET_SWAP_tryMeLWOAssetSwapSpread_inputs";
	const std::string assetSwapSpreadOutputFile_EUR_Floater_UNADJUSTED_ACTACT = TEST_DIR + "XS1794195724_ASSET_SWAP_tryMeLWOAssetSwapSpread_outputs";

	const std::string assetSwapSpreadFixedEqvCouponInputFile_EUR_Floater_UNADJUSTED_ACTACT = TEST_DIR + "XS1794195724_ASSET_SWAP_tryMeLWOAssetSwapFixedEqvCoupon_inputs";
	const std::string assetSwapSpreadFixedEqvCouponOutputFile_EUR_Floater_UNADJUSTED_ACTACT = TEST_DIR + "XS1794195724_ASSET_SWAP_tryMeLWOAssetSwapFixedEqvCoupon_outputs";

	const std::string assetSwapSpreadFromFixedEqvCouponInputFile_EUR_Floater_UNADJUSTED_ACTACT = TEST_DIR + "XS1794195724_ASSET_SWAP_tryMeLWOAssetSwapSpreadFromFixedEqvCoupon_inputs";
	const std::string assetSwapSpreadFromFixedEqvCouponOutputFile_EUR_Floater_UNADJUSTED_ACTACT = TEST_DIR + "XS1794195724_ASSET_SWAP_tryMeLWOAssetSwapSpreadFromFixedEqvCoupon_outputs";

	// --- GBP Floater ---
	const std::string fileName_Bond_GBP_Floater = TEST_DIR + "XS1082839553.JSON";
	const std::string fileName_AssetSwap_GBP_Floater = TEST_DIR + "XS1082839553_ASSET_SWAP.JSON";

	const std::string assetSwapSpreadInputFile_GBP_Floater = TEST_DIR + "XS1082839553_ASSET_SWAP_tryMeLWOAssetSwapSpread_inputs";
	const std::string assetSwapSpreadOutputFile_GBP_Floater = TEST_DIR + "XS1082839553_ASSET_SWAP_tryMeLWOAssetSwapSpread_outputs";

	const std::string assetSwapSpreadFixedEqvCouponInputFile_GBP_Floater = TEST_DIR + "XS1082839553_ASSET_SWAP_tryMeLWOAssetSwapFixedEqvCoupon_inputs";
	const std::string assetSwapSpreadFixedEqvCouponOutputFile_GBP_Floater = TEST_DIR + "XS1082839553_ASSET_SWAP_tryMeLWOAssetSwapFixedEqvCoupon_outputs";

	const std::string assetSwapSpreadFromFixedEqvCouponInputFile_GBP_Floater = TEST_DIR + "XS1082839553_ASSET_SWAP_tryMeLWOAssetSwapSpreadFromFixedEqvCoupon_inputs";
	const std::string assetSwapSpreadFromFixedEqvCouponOutputFile_GBP_Floater = TEST_DIR + "XS1082839553_ASSET_SWAP_tryMeLWOAssetSwapSpreadFromFixedEqvCoupon_outputs";

	// --- USD Floater ---
	const std::string fileName_Bond_USD_Floater = TEST_DIR + "US30231GAE26.JSON";
	const std::string fileName_AssetSwap_USD_Floater = TEST_DIR + "US30231GAE26_ASSET_SWAP.JSON";

	const std::string assetSwapSpreadInputFile_USD_Floater = TEST_DIR + "US30231GAE26_ASSET_SWAP_tryMeLWOAssetSwapSpread_inputs";
	const std::string assetSwapSpreadOutputFile_USD_Floater = TEST_DIR + "US30231GAE26_ASSET_SWAP_tryMeLWOAssetSwapSpread_outputs";

	const std::string assetSwapSpreadFixedEqvCouponInputFile_USD_Floater = TEST_DIR + "US30231GAE26_ASSET_SWAP_tryMeLWOAssetSwapFixedEqvCoupon_inputs";
	const std::string assetSwapSpreadFixedEqvCouponOutputFile_USD_Floater = TEST_DIR + "US30231GAE26_ASSET_SWAP_tryMeLWOAssetSwapFixedEqvCoupon_outputs";

	const std::string assetSwapSpreadFromFixedEqvCouponInputFile_USD_Floater = TEST_DIR + "US30231GAE26_ASSET_SWAP_tryMeLWOAssetSwapSpreadFromFixedEqvCoupon_inputs";
	const std::string assetSwapSpreadFromFixedEqvCouponOutputFile_USD_Floater = TEST_DIR + "US30231GAE26_ASSET_SWAP_tryMeLWOAssetSwapSpreadFromFixedEqvCoupon_outputs";

}

namespace google_test
{

	// Declare Test Fixture Class
	DECLARE_TEST_FIXTURE(TestBondFRNSpreads)

	// --- EUR Floating Bonds---

	// Call Test Fixture Class
	TEST_F(TestBondFRNSpreads, SNAPSHOT_AssetSwapSpread_EUR_Floater)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_Bond_EUR_Floater, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_EUR_Floater, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadInputFile_EUR_Floater.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Asset Swap Spread Matching
		auto actualSpread = validation::tryMeLWOAssetSwapSpread(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadOutputFile_EUR_Floater.c_str(), getBondSpreadTolerance());

	}

	TEST_F(TestBondFRNSpreads, SNAPSHOT_FixedEqvCoupon_EUR_Floater)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_Bond_EUR_Floater, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_EUR_Floater, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadFixedEqvCouponInputFile_EUR_Floater.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Fixed Equivalent Rate Matching
		auto actualRate = validation::tryMeLWOAssetSwapFixedEqvCoupon(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualRate, TEST_DIR.c_str(), assetSwapSpreadFixedEqvCouponOutputFile_EUR_Floater.c_str(), getBondSpreadTolerance());

	}

	TEST_F(TestBondFRNSpreads, SNAPSHOT_AssetSwapSpread_FromFixedEqvCoupon_EUR_Floater)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);

		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_EUR_Floater, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadFromFixedEqvCouponInputFile_EUR_Floater.c_str());

		std::string swapName = inputFile["swapObjectName"];
		double fixedEqvCoupon = inputFile["fixedEqvCoupon"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Asset Spread Matching
		auto actualSpread = validation::tryMeLWOAssetSwapSpreadFromFixedEqvCoupon(swapName, fixedEqvCoupon, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadFromFixedEqvCouponOutputFile_EUR_Floater.c_str(), getBondSpreadTolerance());
	}


	// Call Test Fixture Class
	TEST_F(TestBondFRNSpreads, SNAPSHOT_AssetSwapSpread_EUR_Floater_UNADJUSTED)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_Bond_EUR_Floater_UNADJUSTED, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_EUR_Floater_UNADJUSTED, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadInputFile_EUR_Floater_UNADJUSTED.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Price Matching
		auto actualSpread = validation::tryMeLWOAssetSwapSpread(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadOutputFile_EUR_Floater_UNADJUSTED.c_str(), getBondSpreadTolerance());

	}

	TEST_F(TestBondFRNSpreads, SNAPSHOT_FixedEqvCoupon_EUR_Floater_UNADJUSTED)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_Bond_EUR_Floater_UNADJUSTED, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_EUR_Floater_UNADJUSTED, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadFixedEqvCouponInputFile_EUR_Floater_UNADJUSTED.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Fixed Equivalent Rate Matching
		auto actualRate = validation::tryMeLWOAssetSwapFixedEqvCoupon(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualRate, TEST_DIR.c_str(), assetSwapSpreadFixedEqvCouponOutputFile_EUR_Floater_UNADJUSTED.c_str(), getBondSpreadTolerance());

	}

	TEST_F(TestBondFRNSpreads, SNAPSHOT_AssetSwapSpread_FromFixedEqvCoupon_EUR_Floater_UNADJUSTED)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);

		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_EUR_Floater_UNADJUSTED, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadFromFixedEqvCouponInputFile_EUR_Floater_UNADJUSTED.c_str());

		std::string swapName = inputFile["swapObjectName"];
		double fixedEqvCoupon = inputFile["fixedEqvCoupon"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Asset Spread Matching
		auto actualSpread = validation::tryMeLWOAssetSwapSpreadFromFixedEqvCoupon(swapName, fixedEqvCoupon, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadFromFixedEqvCouponOutputFile_EUR_Floater_UNADJUSTED.c_str(), getBondSpreadTolerance());
	}


	// Call Test Fixture Class
	TEST_F(TestBondFRNSpreads, SNAPSHOT_AssetSwapSpread_EUR_Floater_UNADJUSTED_ACTACT)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_Bond_EUR_Floater_UNADJUSTED_ACTACT, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_EUR_Floater_UNADJUSTED_ACTACT, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadInputFile_EUR_Floater_UNADJUSTED_ACTACT.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Price Matching
		auto actualSpread = validation::tryMeLWOAssetSwapSpread(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadOutputFile_EUR_Floater_UNADJUSTED_ACTACT.c_str(), getBondSpreadTolerance());

	}

	TEST_F(TestBondFRNSpreads, SNAPSHOT_FixedEqvCoupon_EUR_Floater_UNADJUSTED_ACTACT)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_Bond_EUR_Floater_UNADJUSTED_ACTACT, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_EUR_Floater_UNADJUSTED_ACTACT, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadFixedEqvCouponInputFile_EUR_Floater_UNADJUSTED_ACTACT.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Fixed Equivalent Rate Matching
		auto actualRate = validation::tryMeLWOAssetSwapFixedEqvCoupon(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualRate, TEST_DIR.c_str(), assetSwapSpreadFixedEqvCouponOutputFile_EUR_Floater_UNADJUSTED_ACTACT.c_str(), getBondSpreadTolerance());

	}

	TEST_F(TestBondFRNSpreads, SNAPSHOT_AssetSwapSpread_FromFixedEqvCoupon_EUR_Floater_UNADJUSTED_ACTACT)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_EUR_3ML, etrading::JSON);

		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_EUR_Floater_UNADJUSTED_ACTACT, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadFromFixedEqvCouponInputFile_EUR_Floater_UNADJUSTED_ACTACT.c_str());

		std::string swapName = inputFile["swapObjectName"];
		double fixedEqvCoupon = inputFile["fixedEqvCoupon"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Asset Spread Matching
		auto actualSpread = validation::tryMeLWOAssetSwapSpreadFromFixedEqvCoupon(swapName, fixedEqvCoupon, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadFromFixedEqvCouponOutputFile_EUR_Floater_UNADJUSTED_ACTACT.c_str(), getBondSpreadTolerance());
	}

	// --- GBP Floating Bonds---

	// Call Test Fixture Class
	TEST_F(TestBondFRNSpreads, SNAPSHOT_AssetSwapSpread_GBP_Floater)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_GBP_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_GBP_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_Bond_GBP_Floater, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_GBP_Floater, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadInputFile_GBP_Floater.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Price Matching
		auto actualSpread = validation::tryMeLWOAssetSwapSpread(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadOutputFile_GBP_Floater.c_str(), getBondSpreadTolerance());

	}

	TEST_F(TestBondFRNSpreads, SNAPSHOT_FixedEqvCoupon_GBP_Floater)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_GBP_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_GBP_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_Bond_GBP_Floater, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_GBP_Floater, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadFixedEqvCouponInputFile_GBP_Floater.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Fixed Equivalent Rate Matching
		auto actualRate = validation::tryMeLWOAssetSwapFixedEqvCoupon(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualRate, TEST_DIR.c_str(), assetSwapSpreadFixedEqvCouponOutputFile_GBP_Floater.c_str(), getBondSpreadTolerance());

	}

	TEST_F(TestBondFRNSpreads, SNAPSHOT_AssetSwapSpread_FromFixedEqvCoupon_GBP_Floater)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_GBP_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_GBP_3ML, etrading::JSON);

		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_GBP_Floater, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadFromFixedEqvCouponInputFile_GBP_Floater.c_str());

		std::string swapName = inputFile["swapObjectName"];
		double fixedEqvCoupon = inputFile["fixedEqvCoupon"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Asset Spread Matching
		auto actualSpread = validation::tryMeLWOAssetSwapSpreadFromFixedEqvCoupon(swapName, fixedEqvCoupon, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadFromFixedEqvCouponOutputFile_GBP_Floater.c_str(), getBondSpreadTolerance());
	}


	// --- USD Floating Bonds---

	// Call Test Fixture Class
	TEST_F(TestBondFRNSpreads, SNAPSHOT_AssetSwapSpread_USD_Floater)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USD_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USD_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_Bond_USD_Floater, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_USD_Floater, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadInputFile_USD_Floater.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Price Matching
		auto actualSpread = validation::tryMeLWOAssetSwapSpread(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadOutputFile_USD_Floater.c_str(), getBondSpreadTolerance());

	}

	TEST_F(TestBondFRNSpreads, SNAPSHOT_FixedEqvCoupon_USD_Floater)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USD_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USD_3ML, etrading::JSON);

		// Load Bond
		auto loadBond = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_Bond_USD_Floater, etrading::JSON);
		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_USD_Floater, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadFixedEqvCouponInputFile_USD_Floater.c_str());

		std::string swapName = inputFile["swapObjectName"];
		std::string bondName = inputFile["bondObjectName"];
		double bondPrice = inputFile["bondPrice"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Fixed Equivalent Rate Matching
		auto actualRate = validation::tryMeLWOAssetSwapFixedEqvCoupon(swapName, bondName, bondPrice, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualRate, TEST_DIR.c_str(), assetSwapSpreadFixedEqvCouponOutputFile_USD_Floater.c_str(), getBondSpreadTolerance());

	}

	TEST_F(TestBondFRNSpreads, SNAPSHOT_AssetSwapSpread_FromFixedEqvCoupon_USD_Floater)
	{
		// Load Curves
		auto loadOIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USD_OIS, etrading::JSON);
		auto load3ML = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_USD_3ML, etrading::JSON);

		// Load Swap
		auto loadSwap = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + fileName_AssetSwap_USD_Floater, etrading::JSON);

		const ReadDataFile::Load inputFile(assetSwapSpreadFromFixedEqvCouponInputFile_USD_Floater.c_str());

		std::string swapName = inputFile["swapObjectName"];
		double fixedEqvCoupon = inputFile["fixedEqvCoupon"];
		AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];
		AQLStringMatrix fixingTableNames = inputFile["fixingTableNames"];

		//1) Check Asset Spread Matching
		auto actualSpread = validation::tryMeLWOAssetSwapSpreadFromFixedEqvCoupon(swapName, fixedEqvCoupon, valuationSettingsLVB, fixingTableNames);

		CheckTestResultsAndRebaseOnRequest(actualSpread, TEST_DIR.c_str(), assetSwapSpreadFromFixedEqvCouponOutputFile_USD_Floater.c_str(), getBondSpreadTolerance());
	}

	

	

};