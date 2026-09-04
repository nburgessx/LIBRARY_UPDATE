#pragma once

#include <string>
#include <vector>

#include "IsAQObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "CoreEnumerations.h"
#include "CommonConstants.h"
#include "LabelValueBlock.h"

namespace etrading
{
	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, VariantMatrix>  TableInfo;

	class BondGenerator : public IsAQObject 
	{
	public:
		/* @brief Main Constructor
		 * @param[in] objectName    The name of this BondGenerator instance
		 * @param[in] propertyKeys  A vector containing the names of each configuration block
		 * @param[in] infoBlocks    A vector containing the configuation blocks
		 */
		BondGenerator( const std::string& objectName,
                       const std::vector<std::string>& propertyKeys,
                       const std::vector<TableInfo>& infoBlocks );

		/* @brief Constructor used by deserialization
		 * @param[in] objectName    The name of this BondGenerator instance
		 * @param[in] freeObject    A freeObject constructed from the serialized data
		 */
		BondGenerator( const std::string& objectName,
                        const FreeObject& freeObject );

		/* @brief Copy Constructor
		 */
   		BondGenerator( const BondGenerator& rhs );

		virtual ~BondGenerator() {}

		// Schema Helpers
        //std::map<std::string, Variant> getBondGeneratorDescriptionMap() const;

        const SchemaObject toSchemaObject() const;

		//This is a helper method to populate SchemaObject:
        virtual void const toSchemaObject( SchemaObject& schemaObject ) const;

		/* @brief Returns the configuration information for the specified propertyKey. If propertyKey is blank, all properties are returned.
		 * @param [in]   propertyKey   The property to be displayed
	     * @param [out]  A VariantMatrix containing a LabelValue block of properties
		 */
		const VariantMatrix viewInputParameters( const std::string& propertyKey ) const;
		
		/* @brief Returns a LabelValue block containing the configuration information for the specified propertyKey
		 * @param [in]   propertyKey   The property to be displayed
	     * @param [out]  A LabelValue block containing the properties
		 */
		LabelValueBlock toLabelValueBlock( const std::string& propertyKey ) const;

		// These are the keys which are permitted in the BOND_STATICDATA LVB
		static std::vector<std::string> bond_staticdata_lvbKeys()
		{
			const std::string arr[] =
			{
                  BONDGENERATOR_STATICDATA_KEY::BOND_TYPE
				, BONDGENERATOR_STATICDATA_KEY::CURRENCY
				, BONDGENERATOR_STATICDATA_KEY::YIELD_TYPE
                , BONDGENERATOR_STATICDATA_KEY::IS_CLEAN_PRICE
				, BONDGENERATOR_STATICDATA_KEY::BOND_QUOTE_CONVENTION

			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		// These are the keys which are permitted in the BOND_SCHEDULE LVB
		static std::vector<std::string> bond_schedule_lvbKeys()
		{
			const std::string arr[] =
			{
                  BONDGENERATOR_SCHEDULE_KEY::PAY_RECEIVE
				, BONDGENERATOR_SCHEDULE_KEY::NOTIONAL_EXCHANGE
				, BONDGENERATOR_SCHEDULE_KEY::FACE_VALUE
                , BONDGENERATOR_SCHEDULE_KEY::FREQUENCY
				, BONDGENERATOR_SCHEDULE_KEY::ACCRUAL_BUSINESSDAY_ADJUSTMENT
				, BONDGENERATOR_SCHEDULE_KEY::ACCRUAL_CALENDAR
				, BONDGENERATOR_SCHEDULE_KEY::PAYMENT_BUSINESSDAY_ADJUSTMENT
				, BONDGENERATOR_SCHEDULE_KEY::PAYMENT_CALENDAR
                , BONDGENERATOR_SCHEDULE_KEY::PAYMENT_LAG
				, BONDGENERATOR_SCHEDULE_KEY::DAYCOUNT
				, BONDGENERATOR_SCHEDULE_KEY::CALCULATION_TYPE
                , BONDGENERATOR_SCHEDULE_KEY::YIELD_FREQUENCY
                , BONDGENERATOR_SCHEDULE_KEY::TAX_RATE
				, BOND_KEY::EX_DIVIDEND_TENOR
				, BOND_KEY::EX_DIVIDEND_BUSINESSDAYADJUSTMENT

			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		// The BondExpression LVB contains keys which are specific to the concrete bond.
		// These keys are used to further customize the bond, further to the settings provided by the BondGenerator.
		static std::vector<std::string> bond_expression_lvbKeys()
		{
			const std::string arr[] =
			{
                  BONDGENERATOR_EXPRESSION_KEY::BOND_DESCRIPTION
				, BONDGENERATOR_EXPRESSION_KEY::ISIN
				, BONDGENERATOR_EXPRESSION_KEY::ISSUE_DATE
				, BONDGENERATOR_EXPRESSION_KEY::FIRST_COUPON_DATE
				, BONDGENERATOR_EXPRESSION_KEY::LAST_COUPON_DATE
				, BONDGENERATOR_EXPRESSION_KEY::MATURITY_DATE
				, BONDGENERATOR_EXPRESSION_KEY::COUPON
				, BONDGENERATOR_EXPRESSION_KEY::ROLLDAY
				, BONDGENERATOR_EXPRESSION_KEY::ACCRUAL_START_DATE
                , BONDGENERATOR_EXPRESSION_KEY::ISSUE_PRICE
                , BONDGENERATOR_EXPRESSION_KEY::FACE_VALUE          // Duplicate Key - Intentional to allow generator override - expression LVB takes priority over bond generator
                , BONDGENERATOR_EXPRESSION_KEY::FREQUENCY           // Duplicate Key - Intentional to allow generator override - expression LVB takes priority over bond generator
                , BONDGENERATOR_EXPRESSION_KEY::DAYCOUNT            // Duplicate Key - Intentional to allow generator override - expression LVB takes priority over bond generator
                , BONDGENERATOR_EXPRESSION_KEY::YIELD_FREQUENCY     // Duplicate Key - Intentional to allow generator override - expression LVB takes priority over bond generator
                , BONDGENERATOR_EXPRESSION_KEY::TAX_RATE            // Duplicate Key - Intentional to allow generator override - expression LVB takes priority over bond generator
				, BONDGENERATOR_EXPRESSION_KEY::QUOTED_MARGIN
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

	private:

		/*
		*  @brief  Utility method which validates the property keys of this bondGenerator, to verify that all are recognized key names.
		*          Will throw if one of the keys is not expected.
		*/
		void validateKeys() const ;

		FreeObject freeObject_;

	};

	typedef std::shared_ptr< BondGenerator > BondGeneratorPtr;

}

