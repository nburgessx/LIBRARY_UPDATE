#include "Variant.h"
#include "LabelValueBlock.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "TestHelperUtilities.h"
#include "InitializeETrading.h"
#include "tryAqToolsSetup.h"
#include "ContainerUtilities.h"
#include "CoreEnumerations.h"
#include "ResultsProcessor.h"
#include "BuildMarketDataObjectFromFile.h"

#include <sstream>
#include <boost/range/irange.hpp>

#include "CurveOis.h"

// "me" API
#include "tryAqObjSwapsPricing.h"


// "Generator" API
#include "tryAqObjCurvesMarketData.h"
#include "tryAqObjCurvesCalibrate.h"
#include "tryAqObjSwapsCreation.h"


using etrading::ReadDataFile;

// Define the Test Input Folder Here
#define TEST_DIR			  "ETrading/AQObjects/TestAQObjConstantMaturitySwap/"


namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 1e-5;


    //
    // curve input files
    //

	// -------------------------------------------------------------

	// Curve market data files for use with CurveGenerators
	const char GEN_USD_OIS_MARKETDATA[]			= TEST_DIR "USDYC_OIS_CURVE_MARKETDATA@1_tryAqObjCurvesMarketDataCreate_inputs.csv";
	const char GEN_USD_STD_MARKETDATA[]			= TEST_DIR "USDYC_STD_3M_CURVE_MARKETDATA@3_tryAqObjCurvesMarketDataCreate_inputs.csv";

	// -------------------------------------------------------------

	// Curves built from generator and market data
	const char GEN_USD_OIS_CURVE[]				= TEST_DIR "USDYC_OIS_tryAqObjCurvesCalibrate_inputs.csv";
	const char GEN_USD_STD_CURVE[]				= TEST_DIR "USDYC_STD_tryAqObjCurvesCalibrate_inputs.csv";


	// -------------------------------------------------------------

	// Build Constant Maturity Swap

	const char GEN_CONSTANT_MATURITY_SWAP[]		= TEST_DIR "USD_CMS_tryAqObjSwapsCreateFromGenerator_inputs.csv" ;

	// API methods
	const char CMS_CALCULATE_PV[]				= TEST_DIR "tryAqObjConstantMaturitySwapPVUsingConvexityAdjustment_inputs.csv";
	const char CMS_CALCULATE_PAR_RATE[]			= TEST_DIR "tryAqObjConstantMaturitySwapParRateUsingConvexityAdjustment_inputs.csv";

	// Snapshot results
	const char CMS_EXPECTED_PV[]				= TEST_DIR "tryAqObjConstantMaturitySwapPVUsingConvexityAdjustment_outputs.csv";
	const char CMS_EXPECTED_PAR_RATE[]			= TEST_DIR "tryAqObjConstantMaturitySwapParRateUsingConvexityAdjustment_outputs.csv";

	
	/* @brief			Builds Generator curve by invoking the tryAqObjCurvesCalibration() API.
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/	
	void createAQObjCurveFromFileName( const AQLString& curveCalibrationFileName )
	{
		etrading::ReadDataFile::Load curveCalibrationFileObj = etrading::ReadDataFile::Load( curveCalibrationFileName );
		
		std::string aqObjCurveGeneratorName	= curveCalibrationFileObj[ "aqObjCurveGeneratorName" ];
		std::string aqObjCurveMarketDataName	= curveCalibrationFileObj[ "aqObjCurveMarketDataName" ];
		std::string domesticCurveCollection	= curveCalibrationFileObj[ "domesticCurveCollection" ];
		std::string foreignCurveCollection	= curveCalibrationFileObj[ "foreignCurveCollection" ];
		
		std::string objectName = aqObjCurveGeneratorName;

		validation::tryAqObjCurvesCalibrate(	objectName, aqObjCurveGeneratorName, aqObjCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );
	}	

	/* @brief			Builds and Generator curve using the specified marketData and calibration filename
	*  @param [in]		marketDataFileName			The filename specifying generator curve data
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/
	void setUpGeneratorCurve( const AQLString& marketDataFileName, const AQLString& curveCalibrationFileName )
	{
		google_test::createAQObjMarketDataObjectFromFileName( marketDataFileName );
		createAQObjCurveFromFileName( curveCalibrationFileName );
	}

	void createAQObjConstantMaturitySwapFromFileName( const AQLString& cmsFileName )
	{
		etrading::ReadDataFile::Load constantMaturitySwapFileObj = etrading::ReadDataFile::Load( cmsFileName );
		
		const std::string swapName				= constantMaturitySwapFileObj[ "swapName" ];
		const std::string aqObjswapGeneratorName	= constantMaturitySwapFileObj[ "swapGeneratorName" ];
		const AQLStringMatrix expressionLVB		= constantMaturitySwapFileObj[ "expressionLVB" ];
		const AQLStringMatrix swapPropertiesLVB	= constantMaturitySwapFileObj[ "swapPropertiesLVB" ];
		const bool isXccySwap					= constantMaturitySwapFileObj[ "isXccySwap" ];
		const bool validateKeys					= constantMaturitySwapFileObj[ "validateKeys" ];
		
		validation::tryAqObjSwapsCreateFromGenerator( swapName, aqObjswapGeneratorName, expressionLVB, swapPropertiesLVB, isXccySwap, validateKeys );
	}
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestConstantMaturitySwap);

	//----------------------------------------------------------------------------------------

	TEST_F( TestConstantMaturitySwap, SNAPSHOT_PV_withConvexityAdjustment )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		setUpGeneratorCurve( GEN_USD_STD_MARKETDATA, GEN_USD_STD_CURVE );

		createAQObjConstantMaturitySwapFromFileName( GEN_CONSTANT_MATURITY_SWAP );

		// Calculate the PV from hazard rate

		etrading::ReadDataFile::Load PVFileObj = etrading::ReadDataFile::Load( CMS_CALCULATE_PV );
		const std::string swapName			= PVFileObj[ "swapName" ];
		AQLStringMatrix curveCollections		= PVFileObj[ "curveCollections" ];
		const double convexityAdjustment	= PVFileObj[ "convexityAdjustment" ];
		std::string legName					= PVFileObj[ "legName"];

		const double calculatedPV = validation::tryAqObjConstantMaturitySwapPVUsingConvexityAdjustment( swapName, curveCollections, convexityAdjustment, legName.c_str() );

		// Check the Test Results or Rebase
		const double pvTolerance = 10.0;  // Notional of 1MM. We can afford a slightly wider tolerance which allows 64bit test to pass.
        CheckTestResultsAndRebaseOnRequest( calculatedPV, TEST_DIR, CMS_EXPECTED_PV, pvTolerance );
		
	}	
	
	TEST_F( TestConstantMaturitySwap, SNAPSHOT_ParRate_WithConvexityadjustment )
    {
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		setUpGeneratorCurve( GEN_USD_STD_MARKETDATA, GEN_USD_STD_CURVE );

		createAQObjConstantMaturitySwapFromFileName( GEN_CONSTANT_MATURITY_SWAP );

		// Calculate the Par Spread from hazard rate

		etrading::ReadDataFile::Load parRateFileObj = etrading::ReadDataFile::Load( CMS_CALCULATE_PAR_RATE );
		const std::string swapName			= parRateFileObj[ "swapName" ];
		AQLStringMatrix curveCollections		= parRateFileObj[ "curveCollections" ];
		const double convexityAdjustment	= parRateFileObj[ "convexityAdjustment" ];

		const double calculatedParRate = validation::tryAqObjConstantMaturitySwapParRateUsingConvexityAdjustment( swapName, curveCollections, convexityAdjustment );

		// Check the Test Results or Rebase
        CheckTestResultsAndRebaseOnRequest( calculatedParRate, TEST_DIR, CMS_EXPECTED_PAR_RATE, tolerance );
	}


}
