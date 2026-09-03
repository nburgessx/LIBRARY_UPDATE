// SettingsValidation.cpp

/*
 * @brief			Settings Validation Helper Methods
 * @Created:		6th November 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "SettingsValidation.h"

#include "CommonConstants.h"
#include "CurveValidation.h"        // getCurveAsOfDate
#include "LWOUtilities.h"
#include "ParameterValidation.h"

namespace etrading
{
    /* @brief			Get CurveCollection from the valuationSettingsLVB map, by matching the legName 
    *                   *** Supports Curve Objects and the Older Non-Curve Object Types ***
    *
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @output			The matched curveCollection
	*/
	AQLString getLWOCurveCollectionFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const AQLString legName )
    {
        std::string curveCollection         = getOptionalCurveCollectionFromValuationSettings( valuationSettingsLVB, legName ).getCString();
        const std::string curveObjectName   = getCurveObjectFromValuationSettings( valuationSettingsLVB ).getCString();
        
        if( curveCollection != "" && curveObjectName != "" )
        {
            AQ_THROW( "Invalid Valuation Settings: Must not provide both a CurveCollection and a CurveObject in the Valuation Settings Label Value Block" )
        }

        if( curveCollection == "" && curveObjectName == "" )
        {
			// Throw the most appropriate error message
			if (legName == "")
			{
				AQ_THROW("Invalid Valuation Settings: Curve Collection not specified")
			}
			else
			{
				AQ_THROW("Invalid Valuation Settings: Curve Collection for leg '" + legName + "' not specified")
			}
        }

        if ( !curveObjectName.empty() )
        {
            std::shared_ptr<SingleCurveObject> curveObject = getSingleCurveObject( curveObjectName ); // LWO Utility method to get the curve object pointer from the LWO Cache
            curveCollection = curveObject->getCurveCollection().c_str();
        }

        return curveCollection.c_str();
    }

    /* @brief			Get CurveCollection from the valuationSettingsLVB map, by matching the legName 
    *                   *** Supports Curve Objects and the Older Non-Curve Object Types ***
    *
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @output			The matched curveCollection
	*/
	AQLString getValidatedLWOCurveCollectionFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const AQLString legName )
    {
        const AQLString curveCollection  = getLWOCurveCollectionFromValuationSettings( valuationSettingsLVB, legName );
        const AQLDate curveAsOfDate      = getCurveAsOfDate( curveCollection );
                
        // Check Curve AsOf Date Matches the ValuationDate Specified in the Valuation Settings
        const AQLDate valuationDate = getValuationDateFromValuationSettings( valuationSettingsLVB );
        AQ_REQUIRE( curveAsOfDate <= valuationDate, "Invalid Curve Build Date: Curve build date cannot be greater than the valuation date in validation settings" );
        
        // Return Result if the Curve AsOf Date is Consistent with the ValuationSettings ValuationDate
        return curveCollection;
    }

    /* @brief			Get CurveCollection from the valuationSettingsLVB map, by matching the legName
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @output			The matched curveCollection
	*/
	AQLString getOptionalCurveCollectionFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const AQLString legName )
    {
        return getCurveCollectionFromValuationSettings( valuationSettingsLVB, legName, false ); // False = don't throw if curveCollection missing
    }

    /* @brief			Get CurveCollection from the valuationSettingsLVB map, by matching the legName
    * @param [in]		valuationSettingsLVB	ValuationSettings map
    * @param [in]		legName	            legName, required if curve collection set id
    * @output			The matched curveCollection
    */
    AQLString getCurveCollectionFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const AQLString legName, const bool throwIfMissing )
    {
        AQ_REQUIRE( valuationSettingsLVB.size() > 0, "Invalid 'ValuationSettings' - The ValuationSettings Label Value Block is empty" )

		// For Backwards Compatibility 
        // =============================================================================================

        std::vector<std::string> valuationSettingKeys = valuationSettingsLVB.getKeys();

        // LA 1: Global Curve Override
        // Curve Collections were previously specifed using the key 'CurveCollection', which was a global override for all trade legs    
        for ( size_t i = 0; i < valuationSettingKeys.size(); ++i )
        {
			std::string key = valuationSettingKeys[i];
			const std::string value = valuationSettingsLVB.getOptionalValue(key, "");

            if ( same( key, VALUATION_SETTING_KEYS::CURVE_COLLECTION ) )
			{
				return value.c_str();
			}
         }

        // LA 2: Curve Collections as a Key with No Value
        // Curve Collections were also previously specified as the key with no value
        for ( size_t j = 0; j < valuationSettingKeys.size(); ++j )
        {
			std::string key = valuationSettingKeys[j];
			const std::string value = valuationSettingsLVB.getOptionalValue(key, "");

            if ( value.empty() )
			{
				return key.c_str();
			}
         }
        // =============================================================================================

        // Option 3: Curve Collection Specified by Leg Name
        if ( throwIfMissing )
        {
            AQ_REQUIRE( throwIfMissing && legName.size() != 0, "Invalid 'ValuationSettings' Specified - We must specify the trade LegName and corresponding CurveCollection" )
        }

        AQLString uppperLegName = legName;
		uppperLegName.toUpper();
        
        std::string result = valuationSettingsLVB.getOptionalValue(uppperLegName.getCString(), "");

        if ( throwIfMissing )
        {
            AQ_REQUIRE( throwIfMissing && !result.empty(), "Invalid LegName in 'ValuationSettings' - Missing LegName " + uppperLegName + " - We must specify the trade LegName(s) and corresponding CurveCollection" )
        }
		
        return result.c_str();
		
    }
    
    /* @brief			Get CurveCollection from the valuationSettingsLVB map, by matching the legName and curve asOf date
	* @param [in]		valuationSettingsLVB	ValuationSettings map
    * @param [in]		legName	            legName, required if curve collection set id
	* @param [in]		throwIfMissing	    Boolean throw if curve collection is missing True or False
    * @output			The matched curveCollection
	*/
	AQLString getValidatedCurveCollectionFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const AQLString legName, const bool throwIfMissing )
    {
        const AQLString curveCollection  = getCurveCollectionFromValuationSettings( valuationSettingsLVB, legName, throwIfMissing );
        const AQLDate curveAsOfDate      = getCurveAsOfDate( curveCollection );
                
        // Check Curve AsOf Date Matches the ValuationDate Specified in the Valuation Settings
        const AQLDate valuationDate = getValuationDateFromValuationSettings( valuationSettingsLVB );
        AQ_REQUIRE( curveAsOfDate <= valuationDate, "Invalid Curve Build Date: Curve build date cannot be greater than the valuation date in validation settings" );
        
        // Return Result if the Curve AsOf Date is Consistent with the ValuationSettings ValuationDate
        return curveCollection;
    }

    /* @brief			Get CurveObject from the valuationSettingsLVB map, by matching the legName
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @output			The yield curve object
	*/
	AQLString getCurveObjectFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const AQLString legName )
    {
        // Make Optional - Don't throw if key is mssing
        return getKeyFromValuationSettings( VALUATION_SETTING_KEYS::CURVE_OBJECT, valuationSettingsLVB, legName, false ); // false = Make Optional: don't throw if missing
    }

    /* @brief			Get VolatilityModel from the valuationSettingsLVB map, by matching the legName
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @param [in]		throwIfMissing	    Boolean throw if curve collection is missing True or False
	* @output			The volatility model
	*/
	AQLString getVolatilityModelFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const AQLString legName, const bool throwIfMissing)
    {
        return getKeyFromValuationSettings( VALUATION_SETTING_KEYS::VOLATILITY_MODEL, valuationSettingsLVB, legName, throwIfMissing);
    }

    /* @brief			Get VolatilityModel from the valuationSettingsLVB map, by matching the legName
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
	* @output			The credit model
	*/
	AQLString getCreditModelFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const AQLString legName )
    {
		AQLString creditModelName = getKeyFromValuationSettings(VALUATION_SETTING_KEYS::CREDIT_MODEL, valuationSettingsLVB, "", false);

		if (creditModelName.size() == 0)
		{
			// LA: CreditModel as a Key with No Value
			const std::vector<std::string> valuationSettingKeys = valuationSettingsLVB.getKeys();
			for (size_t j = 0; j < valuationSettingKeys.size(); ++j)
			{
				const std::string key = valuationSettingKeys[j];
				const std::string value = valuationSettingsLVB.getOptionalValue(key, "");

				if (value.empty())
				{
					auto modelName = key.c_str();

					auto creditModelPtr = getCreditModel(modelName, false); // do not throw error if it's missing
					if (creditModelPtr)
					{
						creditModelName = modelName;
						break;
					}
				}
			}
		}

		return creditModelName;
    }

    /* @brief			Get FXSot from Valuation Settings map
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName	            legName, required if curve collection set id
    * @param [in]		throwIfMissing	    If TRUE throw an error if the key is missing, otherwise if FALSE return an empty string
	* @output			The credit model
	*/
	double getFXSpotFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing )
    {
        double fxSpot = 1.0;
        std::string fxSpotAsString = getKeyFromValuationSettings( VALUATION_SETTING_KEYS::FX_SPOT, valuationSettingsLVB, "", throwIfMissing ).getCString();

        // Default value for FXSpot is 1.0 when missing
        if ( fxSpotAsString.empty() || fxSpotAsString == "")
        {
            return fxSpot;
        }

        AQ_TRY_SET_VARIABLE( fxSpot, std::stod(fxSpotAsString), "Invalid FX Spot Rate in Valuation Settings" )
        return fxSpot;        
    }

	/* @brief			Get FX on as of date from Valuation Settings map
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		legName					legName, required if curve collection set id
	* @param [in]		throwIfMissing			If TRUE throw an error if the key is missing, otherwise if FALSE return an empty string
	* @output			As of date fx rate
	*/
	double getFXAsOfDateFromValuationSettings(const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing)
	{
		double fxAsOfDate = std::numeric_limits<double>::quiet_NaN();
		std::string fxAsOfDateAsString = getKeyFromValuationSettings(VALUATION_SETTING_KEYS::FX_AS_OF_DATE_RATE, valuationSettingsLVB, "", throwIfMissing).getCString();

		// Default value for FXSpot is 1.0 when missing
		if (fxAsOfDateAsString.empty() || fxAsOfDateAsString == "")
		{
			return fxAsOfDate;
		}

		AQ_TRY_SET_VARIABLE(fxAsOfDate, std::stod(fxAsOfDateAsString), "Invalid FX AsOfDate Rate in Valuation Settings")
		return fxAsOfDate;
	}

	/* @brief			Get ValuationDate from the valuationSettingsLVB map, by matching the legName
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		throwIfMissing	    If TRUE throw an error if the key is missing, otherwise if FALSE return an empty string
	* @output			The ValuationDate
	*/
	AQLDate getValuationDateFromValuationSettings(const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing)
	{
		const AQLString valuationDateStr = getKeyFromValuationSettings(VALUATION_SETTING_KEYS::VALUATION_DATE, valuationSettingsLVB, "", throwIfMissing);

		const AQLDate valuationDate = (valuationDateStr.size() != 0) ? stringToDate(valuationDateStr, "") : AQLDate();

		return valuationDate;
	}

	/* @brief Returns the bond Settlement date from the valuation settings. If the valuationSettingsLVB contains a single value, then use this as
	*		  the settlementDate. Otherwise look up the SettlementDate key in the block and fetch the corresponding value.
	*  @param[in]	valuationSettingsLVB	The LabelValueBlock containing valuation settings
	*  @param[in]	throwIfMissing			Whether to throw an exception if the specified key is missing
	*  @returns		The settlementDate.
	*/
	AQLDate getSettlementDateFromValuationSettings(const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing)
	{

		// For backwards compatibility, where a single SettlementDate is given without key
		if ( valuationSettingsLVB.size() == 1 )
		{
			std::vector<std::string> valuationSettingValues = valuationSettingsLVB.getValues();

			const std::string& value = valuationSettingValues[0];
			if ( value == "" )
			{
				std::vector<std::string> valuationSettingKeys = valuationSettingsLVB.getKeys();
				const std::string settlementDateStr = valuationSettingKeys[0];
				if ( settlementDateStr.size() == 0 )
				{
					AQ_THROW( "Missing SettlementDate.");
				}
				const AQLDate settlementDate = stringToDate(settlementDateStr.c_str(), "Missing SettlementDate.");
				return settlementDate;
			}
		}
		
		const AQLString settlementDateStr = getKeyFromValuationSettings(VALUATION_SETTING_KEYS::SETTLEMENT_DATE, valuationSettingsLVB, "", throwIfMissing);

		const AQLDate settlementDate = (settlementDateStr.size() != 0) ? stringToDate(settlementDateStr, "") : AQLDate();

		return settlementDate;
	}

	/* @brief			Get the Float-Bond Current CouponRate from the valuationSettingsLVB map. This is annualized coupon rate for
	*					the very next coupon which has already fixed.
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		throwIfMissing	    If TRUE throw an error if the key is missing, otherwise if FALSE return NaN
	* @output			The annualized coupon rate
	*/
	double getFloatBondCurrentCouponRateFromValuationSettings(const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing)
	{
		const std::string floatBondCurrentCouponStr = getKeyFromValuationSettings(VALUATION_SETTING_KEYS::FLOAT_BOND_CURRENT_COUPON, valuationSettingsLVB, "", throwIfMissing).getCString();

		double floatBondCurrentCoupon = std::numeric_limits<double>::quiet_NaN();

		if (floatBondCurrentCouponStr.empty())
		{
			return floatBondCurrentCoupon;
		}

		AQ_TRY_SET_VARIABLE(floatBondCurrentCoupon, std::stod(floatBondCurrentCouponStr), "Invalid FloatBondCurrentCoupon in Valuation Settings")

		return floatBondCurrentCoupon;
	}

	/* @brief			Get te Float-Bond AssumedRate from the valuationSettingsLVB map. This is the assumed average index rate for projected coupons
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		throwIfMissing	    If TRUE throw an error if the key is missing, otherwise if FALSE  returns NaN
	* @output			The assumed rate
	*/
	double getFloatBondAssumedRateFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing )
	{
		const std::string floatBondAssumedRateStr = getKeyFromValuationSettings(VALUATION_SETTING_KEYS::FLOAT_BOND_ASSUMED_RATE, valuationSettingsLVB, "", throwIfMissing).getCString();

		double floatBondAssumedRate = std::numeric_limits<double>::quiet_NaN();

		if (floatBondAssumedRateStr.empty())
		{
			return floatBondAssumedRate;
		}

		AQ_TRY_SET_VARIABLE(floatBondAssumedRate, std::stod(floatBondAssumedRateStr), "Invalid FloatBondAssumedRate in Valuation Settings")

		return floatBondAssumedRate;
	}

	/* @brief			Get Float-Bond Index-to-next-fix from the valuationSettingsLVB map. This is the underlying reference index rate for the current coupon interval.
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		throwIfMissing	    If TRUE throw an error if the key is missing, otherwise if FALSE return NaN
	* @output			The index-to-next-fix
	*/
	double getFloatBondIndexToNextCouponFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing )
	{
		const std::string floatBondIndexToNextCouponStr = getKeyFromValuationSettings(VALUATION_SETTING_KEYS::FLOAT_BOND_INDEX_TO_NEXT_COUPON, valuationSettingsLVB, "", throwIfMissing).getCString();

		double floatBondIndexToNextCoupon = std::numeric_limits<double>::quiet_NaN();

		if (floatBondIndexToNextCouponStr.empty())
		{
			return floatBondIndexToNextCoupon;
		}

		AQ_TRY_SET_VARIABLE(floatBondIndexToNextCoupon, std::stod(floatBondIndexToNextCouponStr), "Invalid FloatBondIndexToNextCoupon in Valuation Settings")

		return floatBondIndexToNextCoupon;
	}

	/* @brief			Get Float-Bond quoted-margin from the valuationSettingsLVB map. This is the quoted spread of the bond coupons over the reference index level.
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		throwIfMissing	    If TRUE throw an error if the key is missing, otherwise if FALSE return NaN
	* @output			The quoted-margin
	*/
	double getFloatBondQuotedMarginFromValuationSettings( const LabelValueBlock& valuationSettingsLVB, const bool throwIfMissing )
	{
		const std::string floatBondQuotedMarginStr = getKeyFromValuationSettings(VALUATION_SETTING_KEYS::FLOAT_BOND_QUOTED_MARGIN, valuationSettingsLVB, "", throwIfMissing).getCString();

		double floatBondQuotedMargin = std::numeric_limits<double>::quiet_NaN();

		if (floatBondQuotedMarginStr.empty())
		{
			return floatBondQuotedMargin;
		}

		AQ_TRY_SET_VARIABLE(floatBondQuotedMargin, std::stod(floatBondQuotedMarginStr), "Invalid FloatBondQuotedMargin in Valuation Settings")

		return floatBondQuotedMargin;
	}

	/* @brief			Get VolatilityModel from the valuationSettingsLVB map, by matching the legName
	* @param [in]		searchKey               Key to search for in Valuation Settings Map
    * @param [in]		valuationSettingsLVB	    ValuationSettings map
	* @param [in]		legName	                legName, required if curve collection set id
    * @param [in]		throwIfMissing	    If TRUE throw an error if the key is missing, otherwise if FALSE return an empty string
	* @output			The value corresponding to the searchKey
	*/
	AQLString getKeyFromValuationSettings( const std::string& searchKey, const LabelValueBlock& valuationSettingsLVB, const AQLString legName, const bool throwIfMissing )
    {
        AQ_REQUIRE( valuationSettingsLVB.size() != 0, "Invalid 'ValuationSettings' - The ValuationSettings Label Value Block is empty" )
        AQ_REQUIRE( !searchKey.empty(), "Invalid ValuationSettings - Unable to search the ValuationSettings LVB as the searchKey is missing" )

        // Curve Collection Searches are different since they need to preserve backwards compatibility, for now at least.
        if ( searchKey == VALUATION_SETTING_KEYS::CURVE_COLLECTION  )
        {
            return getOptionalCurveCollectionFromValuationSettings( valuationSettingsLVB, legName );
        }
        
        // Get the Value from the Label Value Block and Trim / Remove the Object Counter if present
        std::string result;
        if ( throwIfMissing )
        {
            result = valuationSettingsLVB.getCompulsoryValue( searchKey, "VALUATION SETTINGS" );
        }
        else
        {
            result = valuationSettingsLVB.getOptionalValue( searchKey, "" );
        }

        result = trimObjectCounter( result );
        return result.c_str();
    }

	/* @brief			get fixingtable name from the fixingTableNames map, by matching the legname
	* @param [in]		fixingTableNames	fixing table LVB
	* @param [in]		legName	        ID of the leg
	* @param [in]		legType	        Type of the leg, e.g. FIXED, FLOAT, FEE
	* @output			the matched fixingtable name
	*/
	std::string findFixingTableName(const LabelValueBlock& fixingTableNames, const std::string& legName, const ScheduleTypeEnum& legType)
	{
		std::string fixingTableName = "";

		if ( (! isFloatLeg(legType) && ! isInflationLeg(legType))  || fixingTableNames.size() == 0)
		{
			return fixingTableName;
		}

		auto lvbKeys = fixingTableNames.getKeys();
		size_t expectedSize = fixingTableNames.size();

		if (expectedSize == 0)
		{
			//If not fixing table is provided, return empty string
			fixingTableName = "";
		}
		else if (expectedSize == 1)
		{
			fixingTableName = lvbKeys.at(0);
		}
		//More than one fixingTableNames
		else
		{
			AQ_REQUIRE(legName.size() != 0, "Invalid Fixing Table Specified - We must specify the trade LegName and corresponding FixingTable")

				auto lvbValues = fixingTableNames.getValues();
			for (size_t i = 0; i < expectedSize; ++i)
			{
				auto key = lvbKeys[i];
				if (same(key, legName))
				{
					fixingTableName = lvbValues[i];
					break;
				}
			}
		}

		return fixingTableName;
	}
}