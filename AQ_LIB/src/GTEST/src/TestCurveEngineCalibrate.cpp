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


// "Generator" API
#include "tryAqObjectsCurveMarketData.h"
#include "tryAqObjectsCurveEngineCalibrate.h"
#include "tryAqObjectsCurveGenerator.h"

// Forward Rates Table

using etrading::ReadDataFile;

// Define the Test Input Folder Here
#define TEST_DIR			  "ETrading/Curves/TestCurveEngineCalibrate/"


namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 1e-8;

	// Test count
	const size_t EUR_TEST_COUNT = 1;
	const size_t USD_TEST_COUNT = 3;

    //
    // curve input files
    //
	
    // NEW EUR Tests
    // -------------
	extern const char FORWARD_RATES_INPUTS[]		= "ForwardRates_Inputs";
	extern const char FORWARD_RATES_OUTPUTS_32bit[] = "ForwardRates_Outputs_32bit";
	extern const char FORWARD_RATES_OUTPUTS_64bit[] = "ForwardRates_Outputs_64bit";

	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, etrading::VariantMatrix>  TableInfo;

	/* @brief			A helper function which converts a AQLStringMatrix into a VariantMatrix
	*                   If the input AQLStringMatrix is empty, creates a dummy VariantMatrix containing two blank columns
	*  @param [in]		stringMatrix		The input AQLStringMatrix
	*  @returns			The corresponding VariantMatrix
	*/
	etrading::VariantMatrix convertStringMatrixToVariantMatrix(AQLStringMatrix stringMatrix)
	{
		etrading::VariantMatrix variantMatrix;

		const size_t numRows = stringMatrix.size();
		if (numRows > 0)
		{
			const size_t numCols = stringMatrix[0].size();

			// Transpose the matrix at the same time as converting to Variant
			for (size_t j = 0; j<numCols; j++)
			{
				etrading::VariantVector variantVector;
				for (size_t i = 0; i<numRows; i++)
				{
					variantVector.push_back(stringMatrix[i][j]);
				}
				variantMatrix.push_back(variantVector);
			}
		}
		else
		{
			// The input AQLStringMatrix is empty.
			// Create a default variantMatrix with 2 columns of dummy data.
			// This simulates an empty block in Excel.
			etrading::VariantVector dummyVector(1, "");
			variantMatrix.push_back(dummyVector);
			variantMatrix.push_back(dummyVector);
		}

		return variantMatrix;
	}

	/* @brief			Builds a "TableInfo" tuple from a AQLStringMatrix of marketdata
	*                   This tuple consists of columnNames, columnTypes and the actual data values.
	*  @param [in]		marketDataBlock		A AQLStringMatrix containing key/value market data values
	*/
	TableInfo getTableInfoFromMarketDataBlock(const AQLStringMatrix& marketDataBlock)
	{
		etrading::VariantMatrix dataValues = convertStringMatrixToVariantMatrix(marketDataBlock);
		size_t numColumns = dataValues.size();

		// EnumTypes for each column
		std::vector<etrading::ContainedTypeEnum> columnEnumTypes = etrading::Variant::getContainedTypeInfo(dataValues);

		// Construct dummy column headings
		std::vector<int> nColCounters;
		boost::push_back(nColCounters, boost::irange(1, static_cast<int>(numColumns) + 1));
		std::vector<std::string> columnNames(numColumns, std::string("COL_"));
		columnNames = etrading::zip_paste<std::vector<std::string>, std::vector<int>, std::string>(columnNames, nColCounters);

		// Construct the TableInfo
		return std::make_tuple(columnNames, columnEnumTypes, dataValues);
	}

	/* @brief			Builds LWO MarketData Object by invoking the tryAqObjectsCurveMarketDataCreate() API.
	*                   The code loops over all of the capitalized data keys in the specified filename and uses
	*                   these blocks to construct the MarketData object.
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*  @return			Handle to the market data object
	*/
	std::string createLWOMarketDataObjectFromFileName(const AQLString& marketDataFileName)
	{
		etrading::ReadDataFile::Load marketDataFileObj = etrading::ReadDataFile::Load(marketDataFileName);

		std::string objectName = marketDataFileObj["objectName"];

		// Process all the market data keys and corresponding blocks of data
		std::vector<std::string> marketDataKeys;
		std::vector<TableInfo> infoBlocks;

		auto keys = marketDataFileObj.getKeys();
		for (auto it = keys.begin(); it != keys.end(); ++it)
		{
			const std::string key = it->getCString();
			try
			{
				// If the key is a market-data key we are interested in, we will be able to cast to enum
				// Conversely, if the key is not associated with market data (such as "objectName") then the cast to enum will fail
				etrading::CurveMarketDataEnum marketDataEnum = etrading::toCurveMarketDataEnum(key);

				// We obtained the enum, so this is a marketData key we are interested in
				marketDataKeys.push_back(key);
				AQLStringMatrix marketDataBlock = marketDataFileObj[*it];
				infoBlocks.push_back(getTableInfoFromMarketDataBlock(marketDataBlock));
			}
			catch (...)
			{
				// We attempted to convert a non-marketData related key to enum. Continue to the next key.
				continue;
			}
		}

		return validation::tryAqObjectsCurveMarketDataCreate(objectName, marketDataKeys, infoBlocks);
	}

	

	std::string buildMarketDataObjectHandle(unsigned int testIndex, const AQLString& ccy, const AQLString& marketDataObj)
	{
		// Market data object
		AQLString marketDataDir = TEST_DIR;
		AQLString prefix = ccy + AQLString("_") + AQLString(static_cast<int>(testIndex)) + AQLString("_");
		marketDataDir += prefix + marketDataObj + AQLString("_MARKETDATA");
		std::string curveMarketObjectHandle = createLWOMarketDataObjectFromFileName(marketDataDir);
		return curveMarketObjectHandle;
	}
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestCurveEngineCalibrate);
	


	// TEST_F(TestCurveEngineCalibrate, SNAPSHOT_USD_FORWARD_RATE_TESTS)
	// {
	// 	unsigned int testCount = USD_TEST_COUNT;
	// 	std::string curveCollection = "USDYC2";
	// 	AQLString ccy = "USD";
    // 
	// 	std::vector<std::string> curveGenerators;
	// 	std::vector<std::string> marketDataHandles;
    // 
	// 	for (unsigned int testIndex = 1; testIndex < testCount + 1; ++testIndex)
	// 	{
	// 		//----------------------------------------------------------------------------------------
	// 		// Test 1: Spot basis swaps. Basis curve's smallest instrument tenor = 1Y
	// 		// Test 2: Forward basis swaps for the 6M curve. Basis curve's smallest instrument tenor = 1Y
	// 		// Test 3: Spot basis swaps. Basis curve's smallest instrument tenor = Libor tenor
    // 
	// 		// OIS
	// 		curveGenerators.push_back("USD_OIS");
	// 		marketDataHandles.push_back(buildMarketDataObjectHandle(testIndex, ccy, "OIS"));
    // 
	// 		// SWAP 3M
	// 		curveGenerators.push_back("USD_SWAP_3M");
	// 		marketDataHandles.push_back(buildMarketDataObjectHandle(testIndex, ccy, "3M"));
    // 
	// 		// TENOR BASIS 1M		
	// 		curveGenerators.push_back("USD_BASIS_1X3");
	// 		marketDataHandles.push_back(buildMarketDataObjectHandle(testIndex, ccy, "1M"));
    // 
	// 		// TENOR BASIS 6M
	// 		if (testIndex == 2)
	// 		{
    //             StandardString key      = "CURVEPROPERTIES:isFwdBasis";
    //             StandardString value    = "TRUE";
	// 			etrading::LabelValueBlock overridingInputs( key, value );
    //             
	// 			std::string fwd6MCurveGenerator = validation::tryAqObjectsCurveGeneratorModify("FWD_USD_BASIS_3X6", "USD_BASIS_3X6", overridingInputs);
	// 			curveGenerators.push_back(fwd6MCurveGenerator);
	// 		}
	// 		else
	// 		{
	// 			curveGenerators.push_back("USD_BASIS_3X6");
	// 		}
	// 		marketDataHandles.push_back(buildMarketDataObjectHandle(testIndex, ccy, "6M"));
    // 
	// 		validation::tryAqObjectsCurveEngineCalibrate("",				// engine name
	// 			                                         curveCollection,	// curve collection name
	// 			                                         AQLStringMatrix(),	// Engine params
	// 			                                         curveGenerators,
	// 			                                         marketDataHandles);
    // 
	// 		// Test forward rates for the OIS curve and the STD curve
	// 		testEngineCurveForwardRates("OIS", ccy, testIndex);
	// 		testEngineCurveForwardRates("3M", ccy, testIndex);
	// 		testEngineCurveForwardRates("1M", ccy, testIndex);
	// 		testEngineCurveForwardRates("6M", ccy, testIndex);
    // 
	// 		// Flush the cache in preparation for a new set of curves
	// 		validation::tryAqToolsClearEntityPool();
	// 		curveGenerators.clear();
	// 		marketDataHandles.clear();
	// 	}
	// }

	//TEST_F(TestCurveEngineCalibrate, SNAPSHOT_EUR_FORWARD_RATE_TESTS)
	//{
	//	unsigned int testCount = EUR_TEST_COUNT;
	//	std::string curveCollection = "EURYC";
	//	AQLString ccy = "EUR";

	//	std::vector<std::string> curveGenerators;
	//	std::vector<std::string> marketDataHandles;

	//	for (unsigned int testIndex = 1; testIndex < testCount + 1; ++testIndex)
	//	{
	//		// OIS
	//		curveGenerators.push_back("EUR_OIS");
	//		marketDataHandles.push_back(buildMarketDataObjectHandle(testIndex, ccy, "OIS"));

	//		// SWAP 3M
	//		curveGenerators.push_back("EUR_SWAP_3M");
	//		marketDataHandles.push_back(buildMarketDataObjectHandle(testIndex, ccy, "3M"));

	//		// TENOR BASIS 1M		
	//		curveGenerators.push_back("EUR_BASIS_1X3");
	//		marketDataHandles.push_back(buildMarketDataObjectHandle(testIndex, ccy, "1M"));

	//		// TENOR BASIS 6M
	//		curveGenerators.push_back("EUR_BASIS_3X6");
	//		marketDataHandles.push_back(buildMarketDataObjectHandle(testIndex, ccy, "6M"));

	//		// TENOR BASIS 12M
	//		curveGenerators.push_back("EUR_BASIS_3X12");
	//		marketDataHandles.push_back(buildMarketDataObjectHandle(testIndex, ccy, "12M"));

	//		validation::tryAqObjectsCurveEngineCalibrate("",				// engine name
	//			curveCollection,	// curve collection name
	//			AQLStringMatrix(),		// Engine params
	//			curveGenerators,
	//			marketDataHandles);

	//		// Test forward rates for the OIS curve and the STD curve
	//		testEngineCurveForwardRates("OIS", ccy, testIndex);
	//		testEngineCurveForwardRates("3M", ccy, testIndex);
	//		testEngineCurveForwardRates("1M", ccy, testIndex);
	//		testEngineCurveForwardRates("6M", ccy, testIndex);
	//		testEngineCurveForwardRates("12M", ccy, testIndex);

	//		// Flush the cache in preparation for a new set of curves
	//		validation::tryAqToolsClearEntityPool();
	//		curveGenerators.clear();
	//		marketDataHandles.clear();
	//	}
	//}
}
