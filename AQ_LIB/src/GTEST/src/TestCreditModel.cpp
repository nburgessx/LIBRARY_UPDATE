#include "Variant.h"
#include "LabelValueBlock.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "TestHelperUtilities.h"
#include "InitializeETrading.h"
#include "tryAqToolSetup.h"
#include "ContainerUtilities.h"
#include "CoreEnumerations.h"
#include "ResultsProcessor.h"
#include "DataUtilities.h"	// For AQ_TO_STRING macros

#include <sstream>
#include <boost/range/irange.hpp>

#include "CurveOis.h"
#include "AQLDateScheduleHelpers.h"

#include "tryAqSwapObjectPricing.h"
#include "JSONInfoBlock.h"          // JSON InfoBlock Helpers

#include "tryAqCurveMarketData.h"
#include "tryAqCurveObjectCalibrate.h"
#include "tryAqSwapObjectCreation.h"
#include "tryAqSwapObjectPricing.h"

#include "tryAqDate.h"


using etrading::ReadDataFile;

// Define the Test Input Folder Here
#define TEST_DIR			  "ETrading/AQObjects/TestAQObjCreditModel/"


namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 1e-8;


    //
    // curve input files
    //

	// -------------------------------------------------------------

	// Curve market data files for use with CurveGenerators
	const char GEN_USD_OIS_MARKETDATA[]			= TEST_DIR "USD_OIS_CURVE_MARKETDATA@6_tryAqCurveMarketDataCreate_inputs.csv";

	// -------------------------------------------------------------

	// Curves built from generator and market data
	const char GEN_USD_OIS_CURVE[]				= TEST_DIR "USD_OIS_tryAqCurveObjectCalibrate_inputs.csv";

	// -------------------------------------------------------------

	// Build Credit Model
	const char GEN_USD_CREDIT_MODEL[]			        = TEST_DIR "CREDITMODEL1@2_tryAqCreditModelCreate_inputs.csv";
	const char VIEW_CREDIT_MODEL_CALIBRATION[]	        = TEST_DIR "tryAqCreditModelCalibrationParameters_inputs.csv";

	// Build Credit Default Swap
	const char GEN_CREDIT_DEFAULT_SWAP_5Y[]		        = TEST_DIR "USD_CDS_IMM_tryAqSwapObjectCreateFromGenerator_5Y_inputs.csv" ;

	// API methods
	const char CDS_CALCULATE_PV[]				        = TEST_DIR "tryAqCDSObjectPV_inputs.csv";
	const char CDS_CALCULATE_PV_BY_INTEGRATION[]        = TEST_DIR "tryAqCDSObjectPVByIntegration_inputs.csv";
	const char CDS_CALCULATE_PV_BY_MONTE_CARLO_MT[]     = TEST_DIR "tryAqCDSObjectPVByMonteCarlo_inputs_MT.csv";
    const char CDS_CALCULATE_PV_BY_MONTE_CARLO_SOBOL[]  = TEST_DIR "tryAqCDSObjectPVByMonteCarlo_inputs_SOBOL.csv";
	const char CDS_CALCULATE_RISKY_ANNUITY[]	        = TEST_DIR "tryAqCDSObjectRiskyAnnuity_inputs.csv";
	const char CDS_CALCULATE_CS01[]				        = TEST_DIR "tryAqCDSObjectCS01_inputs.csv";
	const char CDS_CALCULATE_HAZARD_RATE[]		        = TEST_DIR "tryAqCreditModelHazardRate_inputs.csv";
	const char CDS_CALCULATE_SURVIVAL_PROB[]	        = TEST_DIR "tryAqCreditModelSurvivalProbability_inputs.csv";
	const char CDS_CALCULATE_DEFAULT_PROB[]		        = TEST_DIR "tryAqCreditModelDefaultProbability_inputs.csv";

	const char CDS_REPRICE_PREFIX[]				        = TEST_DIR "USD_CDS_IMM_tryAqSwapObjectCreateFromGenerator_Reprice_inputs_";

	//const char CDS_CALCULATE_HAZARD_RATE[]		    = TEST_DIR "tryAqCDSObjectHazardRateFromParSpread_inputs.csv";

	// Snapshot results
	const char CREDITMODEL_EXPECTED_CALIBRATION[]       = TEST_DIR "tryAqCreditModelCalibrationParameters_outputs.csv";

	const char CDS_EXPECTED_PV[]				                = TEST_DIR "tryAqCDSObjectPV_outputs.csv";
	const char CDS_EXPECTED_PV_BY_INTEGRATION[]	                = TEST_DIR "tryAqObjCreditDefaultSwapPVIntegration_outputs.csv";
	const char CDS_EXPECTED_PV_BY_MONTECARLO_MERSENNE_TWISTER[]	= TEST_DIR "tryAqObjCreditDefaultSwapPVMonteCarlo_outputs.csv";
	const char CDS_EXPECTED_PV_BY_MONTECARLO_SOBOL[]			= TEST_DIR "tryAqObjCreditDefaultSwapPVMonteCarloSobol_outputs.csv";
	const char CDS_EXPECTED_RISKY_ANNUITY[]		                = TEST_DIR "tryAqCDSObjectRiskyAnnuity_outputs.csv";
	const char CDS_EXPECTED_CS01[]				                = TEST_DIR "tryAqCDSObjectCS01_outputs.csv";


	// const char CDS_EXPECTED_HAZARD_RATE[]		= TEST_DIR "tryAqCDSObjectHazardRateFromParSpread_outputs.csv";


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

	/* @brief			Builds AQObj MarketData Object by invoking the tryAqCurveMarketDataCreate() API.
	*                   The code loops over all of the capitalized data keys in the specified filename and uses
	*                   these blocks to construct the MarketData object.
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/	
	void createAQObjMarketDataObjectFromFileName( const AQLString& marketDataFileName )
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

		validation::tryAqCurveMarketDataCreate( objectName, marketDataKeys, infoBlocks );
	}

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

		validation::tryAqCurveObjectCalibrate(	objectName, aqObjCurveGeneratorName, aqObjCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );
	}	

	/* @brief			Builds and Generator curve using the specified marketData and calibration filename
	*  @param [in]		marketDataFileName			The filename specifying generator curve data
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/
	void setUpGeneratorCurve( const AQLString& marketDataFileName, const AQLString& curveCalibrationFileName )
	{
		createAQObjMarketDataObjectFromFileName( marketDataFileName );
		createAQObjCurveFromFileName( curveCalibrationFileName );
	}

	std::string createAQObjCreditModelFromFileName( const AQLString& creditModelFileName )
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

		std::string objectName = validation::tryAqCreditModelCreate( creditModelName, propertyNames, infoBlocks );
		return objectName;
	}

	void checkAQObjCreditModelCalibration( const AQLString& viewCreditModelCalibrationFileName, const AQLString& expectedCreditModelCalibrationFileName )
	{
		etrading::ReadDataFile::Load viewCreditModelCalibrationFileObj = etrading::ReadDataFile::Load( viewCreditModelCalibrationFileName );
		std::string creditModelName = viewCreditModelCalibrationFileObj[ "creditModelName" ];
		AnyTypeMatrix results = validation::tryAqCreditModelCalibrationParameters( creditModelName );

		const size_t nRows = results.size();
		ASSERT_EQ( nRows, 8 ) << "#Error: Expected 8 rows of calibration results";  // One row per calibration point
		
		const size_t nCols = results[0].size();
		ASSERT_EQ( nCols, 4) << "#Error: Expected 4 columns of calibration results";  // MaturityDate, HazardRate, SurvivialProbability, DefaultProbability

		if ( etrading::CreateDataFile::rebaseResultsEnabled() )
		{
			etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
			AQLStringVector v = AQLString( expectedCreditModelCalibrationFileName ).toToken( '/' );
            AQLString resultFileName = v.back();
			etrading::CreateDataFile file( etrading::CreateDataFile::makeFilename( resultFileName ) );
            file.write( "output", results );
		}
		else
		{
			etrading::ReadDataFile::Load expectedCreditModelCalibrationFileObj = etrading::ReadDataFile::Load( expectedCreditModelCalibrationFileName );		
			AQLStringMatrix expectedResults = expectedCreditModelCalibrationFileObj[ "output" ];
			ASSERT_EQ( nRows, expectedResults.size() ) << "#Error: Expected number of rows differ";
			ASSERT_EQ( nCols, expectedResults[0].size() ) << "#Error: Expected number of columns differ";

			for ( size_t row=0; row<nRows; row++ )
			{
				for( size_t col=0; col<nCols; col++ )
				{
					AnyType anyValue = results[row][col];

					double calculatedValueAsDouble = 0.;
					if ( col == 0)
					{
						// First column contains a date
						calculatedValueAsDouble = boost::get<int>( anyValue );
					}
					else
					{
						// All other columns are doubles
						calculatedValueAsDouble = boost::get<double>( anyValue );
					}
				
					AQLString expectedValue = expectedResults[row][col];
					double expectedValueAsDouble = expectedValue.getDoubleValue();

					EXPECT_NEAR( calculatedValueAsDouble, expectedValueAsDouble, tolerance ) << "Difference in calibration parameters at " << row << ", " << col ;
				}
			}

		}
	}

	std::string createAQObjCreditDefaultSwapFromFileName( const AQLString& cdsFileName )
	{
		etrading::ReadDataFile::Load creditDefaultSwapFileObj = etrading::ReadDataFile::Load( cdsFileName );
		
		const std::string swapName				= creditDefaultSwapFileObj[ "swapName" ];
		const std::string aqObjswapGeneratorName	= creditDefaultSwapFileObj[ "swapGeneratorName" ];
		const AQLStringMatrix expressionLVB		= creditDefaultSwapFileObj[ "expressionLVB" ];
		const AQLStringMatrix swapPropertiesLVB	= creditDefaultSwapFileObj[ "swapPropertiesLVB" ];
		const bool isXccySwap					= creditDefaultSwapFileObj[ "isXccySwap" ];
		const bool validateKeys					= creditDefaultSwapFileObj[ "validateKeys" ];
		
		std::string cdsName = validation::tryAqSwapObjectCreateFromGenerator( swapName, aqObjswapGeneratorName, expressionLVB, swapPropertiesLVB, isXccySwap, validateKeys );
		return cdsName;
	}

	void checkAQObjCreditModelConsistency( const AQLString& viewCreditModelCalibrationFileName )
	{
		etrading::ReadDataFile::Load viewCreditModelCalibrationFileObj = etrading::ReadDataFile::Load( viewCreditModelCalibrationFileName );
		std::string creditModelName = viewCreditModelCalibrationFileObj[ "creditModelName" ];
		AnyTypeMatrix results = validation::tryAqCreditModelCalibrationParameters( creditModelName );

		const size_t nRows = results.size();
		ASSERT_EQ( nRows, 8 ) << "#Error: Expected 8 rows of calibration results";  // One row per calibration point
		
		const size_t nCols = results[0].size();
		ASSERT_EQ( nCols, 4) << "#Error: Expected 4 columns of calibration results";  // MaturityDate, HazardRate, SurvivialProbability, DefaultProbability

		int prevMaturityDateAsInt = 0;
		double prevDefaultProbability = 0.;

		for ( size_t row=0; row<nRows; row++ )
		{

			AnyType anyMaturityDate = results[row][0];
			int maturityDateAsInt = boost::get<int>( anyMaturityDate );

			AnyType anyHazardRate = results[row][1];
			double hazardRate = boost::get<double>( anyHazardRate );

			AnyType  anySurvivalProbability = results[row][2];
			double survivalProbability = boost::get<double>( anySurvivalProbability );

			AnyType anyDefaultProbability = results[row][3];
			double defaultProbability = boost::get<double>( anyDefaultProbability );

			// 1. Check consistency between calibration survival probability and default probability
			double expectedDefaultProbability = 1 - survivalProbability;
			ASSERT_NEAR( defaultProbability, expectedDefaultProbability, tolerance ) << "#Error: Mismatch in default probability";

			// 2. Check consistency between calibration hazard rate and API hazard rate
			AQLDate fromDate;
			AQLDate toDate = etrading::AQLDateScheduleHelpers::getAQLDate( maturityDateAsInt );
			double apiHazardRate = validation::tryAqCreditModelHazardRate( creditModelName, toDate );
			ASSERT_NEAR( hazardRate, apiHazardRate, tolerance ) << "#Error: Mismatch in hazard rate";

			// 3. Check consistency between calibration survival probability and API survival probability
			double apiSurvivalProbability = validation::tryAqCreditModelSurvivalProbability( creditModelName, toDate, fromDate );
			ASSERT_NEAR( survivalProbability, apiSurvivalProbability, tolerance ) << "#Error: Mismatch in survival probability";
			
			if ( row > 0 )
			{
				// 4. Check consistency between calibration default probabability and API marginal default probability
				double marginalDefaultProbability = defaultProbability - prevDefaultProbability;

				fromDate = etrading::AQLDateScheduleHelpers::getAQLDate( prevMaturityDateAsInt );
				double apiMarginalDefaultProbability = validation::tryAqCreditModelDefaultProbability( creditModelName, toDate, fromDate );
				ASSERT_NEAR( marginalDefaultProbability, apiMarginalDefaultProbability, tolerance ) << "#Error: Mismatch in marginal default probability";
			}

			prevMaturityDateAsInt = maturityDateAsInt;
			prevDefaultProbability = defaultProbability;
		}

	}

	void checkAQObjCreditModelExtrapolation( const AQLString& viewCreditModelCalibrationFileName )
	{
		etrading::ReadDataFile::Load viewCreditModelCalibrationFileObj = etrading::ReadDataFile::Load( viewCreditModelCalibrationFileName );
		std::string creditModelName = viewCreditModelCalibrationFileObj[ "creditModelName" ];
		AnyTypeMatrix results = validation::tryAqCreditModelCalibrationParameters( creditModelName );

		const size_t nRows = results.size();
		ASSERT_EQ( nRows, 8 ) << "#Error: Expected 8 rows of calibration results";  // One row per calibration point
		
		const size_t nCols = results[0].size();
		ASSERT_EQ( nCols, 4) << "#Error: Expected 4 columns of calibration results";  // MaturityDate, HazardRate, SurvivialProbability, DefaultProbability

		// Fetch the final calibration row
		size_t row = nRows - 1;
		AnyType anyMaturityDate = results[row][0];
		int maturityDateAsInt = boost::get<int>( anyMaturityDate );

		AnyType  anySurvivalProbability = results[row][2];
		double survivalProbability = boost::get<double>( anySurvivalProbability );

		AnyType anyDefaultProbability = results[row][3];
		double defaultProbability = boost::get<double>( anyDefaultProbability );

		// Check consistency between calibration survival probability and API survival probability at the final calibration point
		const AQLDate asOfDate = validation::tryAqCreditModelAsOfDate( creditModelName );
		AQLDate toDate = etrading::AQLDateScheduleHelpers::getAQLDate( maturityDateAsInt );
		const double apiSurvivalProbability = validation::tryAqCreditModelSurvivalProbability( creditModelName, toDate, asOfDate );
		ASSERT_NEAR( survivalProbability, apiSurvivalProbability, tolerance ) << "#Error: Mismatch in survival probability";

		const double apiMarginalDefaultProbability = validation::tryAqCreditModelDefaultProbability( creditModelName, toDate, asOfDate );
		ASSERT_NEAR( defaultProbability, apiMarginalDefaultProbability, tolerance ) << "#Error: Mismatch in marginal default probability";

		// Calculate survival probabilities for extrapolation dates and perform sanity checks
		double prevSurvivalProbability = survivalProbability;
		double prevDefaultProbability = defaultProbability;

		// Extrapolate for 12 months
		for (int i=0; i<12; i++)
		{
			const AQLString tenor("1M");
			const AQLString businessDayAdj;
			const AQLString calendar;
			const AQLString rolLConvention;
			toDate = validation::tryAqDateFromTenor( toDate, tenor, businessDayAdj, calendar, rolLConvention );

			const double survivalProbability = validation::tryAqCreditModelSurvivalProbability( creditModelName, toDate, asOfDate );
			ASSERT_TRUE( survivalProbability < prevSurvivalProbability ) << "#ERROR: survivalProbability should be monotonically decreasing when extrapolating";

			const double defaultProbability = validation::tryAqCreditModelDefaultProbability( creditModelName, toDate, asOfDate );
			double marginalDefaultProbability = defaultProbability - prevDefaultProbability;
			ASSERT_TRUE( marginalDefaultProbability > 0 ) << "#ERROR: marginalDefaultProbability should be positive in each time period, when extrapolating";

			prevSurvivalProbability = survivalProbability;
			prevDefaultProbability = defaultProbability;
		}
	}

	void calculateAndCheckCDS_PV()
	{
		etrading::ReadDataFile::Load PVFileObj = etrading::ReadDataFile::Load( CDS_CALCULATE_PV );
		const std::string swapName			= PVFileObj[ "swapName" ];
		std::string creditModelName			= PVFileObj[ "creditModelName" ];
		std::string legName					= PVFileObj[ "legName"];
		const double calculatedPV = validation::tryAqCDSObjectPV( swapName, creditModelName, legName.c_str() );
		
		const double pvTolerance = 0.01;
        google_test::CheckTestResultsAndRebaseOnRequest( calculatedPV, TEST_DIR, CDS_EXPECTED_PV, pvTolerance );
	}

	void calculateAndCheckCDS_PV_by_Integration()
	{
		// First calculate the closed-form PV
		etrading::ReadDataFile::Load PVFileObj = etrading::ReadDataFile::Load( CDS_CALCULATE_PV_BY_INTEGRATION );
		const std::string swapName			= PVFileObj[ "swapName" ];
		std::string creditModelName			= PVFileObj[ "creditModelName" ];
		std::string legName					= PVFileObj[ "legName"];
		const double analyticPV = validation::tryAqCDSObjectPV( swapName, creditModelName, legName.c_str() );
		
		// Calculate the PV by integration over survival probability / stopping time
		const size_t nIntegrationPoints		= PVFileObj[ "numberOfIntegrationPoints"];
		const bool evaluateInParallel		= PVFileObj[ "evaluateInParallel"];
		const bool payDefaultCashflowsOnNextCouponDate = PVFileObj[ "payDefaultCashflowsOnNextCouponDate"];
		const double pvByIntegration = validation::tryAqCDSObjectPVByIntegration( swapName, creditModelName, legName.c_str(), nIntegrationPoints, evaluateInParallel, payDefaultCashflowsOnNextCouponDate );

		const double pvTolerance = 0.01;
		google_test::CheckTestResultsAndRebaseOnRequest( pvByIntegration, TEST_DIR, CDS_EXPECTED_PV_BY_INTEGRATION, pvTolerance );

		// PV by integration and analytic PV should be close
		const double comparisonTolerance = 3.5;
		ASSERT_NEAR( analyticPV, pvByIntegration, comparisonTolerance );

	}

	void calculateAndCheckCDS_PV_by_MonteCarlo()
	{
		// First calculate the closed-form PV
		etrading::ReadDataFile::Load PVFileObj = etrading::ReadDataFile::Load( CDS_CALCULATE_PV_BY_MONTE_CARLO_MT );
        const std::string swapName			= PVFileObj[ "swapName" ];
		std::string creditModelName			= PVFileObj[ "creditModelName" ];
		std::string legName					= PVFileObj[ "legName"];
		const double analyticPV = validation::tryAqCDSObjectPV( swapName, creditModelName, legName.c_str() );
		
		// Calculate the PV by monte-carlo simulation over survival probability / stopping time
		// The default test setup uses Mersenne-Twister derived paths
		double standardError = 0.0;
		AQLStringMatrix mcParameters		= PVFileObj[ "mcParametersLVB"];
		const bool payDefaultCashflowsOnNextCouponDate= PVFileObj[ "payDefaultCashflowsOnNextCouponDate" ];
		
		const double pvByMersenneTwisterMC = validation::tryAqCDSObjectPVByMonteCarlo( swapName, creditModelName, legName.c_str(), mcParameters, payDefaultCashflowsOnNextCouponDate, standardError );
		
		// Check PV against recorded Mersenne Twister baseline
		const double pvTolerance = 0.01;
		google_test::CheckTestResultsAndRebaseOnRequest( pvByMersenneTwisterMC, TEST_DIR, CDS_EXPECTED_PV_BY_MONTECARLO_MERSENNE_TWISTER, pvTolerance );

		// Check PV against analytic value
		const double comparisonTolerance = 3.5;
		ASSERT_NEAR( analyticPV, pvByMersenneTwisterMC, comparisonTolerance );
		
		// Now re-run the test using Sobol sequence
        etrading::ReadDataFile::Load PVFileObjSOBOL = etrading::ReadDataFile::Load( CDS_CALCULATE_PV_BY_MONTE_CARLO_SOBOL );
        AQLStringMatrix mcParametersSobol		= PVFileObjSOBOL[ "mcParametersLVB"];
		const double pvBySobolMC = validation::tryAqCDSObjectPVByMonteCarlo( swapName, creditModelName, legName.c_str(), mcParametersSobol, payDefaultCashflowsOnNextCouponDate, standardError );

		// Check PV against recorded Sobol baseline
		google_test::CheckTestResultsAndRebaseOnRequest( pvBySobolMC, TEST_DIR, CDS_EXPECTED_PV_BY_MONTECARLO_SOBOL, pvTolerance );

		// Check PV against analytic value
		ASSERT_NEAR( analyticPV, pvBySobolMC, comparisonTolerance );

	}

	void calculateAndCheckCDS_RiskyAnnuity()
	{
		etrading::ReadDataFile::Load PVFileObj = etrading::ReadDataFile::Load( CDS_CALCULATE_RISKY_ANNUITY );
		const std::string swapName			= PVFileObj[ "swapName" ];
		std::string creditModelName			= PVFileObj[ "creditModelName" ];
		std::string legName					= PVFileObj[ "legName"];
		const double calculatedRiskyAnnuity = validation::tryAqCDSObjectRiskyAnnuity( swapName, creditModelName, legName.c_str() );

		// Check the Test Results or Rebase
		// Since the CDS has a CDS spread matching the 5Y point in the calibration market data, the PV had better be close to zero
		double riskyAnnuityTolerance = 0.01;
        google_test::CheckTestResultsAndRebaseOnRequest( calculatedRiskyAnnuity, TEST_DIR, CDS_EXPECTED_RISKY_ANNUITY, riskyAnnuityTolerance );
	}

	void calculateAndCheckCDS_CS01()
	{
		etrading::ReadDataFile::Load PVFileObj = etrading::ReadDataFile::Load( CDS_CALCULATE_CS01 );
		const std::string swapName			= PVFileObj[ "swapName" ];
		std::string creditModelName			= PVFileObj[ "creditModelName" ];
		std::string legName					= PVFileObj[ "legName"];
		const double calculatedCS01 = validation::tryAqCDSObjectCS01( swapName, creditModelName, legName.c_str() );

		// Check the Test Results or Rebase
		// Since the CDS has a CDS spread matching the 5Y point in the calibration market data, the PV had better be close to zero
        double cs01Tolerance = 0.01;
        google_test::CheckTestResultsAndRebaseOnRequest( calculatedCS01, TEST_DIR, CDS_EXPECTED_CS01, cs01Tolerance );
	}

}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestCreditModel);

	//----------------------------------------------------------------------------------------

	/* @brief Compares the calibration parameters from the CreditModel against snapshot values
	*         Also checks snapshot values of PV, RiskyAnnuity and CS01 for a 5Y CDS.
	*/
	TEST_F( TestCreditModel, SNAPSHOT_CreditModel_Calibrate_And_Price )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		
		// Create and Calibrate the credit model
		createAQObjCreditModelFromFileName( GEN_USD_CREDIT_MODEL );
		
		// Check the calibration parameters match snapshot results
		checkAQObjCreditModelCalibration( VIEW_CREDIT_MODEL_CALIBRATION, CREDITMODEL_EXPECTED_CALIBRATION );

		// Now create a CDS which matches the 5Y point in calibration data
		createAQObjCreditDefaultSwapFromFileName( GEN_CREDIT_DEFAULT_SWAP_5Y );

		// Calculate the CDS PV using CreditModel.
		calculateAndCheckCDS_PV();

		// Calculate the CDS Risky Annuity using CreditModel
		calculateAndCheckCDS_RiskyAnnuity();

		// Calculate the CDS CS01 using CreditModel
		calculateAndCheckCDS_CS01();

	}

	TEST_F( TestCreditModel, SNAPSHOT_CreditModel_Price_by_Integration )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		
		// Create and Calibrate the credit model
		createAQObjCreditModelFromFileName( GEN_USD_CREDIT_MODEL );

		// Now create a CDS which matches the 5Y point in calibration data
		createAQObjCreditDefaultSwapFromFileName( GEN_CREDIT_DEFAULT_SWAP_5Y );

		// Calculate the CDS PV using CreditModel.
		calculateAndCheckCDS_PV_by_Integration();
	}

	
	TEST_F( TestCreditModel, SNAPSHOT_CreditModel_Price_by_MonteCarlo )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		
		// Create and Calibrate the credit model
		createAQObjCreditModelFromFileName( GEN_USD_CREDIT_MODEL );

		// Now create a CDS which matches the 5Y point in calibration data
		createAQObjCreditDefaultSwapFromFileName( GEN_CREDIT_DEFAULT_SWAP_5Y );

		// Calculate the CDS PV using CreditModel.
		calculateAndCheckCDS_PV_by_MonteCarlo();
	}
	
	/* @brief Checks that the CreditModel is able to reprice CDSs exactly, such that the CDS par rate matches input market data
	*/
	TEST_F( TestCreditModel, CONSISTENCY_Reprice_MarketData )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		
		// Create and Calibrate the credit model
		std::string creditModelName = createAQObjCreditModelFromFileName( GEN_USD_CREDIT_MODEL );

		etrading::ReadDataFile::Load creditModelFileObj = etrading::ReadDataFile::Load( GEN_USD_CREDIT_MODEL );
		const AQLStringMatrix cdsMarketData		= creditModelFileObj[ "CDS_MARKETDATA" ];

		const size_t nRows = cdsMarketData.size();
		for (size_t row = 0; row < nRows; row++ )
		{
			std::string filename = std::string( CDS_REPRICE_PREFIX )  + AQ_TO_STRING_FROM_SIZE_T(row) + ".csv";
			std::string cdsName = createAQObjCreditDefaultSwapFromFileName( filename.c_str() );

			AQLString premiumLegName;
			AQLString protectionLegName;
			const double calculatedCDSSpread = validation::tryAqCDSObjectParSpread( cdsName, creditModelName, premiumLegName, protectionLegName );

            char * pFirstNonNumber;
			const double expectedCDSSpread = strtod( cdsMarketData[row][1].getCString(), &pFirstNonNumber );
			ASSERT_NEAR( calculatedCDSSpread, expectedCDSSpread, tolerance ) << "#Error: Mismatch in reprice CDS Spread";	
		}
	}	

	/* @brief Checks the consistency between CreditModel calibration parameters and API function calls:
	*         Checks the hazard rate, survival probability, default probability, marginal default probability
	*/
	TEST_F( TestCreditModel, CONSISTENCY_Check_API_Methods )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		
		// Create and Calibrate the credit model
		std::string creditModelName = createAQObjCreditModelFromFileName( GEN_USD_CREDIT_MODEL );

		checkAQObjCreditModelConsistency( VIEW_CREDIT_MODEL_CALIBRATION );

	}

	/* @brief Checks that the CreditModel is capable of extrapolation beyond the final calibration point
	*/
	TEST_F( TestCreditModel, CONSISTENCY_CheckExtrapolation )
    {
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		
		// Create and Calibrate the credit model
		std::string creditModelName = createAQObjCreditModelFromFileName( GEN_USD_CREDIT_MODEL );

		checkAQObjCreditModelExtrapolation( VIEW_CREDIT_MODEL_CALIBRATION );
	}

	std::string setUpManualCreditModel( const std::string creditModelName )
	{
		// Create OIS discount curve and CDS object
		setUpGeneratorCurve( GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		
		// Create and Calibrate the credit model
        // --------------------------------------

        const etrading::VariantMatrix modelParameters =
        {
            {"AsOfDate",                    "20180312"},
            {"SpotLag",                     "5D"},
            {"SpotBusinessDayAdjustment",   "FOLLOWING"},
            {"SpotCalendar",                "NYB"},
            {"AccrualStartDate",            "20180312"},
            {"IMMReferenceDate",            "20171217"},
            {"Currency",                    "USD"},
            {"RecoveryRate",                "0.4"},
            {"IncludeAccruedInterest",      "TRUE"},
            {"CDSCurveCollection",          "USDYC"},
            {"SwapGenerator",               "USD_CDS"},
            {"CreditIndex",                 "US9128282R06"},
            {"Interpolation",               "PiecewiseConstant"},
            {"Extrapolation",               "Flat"} 
        };

        const etrading::VariantMatrix cdsMarketData =
        {
            {"6M",  "0.00193900"},
            {"1Y",  "0.00183013"},
            {"2Y",  "0.00205930"},
            {"3Y",  "0.00219438"},
            {"4Y",  "0.00232063"},
            {"5Y",  "0.00233700"},
            {"7Y",  "0.00275507"},
            {"10Y", "0.00316097"}
        };

        // Credit Model ModelDataTypes
        const std::vector<std::string> modelDataTypes       = { "MODEL_PROPERTIES", "CDS_MARKETDATA" };

		 // Create InfoBlock Tuples - A data container; a tuple of columnNames, columnTypes and a variant data matrix
        const etrading::JSONInfoBlockTuple modelInfoBlock   = etrading::JSONInfoBlock::createInfoBlock( modelParameters );
        const etrading::JSONInfoBlockTuple cdsInfoBlock     = etrading::JSONInfoBlock::createInfoBlock( cdsMarketData );
        const etrading::JSONInfoBlockTuples modelData       = { modelInfoBlock, cdsInfoBlock };

        // Calibrate and Create the Credit Model
        const std::string result = validation::tryAqCreditModelCreate( creditModelName, modelDataTypes, modelData );
		return result;
	}

    TEST_F( TestCreditModel, UNIT_ManualTest_CreditModelCalibration )
    {

        // Calibrate and Create the Credit Model
		const std::string creditModelName                   = "USD_CREDIT_MODEL";
        const std::string result = setUpManualCreditModel( creditModelName );
        
        // Test Credit Model Calibration

        EXPECT_EQ( creditModelName, result );
    }

	// This test iterates over a vector of stopping dates. For each date it calculates a survival probability,
	// then from that survival probability it backs out the implied stopping date. The test checks that
	// the round trip works perfectly i.e. that the same dates are reproduced.
    TEST_F( TestCreditModel, UNIT_ManualTest_SurvivalProbabilityRoundTrip )
    {

		const std::string creditModelName                   = "USD_CREDIT_MODEL";
        const std::string result = setUpManualCreditModel( creditModelName );

		const std::string stoppingDates[] =
        {
            "20180312",
			"20180912",
			"20190312",
			"20200312",
			"20210312",
			"20220312",
			"20230312",
			"20250312",
			"20280312"
        };


		AQLDate toDate;
		AQLDate fromDate(stoppingDates[0].c_str());

		const size_t numDates = sizeof( stoppingDates ) / sizeof(stoppingDates[0]);

		for (size_t i=1; i< numDates; i++)
		{
			AQLDate toDate(stoppingDates[i].c_str());

			const double survivalProbability = validation::tryAqCreditModelSurvivalProbability( creditModelName, toDate, fromDate );
			const AQLDate impliedSurvivalDate = validation::tryAqCreditModelImpliedSurvivalDate( creditModelName, survivalProbability );

			EXPECT_EQ( toDate, impliedSurvivalDate );
			if ( toDate != impliedSurvivalDate )
			{
				std::cout << "Error: " << impliedSurvivalDate.convertDateToString() << " not equal to " << toDate.convertDateToString() << std::endl;
			}
		}
	}

}

