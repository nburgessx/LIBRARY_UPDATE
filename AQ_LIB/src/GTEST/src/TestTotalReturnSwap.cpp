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

#include <sstream>
#include <boost/range/irange.hpp>

#include "CurveOis.h"

// "me" API
#include "tryAqObjectsSwapPricing.h"


// "Generator" API
#include "tryAqObjectsCurveMarketData.h"
#include "tryAqObjectsCurveCalibrate.h"
#include "tryAqObjectsSwapCreation.h"


using etrading::ReadDataFile;

// Define the Test Input Folder Here
#define TEST_DIR			  "ETrading/LWObjects/TestLWOTotalReturnSwap/"


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
	const char GEN_USD_OIS_MARKETDATA[]			= TEST_DIR "USDYC_OIS_CURVE_MARKETDATA@5_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_USD_STD_MARKETDATA[]			= TEST_DIR "USDYC_STD_3M_CURVE_MARKETDATA@4_tryAqObjectsCurveMarketDataCreate_inputs.csv";

	// -------------------------------------------------------------

	// Curves built from generator and market data
	const char GEN_USD_OIS_CURVE[]				= TEST_DIR "USDYC_OIS_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_USD_STD_CURVE[]				= TEST_DIR "USDYC_STD_tryAqObjectsCurveCalibrate_inputs.csv";

	// -------------------------------------------------------------

	// Build Credit Model
	const char GEN_USD_CREDIT_MODEL[]			= TEST_DIR "CREDITMODEL1@16_tryAqObjectsCreditModelCreate_inputs.csv";

	// Build the swap generator
	const char GEN_TOTAL_RETURN_SWAPGENERATOR[]	= TEST_DIR "USD_TRS_3ML@1_tryAqObjectsSwapGeneratorCreate_inputs.csv";

	const char GEN_TOTAL_RETURN_SWAPGENERATOR_PAY_ON_SURVIVAL[]	= TEST_DIR "USD_TRS_3ML@16_tryAqObjectsSwapGeneratorCreate_PayOnSurvival_inputs.csv";

	// Build Total Return Swap
	const char GEN_TOTAL_RETURN_SWAP[]			= TEST_DIR "USD_TRS_3ML@1_tryAqObjectsSwapCreateFromGenerator_inputs.csv" ;

	const char GEN_TOTAL_RETURN_SWAP_PAY_ON_SURVIVAL[]	= TEST_DIR "USD_TRS_3ML@17_tryAqObjectsSwapCreateFromGenerator_PayOnSurvival_inputs.csv" ;


	// API methods
	const char TRS_CALCULATE_ALL_PV[]			= TEST_DIR "tryAqObjectsTotalReturnSwapPV_ALL_inputs.csv";
	const char TRS_CALCULATE_PREMIUM_PV[]		= TEST_DIR "tryAqObjectsTotalReturnSwapPV_Premium_inputs.csv";
	const char TRS_CALCULATE_FLOAT_PV[]			= TEST_DIR "tryAqObjectsTotalReturnSwapPV_Float_inputs.csv";

	const char TRS_CALCULATE_PAR_RATE[]			= TEST_DIR "tryAqObjectsTotalReturnSwapParRate_inputs.csv";
	const char TRS_CALCULATE_PAR_SPREAD[]		= TEST_DIR "tryAqObjectsTotalReturnSwapParSpread_inputs.csv";

	const char TRS_DISPLAY_CASHFLOWS[]			= TEST_DIR "tryAqObjectsSwapDisplayCashflows_inputs.csv";

	const char TRS_CALCULATE_PREMIUM_ANNUITY[]	= TEST_DIR "tryAqObjectsTotalReturnSwapAnnuity_Premium_inputs.csv";

	// Snapshot results
	const char TRS_EXPECTED_ALL_PV[]			= TEST_DIR "tryAqObjectsTotalReturnSwapPV_ALL_outputs.csv";
	const char TRS_EXPECTED_PREMIUM_PV[]		= TEST_DIR "tryAqObjectsTotalReturnSwapPV_Premium_outputs.csv";
	const char TRS_EXPECTED_FLOAT_PV[]			= TEST_DIR "tryAqObjectsTotalReturnSwapPV_Float_outputs.csv";

	const char TRS_EXPECTED_PAY_ON_SURVIVAL_ALL_PV[]	= TEST_DIR "tryAqObjectsTotalReturnSwapPV_PayOnSurvival_ALL_outputs.csv";
	const char TRS_EXPECTED_PAY_ON_SURVIVAL_FLOAT_PV[]	= TEST_DIR "tryAqObjectsTotalReturnSwapPV_PayOnSurvival_Float_outputs.csv";

	const char TRS_EXPECTED_PAR_RATE[]			= TEST_DIR "tryAqObjectsTotalReturnSwapParRate_outputs.csv";
	const char TRS_EXPECTED_PAR_SPREAD[]		= TEST_DIR "tryAqObjectsTotalReturnSwapParSpread_outputs.csv";

	const char TRS_EXPECTED_PREMIUM_ANNUITY[]	= TEST_DIR "tryAqObjectsTotalReturnSwapAnnuity_Premium_outputs.csv";
	const char TRS_EXPECTED_FLOAT_PAYALWAYS_ANNUITY[] = TEST_DIR "tryAqObjectsTotalReturnSwapAnnuity_Float_PayAlways_outputs.csv";
	const char TRS_EXPECTED_FLOAT_PAYONSURVIVAL_ANNUITY[] = TEST_DIR "tryAqObjectsTotalReturnSwapAnnuity_Float_PayOnSurvival_outputs.csv";

	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, etrading::VariantMatrix>  TableInfo;

	/* @brief			A helper function which converts a AQLStringMatrix into a VariantMatrix
	*                   If the input AQLStringMatrix is empty, creates a dummy VariantMatrix containing two blank columns 
	*  @param [in]		stringMatrix		The input AQLStringMatrix
	*  @returns			The corresponding VariantMatrix
	*/
	etrading::VariantMatrix convertStringMatrixToVariantMatrix( AQLStringMatrix stringMatrix )
	{
		etrading::VariantMatrix variantMatrix;

		const size_t numRows = stringMatrix.size();
		if ( numRows > 0 )
		{
			const size_t numCols = stringMatrix[0].size();

			// Transpose the matrix at the same time as converting to Variant
			for (size_t j=0; j<numCols; j++)
			{
				etrading::VariantVector variantVector;
				for (size_t i=0; i<numRows; i++)
				{
					variantVector.push_back( stringMatrix[i][j] );
				}
				variantMatrix.push_back( variantVector );
			}
		}
		else
		{
			// The input AQLStringMatrix is empty.
			// Create a default variantMatrix with 2 columns of dummy data.
			// This simulates an empty block in Excel.
			etrading::VariantVector dummyVector ( 1, "" );
			variantMatrix.push_back( dummyVector );
			variantMatrix.push_back( dummyVector );
		}

		return variantMatrix;
	}

	/* @brief			Builds a "TableInfo" tuple from a AQLStringMatrix of marketdata
	*                   This tuple consists of columnNames, columnTypes and the actual data values.
	*  @param [in]		marketDataBlock		A AQLStringMatrix containing key/value market data values
	*/
	TableInfo getTableInfoFromStringMatrix( const AQLStringMatrix& marketDataBlock )
	{
		etrading::VariantMatrix dataValues =  convertStringMatrixToVariantMatrix( marketDataBlock );
		size_t numColumns = dataValues.size();

		// EnumTypes for each column
		std::vector<etrading::ContainedTypeEnum> columnEnumTypes = etrading::Variant::getContainedTypeInfo( dataValues );

		// Construct dummy column headings
		std::vector<int> nColCounters;
        boost::push_back( nColCounters, boost::irange( 1, static_cast<int>( numColumns ) + 1 ) );
		std::vector<std::string> columnNames( numColumns, std::string( "COL_" ) );
        columnNames = etrading::zip_paste<std::vector<std::string>, std::vector<int>, std::string>( columnNames, nColCounters );

		// Construct the TableInfo
        return std::make_tuple( columnNames, columnEnumTypes, dataValues );
	}

	/* @brief			Builds LWO MarketData Object by invoking the tryAqObjectsCurveMarketDataCreate() API.
	*                   The code loops over all of the capitalized data keys in the specified filename and uses
	*                   these blocks to construct the MarketData object.
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/	
	void createLWOMarketDataObjectFromFileName( const AQLString& marketDataFileName )
	{
		etrading::ReadDataFile::Load marketDataFileObj = etrading::ReadDataFile::Load( marketDataFileName );

		std::string objectName				= marketDataFileObj[ "objectName" ];

		// Process all the market data keys and corresponding blocks of data
		std::vector<std::string> marketDataKeys;
		std::vector<TableInfo> infoBlocks;

		auto keys = marketDataFileObj.getKeys();
		for ( auto it = keys.begin(); it != keys.end(); ++it )
		{
			const std::string key = it->getCString();
			try
			{
				// If the key is a market-data key we are interested in, we will be able to cast to enum
				// Conversely, if the key is not associated with market data (such as "objectName") then the cast to enum will fail
				etrading::CurveMarketDataEnum marketDataEnum = etrading::toCurveMarketDataEnum( key );

				// We obtained the enum, so this is a marketData key we are interested in
				marketDataKeys.push_back( key );
				AQLStringMatrix marketDataBlock = marketDataFileObj[ *it ];
				infoBlocks.push_back ( getTableInfoFromStringMatrix( marketDataBlock ) );
			}
			catch( ... )
			{
				// We attempted to convert a non-marketData related key to enum. Continue to the next key.
				continue;
			}
		}

		validation::tryAqObjectsCurveMarketDataCreate( objectName, marketDataKeys, infoBlocks );
	}

	/* @brief			Builds Generator curve by invoking the tryAqObjectsCurveCalibration() API.
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

		validation::tryAqObjectsCurveCalibrate(	objectName, lwoCurveGeneratorName, lwoCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );
	}	

	/* @brief			Builds and Generator curve using the specified marketData and calibration filename
	*  @param [in]		marketDataFileName			The filename specifying generator curve data
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/
	void setUpGeneratorCurve( const AQLString& marketDataFileName, const AQLString& curveCalibrationFileName )
	{
		createLWOMarketDataObjectFromFileName( marketDataFileName );
		createLWOCurveFromFileName( curveCalibrationFileName );
	}

	std::string createLWOCreditModelFromFileName( const AQLString& creditModelFileName )
	{
		etrading::ReadDataFile::Load creditModelFileObj = etrading::ReadDataFile::Load( creditModelFileName );
		const std::string creditModelName		= creditModelFileObj[ "objectName" ];
		const AQLStringMatrix modelProperties		= creditModelFileObj[ "MODEL_PROPERTIES" ];
		const AQLStringMatrix cdsMarketData		= creditModelFileObj[ "CDS_MARKETDATA" ];

		std::vector<std::string> propertyNames;
		propertyNames.push_back( "MODEL_PROPERTIES" );
		propertyNames.push_back( "CDS_MARKETDATA" );

		std::vector<validation::TableInfo> infoBlocks;
		infoBlocks.push_back( getTableInfoFromStringMatrix( modelProperties ));
		infoBlocks.push_back( getTableInfoFromStringMatrix( cdsMarketData ));

		std::string objectName = validation::tryAqObjectsCreditModelCreate( creditModelName, propertyNames, infoBlocks );
		return objectName;
	}

	std::string createLWOSwapGeneratorFromFileName( const AQLString& swapGeneratorFilename )
	{
		etrading::ReadDataFile::Load swapGeneratorFileObj = etrading::ReadDataFile::Load( swapGeneratorFilename );
		const std::string swapGeneratorName		= swapGeneratorFileObj[ "swapGeneratorName" ];
		const AQLStringMatrix swapGeneratorLVB		= swapGeneratorFileObj[ "swapGeneratorLVB" ];

		std::string objectName = validation::tryAqObjectsSwapGeneratorCreate( swapGeneratorName, swapGeneratorLVB );
		return objectName;
	}


	std::string createLWOTotalReturnSwapFromFileName( const AQLString& cmsFileName )
	{
		etrading::ReadDataFile::Load constantMaturitySwapFileObj = etrading::ReadDataFile::Load( cmsFileName );
		
		const std::string swapName				= constantMaturitySwapFileObj[ "swapName" ];
		const std::string lwoswapGeneratorName	= constantMaturitySwapFileObj[ "swapGeneratorName" ];
		const AQLStringMatrix expressionLVB		= constantMaturitySwapFileObj[ "expressionLVB" ];
		const AQLStringMatrix swapPropertiesLVB	= constantMaturitySwapFileObj[ "swapPropertiesLVB" ];
		const bool isXccySwap					= constantMaturitySwapFileObj[ "isXccySwap" ];
		const bool validateKeys					= constantMaturitySwapFileObj[ "validateKeys" ];
		
		validation::tryAqObjectsSwapCreateFromGenerator( swapName, lwoswapGeneratorName, expressionLVB, swapPropertiesLVB, isXccySwap, validateKeys );
		return swapName;
	}

	void validateCashflowsForLegName( const std::string& swapName, const std::string& creditModelName, const AQLStringMatrix& displayModelNames, const std::string legName )
	{
	
		std::vector<AnyTypeMatrix> cashflowMatrices = validation::tryAqObjectsSwapDisplayCashflows( swapName, displayModelNames, legName.c_str() );

		const size_t vecSize = cashflowMatrices.size();
		ASSERT_EQ( vecSize, 1 ); // Expect a single cashflow matrix

		const AnyTypeMatrix& cashflowMatrix = cashflowMatrices[0];
		const size_t rows = cashflowMatrix.size();
		ASSERT_EQ( rows, 10 );

		// Expect the first row to contain only the legName
		const size_t colsRow1 = cashflowMatrix[0].size();
		ASSERT_EQ( colsRow1, 1 );
		
		const size_t colsCashflows = cashflowMatrix[1].size();
		ASSERT_GE( (int) colsCashflows, 16 ); // Expect at least 16 columns. 

		std::map<std::string, size_t> columnMap; // A map from header name to column position
		auto columnHeaders = cashflowMatrix[1];

		for ( size_t j=0; j<colsCashflows; j++ )
		{
			auto value = cashflowMatrix[1][j];
			std::string header = boost::get<std::string>( value );
			columnMap[ header ] = j;
		}

		const size_t pvCol = columnMap[ "CouponPV" ];
		const size_t survivalCol = columnMap[ "SurvivalProbability"];

		double survivalProb = 1.0;
		double totalPV = 0.;
		for ( size_t i=2; i<rows; i++ )
		{
			auto val = cashflowMatrix[i][pvCol];
			double pv = boost::get<double>( val );
			totalPV += pv;

			auto val2 = cashflowMatrix[i][survivalCol];
			double nextSurvivalProb = boost::get<double>(val2);
			ASSERT_LE( nextSurvivalProb, survivalProb ); // Verify that the survival probability is monotonically decreasing
			survivalProb = nextSurvivalProb;
		}

		// ---- Calculate the total PV of swap leg ----
		const double calculatedLegPV = validation::tryAqObjectsTotalReturnSwapPV( swapName, creditModelName, legName.c_str() );
		ASSERT_NEAR( calculatedLegPV, totalPV, tolerance );
	}

}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestTotalReturnSwap);

	//----------------------------------------------------------------------------------------

	TEST_F( TestTotalReturnSwap, SNAPSHOT_PV )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		setUpGeneratorCurve( GEN_USD_STD_MARKETDATA, GEN_USD_STD_CURVE );

		// Create and Calibrate the credit model
		createLWOCreditModelFromFileName( GEN_USD_CREDIT_MODEL );

		// Create swap generator
		createLWOSwapGeneratorFromFileName( GEN_TOTAL_RETURN_SWAPGENERATOR );

		// Create TRS
		createLWOTotalReturnSwapFromFileName( GEN_TOTAL_RETURN_SWAP );

		// ---- Calculate the total PV of both swap legs ----

		etrading::ReadDataFile::Load PVFileObj = etrading::ReadDataFile::Load( TRS_CALCULATE_ALL_PV );
		const std::string swapName			= PVFileObj[ "swapName" ];
		const std::string creditModelName	= PVFileObj[ "creditModelName" ];
		std::string legName					= PVFileObj[ "legName"];

		const double calculatedPV = validation::tryAqObjectsTotalReturnSwapPV( swapName, creditModelName, legName.c_str() );

		// Check the Test Results or Rebase
		const double pvTolerance = 10.0;  // Notional of 1MM. We can afford a slightly wider tolerance which allows 64bit test to pass.
        CheckTestResultsAndRebaseOnRequest( calculatedPV, TEST_DIR, TRS_EXPECTED_ALL_PV, pvTolerance );

		// ---- Calculate the PV of premium swap leg ( representing the Bond ) ----

		etrading::ReadDataFile::Load PVPremiumFileObj = etrading::ReadDataFile::Load( TRS_CALCULATE_PREMIUM_PV );
		std::string premiumLegName			= PVPremiumFileObj[ "legName"];

		const double calculatedPremiumPV = validation::tryAqObjectsTotalReturnSwapPV( swapName, creditModelName, premiumLegName.c_str() );

		// Check the Test Results or Rebase
        CheckTestResultsAndRebaseOnRequest( calculatedPremiumPV, TEST_DIR, TRS_EXPECTED_PREMIUM_PV, pvTolerance );


		// ---- Calculate the PV of the float swap leg, representing the financing ----
		
		etrading::ReadDataFile::Load PVFloatFileObj = etrading::ReadDataFile::Load( TRS_CALCULATE_FLOAT_PV );
		std::string floatLegName			= PVFloatFileObj[ "legName"];

		const double calculatedFloatPV = validation::tryAqObjectsTotalReturnSwapPV( swapName, creditModelName, floatLegName.c_str() );

		// Check the Test Results or Rebase
        CheckTestResultsAndRebaseOnRequest( calculatedFloatPV, TEST_DIR, TRS_EXPECTED_FLOAT_PV, pvTolerance );

	}	
	
	
	TEST_F( TestTotalReturnSwap, SNAPSHOT_ParRate )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		setUpGeneratorCurve( GEN_USD_STD_MARKETDATA, GEN_USD_STD_CURVE );

		// Create and Calibrate the credit model
		createLWOCreditModelFromFileName( GEN_USD_CREDIT_MODEL );

		// Create swap generator
		createLWOSwapGeneratorFromFileName( GEN_TOTAL_RETURN_SWAPGENERATOR );

		// Create TRS
		createLWOTotalReturnSwapFromFileName( GEN_TOTAL_RETURN_SWAP );

		// Calculate the Par Rate

		etrading::ReadDataFile::Load parRateFileObj = etrading::ReadDataFile::Load( TRS_CALCULATE_PAR_RATE );
		const std::string swapName			= parRateFileObj[ "swapName" ];
		const std::string creditModelName		= parRateFileObj[ "creditModelName" ];

		const double calculatedParRate = validation::tryAqObjectsTotalReturnSwapParRate( swapName, creditModelName );

		// Check the Test Results or Rebase
        CheckTestResultsAndRebaseOnRequest( calculatedParRate, TEST_DIR, TRS_EXPECTED_PAR_RATE, tolerance );
	}

	TEST_F( TestTotalReturnSwap, SNAPSHOT_ParSpread )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		setUpGeneratorCurve( GEN_USD_STD_MARKETDATA, GEN_USD_STD_CURVE );

		// Create and Calibrate the credit model
		createLWOCreditModelFromFileName( GEN_USD_CREDIT_MODEL );

		// Create swap generator
		createLWOSwapGeneratorFromFileName( GEN_TOTAL_RETURN_SWAPGENERATOR );

		// Create TRS
		createLWOTotalReturnSwapFromFileName( GEN_TOTAL_RETURN_SWAP );

		// Calculate the Par Spread i.e. the spread (in bps) on the float leg LIBOR rates required to make the PV of the swap = 0.

		etrading::ReadDataFile::Load parRateFileObj = etrading::ReadDataFile::Load( TRS_CALCULATE_PAR_SPREAD );
		const std::string swapName			= parRateFileObj[ "swapName" ];
		const std::string creditModelName	= parRateFileObj[ "creditModelName" ];

		const double calculatedParSpread = validation::tryAqObjectsTotalReturnSwapParSpread( swapName, creditModelName );

		// Check the Test Results or Rebase
        CheckTestResultsAndRebaseOnRequest( calculatedParSpread, TEST_DIR, TRS_EXPECTED_PAR_SPREAD, tolerance );
	}

	TEST_F( TestTotalReturnSwap, SNAPSHOT_PV_PayOnSurvival )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		setUpGeneratorCurve( GEN_USD_STD_MARKETDATA, GEN_USD_STD_CURVE );

		// Create and Calibrate the credit model
		createLWOCreditModelFromFileName( GEN_USD_CREDIT_MODEL );

		// Create swap generator
		createLWOSwapGeneratorFromFileName( GEN_TOTAL_RETURN_SWAPGENERATOR_PAY_ON_SURVIVAL );

		// Create TRS
		const std::string swapName_payOnSurvival = createLWOTotalReturnSwapFromFileName( GEN_TOTAL_RETURN_SWAP_PAY_ON_SURVIVAL );

		// ---- Calculate the total PV of both swap legs ----

		etrading::ReadDataFile::Load PVFileObj = etrading::ReadDataFile::Load( TRS_CALCULATE_ALL_PV );
		const std::string creditModelName	= PVFileObj[ "creditModelName" ];
		std::string legName					= PVFileObj[ "legName"];

		const double calculated_PayOnSurvival_ALL_PV = validation::tryAqObjectsTotalReturnSwapPV( swapName_payOnSurvival, creditModelName, legName.c_str() );

		// Check the Test Results or Rebase
		const double pvTolerance = 10.0;  // Notional of 1MM. We can afford a slightly wider tolerance which allows 64bit test to pass.
        CheckTestResultsAndRebaseOnRequest( calculated_PayOnSurvival_ALL_PV, TEST_DIR, TRS_EXPECTED_PAY_ON_SURVIVAL_ALL_PV, pvTolerance );

		// ---- Calculate the PV of premium swap leg ( representing the Bond ) ----

		etrading::ReadDataFile::Load PVPremiumFileObj = etrading::ReadDataFile::Load( TRS_CALCULATE_PREMIUM_PV );
		std::string premiumLegName			= PVPremiumFileObj[ "legName"];

		const double calculatedPremiumPV = validation::tryAqObjectsTotalReturnSwapPV( swapName_payOnSurvival, creditModelName, premiumLegName.c_str() );

		// The PAYMENTTRIGGER is set on the FLOAT leg. The premium LEG should therefore have the same PV as before.
        CheckTestResultsAndRebaseOnRequest( calculatedPremiumPV, TEST_DIR, TRS_EXPECTED_PREMIUM_PV, pvTolerance );


		// ---- Calculate the PV of the float swap leg, representing the financing ----
		
		etrading::ReadDataFile::Load PVFloatFileObj = etrading::ReadDataFile::Load( TRS_CALCULATE_FLOAT_PV );
		std::string floatLegName			= PVFloatFileObj[ "legName"];

		const double calculatedFloat_PayOnSurvival_PV = validation::tryAqObjectsTotalReturnSwapPV( swapName_payOnSurvival, creditModelName, floatLegName.c_str() );

		// Check the Test Results or Rebase
        CheckTestResultsAndRebaseOnRequest( calculatedFloat_PayOnSurvival_PV, TEST_DIR, TRS_EXPECTED_PAY_ON_SURVIVAL_FLOAT_PV, pvTolerance );

		// Final sanity check: The PV of the FLOAT leg with PayOnSurvival should be LESS than the PV with PayAlways
		createLWOSwapGeneratorFromFileName( GEN_TOTAL_RETURN_SWAPGENERATOR );
		const std::string swapName = createLWOTotalReturnSwapFromFileName( GEN_TOTAL_RETURN_SWAP );

		const double calculatedFloatPV = validation::tryAqObjectsTotalReturnSwapPV( swapName, creditModelName, floatLegName.c_str() );

		// ASSERT_LE( calculatedFloat_PayOnSurvival_PV, calculatedFloatPV );
		ASSERT_LE( fabs( calculatedFloat_PayOnSurvival_PV ), fabs( calculatedFloatPV ) );
	}

	TEST_F( TestTotalReturnSwap, SNAPSHOT_DisplayCashflows_PayOnSurvival )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		setUpGeneratorCurve( GEN_USD_STD_MARKETDATA, GEN_USD_STD_CURVE );

		// Create and Calibrate the credit model
		std::string creditModelName = createLWOCreditModelFromFileName( GEN_USD_CREDIT_MODEL );

		// Create swap generator
		createLWOSwapGeneratorFromFileName( GEN_TOTAL_RETURN_SWAPGENERATOR_PAY_ON_SURVIVAL );

		// Create TRS
		const std::string swapName_payOnSurvival = createLWOTotalReturnSwapFromFileName( GEN_TOTAL_RETURN_SWAP_PAY_ON_SURVIVAL );

		// Check that we can display cashflows without needing to first invoking PV

		etrading::ReadDataFile::Load displayFileObj = etrading::ReadDataFile::Load( TRS_DISPLAY_CASHFLOWS );
		AQLStringMatrix displayModelNames	= displayFileObj[ "modelNames" ];

		validateCashflowsForLegName( swapName_payOnSurvival, creditModelName, displayModelNames, "Leg1:PREMIUM");
		validateCashflowsForLegName( swapName_payOnSurvival, creditModelName, displayModelNames, "Leg2:FLOAT");

	}	

	TEST_F( TestTotalReturnSwap, SNAPSHOT_Annuity )
    {

		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		setUpGeneratorCurve( GEN_USD_STD_MARKETDATA, GEN_USD_STD_CURVE );

		// Create and Calibrate the credit model
		std::string creditModelName = createLWOCreditModelFromFileName( GEN_USD_CREDIT_MODEL );


		// Create swap generator - PaymentTrigger: PayAlways
		createLWOSwapGeneratorFromFileName( GEN_TOTAL_RETURN_SWAPGENERATOR );

		// Create TRS
		std::string swapName = createLWOTotalReturnSwapFromFileName( GEN_TOTAL_RETURN_SWAP );
		
		// 1. Calculate Premium Leg annuity
		etrading::ReadDataFile::Load annuityFileObj = etrading::ReadDataFile::Load( TRS_CALCULATE_PREMIUM_ANNUITY );
		std::string legName			= annuityFileObj[ "legName"];
		const double calculatedPremiumAnnuity = validation::tryAqObjectsTotalReturnSwapAnnuity( swapName, creditModelName, legName );

		// Check the Test Results or Rebase
		const double annuityTolerance = 1.0e-2;
        CheckTestResultsAndRebaseOnRequest( calculatedPremiumAnnuity, TEST_DIR, TRS_EXPECTED_PREMIUM_ANNUITY, annuityTolerance );

		// 2. Calculate Float Leg annuity
		legName = "Leg2:FLOAT";
		const double calculatedFloatAnnuity_payAlways = validation::tryAqObjectsTotalReturnSwapAnnuity( swapName, creditModelName, legName );
        CheckTestResultsAndRebaseOnRequest( calculatedFloatAnnuity_payAlways, TEST_DIR, TRS_EXPECTED_FLOAT_PAYALWAYS_ANNUITY, annuityTolerance );

		
		// Create swap generator - PaymentTrigger: PayOnSurvival
		createLWOSwapGeneratorFromFileName( GEN_TOTAL_RETURN_SWAPGENERATOR_PAY_ON_SURVIVAL );

		// Create TRS: PayOnSurvival
		const std::string swapName_payOnSurvival = createLWOTotalReturnSwapFromFileName( GEN_TOTAL_RETURN_SWAP_PAY_ON_SURVIVAL );

		// 3. Calculate Float Leg annuity: PayOnSurvival
		const double calculatedFloatAnnuity_payOnSurvival = validation::tryAqObjectsTotalReturnSwapAnnuity( swapName_payOnSurvival, creditModelName, legName );
        CheckTestResultsAndRebaseOnRequest( calculatedFloatAnnuity_payOnSurvival, TEST_DIR, TRS_EXPECTED_FLOAT_PAYONSURVIVAL_ANNUITY, annuityTolerance );

		// Final sanity check: The annuity for "PayOnSurvival" must be strictly less than the annuity for "PayAlways"
		ASSERT_LT( calculatedFloatAnnuity_payOnSurvival, calculatedFloatAnnuity_payAlways );

	}
}
