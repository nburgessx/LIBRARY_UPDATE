#pragma once

/*
 * @brief: This class represents the market data object used to build a SABR Market Data
 * @Created:		19 Aug 2020
 * @Author:			Yongyan Zheng
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#include "IsAQObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "Variant.h"
#include "LabelValueBlock.h"
#include "CommonConstants.h"

namespace etrading
{
	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, VariantMatrix>  TableInfo;

    class SabrMarketData : public IsAQObject
    {
	public:

		/* @brief Main Constructor
		 * @param[in] objectName      The name of this SabrData instance
		 * @param[in] marketDataKeys  A vector containing the names of each marketData block
		 * @param[in] infoBlocks      A vector containing the marketData blocks
		 */
		SabrMarketData( const std::string& objectName, const std::vector<std::string>& marketDataKeys, const std::vector<TableInfo>& infoBlocks );

		/* @brief Constructor used by deserialization
		 * @param[in] objectName    The name of this SabrData instance
		 * @param[in] freeObject    A freeObject constructed from the serialized data
		 */
		SabrMarketData( const std::string& objectName, const FreeObject& freeObject );

		//Getter
		SabrMarketDataTypeEnum getMarketDataType() const { return marketDataType_; };
		double getStrikeAdjustAmount() const { return strikeAdjustAmount_; };
		double isATM() const { return AQ_IS_EQUAL_ZERO(strikeAdjustAmount_); };
		SabrMarketDataValueTypeEnum getValyeType() const { return valueType_; };
		AQLStringVector getExpiryStrVector() const { return expiryStrVector_; };
		AQLStringVector getTenorStrVector() const { return tenorStrVector_; };
		DoubleMatrix getInstrumentQuoteMatrix() const { return instrumentQuoteMatrix_; };

		/* @brief Copy Constructor
		 */
   		SabrMarketData(const SabrMarketData& rhs);
		virtual ~SabrMarketData() {}

		std::shared_ptr<SabrMarketData> clone() const;

		/* @brief Used to serialize an instance of this class
		 * @param[out] the populated SchemaObject
		 */
		virtual const SchemaObject toSchemaObject() const;

        //This is a helper method to populate SchemaObject:
        virtual void const toSchemaObject(SchemaObject& schemaObject) const;

		/* @brief Returns the configuration information for the specified propertyKey. If propertyKey is blank, all properties are returned.
		 * @param [in]   propertyKey   The property to be displayed
		 * @param [out]  A VariantMatrix containing a LabelValue block of properties
		 */
		VariantMatrix viewInputParameters(const std::string& propertyKey) const;

        /* @brief Returns a LabelValueBlock containing the marketData information for the specified marketDataKey
		 * @param [in]   marketDataKey   The property to be displayed
		 * @param [in]   trimBlankRows   Whether to remove blank rows from the end of the AQLStringMatrix
	     * @param [out]  A AQLStringMatrix containing the LabelValue block
		 */
		LabelValueBlock toLabelValueBlock( const std::string& marketDataKey, const bool trimBlankRows = true ) const;

		/*
		*  @brief  Utility method which validates the property keys of this SabrMarketData, to verify that all are recognized key names.
		*          Will throw if one of the keys is not expected.
		*/
		void validateKeys() const;

		// @brief	Called by constructors to load market data
		void loadMarketData();

		static std::vector<std::string> marketdata_properties_lvbKeys()
		{
			std::vector<std::string> expectedKeys =
			{
				SABR_MARKETDATA_PROPERTIES_KEY::MARKET_DATA_TYPE
				, SABR_MARKETDATA_PROPERTIES_KEY::STRIKE_ADJUST_AMOUNT
				, SABR_MARKETDATA_PROPERTIES_KEY::VALUE_TYPE
			};

			return expectedKeys;
		}


	private:

		FreeObject freeObject_;

		SabrMarketDataTypeEnum marketDataType_;
		double strikeAdjustAmount_;
		SabrMarketDataValueTypeEnum valueType_;		

		AQLStringVector expiryStrVector_;
		AQLStringVector tenorStrVector_;
		DoubleMatrix instrumentQuoteMatrix_;
		
	};

	typedef std::shared_ptr< SabrMarketData > SabrMarketDataPtr;
}
