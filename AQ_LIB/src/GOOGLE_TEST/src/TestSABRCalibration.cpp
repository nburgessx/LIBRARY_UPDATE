#include "Dependency.h"
#include "InitializeMLibGoogleTest.h"
#include "ParameterValidation.h"
#include "tryMeUtilitySetup.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"

#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LADateScheduleHelpers.h"
#include "LAMathDefine.h"
#include "LADataBasics.h"
#include "LAMathSwaptionVolUtility.h"
#include "LAMathParameterUtility.h"
#include "LAPriceCMSTools.h"
#include "LADateScheduleHelpers.h"
#include "LAPriceSABRSwaptionCalculator.h"
#include "LAMathOptionTools.h"
#include "ModelTools.h"

#include <vector>
#include <string>

// Include: Google Test Library
#include "ModelTools.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;
using namespace std;

#define TEST_DIR "ETrading/IRVol/SABR/"

namespace
{
	// test tolerance
	const double tolerance = 1e-8;

	// Input files
	extern const char DIR_SABR_INPUTS_1[]				= TEST_DIR "1_USD_SABR_inputs.csv";

	extern const char DIR_USD_SABR_CALIBRATION_INPUTS_1[]	= TEST_DIR "1_USD_SABR_Calibration_inputs.csv";
	extern const char DIR_USD_SABR_CONVENTION_INPUTS_1[]	= TEST_DIR "1_USD_tryMeSABRSetupConvention_inputs.csv";
	extern const char DIR_USD_SABR_ALPHA_INPUTS_1[]			= TEST_DIR "1_USD_tryMeSABRSetupParameter_Alpha_inputs.csv";
	extern const char DIR_USD_SABR_BETA_INPUTS_1[]			= TEST_DIR "1_USD_tryMeSABRSetupParameter_Beta_inputs.csv";
	extern const char DIR_USD_SABR_NU_INPUTS_1[]			= TEST_DIR "1_USD_tryMeSABRSetupParameter_Nu_inputs.csv";
	extern const char DIR_USD_SABR_RHO_INPUTS_1[]			= TEST_DIR "1_USD_tryMeSABRSetupParameter_Rho_inputs.csv";
	extern const char DIR_USD_SABR_FORWARDS_INPUTS_1[]		= TEST_DIR "1_USD_tryMeSABRSetupParameter_Forwards_inputs.csv";
	extern const char DIR_USD_SABR_ANNUITY_INPUTS_1[]		= TEST_DIR "1_USD_tryMeSABRSetupParameter_Annuity_inputs.csv";

	extern const vector<string> DIR_USD_SABR_VOL_INPUTS_1 = { TEST_DIR "1_USD_tryMeSABRSetupSwaptionVol_5delta_inputs.csv", TEST_DIR "1_USD_tryMeSABRSetupSwaptionVol_10delta_inputs.csv", TEST_DIR "1_USD_tryMeSABRSetupSwaptionVol_20delta_inputs.csv", TEST_DIR "1_USD_tryMeSABRSetupSwaptionVol_30delta_inputs.csv", TEST_DIR "1_USD_tryMeSABRSetupSwaptionVol_40delta_inputs.csv", TEST_DIR "1_USD_tryMeSABRSetupSwaptionVol_60delta_inputs.csv", TEST_DIR "1_USD_tryMeSABRSetupSwaptionVol_70delta_inputs.csv", TEST_DIR "1_USD_tryMeSABRSetupSwaptionVol_80delta_inputs.csv", TEST_DIR "1_USD_tryMeSABRSetupSwaptionVol_90delta_inputs.csv", TEST_DIR "1_USD_tryMeSABRSetupSwaptionVol_95delta_inputs.csv", TEST_DIR "1_USD_tryMeSABRSetupSwaptionVol_ATM_inputs.csv" };
	
	// Output files
	extern const char DIR_ALPHA_OUTPUT_32BIT[] = TEST_DIR "1_USD_SABR_Calibrated_ALPHA_ouputs_32bit.csv";
	extern const char DIR_ALPHA_OUTPUT_64BIT[] = TEST_DIR "1_USD_SABR_Calibrated_ALPHA_ouputs_64bit.csv";

	extern const char DIR_BETA_OUTPUT_32BIT[] = TEST_DIR "1_USD_SABR_Calibrated_BETA_ouputs_32bit.csv";
	extern const char DIR_BETA_OUTPUT_64BIT[] = TEST_DIR "1_USD_SABR_Calibrated_BETA_ouputs_64bit.csv";

	extern const char DIR_NU_OUTPUT_32BIT[] = TEST_DIR "1_USD_SABR_Calibrated_NU_ouputs_32bit.csv";
	extern const char DIR_NU_OUTPUT_64BIT[] = TEST_DIR "1_USD_SABR_Calibrated_NU_ouputs_64bit.csv";

	extern const char DIR_RHO_OUTPUT_32BIT[] = TEST_DIR "1_USD_SABR_Calibrated_RHO_ouputs_32bit.csv";
	extern const char DIR_RHO_OUTPUT_64BIT[] = TEST_DIR "1_USD_SABR_Calibrated_RHO_ouputs_64bit.csv";

	extern const char DIR_IMPLIEDVOL_OUTPUT_32BIT[] = TEST_DIR "1_USD_SABR_IMPLIED_VOL_ouputs_32bit.csv";
	extern const char DIR_IMPLIEDVOL_OUTPUT_64BIT[] = TEST_DIR "1_USD_SABR_IMPLIED_VOL_ouputs_64bit.csv";

	// A list of swap and expiry tenors
	const vector<string> TENOR_FOR_TEST { "3M", "6M", "1Y", "18M", "2Y", "3Y", "4Y", "4Y6M", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "11Y", "12Y", "13Y", "13Y3M", "14Y", "15Y", "17Y", "19Y", "20Y", "22Y", "25Y", "28Y", "30Y" };

	const double ATM_FORWARD = 0.02;
	const vector<double> STRIKE_SHIFTS { -0.001, -0.0008, -0.0005, -0.0003, -0.0001, 0.0, 0.0001, 0.0003, 0.0005, 0.0008, 0.001 };

	using namespace etrading;
	using namespace google_test;
	
	// Utility method that calibrates SABR model
	void calibrateSABR(LAStringVector& sabrIDs, LAString& forwardID, LAString& approxMethod, LADate& asOfDate)
	{
		LAString ccy("USD");

		// Set file object
		etrading::ReadDataFile::Load inputFile = etrading::ReadDataFile::Load(DIR_USD_SABR_CALIBRATION_INPUTS_1);

		// Create conventions
		etrading::ReadDataFile::Load conventionInput = etrading::ReadDataFile::Load(DIR_USD_SABR_CONVENTION_INPUTS_1);
		LAString convID = conventionInput["conventionID"];
		LAStringMatrix convData = conventionInput["convData"];
		LAMathSwaptionVolUtility::setUpConvention(etrading::getDataInstance(), convID, convData);

		// Setup initial Alpha grid
		etrading::ReadDataFile::Load alphaInput = etrading::ReadDataFile::Load(DIR_USD_SABR_ALPHA_INPUTS_1);
		LAString alphaID = alphaInput["gridID"];
		sabrIDs.push_back(alphaID);
		LAStringMatrix alphaMatrix = alphaInput["gridData"];
		LAMathSwaptionVolUtility::setUpSABRGrid(getDataInstance(), alphaID, convID, alphaMatrix);

		// Setup initial Beta grid
		etrading::ReadDataFile::Load betaInput = etrading::ReadDataFile::Load(DIR_USD_SABR_BETA_INPUTS_1);
		LAString betaID = betaInput["gridID"];
		sabrIDs.push_back(betaID);
		LAStringMatrix betaMatrix = betaInput["gridData"];
		LAMathSwaptionVolUtility::setUpSABRGrid(getDataInstance(), betaID, convID, betaMatrix);

		// Setup initial Nu grid
		etrading::ReadDataFile::Load nuInput = etrading::ReadDataFile::Load(DIR_USD_SABR_NU_INPUTS_1);
		LAString nuID = nuInput["gridID"];
		sabrIDs.push_back(nuID);
		LAStringMatrix nuMatrix = nuInput["gridData"];
		LAMathSwaptionVolUtility::setUpSABRGrid(getDataInstance(), nuID, convID, nuMatrix);

		// Setup initial Rho grid
		etrading::ReadDataFile::Load rhoInput = etrading::ReadDataFile::Load(DIR_USD_SABR_RHO_INPUTS_1);
		LAString rhoID = rhoInput["gridID"];
		sabrIDs.push_back(rhoID);
		LAStringMatrix rhoMatrix = rhoInput["gridData"];
		LAMathSwaptionVolUtility::setUpSABRGrid(getDataInstance(), rhoID, convID, rhoMatrix);

		// Setup forward grid
		etrading::ReadDataFile::Load forwardsInput = etrading::ReadDataFile::Load(DIR_USD_SABR_FORWARDS_INPUTS_1);
		LAString temp = forwardsInput["gridID"];
		forwardID = temp;
		LAStringMatrix forwardMatrix = forwardsInput["gridData"];
		LAMathSwaptionVolUtility::setUpSABRGrid(getDataInstance(), forwardID, convID, forwardMatrix);

		// Setup annuity grid
		etrading::ReadDataFile::Load annuityInput = etrading::ReadDataFile::Load(DIR_USD_SABR_ANNUITY_INPUTS_1);
		LAString annuityID = annuityInput["gridID"];
		LAStringMatrix annuityMatrix = annuityInput["gridData"];
		LAMathSwaptionVolUtility::setUpSABRGrid(getDataInstance(), annuityID, convID, annuityMatrix);

		// Set up target vols and strikes
		LAStringVector volSmileIDs;
		for (const auto& volDataDir : DIR_USD_SABR_VOL_INPUTS_1)
		{
			etrading::ReadDataFile::Load volInput = etrading::ReadDataFile::Load(volDataDir.c_str());

			LAStringMatrix volMatrix = volInput["volMat"];
			LAStringMatrix strikeMatrix = volInput["strikeMat"];
			LAString volSmileID = volInput["gridID"];
			volSmileIDs.push_back(volSmileID);
			LAStringMatrix signMat;
			LAMathSwaptionVolUtility::setUpSwaptionVol(getDataInstance(), volSmileID, volMatrix, strikeMatrix, signMat);
		}

		// Get other inputs
		LAString temp2 = inputFile["approxMethod"];
		approxMethod = temp2;
		LAString calibMethod = inputFile["calibMethod"];
		LAString calibTarget = inputFile["target"];

		// Get calibration flags
		LAStringVector calibFlagString = inputFile["calibFlag"];
		std::vector<bool> calibFlag;
		for (const auto& calibflag : calibFlagString)
		{
			if (calibflag == "TRUE")
			{
				calibFlag.push_back(true);
			}
			else
			{
				calibFlag.push_back(false);
			}
		}

		// Get weights
		LAStringVector weightsStr = inputFile["weight"];
		DoubleVector weights;
		for (const auto& weightStr : weightsStr)
		{
            char * pFirstNonNumber;
			weights.push_back( std::strtod(weightStr.getCString(), &pFirstNonNumber) );
		}

		// Get signs
		LAStringVector signsStr = inputFile["sign"];
		std::vector<int> signs;
		for (const auto& signStr : signsStr)
		{
            char * pFirstNonNumber;
			signs.push_back(std::strtol(signStr.getCString(), &pFirstNonNumber, 10 )); // base 10 numbers
		}

		// Calibrate SABR parameters
		LAString msg;
		LAMathSwaptionVolUtility::calibrateSABRMatrix(getDataInstance(),
													approxMethod,
													calibFlag,
													calibMethod,
													"",	// curveSetName,
													alphaID,
													betaID,
													nuID,
													rhoID,
													convID,
													"",	// capConvID,
													volSmileIDs,
													calibTarget,
													weights,
													signs,
													forwardID,
													0.0, // forwardShiftValue,
													annuityID,
													msg);

		// As of date
		asOfDate = google_test::ModelUtility::AsOfDate(convID);
	}

	// Utility method that calibrates SABR model
	void calibrateSABR(LAStringVector& sabrIDs)
	{
		LAString forwardID, approxMethod;
		LADate asofdate;
		calibrateSABR(sabrIDs, forwardID, approxMethod, asofdate);
	}

	// Utility method that calibrates SABR model
	void calibrateSABR(LAStringVector& sabrIDs, LADate& asOfDate)
	{
		LAString forwardID, approxMethod;
		calibrateSABR(sabrIDs, forwardID, approxMethod, asOfDate);
	}

	// Utility method that calibrates SABR model
	void calibrateSABR(LAStringVector& sabrIDs, LAString& forwardID, LAString& approxMethod)
	{
		LADate asofdate;
		calibrateSABR(sabrIDs, forwardID, approxMethod, asofdate);
	}


}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestSABRCalibration);

	//
	// Regression test on the calibration SABR parameters
	//
    TEST_F(TestSABRCalibration, SNAPSHOT_USD_SABR_Parameters)
    {
		// Calibrate SABR
		LAStringVector sabrIDs;
		calibrateSABR(sabrIDs);

		if (sabrIDs.size() != 4)
		{
			AQ_THROW("Failure to retrieve all four SABR parameters");
		}

		LAString alphaID = sabrIDs[0];
		LAString betaID = sabrIDs[1];
		LAString nuID = sabrIDs[2];
		LAString rhoID = sabrIDs[3];

		size_t row, column;
		DoubleVector calibratedAlpha;
		DoubleVector calibratedBeta;
		DoubleVector calibratedNu;
		DoubleVector calibratedRho;

		// Retrieve calibrated SABR parameters
		LAMathSwaptionVolUtility::outPutSABRGrid(getDataInstance(), alphaID, calibratedAlpha, row, column);
		LAMathSwaptionVolUtility::outPutSABRGrid(getDataInstance(), betaID, calibratedBeta, row, column);
		LAMathSwaptionVolUtility::outPutSABRGrid(getDataInstance(), nuID, calibratedNu, row, column);
		LAMathSwaptionVolUtility::outPutSABRGrid(getDataInstance(), rhoID, calibratedRho, row, column);

#ifdef GTEST32
		LAString outputFileName_alph = DIR_ALPHA_OUTPUT_32BIT;
		LAString outputFileName_beta = DIR_BETA_OUTPUT_32BIT;
		LAString outputFileName_nu	= DIR_NU_OUTPUT_32BIT;
		LAString outputFileName_rho = DIR_RHO_OUTPUT_32BIT;
#else
		LAString outputFileName_alph = DIR_ALPHA_OUTPUT_64BIT;
		LAString outputFileName_beta = DIR_BETA_OUTPUT_64BIT;
		LAString outputFileName_nu = DIR_NU_OUTPUT_64BIT;
		LAString outputFileName_rho = DIR_RHO_OUTPUT_64BIT;
#endif

		CheckTestResultsAndRebaseOnRequest(calibratedAlpha, TEST_DIR, outputFileName_alph, tolerance);
		CheckTestResultsAndRebaseOnRequest(calibratedBeta, TEST_DIR, outputFileName_beta, tolerance);
		CheckTestResultsAndRebaseOnRequest(calibratedNu, TEST_DIR, outputFileName_nu, tolerance);
		CheckTestResultsAndRebaseOnRequest(calibratedRho, TEST_DIR, outputFileName_rho, tolerance);

		// Flush the cache in preparation for a new set of curves
		validation_api::tryMeUtilityClearEntityPool();
		
    }

	//
	// Consistency test that compare implied vol on calibration points against input vols of the same calib point
	//
	//TEST_F(TestSABRCalibration, CONSISTENCY_USD_SwaptionVol)
	//{
	//	// Set file object
	//	etrading::ReadDataFile::Load inputMarketFile = etrading::ReadDataFile::Load(DIR_SABR_MARKETDATA_INPUTS_1);
	//	etrading::ReadDataFile::Load inputFile = etrading::ReadDataFile::Load(DIR_SABR_INPUTS_1);

	//	// Calibrate SABR parameters
	//	LAStringVector sabrIDs;
	//	LAString forwardID, approxMethod;
	//	calibrateSABR(sabrIDs, forwardID, approxMethod);

	//	if (sabrIDs.size() != 4)
	//	{
	//		AQ_THROW("Failure to retrieve all four SABR parameters");
	//	}

	//	LAString alphaID = sabrIDs[0];
	//	LAString betaID = sabrIDs[1];
	//	LAString nuID = sabrIDs[2];
	//	LAString rhoID = sabrIDs[3];

	//	// Loop through each vol matrix and examine the accuracy of implied vol for each pair of expiry and swap tenor
	//	LAStringVector volSmileIDs = inputFile["SwaptionVolID"];
	//	for (const auto& volSmileID : volSmileIDs)
	//	{
	//		LAString volStrikeID = volSmileID.subString(0, volSmileID.size() - 5) + "_strike";

	//		LAStringMatrix volMatrix = inputMarketFile[volSmileID];
	//		LAStringMatrix strikeMatrix = inputMarketFile[volStrikeID];

	//		size_t expiryTenorCount = volMatrix.size();
	//		size_t swapTenorCount = volMatrix[0].size();

	//		for (size_t i = 1; i < expiryTenorCount; ++i)
	//		{
	//			LAString expiryTenor = volMatrix[i][0];

	//			for (size_t j = 1; j < swapTenorCount; ++j)
	//			{
	//				LAString swapTenor = volMatrix[0][j];

	//				// A vol smile is identified by each pair of expiryTenor and swapTenor 
	//				// For each vol smile, loop through the calibration points and get the implied vol for each calibration point
	//				//for (auto iter = allVolsAndStrikes.begin(); iter != allVolsAndStrikes.end(); ++iter)
    
    //              char * pFirstNonNumber1;
	//				double strike = std::strtod(strikeMatrix[i][j].getCString(), &pFirstNonNumber1);
	//
    //              char * pFirstNonNumber2;
    //				double vol = std::strtod(volMatrix[i][j].getCString(), &pFirstNonNumber2);

	//				double ret = LAMathSwaptionVolUtility::getSABRVol(getDataInstance(),
	//																expiryTenor,
	//																swapTenor,
	//																strike,
	//																forwardID,
	//																alphaID,
	//																betaID,
	//																nuID,
	//																rhoID,
	//																approxMethod == "" ? "Hagan" : approxMethod);

	//				if (abs(ret - vol) > tolerance)
	//				{
	//					EXPECT_NEAR(ret, vol, tolerance) << "Difference between implied vol and input vol exceeds tolerance at expiry: " + expiryTenor + ", tenor: " + swapTenor + " and vol smile ID: " + volSmileID << std::endl;
	//				}
	//			}
	//		}
	//	}
	//}


	//
	// Regression tests on off-tenor-point implied vols
	//
	TEST_F(TestSABRCalibration, SNAPSHOT_USD_ImpliedSwaptionVol)
	{
		// Set file object
		etrading::ReadDataFile::Load inputFile = etrading::ReadDataFile::Load(DIR_SABR_INPUTS_1);

		// Calibrate SABR parameters
		LAStringVector sabrIDs;
		LADate asOfDate;
		calibrateSABR(sabrIDs, asOfDate);

		if (sabrIDs.size() != 4)
		{
			AQ_THROW("Failure to retrieve all four SABR parameters");
		}
						
		// Loop through a predefined list of swap and expiry tenors
		DoubleVector impliedVols;
		for (const auto& swapTenor : TENOR_FOR_TEST)
		{
			for (const auto& expiryTenor : TENOR_FOR_TEST)
			{
				// Get expiry date and term
				LADate expiryDate = etrading::LADateScheduleHelpers::getDate(asOfDate, LAString(expiryTenor.c_str()), "", "");
				LAString expiryDayCount("ACT/365");
				double expiryTerm = etrading::LADateScheduleHelpers::getTerm(asOfDate, expiryDate, expiryDayCount);

				// Build SABR model using SABR parameters interpolated according the expiry and swap tenor
				DoubleVector sabrParameters = LAPriceCMSObject::InterpolateParameterMatrix(getDataInstance(), expiryDate, LAString(swapTenor.c_str()), sabrIDs);
				LAMathSABR_Hagan sabrModel = LAMathSABR_Hagan(sabrParameters);

				for (const auto& strikeShift : STRIKE_SHIFTS)
				{
					double strike = ATM_FORWARD + strikeShift;
					double vol = sabrModel.getSABRVol(expiryTerm, ATM_FORWARD, strike);
					impliedVols.push_back(vol);
				}
			}
		}

#ifdef GTEST32
		LAString outputFileName = DIR_IMPLIEDVOL_OUTPUT_32BIT;
#else
		LAString outputFileName = DIR_IMPLIEDVOL_OUTPUT_64BIT;
#endif
				
		CheckTestResultsAndRebaseOnRequest(impliedVols, TEST_DIR, outputFileName, tolerance);
	}
		

		

	
}