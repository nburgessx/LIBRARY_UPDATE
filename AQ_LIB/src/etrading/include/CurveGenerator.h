
#pragma once

/*
 * @File: CurveGenerator.h
 * @brief: This class holds the configuration used to build a curve
 * @Created: 6 Dec 2016
 * @Author: Ian Castleton
 * @Department: ISO Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include <unordered_map>

#include "IsAQObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "Variant.h"
#include "LabelValueBlock.h"
#include "CommonConstants.h"


namespace etrading
{
	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, VariantMatrix>  TableInfo;

    class CurveGenerator : public IsAQObject
    {
	public:
		/* @brief Main Constructor
		 * @param[in] objectName    The name of this CurveGenerator instance
		 * @param[in] propertyKeys  A vector containing the names of each configuration block
		 * @param[in] infoBlocks    A vector containing the configuation blocks
		 */
		CurveGenerator( const std::string& objectName,
                        const std::vector<std::string>& propertyKeys,
                        const std::vector<TableInfo>& infoBlocks );

		/* @brief Constructor used by deserialization
		 * @param[in] objectName    The name of this CurveGenerator instance
		 * @param[in] freeObject    A freeObject constructed from the serialized data
		 */
		CurveGenerator( const std::string& objectName,
                        const FreeObject& freeObject );

		/* @brief Copy Constructor
		 */
   		CurveGenerator(const CurveGenerator& rhs);

		/* @brief Builds a new CurveGenerator from an existing base CurveGenerator, with additional modified properties
		*         This method may be viewed as a specialized type of copy constructor.
		*  @param[in] newObjectName			The AQO object cache handle name to use for the new object
		*  @param[in] baseCurveGenerator	The CurveGenerator object on which to base the properties of this new object
		*  @param[in] modifiedValuesLVB		A LabelValueBlock containing key/value pairs to update. Each key is specified in the format: PROPERTYNAME:KEYNAME.
		*/
		CurveGenerator( const std::string& newObjectName, const CurveGenerator& baseCurveGenerator, const LabelValueBlock& modifiedValuesLVB );

		virtual ~CurveGenerator() {}

        std::shared_ptr<CurveGenerator> clone() const;

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
		const VariantMatrix viewInputParameters( const std::string& propertyKey ) const;

		/* @brief Returns a VariantMatrix containing the configuration information for the specified propertyKey
		 * @param [in]   propertyKey   The property to be displayed
	     * @param [out]  A VariantMatrix containing the LabelValue block
		 */
		VariantMatrix toVariantMatrix( const std::string& propertyKey ) const;

		/* @brief Returns a AQLStringMatrix containing the configuration information for the specified propertyKey
		 * @param [in]   propertyKey   The property to be displayed
		 * @param [in]   trimBlankRows Whether to remove blank rows from the end of the AQLStringMatrix
	     * @param [out]  A AQLStringMatrix containing the LabelValue block
		 */
		AQLStringMatrix toAQLStringMatrix( const std::string& propertyKey, const bool trimBlankRows = true ) const;

        /* @brief Returns a StandardStringMatrix containing the configuration information for the specified propertyKey
		 * @param [in]   propertyKey   The property to be displayed
		 * @param [in]   trimBlankRows Whether to remove blank rows from the end of the AQLStringMatrix
	     * @param [out]  A StandardStringMatrix containing the LabelValue block
		 */
		StandardStringMatrix toStandardStringMatrix( const std::string& propertyKey, const bool trimBlankRows = true ) const;

        /* @brief Returns a LabelValueBlock containing the configuration information for the specified propertyKey
		 * @param [in]   propertyKey   The property to be displayed
		 * @param [in]   trimBlankRows Whether to remove blank rows from the end of the AQLStringMatrix
	     * @param [out]  A LabelValueBlock containing the LabelValue block
		 */
		LabelValueBlock toLabelValueBlock( const std::string& propertyKey, const bool trimBlankRows = true ) const;
		        
        /* @brief Returns a boolean to confirm if an optional property key has been specified
		 * @param [in]   propertyKey   The property to be displayed
	     * @param [out]  A bool to confirm if the optional property key is present
		 */
		const bool doesKeyExist( const std::string& propertyKey ) const;

		static std::vector<std::string> curve_properties_lvbKeys()
		{
			const std::string arr[] =
			{
                CURVEGENERATOR_CURVEPROPERTIES_KEY::CURVE_TYPE,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::FREQUENCY,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::CURVE_INDEX_FREQUENCY,
                CURVEGENERATOR_CURVEPROPERTIES_KEY::STATIC_DATA_TABLE,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::INDEX_NAME,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::CURRENCY,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::INTERPOLATION,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::YIELDGEN_INTERPOLATION,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::BASIS_INTERPOLATION,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::DAYCOUNT,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::IS_FRA_USE,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::IS_FUTURE_USE,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::IS_FWD_BASIS,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::IS_FWD_FX,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::IS_RENOTIONAL_ADJUST,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::IS_XCCY_MARKED_TO_MARKET,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::DF_CURVE_NAME,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::GENERATE_FORWARDS_FROM_SWAPS_ONLY,
                CURVEGENERATOR_CURVEPROPERTIES_KEY::FIXING_SOURCE,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::FUTURE_INTERPOLATION,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::FRA_INTERPOLATION,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::INTERPOLATION_JOIN_DATE,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::ALWAYS_CALC_JOIN_DATE,
                CURVEGENERATOR_CURVEPROPERTIES_KEY::FAST_REBUILD,
                CURVEGENERATOR_CURVEPROPERTIES_KEY::IS_SWAP_TENOR_ADJUST,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::USE_TENOR_BASIS,
                CURVEGENERATOR_CURVEPROPERTIES_KEY::TENOR_SWAP_NAME,
                CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_BUILD_FREQUENCY,
                CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_SHIFT_TYPE,
                CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_RISK_TYPE,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE,
				CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		static std::vector<std::string> ois_lvbKeys()
		{
			const std::string arr[] =
			{
				CURVEGENERATOR_OIS_KEY::CALENDAR
				, CURVEGENERATOR_OIS_KEY::DAYCOUNT
				, CURVEGENERATOR_OIS_KEY::FREQUENCY
				, CURVEGENERATOR_OIS_KEY::RESET_LAG
				, CURVEGENERATOR_OIS_KEY::SLIDING_RULE
				, CURVEGENERATOR_OIS_KEY::GENERATE_METHOD
				, CURVEGENERATOR_OIS_KEY::FIRST_RATE
				, CURVEGENERATOR_OIS_KEY::SHORT_TERM_CONVENTION
				, CURVEGENERATOR_OIS_KEY::EPSILON
				, CURVEGENERATOR_OIS_KEY::MAX_LOOP
				, CURVEGENERATOR_OIS_KEY::SMOOTH_SHORT_END
				, CURVEGENERATOR_OIS_KEY::COMPOUNDING_METHOD
				, CURVEGENERATOR_OIS_KEY::LONG_TERM_CONVENTION
				, CURVEGENERATOR_OIS_KEY::LONG_TERM
				, CURVEGENERATOR_OIS_KEY::LONG_TERM_GENERATE_METHOD
				, CURVEGENERATOR_OIS_KEY::LONG_TERM_GENERATE_METHOD_ARROIS
                , CURVEGENERATOR_OIS_KEY::MEAN_REVERSION
				, CURVEGENERATOR_OIS_KEY::IS_EOM_ROLL
				, CURVEGENERATOR_OIS_KEY::EOM_DAY
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		static std::vector<std::string> libor_ois_basis_lvbKeys()
		{
			const std::string arr[] =
			{
				CURVEGENERATOR_LIBOROISBASIS_KEY::CALENDAR
				, CURVEGENERATOR_LIBOROISBASIS_KEY::DAYCOUNT
				, CURVEGENERATOR_LIBOROISBASIS_KEY::FREQUENCY
				, CURVEGENERATOR_LIBOROISBASIS_KEY::SLIDING_RULE
				, CURVEGENERATOR_LIBOROISBASIS_KEY::CALENDAR_ARROIS
				, CURVEGENERATOR_LIBOROISBASIS_KEY::DAYCOUNT_ARROIS
				, CURVEGENERATOR_LIBOROISBASIS_KEY::FREQUENCY_ARROIS
				, CURVEGENERATOR_LIBOROISBASIS_KEY::SLIDING_RULE_ARROIS
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		static std::vector<std::string> swaps_lvbKeys()
		{
			const std::string arr[] =
			{
				CURVEGENERATOR_SWAPS_KEY::CALENDAR
				, CURVEGENERATOR_SWAPS_KEY::DAYCOUNT
				, CURVEGENERATOR_SWAPS_KEY::DAYCOUNT_FIX
				, CURVEGENERATOR_SWAPS_KEY::DAYCOUNT_FLOAT
				, CURVEGENERATOR_SWAPS_KEY::FREQUENCY
                , CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FIX
                , CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FIX_1Y
                , CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FIX_2Y
                , CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FIX_3Y
                , CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FIX_4Y
                , CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FIX_5Y
				, CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FLOAT
                , CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FLOAT_1Y
                , CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FLOAT_2Y
                , CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FLOAT_3Y
                , CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FLOAT_4Y
                , CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FLOAT_5Y
				, CURVEGENERATOR_SWAPS_KEY::INTERPOLATION
				, CURVEGENERATOR_SWAPS_KEY::RESET_LAG
				, CURVEGENERATOR_SWAPS_KEY::SLIDING_RULE
				, CURVEGENERATOR_SWAPS_KEY::IS_NEWTON_RAPHSON
				, CURVEGENERATOR_SWAPS_KEY::IS_SIMULTANEOUS_EQ
				, CURVEGENERATOR_SWAPS_KEY::EPSILON
				, CURVEGENERATOR_SWAPS_KEY::MAX_LOOP
				, CURVEGENERATOR_SWAPS_KEY::IS_EOM_ROLL
				, CURVEGENERATOR_SWAPS_KEY::EOM_DAY
                , CURVEGENERATOR_SWAPS_KEY::BASE_FREQUENCY_FLOAT

				, CURVEGENERATOR_SWAPS_KEY::CALENDAR_ARROIS
				, CURVEGENERATOR_SWAPS_KEY::SLIDING_RULE_ARROIS
				, CURVEGENERATOR_SWAPS_KEY::DAYCOUNT_FIX_ARROIS
				, CURVEGENERATOR_SWAPS_KEY::FREQUENCY_FIX_ARROIS
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		static std::vector<std::string> money_market_lvbKeys()
		{
			const std::string arr[] =
			{
				CURVEGENERATOR_MONEYMARKET_KEY::CALENDAR
				, CURVEGENERATOR_MONEYMARKET_KEY::DAYCOUNT
				, CURVEGENERATOR_MONEYMARKET_KEY::SLIDING_RULE
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}
		
		static std::vector<std::string> libor_fixings_lvbKeys()
		{
			const std::string arr[] =
			{
				CURVEGENERATOR_LIBORFIXINGS_KEY::CALENDAR
				, CURVEGENERATOR_LIBORFIXINGS_KEY::DAYCOUNT
				, CURVEGENERATOR_LIBORFIXINGS_KEY::FREQUENCY
				, CURVEGENERATOR_LIBORFIXINGS_KEY::RESET_LAG
				, CURVEGENERATOR_LIBORFIXINGS_KEY::SLIDING_RULE
				, CURVEGENERATOR_LIBORFIXINGS_KEY::IS_EOM_ROLL
				, CURVEGENERATOR_LIBORFIXINGS_KEY::EOM_DAY
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		static std::vector<std::string> fras_lvbKeys()
		{
			const std::string arr[] =
			{
				CURVEGENERATOR_FRAS_KEY::CALENDAR
				, CURVEGENERATOR_FRAS_KEY::DAYCOUNT
				, CURVEGENERATOR_FRAS_KEY::RESET_LAG
				, CURVEGENERATOR_FRAS_KEY::SLIDING_RULE
				, CURVEGENERATOR_FRAS_KEY::APPLY_TENSION
				, CURVEGENERATOR_FRAS_KEY::TENSION_GAP
				, CURVEGENERATOR_FRAS_KEY::SMOOTH_SHORT_END
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		static std::vector<std::string> futures_lvbKeys()
		{
			const std::string arr[] =
			{
				CURVEGENERATOR_FUTURES_KEY::CALENDAR
				, CURVEGENERATOR_FUTURES_KEY::DAYCOUNT
				, CURVEGENERATOR_FUTURES_KEY::RESET_LAG
				, CURVEGENERATOR_FUTURES_KEY::SLIDING_RULE
				, CURVEGENERATOR_FUTURES_KEY::MEAN_REVERSION
				, CURVEGENERATOR_FUTURES_KEY::USE_CONVEX_ADJUSTMENT // LEGACY PARAMETER - MEANING CONVEXITY QUOTED AS PRICE
				, CURVEGENERATOR_FUTURES_KEY::CONVEXITY_QUOTE_TYPE // ALIAS for USE_CONVEX_ADJUSTMENT
				, CURVEGENERATOR_FUTURES_KEY::APPLY_TENSION
				, CURVEGENERATOR_FUTURES_KEY::TENSION_GAP
				, CURVEGENERATOR_FUTURES_KEY::SMOOTH_SHORT_END
				, CURVEGENERATOR_FUTURES_KEY::SERIAL_CALC_TYPE
				, CURVEGENERATOR_FUTURES_KEY::INSTRUMENT_TYPE
				, CURVEGENERATOR_FUTURES_KEY::INTERPOLATION_JOIN_DATE
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		static std::vector<std::string> basisswaps_lvbKeys()
		{
			const std::string arr[] =
			{
				CURVEGENERATOR_BASISSWAPS_KEY::MARKET_NAME
                , CURVEGENERATOR_BASISSWAPS_KEY::ADJUST_VALUE_INTERPOLATION
				, CURVEGENERATOR_BASISSWAPS_KEY::IS_LEG1_SPREAD
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_CASHLET_CALENDAR
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_CASHLET_DAYCOUNT
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_CASHLET_FREQUENCY
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_CASHLET_FREQUENCY_COMPOUND
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_CASHLET_SLIDING_RULE
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_CASHLET_SPOT_LAG
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_FORECAST
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_DISCOUNT
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_INDEX_ACCESSARY
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_INDEX_DAYCOUNT
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_INDEX_FIXING_CALENDAR
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_INDEX_FREQUENCY
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_INDEX_PAYMENT_CALENDAR
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_INDEX_RESET_LAG
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG1_INDEX_SLIDING_RULE
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_CALENDAR
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_DAYCOUNT
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_FREQUENCY
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_FREQUENCY_COMPOUND
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_SLIDING_RULE
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_SPOT_LAG
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_FORECAST
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_DISCOUNT
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_CALENDAR
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_DAYCOUNT
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_FREQUENCY
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_FREQUENCY_COMPOUND
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_SLIDING_RULE
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_SPOT_LAG
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_FORECAST
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_DISCOUNT
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_ACCESSARY
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_DAYCOUNT
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_FIXING_CALENDAR
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_FREQUENCY
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_PAYMENT_CALENDAR
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_RESET_LAG
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_SLIDING_RULE
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_CALENDAR
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_DAYCOUNT
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_FREQUENCY
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_FREQUENCY_COMPOUND
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_SLIDING_RULE
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_SPOT_LAG
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_FORECAST
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_DISCOUNT
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_ACCESSARY
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_DAYCOUNT
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_FIXING_CALENDAR
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_FREQUENCY
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_PAYMENT_CALENDAR
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_RESET_LAG
				, CURVEGENERATOR_BASISSWAPS_KEY::LEG2_INDEX_SLIDING_RULE
				, CURVEGENERATOR_BASISSWAPS_KEY::TARGET
				, CURVEGENERATOR_BASISSWAPS_KEY::IS_SIMULTANEOUS_EQ
				, CURVEGENERATOR_BASISSWAPS_KEY::FWD_INTERPOLATION
				, CURVEGENERATOR_BASISSWAPS_KEY::IS_SAME_GRID_INDEX
				, CURVEGENERATOR_BASISSWAPS_KEY::SPOT_RATE_TERM
				, CURVEGENERATOR_BASISSWAPS_KEY::IS_YIELD_SPREAD_CALC
				, CURVEGENERATOR_BASISSWAPS_KEY::EPSILON
				, CURVEGENERATOR_BASISSWAPS_KEY::MAX_LOOP
				, CURVEGENERATOR_BASISSWAPS_KEY::IS_FWD_INTER
				, CURVEGENERATOR_BASISSWAPS_KEY::IS_EOM_ROLL
				, CURVEGENERATOR_BASISSWAPS_KEY::EOM_DAY
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		static std::vector<std::string> fxfwds_lvbKeys()
		{
			const std::string arr[] =
			{
				CURVEGENERATOR_FXFWDS_KEY::CALENDAR
				, CURVEGENERATOR_FXFWDS_KEY::RESET_LAG
				, CURVEGENERATOR_FXFWDS_KEY::SLIDING_RULE
				, CURVEGENERATOR_FXFWDS_KEY::IS_PRICE_CCY
				, CURVEGENERATOR_FXFWDS_KEY::IS_DOMESTIC_CURRENCY
				, CURVEGENERATOR_FXFWDS_KEY::IS_RATIO
				, CURVEGENERATOR_FXFWDS_KEY::IS_FX_OUTRIGHT
				, CURVEGENERATOR_FXFWDS_KEY::IS_EOM_ROLL
				, CURVEGENERATOR_FXFWDS_KEY::EOM_DAY
				, CURVEGENERATOR_FXFWDS_KEY::DENOMINATOR
				, CURVEGENERATOR_FXFWDS_KEY::PIPSIZE
				, CURVEGENERATOR_FXFWDS_KEY::FREQUENCY
				, CURVEGENERATOR_FXFWDS_KEY::FIXEDRATEXCCYSTARTTENOR
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		static std::vector<std::string> fwdfxconst_lvbKeys()
		{
			const std::string arr[] =
			{
				CURVEGENERATOR_FWDFXCONST_KEY::LEG1_FORECAST
				, CURVEGENERATOR_FWDFXCONST_KEY::LEG1_DISCOUNT
				, CURVEGENERATOR_FWDFXCONST_KEY::LEG2_FORECAST
				, CURVEGENERATOR_FWDFXCONST_KEY::LEG2_DISCOUNT
				, CURVEGENERATOR_FWDFXCONST_KEY::TARGET
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

	private:
		/*
		*  @brief  Utility method which validates the property keys of this curveGenerator, to verify that all are recognized key names.
		*          Will throw if one of the keys is not expected.
		*/
		void validateKeys() const ;
		
		/*
		*  @brief  Utility method which reads in a BaseCurveGenerator and modifiedValuesLVB, and constructs a merged set of properties and searchable index of base property keys as follows:
		*          If a key from modifiedValuesLVB exists in the BaseCurveGenerator, the property index is updated with the new value
		*          If this is a new key, the key-value pair is added directly to the mergedProperties
		*
		*  @param[in]	baseCurveGenerator	The input curveGenerator from which properties are extracted
		*  @param[in]	modifiedValuesLVB	A LabelValueBlock containing keys/values
		*  @param[out]	propertyIndex		A searcheable index of base property keys initialized from the baseCurveGenerator, updated from the modifiedValuesLVB.
		*  @param[out]	mergedProperties	Properties obtained from the baseCurveGenerator, merged with any additional keys/values from the LabelValueBlock that were not found in the baseCurveGenerator
		*                                   These merged properties are in a format convenient for constructing a new CurveGenerator.
		*/
		void mergePropertiesFromLVB( const CurveGenerator& baseCurveGenerator,
									 const LabelValueBlock& modifiedValuesLVB,
									 std::unordered_map< std::string, std::unordered_map<std::string, Variant > >& propertyIndex,
									 std::unordered_map< std::string, VariantMatrix>& mergedProperties ) const;
		
		/*
		*  @brief  Utility method which constructs a searchable index containing property keys from the specified baseCurveGenerator.
		*          This index allows convenient property lookup.
		*          The method also stores the property blocks from the baseCurveGenerator in a baseProperties map.
		*  @param[in]  baseCurveGenerator	The input curveGenerator from which properties are extracted
		*  @param[out] propertyIndex		A searchable index of property values, initialized from the baseCurveGenerator.
		*  @param[out] baseProperties		A map of VariantMatrix property data, initialized from the baseCurveGenerator. Convenient for creating a new CurveGenerator.
		*/
		void constructPropertyIndexFromCurveGenerator( const CurveGenerator& baseCurveGenerator,
													   std::unordered_map< std::string, std::unordered_map<std::string, Variant > >& propertyIndex,
													   std::unordered_map< std::string, VariantMatrix>& baseProperties ) const;

		FreeObject freeObject_;

		mutable std::map<std::string, VariantMatrix> variantMatrixByKey_;

		mutable std::map<std::string, AQLStringMatrix> stringMatrixByKey_;
	};

	typedef std::shared_ptr< CurveGenerator > CurveGeneratorPtr;
}
