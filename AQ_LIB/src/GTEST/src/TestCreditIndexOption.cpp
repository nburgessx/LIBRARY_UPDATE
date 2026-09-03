 // Curves
#include "tryMeLWO.h"

// API functions
#include "tryMeLWOSwapPricing.h"


// Helper to extract par rates from a curve
#include "ExtractCurveCalibrationData.h"

// Test Infrastructure
#include "Dependency.h"   // Curve Macros are Here !!!
#include "GetGoogleTestFolder.h"
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"

// etrading
#include "LAMathDateUtilities.h"
#include "LADateScheduleHelpers.h"
#include "LWOUtilities.h"



using etrading::ReadDataFile;




namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 1e-8;

	// Define the Test Input Folder Here
	const std::string TEST_DIR =		  "ETrading/LWObjects/TestLWOCreditIndexOption/";

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
	const std::string CDS_INDEX_CALCULATE_FORWARD_SPREAD = TEST_DIR + "tryMeLWOCreditIndexSpread_inputs.csv";
	const std::string CDS_INDEX_OPTION_PV				 = TEST_DIR + "tryMeLWOCreditIndexOptionPV_inputs.csv";
	const std::string CDS_INDEX_OPTION_IMPLIED_VOL		 = TEST_DIR + "tryMeLWOCreditIndexOptionImpliedVol_inputs.csv";
	const std::string CDS_INDEX_OPTION_VEGA				 = TEST_DIR + "tryMeLWOCreditIndexOptionVega_inputs.csv";
	const std::string CDS_INDEX_OPTION_CS01				 = TEST_DIR + "tryMeLWOCreditIndexOptionCS01_inputs.csv";
	const std::string CDS_INDEX_OPTION_THETA			 = TEST_DIR + "tryMeLWOCreditIndexOptionTheta_inputs.csv";

	// Snapshot results
	const std::string EXPECTED_CDS_INDEX_CALCULATE_FORWARD_SPREAD	= TEST_DIR + "tryMeLWOCreditIndexSpread_outputs.csv";
	const std::string EXPECTED_CDS_INDEX_OPTION_PV					= TEST_DIR + "tryMeLWOCreditIndexOptionPV_outputs.csv";
	const std::string EXPECTED_CDS_INDEX_OPTION_IMPLIED_VOL			= TEST_DIR + "tryMeLWOCreditIndexOptionImpliedVol_outputs.csv";
	const std::string EXPECTED_CDS_INDEX_OPTION_VEGA				= TEST_DIR + "tryMeLWOCreditIndexOptionVega_outputs.csv";
	const std::string EXPECTED_CDS_INDEX_OPTION_CS01				= TEST_DIR + "tryMeLWOCreditIndexOptionCS01_outputs.csv";
	const std::string EXPECTED_CDS_INDEX_OPTION_THETA				= TEST_DIR + "tryMeLWOCreditIndexOptionTheta_outputs.csv";


	void buildCurveAndCreditObjects()
	{
		auto loadEUROIS = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + EUR_OIS, etrading::JSON);
		auto loadCDSGenerator = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + GEN_EUR_CDSINDEX, etrading::JSON);
		auto loadCreditModel = validation::tryMeLWOLoad(etrading::getGoogleTestFolder() + EUR_CREDIT_MODEL_FLAT, etrading::JSON);
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
		const double calcForwardSpread = validation::tryMeLWOCreditIndexSpread( creditModelName, startDate, endDate );

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
		const double calcOptionPV = validation::tryMeLWOCreditIndexOptionPV( creditModelName, optionLVB );

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
		const double calcOptionImpliedVol = validation::tryMeLWOCreditIndexOptionImpliedVol(creditModelName, optionLVB);

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
		const double calcOptionVega = validation::tryMeLWOCreditIndexOptionVega(creditModelName, optionLVB, volatilityBump );

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
		const double calcOptionCS01 = validation::tryMeLWOCreditIndexOptionCS01(creditModelName, optionLVB);

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
		const double calcOptionTheta = validation::tryMeLWOCreditIndexOptionTheta(creditModelName, optionLVB);

		google_test::CheckTestResultsAndRebaseOnRequest(calcOptionTheta, TEST_DIR, EXPECTED_CDS_INDEX_OPTION_THETA, tolerance);


	}
}

