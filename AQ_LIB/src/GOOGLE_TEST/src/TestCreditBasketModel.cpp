
/*
 * @brief			Tests of the CreditBasketModel calibration
 *
 * @Created:		27 Feb 2019
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 *
 */

#include "Variant.h"
#include "LabelValueBlock.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "TestHelperUtilities.h"
#include "InitializeMLibETrading.h"
#include "tryMeUtilitySetup.h"
#include "ContainerUtilities.h"
#include "CoreEnumerations.h"
#include "ResultsProcessor.h"

#include <sstream>
#include <boost/range/irange.hpp>

#include "CurveOis.h"
#include "LAMathDateUtilities.h"

#include "tryMeLWOSwapPricing.h"
#include "JSONInfoBlock.h"          // JSON InfoBlock Helpers

#include "tryMeLWOCurveMarketData.h"
#include "tryMeLWOCurveCalibrate.h"
#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"

#include "tryMeDate.h"

using etrading::ReadDataFile;

// Define the Test Input Folder Here
#define TEST_DIR			  "ETrading/LWObjects/TestLWOCreditModel/"


namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 1e-7;

    //
    // curve input files
    //

	// -------------------------------------------------------------

	// Curve market data files for use with CurveGenerators
	const char GEN_USD_OIS_MARKETDATA[]			= TEST_DIR "USD_OIS_CURVE_MARKETDATA@6_tryMeLWOCurveMarketDataCreate_inputs.csv";

	// -------------------------------------------------------------

	// Curves built from generator and market data
	const char GEN_USD_OIS_CURVE[]				= TEST_DIR "USD_OIS_tryMeLWOCurveCalibrate_inputs.csv";

	// Build Credit Basket Model
	const char CREDIT_BASKET_MODEL[]			= TEST_DIR "CREDITBASKETMODEL1@14_tryMeLWOCreditBasketModelCreate_inputs.csv";


	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, etrading::VariantMatrix>  TableInfo;

	/* @brief			A helper function which converts a LAStringMatrix into a VariantMatrix
	*                   If the input LAStringMatrix is empty, creates a dummy VariantMatrix containing two blank columns 
	*  @param [in]		stringMatrix		The input LAStringMatrix
	*  @returns			The corresponding VariantMatrix
	*/
	etrading::VariantMatrix convertStringMatrixToVariantMatrix( LAStringMatrix stringMatrix )
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
			// The input LAStringMatrix is empty.
			// Create a default variantMatrix with 2 columns of dummy data.
			// This simulates an empty block in Excel.
			etrading::VariantVector dummyVector ( 1, "" );
			variantMatrix.push_back( dummyVector );
			variantMatrix.push_back( dummyVector );
		}

		return variantMatrix;
	}


	/* @brief			Builds a "TableInfo" tuple from a LAStringMatrix of marketdata
	*                   This tuple consists of columnNames, columnTypes and the actual data values.
	*  @param [in]		marketDataBlock		A LAStringMatrix containing key/value market data values
	*/
	TableInfo getTableInfoFromStringMatrix( const LAStringMatrix& marketDataBlock )
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
	void createLWOMarketDataObjectFromFileName( const LAString& marketDataFileName )
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
				LAStringMatrix marketDataBlock = marketDataFileObj[ *it ];
				infoBlocks.push_back ( getTableInfoFromStringMatrix( marketDataBlock ) );
			}
			catch( ... )
			{
				// We attempted to convert a non-marketData related key to enum. Continue to the next key.
				continue;
			}
		}

		validation_api::tryMeLWOCurveMarketDataCreate( objectName, marketDataKeys, infoBlocks );
	}

	/* @brief			Builds Generator curve by invoking the tryMeLWOCurveCalibration() API.
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/	
	void createLWOCurveFromFileName( const LAString& curveCalibrationFileName )
	{
		etrading::ReadDataFile::Load curveCalibrationFileObj = etrading::ReadDataFile::Load( curveCalibrationFileName );
		
		std::string lwoCurveGeneratorName	= curveCalibrationFileObj[ "lwoCurveGeneratorName" ];
		std::string lwoCurveMarketDataName	= curveCalibrationFileObj[ "lwoCurveMarketDataName" ];
		std::string domesticCurveCollection	= curveCalibrationFileObj[ "domesticCurveCollection" ];
		std::string foreignCurveCollection	= curveCalibrationFileObj[ "foreignCurveCollection" ];
		
		std::string objectName = lwoCurveGeneratorName;

		validation_api::tryMeLWOCurveCalibrate(	objectName, lwoCurveGeneratorName, lwoCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );
	}	

	/* @brief			Builds and Generator curve using the specified marketData and calibration filename
	*  @param [in]		marketDataFileName			The filename specifying generator curve data
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/
	void setUpGeneratorCurve( const LAString& marketDataFileName, const LAString& curveCalibrationFileName )
	{
		createLWOMarketDataObjectFromFileName( marketDataFileName );
		createLWOCurveFromFileName( curveCalibrationFileName );
	}

	std::string createLWOCreditBasketModelFromFileName( const LAString& creditBasketModelFileName )
	{
		etrading::ReadDataFile::Load creditBasketModelFileObj = etrading::ReadDataFile::Load( creditBasketModelFileName );
		const std::string creditBasketModelName	= creditBasketModelFileObj[ "objectName" ];
		const LAStringMatrix modelProperties		= creditBasketModelFileObj[ "MODEL_PROPERTIES" ];
		const LAStringMatrix creditModels			= creditBasketModelFileObj[ "CREDIT_MODELS" ];

		std::vector<std::string> propertyNames;
		propertyNames.push_back( "MODEL_PROPERTIES" );
		propertyNames.push_back( "CREDIT_MODELS" );

		std::vector<validation_api::TableInfo> infoBlocks;
		infoBlocks.push_back( getTableInfoFromStringMatrix( modelProperties ));
		infoBlocks.push_back( getTableInfoFromStringMatrix( creditModels ));

		std::string objectName = validation_api::tryMeLWOCreditBasketModelCreate( creditBasketModelName, propertyNames, infoBlocks );
		return objectName;
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
        const std::string result = validation_api::tryMeLWOCreditModelCreate( creditModelName, modelDataTypes, modelData );
		return result;
	}

}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestCreditBasketModel);


	// This test iterates over a vector of stopping dates. For each date it calculates a survival probability,
	// then from that survival probability it backs out the implied stopping date. The test checks that
	// the round trip works perfectly i.e. that the same dates are reproduced.
    TEST_F( TestCreditBasketModel, UNIT_Homogeneous_SurvivalProbabilityFirstToDefault )
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

		std::string creditBasketModelName = createLWOCreditBasketModelFromFileName( CREDIT_BASKET_MODEL );


		LADate fromDate(stoppingDates[0].c_str());
		const size_t numDates = sizeof( stoppingDates ) / sizeof(stoppingDates[0]);

		for (size_t i=1; i< numDates; i++)
		{
			LADate toDate(stoppingDates[i].c_str());

			const double singleSurvivalProbability = validation_api::tryMeLWOCreditModelSurvivalProbability( creditModelName, toDate, fromDate );

			const double basketSurvivalProbability = validation_api::tryMeLWOCreditBasketModelSurvivalProbability( creditBasketModelName, toDate, fromDate );
			
			ASSERT_NEAR( singleSurvivalProbability, basketSurvivalProbability, tolerance );
		}
	}

}

