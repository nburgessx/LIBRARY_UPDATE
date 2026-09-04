 // Curves
#include "tryAqObjects.h"

// API functions
#include "tryAqObjSwapsPricing.h"


// Helper to extract par rates from a curve
#include "ExtractCurveCalibrationData.h"

// Test Infrastructure
#include "Dependency.h"   // Curve Macros are Here !!!
#include "GetGoogleTestFolder.h"
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"

// etrading
#include "AQLMathDateUtilities.h"
#include "AQLDateScheduleHelpers.h"
#include "AQOUtilities.h"



using etrading::ReadDataFile;




namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 1e-8;

	// Define the Test Input Folder Here
	const std::string TEST_DIR =		  "ETrading/AQObjects/TestAQOCreditIndexOption/";

    //
    // curve input files
    //

	// -------------------------------------------------------------

	const std::string EUR_OIS			= TEST_DIR + "EUR_OIS_CURVE@24.JSON";


	// -------------------------------------------------------------

	// Build CDS Conventions Generator
	const std::string GEN_EUR_CDSINDEX = TEST_DIR + "EUR_CDS_GENERATOR@62.JSON";


	// Build Credit Model
	const std::string EUR_CREDIT_MODEL			        = TEST_DIR + "CREDITMODEL1@64.JSON";
	const std::string EUR_CREDIT_MODEL_FLAT				= TEST_DIR + "FLATCURVE@66.JSON";

	// Build Credit Default Swap
	const std::string GEN_CREDIT_DEFAULT_SWAP_5Y		= TEST_DIR + "EUR_CDS_GENERATOR@62.JSON" ;

	// API methods
	const std::string CDS_INDEX_CALCULATE_FORWARD_SPREAD = TEST_DIR + "tryAqObjCreditIndexSpread_inputs.csv";
	const std::string CDS_INDEX_OPTION_PV				 = TEST_DIR + "tryAqObjCreditIndexOptionPV_inputs.csv";
	const std::string CDS_INDEX_OPTION_IMPLIED_VOL		 = TEST_DIR + "tryAqObjCreditIndexOptionImpliedVol_inputs.csv";
	const std::string CDS_INDEX_OPTION_VEGA				 = TEST_DIR + "tryAqObjCreditIndexOptionVega_inputs.csv";
	const std::string CDS_INDEX_OPTION_CS01				 = TEST_DIR + "tryAqObjCreditIndexOptionCS01_inputs.csv";
	const std::string CDS_INDEX_OPTION_THETA			 = TEST_DIR + "tryAqObjCreditIndexOptionTheta_inputs.csv";

	// Snapshot results
	const std::string EXPECTED_CDS_INDEX_CALCULATE_FORWARD_SPREAD	= TEST_DIR + "tryAqObjCreditIndexSpread_outputs.csv";
	const std::string EXPECTED_CDS_INDEX_OPTION_PV					= TEST_DIR + "tryAqObjCreditIndexOptionPV_outputs.csv";
	const std::string EXPECTED_CDS_INDEX_OPTION_IMPLIED_VOL			= TEST_DIR + "tryAqObjCreditIndexOptionImpliedVol_outputs.csv";
	const std::string EXPECTED_CDS_INDEX_OPTION_VEGA				= TEST_DIR + "tryAqObjCreditIndexOptionVega_outputs.csv";
	const std::string EXPECTED_CDS_INDEX_OPTION_CS01				= TEST_DIR + "tryAqObjCreditIndexOptionCS01_outputs.csv";
	const std::string EXPECTED_CDS_INDEX_OPTION_THETA				= TEST_DIR + "tryAqObjCreditIndexOptionTheta_outputs.csv";


	void buildCurveAndCreditObjects()
	{
		auto loadEUROIS = validation::tryAqObjLoad(etrading::getGoogleTestFolder() + EUR_OIS, etrading::JSON);
		auto loadCDSGenerator = validation::tryAqObjLoad(etrading::getGoogleTestFolder() + GEN_EUR_CDSINDEX, etrading::JSON);
		auto loadCreditModel = validation::tryAqObjLoad(etrading::getGoogleTestFolder() + EUR_CREDIT_MODEL_FLAT, etrading::JSON);
	}
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestCreditIndexOption);

	//----------------------------------------------------------------------------------------

	/* @brief Compares the calibration parameters from the CreditModel against snapshot values
	*         Also checks snapshot values of PV, RiskyAnnuity and CS01 for a 5Y CDS.
	*/
	TEST_F( TestCreditIndexOption, SNAPSHOT_CreditIndexForwardSpread )
    {
		buildCurveAndCreditObjects();

		// Load API parameters
		etrading::ReadDataFile::Load forwardSpreadParams = etrading::ReadDataFile::Load( CDS_INDEX_CALCULATE_FORWARD_SPREAD );
		std::string creditModelName	= forwardSpreadParams["creditModelName"];
		AQLDate startDate			= forwardSpreadParams["startDate"];
		AQLDate endDate				= forwardSpreadParams["endDate"];

		// Invoke API
		const double calcForwardSpread = validation::tryAqObjCreditIndexSpread( creditModelName, startDate, endDate );

		google_test::CheckTestResultsAndRebaseOnRequest( calcForwardSpread, TEST_DIR, EXPECTED_CDS_INDEX_CALCULATE_FORWARD_SPREAD, tolerance );

	}

	TEST_F(TestCreditIndexOption, SNAPSHOT_OptionPV )
	{
		buildCurveAndCreditObjects();

		// Load API parameters
		etrading::ReadDataFile::Load pvParameters	= etrading::ReadDataFile::Load(CDS_INDEX_OPTION_PV);
		std::string creditModelName = pvParameters["creditModelName"];
		AQLStringMatrix optionLVB			= pvParameters["optionLVB"];

		// Invoke API
		const double calcOptionPV = validation::tryAqObjCreditIndexOptionPV( creditModelName, optionLVB );

		google_test::CheckTestResultsAndRebaseOnRequest( calcOptionPV, TEST_DIR, EXPECTED_CDS_INDEX_OPTION_PV, tolerance);

	}

	TEST_F(TestCreditIndexOption, SNAPSHOT_OptionImpliedVol)
	{
		buildCurveAndCreditObjects();

		// Load API parameters
		etrading::ReadDataFile::Load impliedVolParameters = etrading::ReadDataFile::Load(CDS_INDEX_OPTION_IMPLIED_VOL);
		std::string creditModelName = impliedVolParameters["creditModelName"];
		AQLStringMatrix optionLVB = impliedVolParameters["optionLVB"];

		// Invoke API
		const double calcOptionImpliedVol = validation::tryAqObjCreditIndexOptionImpliedVol(creditModelName, optionLVB);

		google_test::CheckTestResultsAndRebaseOnRequest(calcOptionImpliedVol, TEST_DIR, EXPECTED_CDS_INDEX_OPTION_IMPLIED_VOL, tolerance);
	}

	TEST_F(TestCreditIndexOption, SNAPSHOT_OptionVega)
	{
		buildCurveAndCreditObjects();

		// Load API parameters
		etrading::ReadDataFile::Load vegaParameters = etrading::ReadDataFile::Load(CDS_INDEX_OPTION_VEGA);
		std::string creditModelName = vegaParameters["creditModelName"];
		AQLStringMatrix optionLVB = vegaParameters["optionLVB"];

		double volatilityBump = vegaParameters["volatilityBump"];

		// Invoke API
		const double calcOptionVega = validation::tryAqObjCreditIndexOptionVega(creditModelName, optionLVB, volatilityBump );

		google_test::CheckTestResultsAndRebaseOnRequest(calcOptionVega, TEST_DIR, EXPECTED_CDS_INDEX_OPTION_VEGA, tolerance);

	}

	TEST_F(TestCreditIndexOption, SNAPSHOT_OptionCS01)
	{
		buildCurveAndCreditObjects();

		// Load API parameters
		etrading::ReadDataFile::Load cs01Parameters = etrading::ReadDataFile::Load(CDS_INDEX_OPTION_CS01);
		std::string creditModelName = cs01Parameters["creditModelName"];
		AQLStringMatrix optionLVB = cs01Parameters["optionLVB"];

		// Invoke API
		const double calcOptionCS01 = validation::tryAqObjCreditIndexOptionCS01(creditModelName, optionLVB);

		google_test::CheckTestResultsAndRebaseOnRequest(calcOptionCS01, TEST_DIR, EXPECTED_CDS_INDEX_OPTION_CS01, tolerance);

	}

	TEST_F(TestCreditIndexOption, SNAPSHOT_OptionTheta)
	{
		buildCurveAndCreditObjects();

		// Load API parameters
		etrading::ReadDataFile::Load thetaParameters = etrading::ReadDataFile::Load(CDS_INDEX_OPTION_THETA);
		std::string creditModelName = thetaParameters["creditModelName"];
		AQLStringMatrix optionLVB = thetaParameters["optionLVB"];

		// Invoke API
		const double calcOptionTheta = validation::tryAqObjCreditIndexOptionTheta(creditModelName, optionLVB);

		google_test::CheckTestResultsAndRebaseOnRequest(calcOptionTheta, TEST_DIR, EXPECTED_CDS_INDEX_OPTION_THETA, tolerance);


	}
}

