#include "BuildMarketDataObjectFromFile.h"
#include "tryAqObjectsCurveMarketData.h"
#include "ReadDataFile.h"
#include "ContainerUtilities.h"
#include "Variant.h"
#include <boost/range/irange.hpp>

using etrading::ReadDataFile;

namespace
{
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
			for (size_t j = 0; j < numCols; j++)
			{
				etrading::VariantVector variantVector;
				for (size_t i = 0; i < numRows; i++)
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
}


namespace google_test
{

	/* @brief			Builds AQO MarketData Object by invoking the tryAqObjectsCurveMarketDataCreate() API.
	*                   The code loops over all of the capitalized data keys in the specified filename and uses
	*                   these blocks to construct the MarketData object.
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*  @return			Handle to the market data object
	*/
	std::string createAQOMarketDataObjectFromFileName(const AQLString& marketDataFileName)
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

}