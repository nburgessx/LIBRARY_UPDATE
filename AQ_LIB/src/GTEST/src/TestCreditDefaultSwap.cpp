#include "Variant.h"
#include "LabelValueBlock.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "TestHelperUtilities.h"
#include "InitializeETrading.h"
#include "tryMeUtilitySetup.h"
#include "ContainerUtilities.h"
#include "CoreEnumerations.h"
#include "ResultsProcessor.h"
#include "BuildMarketDataObjectFromFile.h"

#include <sstream>
#include <boost/range/irange.hpp>

#include "CurveOis.h"

// "me" API
#include "tryMeLWOSwapPricing.h"


// "Generator" API
#include "tryMeLWOCurveMarketData.h"
#include "tryMeLWOCurveCalibrate.h"
#include "tryMeLWOSwapCreation.h"


using etrading::ReadDataFile;

// Define the Test Input Folder Here
#define TEST_DIR			  "ETrading/LWObjects/TestLWOCreditDefaultSwap/"


namespace
{
    // test tolerances
    // ---------------
    const double pvTolerance = 0.01;
    const double tolerance = 1e-8;


    //
    // curve input files
    //

	// -------------------------------------------------------------

	// Curve market data files for use with CurveGenerators
	const char GEN_USD_OIS_MARKETDATA[]			= TEST_DIR "USD_OIS_CURVE_MARKETDATA@553_tryMeLWOCurveMarketDataCreate_inputs.csv";

	// -------------------------------------------------------------

	// Curves built from generator and market data
	const char GEN_USD_OIS_CURVE[]				= TEST_DIR "USD_OIS_tryMeLWOCurveCalibrate_inputs.csv";

	// -------------------------------------------------------------

	// Build Credit Default Swap

	const char GEN_CREDIT_DEFAULT_SWAP[]		= TEST_DIR "TEST_CDS@1_tryMeLWOSwapCreateFromGenerator_inputs.csv" ;

	// API methods
	const char CDS_CALCULATE_PV[]				= TEST_DIR "tryMeLWOCreditDefaultSwapPVFromHazardRate_inputs.csv";
	const char CDS_CALCULATE_RISKY_ANNUITY[]	= TEST_DIR "tryMeLWOCreditDefaultSwapAnnuityFromHazardRate_inputs.csv";
	const char CDS_CALCULATE_PAR_SPREAD[]		= TEST_DIR "tryMeLWOCreditDefaultSwapParSpreadFromHazardRate_inputs.csv";
	const char CDS_CALCULATE_HAZARD_RATE[]		= TEST_DIR "tryMeLWOCreditDefaultSwapHazardRateFromParSpread_inputs.csv";

	// Snapshot results
	const char CDS_EXPECTED_PV[]				= TEST_DIR "tryMeLWOCreditDefaultSwapPVFromHazardRate_outputs.csv";
	const char CDS_EXPECTED_RISKY_ANNUITY[]		= TEST_DIR "tryMeLWOCreditDefaultSwapAnnuityFromHazardRate_outputs.csv";
	const char CDS_EXPECTED_PAR_SPREAD[]		= TEST_DIR "tryMeLWOCreditDefaultSwapParSpreadFromHazardRate_outputs.csv";
	const char CDS_EXPECTED_HAZARD_RATE[]		= TEST_DIR "tryMeLWOCreditDefaultSwapHazardRateFromParSpread_outputs.csv";

	/* @brief			Builds Generator curve by invoking the tryMeLWOCurveCalibration() API.
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/	
	void createLWOCurveFromFileName( const AQLString& curveCalibrationFileName )
	{
		etrading::ReadDataFile::Load curveCalibrationFileObj = etrading::ReadDataFile::Load( curveCalibrationFileName );
		
		std::string lwoCurveGeneratorName	= curveCalibrationFileObj[ "lwoCurveGeneratorName" ];
		std::string lwoCurveMarketDataName	= curveCalibrationFileObj[ "lwoCurveMarketDataName" ];
		std::string domesticCurveCollection	= curveCalibrationFileObj[ "domesticCurveCollection" ];
		std::string foreignCurveCollection	= curveCalibrationFileObj[ "foreignCurveCollection" ];
		
		std::string objectName = lwoCurveGeneratorName;

		validation::tryMeLWOCurveCalibrate(	objectName, lwoCurveGeneratorName, lwoCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );
	}	

	/* @brief			Builds and Generator curve using the specified marketData and calibration filename
	*  @param [in]		marketDataFileName			The filename specifying generator curve data
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/
	void setUpGeneratorCurve( const AQLString& marketDataFileName, const AQLString& curveCalibrationFileName )
	{
		google_test::createLWOMarketDataObjectFromFileName( marketDataFileName );
		createLWOCurveFromFileName( curveCalibrationFileName );
	}

	void createLWOCreditDefaultSwapFromFileName( const AQLString& cdsFileName )
	{
		etrading::ReadDataFile::Load creditDefaultSwapFileObj = etrading::ReadDataFile::Load( cdsFileName );
		
		const std::string swapName				= creditDefaultSwapFileObj[ "swapName" ];
		const std::string lwoswapGeneratorName	= creditDefaultSwapFileObj[ "swapGeneratorName" ];
		const AQLStringMatrix expressionLVB		= creditDefaultSwapFileObj[ "expressionLVB" ];
		const AQLStringMatrix swapPropertiesLVB	= creditDefaultSwapFileObj[ "swapPropertiesLVB" ];
		const bool isXccySwap					= creditDefaultSwapFileObj[ "isXccySwap" ];
		const bool validateKeys					= creditDefaultSwapFileObj[ "validateKeys" ];
		
		validation::tryMeLWOSwapCreateFromGenerator( swapName, lwoswapGeneratorName, expressionLVB, swapPropertiesLVB, isXccySwap, validateKeys );
	}
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestCreditDefaultSwap);

	//----------------------------------------------------------------------------------------

	TEST_F( TestCreditDefaultSwap, SNAPSHOT_PV_fromHazardRate )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		createLWOCreditDefaultSwapFromFileName( GEN_CREDIT_DEFAULT_SWAP );

		// Calculate the PV from hazard rate

		etrading::ReadDataFile::Load PVFileObj = etrading::ReadDataFile::Load( CDS_CALCULATE_PV );
		const std::string swapName			= PVFileObj[ "swapName" ];
		AQLStringMatrix curveCollections		= PVFileObj[ "curveCollections" ];
		const double hazardRate				= PVFileObj[ "hazardRate" ];
		const double recoveryRate			= PVFileObj[ "recoveryRate" ];
		std::string legName					= PVFileObj[ "legName"];
		bool includeAccruedInterest			= PVFileObj[ "includeAccruedInterest" ];

		const double calculatedPV = validation::tryMeLWOCreditDefaultSwapPVFromHazardRate( swapName, curveCollections, hazardRate, recoveryRate, legName.c_str(), includeAccruedInterest );

		// Check the Test Results or Rebase
        CheckTestResultsAndRebaseOnRequest( calculatedPV, TEST_DIR, CDS_EXPECTED_PV, pvTolerance );
		
	}	
	
	
	TEST_F( TestCreditDefaultSwap, SNAPSHOT_RiskyAnnuity_FromHazardRate )
    {
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		createLWOCreditDefaultSwapFromFileName( GEN_CREDIT_DEFAULT_SWAP );

		// Calculate the Risky Annuity from hazard rate

		etrading::ReadDataFile::Load riskyAnnuityFileObj = etrading::ReadDataFile::Load( CDS_CALCULATE_RISKY_ANNUITY );
		const std::string swapName			= riskyAnnuityFileObj[ "swapName" ];
		etrading::LabelValueBlock curveCollection		= etrading::fromStringToLVB(riskyAnnuityFileObj[ "curveCollection" ]);
		const double hazardRate				= riskyAnnuityFileObj[ "hazardRate" ];
		const double recoveryRate			= riskyAnnuityFileObj[ "recoveryRate" ];
		std::string legName					= riskyAnnuityFileObj[ "legName"];
		bool includeAccruedInterest			= riskyAnnuityFileObj[ "includeAccruedInterest" ];

		const double calculatedAnnuity = validation::tryMeLWOCreditDefaultSwapRiskyAnnuityFromHazardRate( swapName, curveCollection, hazardRate, recoveryRate, legName.c_str(), includeAccruedInterest );

		// Check the Test Results or Rebase
		const double annuityTolerance = 0.01;  // The expected annuity is a large number ( 13e6) ; we can afford to relax tolerance ( to allow 64 bit test to pass ).
        CheckTestResultsAndRebaseOnRequest( calculatedAnnuity, TEST_DIR, CDS_EXPECTED_RISKY_ANNUITY, annuityTolerance );
	}

	
	TEST_F( TestCreditDefaultSwap, SNAPSHOT_ParSpread_FromHazardRate )
    {
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		createLWOCreditDefaultSwapFromFileName( GEN_CREDIT_DEFAULT_SWAP );

		// Calculate the Par Spread from hazard rate

		etrading::ReadDataFile::Load parSpreadFileObj = etrading::ReadDataFile::Load( CDS_CALCULATE_PAR_SPREAD );
		const std::string swapName			= parSpreadFileObj[ "swapName" ];
		AQLStringMatrix curveCollections		= parSpreadFileObj[ "curveCollections" ];
		const double hazardRate				= parSpreadFileObj[ "hazardRate" ];
		const double recoveryRate			= parSpreadFileObj[ "recoveryRate" ];
		std::string premiumLegName			= parSpreadFileObj[ "premiumLegName"];
		std::string protectionLegName		= parSpreadFileObj[ "protectionLegName"];
		bool includeAccruedInterest			= parSpreadFileObj[ "includeAccruedInterest" ];

		const double calculatedParSpread = validation::tryMeLWOCreditDefaultSwapParSpreadFromHazardRate( swapName, curveCollections, hazardRate, recoveryRate, premiumLegName.c_str(), protectionLegName.c_str(), includeAccruedInterest );

		// Check the Test Results or Rebase
        CheckTestResultsAndRebaseOnRequest( calculatedParSpread, TEST_DIR, CDS_EXPECTED_PAR_SPREAD, tolerance );
	}

	
	TEST_F( TestCreditDefaultSwap, SNAPSHOT_HazardRate_FromParSpread )
    {
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		createLWOCreditDefaultSwapFromFileName( GEN_CREDIT_DEFAULT_SWAP );

		// Calculate the Hazard Rate from par spread

		etrading::ReadDataFile::Load hazardRateFileObj = etrading::ReadDataFile::Load( CDS_CALCULATE_HAZARD_RATE );
		const std::string swapName			= hazardRateFileObj[ "swapName" ];
		AQLStringMatrix curveCollections		= hazardRateFileObj[ "curveCollections" ];
		const double parSpread				= hazardRateFileObj[ "parSpread" ];
		const double recoveryRate			= hazardRateFileObj[ "recoveryRate" ];
		std::string premiumLegName			= hazardRateFileObj[ "premiumLegName"];
		std::string protectionLegName		= hazardRateFileObj[ "protectionLegName"];
		bool includeAccruedInterest			= hazardRateFileObj[ "includeAccruedInterest" ];

		const double calculatedHazardRate = validation::tryMeLWOCreditDefaultSwapHazardRateFromParSpread( swapName, curveCollections, parSpread, recoveryRate, premiumLegName.c_str(), protectionLegName.c_str(), includeAccruedInterest );

		// Check the Test Results or Rebase
        CheckTestResultsAndRebaseOnRequest( calculatedHazardRate, TEST_DIR, CDS_EXPECTED_HAZARD_RATE, tolerance );



	}
	

}
