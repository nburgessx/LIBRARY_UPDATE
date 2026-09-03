
/*
 * @brief: This class represents the market data object used to build a SABR Market Data
 * @Created:		19 Aug 2020
 * @Author:			Yongyan Zheng
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "SabrMarketData.h"
#include "ObjectUtilities.h"
#include "DataUtilities.h"

namespace etrading
{

	/* @brief Main Constructor
	 * @param[in] objectName      The name of this SabrMarketData instance
	 * @param[in] marketDataKeys  A vector containing the names of each marketData block
	 * @param[in] infoBlocks      A vector containing the marketData blocks
	 */
	SabrMarketData::SabrMarketData( const std::string& objectName,  const std::vector<std::string>& marketDataKeys, const std::vector<TableInfo>& infoBlocks ) : IsLWOObject(objectName, SABR_MARKETDATA), freeObject_(objectName)
	{

		// Create a FreeObject from each property label-value block, and concatenate
		const bool allowJaggedData = false;
        for( unsigned int gridCounter = 0; gridCounter < infoBlocks.size(); gridCounter++ )
        {
            const std::vector<std::string>& columnNames = std::get<0>( infoBlocks[ gridCounter ] );
            const std::vector<etrading::ContainedTypeEnum>& colTypes = std::get<1>( infoBlocks[ gridCounter ]) ;
            const VariantMatrix& rangeData = std::get<2>( infoBlocks[ gridCounter ] );

            // Skip Empty Data Blocks
            // ----------------------
            if ( rangeData.size() == 0 )
            {
                continue;
            }
            
            freeObject_ += createFreeObjectFromGrid( objectName, columnNames, colTypes, rangeData, marketDataKeys[ gridCounter ], allowJaggedData );
        }

		// Verify that all SabrMarketData properties are valid and expected
		validateKeys();

		loadMarketData();
	}

	/* @brief Constructor used by deserialization
	 * @param[in] objectName    The name of this SabrData instance
	 * @param[in] freeObject    A freeObject constructed from the serialized data
	 */
	SabrMarketData::SabrMarketData( const std::string& objectName, const FreeObject& freeObject )  : IsLWOObject(objectName, SABR_MARKETDATA), freeObject_(freeObject)
	{
		loadMarketData();
    }

	/* @brief Copy Constructor
	 */
	SabrMarketData::SabrMarketData(const SabrMarketData& rhs) 
		:	IsLWOObject(rhs.getRefToName(), SABR_MARKETDATA),
			freeObject_(rhs.freeObject_),
			marketDataType_(rhs.marketDataType_),
			strikeAdjustAmount_(rhs.strikeAdjustAmount_),
			valueType_(rhs.valueType_),
			expiryStrVector_(rhs.expiryStrVector_),
			tenorStrVector_(rhs.tenorStrVector_),
			instrumentQuoteMatrix_(rhs.instrumentQuoteMatrix_) 
	{}

    std::shared_ptr<SabrMarketData> SabrMarketData::clone() const
    {
       auto data = std::make_shared<SabrMarketData>(*this);
       return data;
    }

	void SabrMarketData::loadMarketData()
	{
		// Read Credit Model properties
		LabelValueBlock modelProperties = toLabelValueBlock(toString(SABR_MKTDATA_PROPERTIES));

		marketDataType_ = toSabrMarketDataTypeEnum(modelProperties.getCompulsoryValue(SABR_MARKETDATA_PROPERTIES_KEY::MARKET_DATA_TYPE));
		strikeAdjustAmount_ = modelProperties.getOptionalValueAsDouble(SABR_MARKETDATA_PROPERTIES_KEY::STRIKE_ADJUST_AMOUNT, 0.0);
		auto valueTypeStr = modelProperties.getOptionalValue(SABR_MARKETDATA_PROPERTIES_KEY::VALUE_TYPE,"");
		valueType_ = toSabrMarketDataValueTypeEnum(modelProperties.getOptionalValue(SABR_MARKETDATA_PROPERTIES_KEY::VALUE_TYPE, toString(ABSOLUTE_SABR_VALUETYPE)));

		auto mat = getLAStringMatrixFromFreeObject(freeObject_, toString(SABR_MKTDATA));

		DoubleVector tenorVec, expiryVec;
		LAStringVector tenorStr, expiryStr;
		DateVector expiDateVec;
		for (size_t i = 1; i < mat.size(); i++)
		{
			expiryStr.push_back(mat[i][0]);
		}

		for (size_t i = 1; i < mat[0].size(); i++)
		{
			tenorStr.push_back(mat[0][i]);
		}

		mat.erase(mat.begin());
		for (size_t i = 0; i < mat.size(); i++)
		{
			mat[i].erase(mat[i].begin());
		}

		DoubleMatrix mat_per(mat.size(), DoubleVector(mat[0].size()));
		for (size_t i = 0; i < mat.size(); i++)
		{
			for (size_t j = 0; j < mat[0].size(); j++)
			{
				mat_per[i][j] = mat[i][j].getDoubleValue();
			}
		}

		expiryStrVector_ = expiryStr;
		tenorStrVector_ = tenorStr;
		instrumentQuoteMatrix_ = mat_per;
	}

	/* @brief Used to serialize an instance of this class
	 * @param[out] the populated SchemaObject
	 */
	const SchemaObject SabrMarketData::toSchemaObject() const
	{
		SchemaObject schemaObject(SABR_MARKETDATA, getRefToName());
        
        toSchemaObject(schemaObject);

        return schemaObject;
	}

     //This is a helper method which is not in the abstract class:
    void const SabrMarketData::toSchemaObject(SchemaObject& schemaObject) const
    {
		const std::vector<std::string> keyNames = freeObject_.keyNames();
		const std::vector<std::vector<Variant> >& allDataView = freeObject_.viewAllData();
		const int nSchemas = freeObject_.numberOfSchemas();
		for (int i=0; i<nSchemas; i++)
		{
			schemaObject.addDataSchema(freeObject_.viewSchema(i));

			const std::string& propertyName = keyNames[i];
			const auto columnIndices = freeObject_.columnsOfSchema( propertyName );
			VariantMatrix variantMatrix;
			std::for_each( columnIndices.cbegin(), columnIndices.cend(), [&variantMatrix, &allDataView]( const int idx )
            {
				variantMatrix.push_back( allDataView[ idx ] );
            } );

			schemaObject.setDataForSchema(propertyName, variantMatrix);
		}
	}

	/* @brief Returns the configuration information for the specified propertyKey. If propertyKey is blank, all properties are returned.
	* @param [in]   propertyKey   The property to be displayed
	* @param [out]  A VariantMatrix containing a LabelValue block of properties
	*/
	VariantMatrix SabrMarketData::viewInputParameters(const std::string& propertyKey) const
	{
		return viewInputParametersFromFreeObject(freeObject_, propertyKey);
	}


	/*
	*  @brief  validates the property keys of this SabrModel, to verify that all are recognized key names.
	*          Will throw if one of the keys is not expected.
	*/
	void SabrMarketData::validateKeys() const
	{
		const bool validateKeys = true;
		validateKeysForLVB(marketdata_properties_lvbKeys(), toLabelValueBlock(toString(SABR_MKTDATA_PROPERTIES)).getKeys(), validateKeys);
	}

	/* @brief Returns a LabelValueBlock containing the marketData information for the specified marketDataKey
	 * @param [in]   marketDataKey   The property to be displayed
	 * @param [in]   trimBlankRows   Whether to remove blank rows from the end of the LAStringMatrix
	 * @param [out]  A LAStringMatrix containing the LabelValue block
	 */
	LabelValueBlock SabrMarketData::toLabelValueBlock( const std::string& marketDataKey, const bool trimBlankRows ) const
    {
        // Reuse the above toLAStringMtrix() method
        const LAStringMatrix laStringMatrix = getLAStringMatrixFromFreeObject(freeObject_, marketDataKey, trimBlankRows); 
        const LabelValueBlock LVB( laStringMatrix );
		return LVB;
    }

}
