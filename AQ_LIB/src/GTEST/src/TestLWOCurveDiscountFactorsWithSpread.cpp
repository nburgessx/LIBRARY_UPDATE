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
#include "DataUtilities.h"	// For AQ_TO_STRING macros

#include <sstream>
#include <boost/range/irange.hpp>

#include "CurveOis.h"
#include "AQLDateScheduleHelpers.h"

#include "tryMeLWOSwapPricing.h"
#include "JSONInfoBlock.h"          // JSON InfoBlock Helpers

#include "tryMeLWOCurveMarketData.h"
#include "tryMeLWOCurveCalibrate.h"

#include "tryMeLWOCurveDiscountFactor.h"
#include "tryMeLWOFixingTable.h"


using etrading::ReadDataFile;

// Define the Test Input Folder Here
#define TEST_DIR			  "ETrading/Curves/TestCurveDiscountFactorsWithSpread/"

namespace
{
	// test tolerances
    // ---------------
    const double tolerance = 1e-8;


    //
    // curve input files
    //

	// -------------------------------------------------------------

	// Curves objects saved with meLWOSave
	const char EUR_OIS_MARKET_DATA[]			= TEST_DIR "EUR_OIS_CURVE_MARKETDATA@34_tryMeLWOCurveMarketDataCreate_inputs.csv";
	const char EUR_OIS_CURVE[]					= TEST_DIR "EUR_OIS_CURVE@35_tryMeLWOCurveCalibrate_inputs.csv";

	const char EUR_STD_MARKET_DATA[]			= TEST_DIR "EUR_SWAP_3M_CURVE_MARKETDATA@19_tryMeLWOCurveMarketDataCreate_inputs.csv";
	const char EUR_STD_CURVE[]				    = TEST_DIR "EUR_SWAP_3M_CURVE@20_tryMeLWOCurveCalibrate_inputs.csv";

	const char DISCOUNT_FACTORS_NO_SPREAD_INPUT[]	= TEST_DIR "tryMeLWOCurveDiscountFactorsWithSpread_0bp_inputs.csv";
	const char DISCOUNT_FACTORS_NO_SPREAD_OUTPUT[]	= TEST_DIR "tryMeLWOCurveDiscountFactorsWithSpread_0bp_outputs.csv";

	const char DISCOUNT_FACTORS_1BP_INPUT[]		= TEST_DIR "tryMeLWOCurveDiscountFactorsWithSpread_1bp_inputs.csv";
	const char DISCOUNT_FACTORS_1BP_OUTPUT[]	= TEST_DIR "tryMeLWOCurveDiscountFactorsWithSpread_1bp_outputs.csv";

	const char EUR_STD_FIXING_TABLE[]			= TEST_DIR "EUR3M_FIXINGS@74_tryMeLWOFixingTableCreate_inputs.csv";

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


	/* @brief			Builds LWO MarketData Object by invoking the tryMeLWOCurveMarketDataCreate() API.
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

		validation::tryMeLWOCurveMarketDataCreate( objectName, marketDataKeys, infoBlocks );
	}

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
		createLWOMarketDataObjectFromFileName( marketDataFileName );
		createLWOCurveFromFileName( curveCalibrationFileName );
	}

	void calculateDFsWithFlatSpread( const std::string& inputFile, const std::string& expectedResultsFile )
	{
		etrading::ReadDataFile::Load DFInputFileObj = etrading::ReadDataFile::Load( inputFile );
		const DateVector paymentDates = DFInputFileObj[ "paymentDates" ];
		const std::string curveCollection	= DFInputFileObj[ "curveCollection" ];
		const std::string curveIndex		= DFInputFileObj[ "curveIndex" ];
		const double spread					= DFInputFileObj[ "spread"];
		const std::string fixingTableName	= DFInputFileObj[ "fixingTableName" ];
	
		const DoubleVector calculatedDFs = validation::tryMeLWOCurveDiscountFactorsWithSpread( paymentDates, curveCollection, curveIndex, spread, fixingTableName );
		
        google_test::CheckTestResultsAndRebaseOnRequest( calculatedDFs, TEST_DIR, expectedResultsFile, tolerance );
	
	}

	void setUpFixingTable( const AQLString& fixingTableFileName )
	{
		etrading::ReadDataFile::Load fixingInputFileObj = etrading::ReadDataFile::Load( fixingTableFileName );
		auto tableName     = fixingInputFileObj["tableName"];
		auto currency      = fixingInputFileObj["currency"];
		auto curveTenor    = fixingInputFileObj["curveTenor"];
		auto fixingDates   = fixingInputFileObj["fixingDates"];
		auto fixingValues  = fixingInputFileObj["fixingValues"];
		
		const std::string fixingTableName = validation::tryMeLWOFixingTableCreate(tableName, currency, curveTenor, fixingDates, fixingValues);
	}

}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestCurveDiscountFactorsWithSpread);

	//----------------------------------------------------------------------------------------


	TEST_F( TestCurveDiscountFactorsWithSpread, SNAPSHOT_CurveDiscountFactors_0bp_Spread )
    {
		// Create OIS discount curve 
		setUpGeneratorCurve( EUR_OIS_MARKET_DATA, EUR_OIS_CURVE );
		setUpGeneratorCurve( EUR_STD_MARKET_DATA, EUR_STD_CURVE );

		setUpFixingTable(EUR_STD_FIXING_TABLE);

		calculateDFsWithFlatSpread( DISCOUNT_FACTORS_NO_SPREAD_INPUT, DISCOUNT_FACTORS_NO_SPREAD_OUTPUT );

	}

	TEST_F( TestCurveDiscountFactorsWithSpread, SNAPSHOT_CurveDiscountFactors_1bp_Spread )
    {
		// Create OIS discount curve 
		setUpGeneratorCurve( EUR_OIS_MARKET_DATA, EUR_OIS_CURVE );
		setUpGeneratorCurve( EUR_STD_MARKET_DATA, EUR_STD_CURVE );

		setUpFixingTable( EUR_STD_FIXING_TABLE );

		calculateDFsWithFlatSpread( DISCOUNT_FACTORS_1BP_INPUT, DISCOUNT_FACTORS_1BP_OUTPUT );

	}
}

