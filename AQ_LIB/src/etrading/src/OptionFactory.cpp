/*
* @brief			Factory Class to create options
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/
#include "OptionFactory.h"
#include "CapFloorOption.h"
#include "SwaptionTrade.h"
#include "BondOption.h"
#include "CommonConstants.h"
#include "LabelValueBlockValidation.h"
#include "ExceptionMacros.h"

#include <string>
#include <memory>
#include <boost/algorithm/string.hpp>

namespace etrading
{

	static std::vector<std::string> capFloorLVBKeys()
	{
		const std::string arr[] =
		{
            OBJECT_KEY::OBJECT_TYPE
            ,OBJECT_KEY::TRADE_TYPE

			, MARKET_KEY::FORECAST_CURVE //Optional for fixed leg, compulsory for float leg
			,IRS_KEY::COMPOUND_METHOD //Not required for fixed leg, optional for float leg

			, MARKET_KEY::DISCOUNT_CURVE
			, IRS_KEY::VALUATION_CURRENCY
			, IRS_KEY::CURRENCY

			// fields for fixing
			, IRS_KEY::FIRSTFIXING
			, IRS_KEY::LASTFIXING
			, IRS_KEY::FIRSTSTUBCURVEINDEX
			, IRS_KEY::LASTSTUBCURVEINDEX
			, IRS_KEY::IS_FWD_INTER //This is to allow user to change isFwdInter to true or false

			//Schedule related fields:
			, IRS_KEY::NOTIONAL
			//, IRS_KEY::NOTIONAL_EXCHANGE
			, IRS_KEY::LEVERAGE

			, IRS_KEY::EFFECTIVE_DATE
			, IRS_KEY::MATURITY_DATE
			, IRS_KEY::ACCRUALFREQUENCY
			, IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT
			, IRS_KEY::ACCRUALCALENDAR
			, IRS_KEY::ACCRUALDAYCOUNT
			, IRS_KEY::PAYMENTFREQUENCY
			, IRS_KEY::PAYMENTBUSINESSDAYADJUSTMENT
			, IRS_KEY::PAYMENTCALENDAR
			, IRS_KEY::PAYMENTLAG
			, IRS_KEY::FIRSTSTUBDATE
			, IRS_KEY::LASTSTUBDATE
			, IRS_KEY::ROLLDAY
			, IRS_KEY::STUBTYPE
			, IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT
			, IRS_KEY::FIXINGCALENDAR
			, IRS_KEY::FIXINGLAG
			, IRS_KEY::FIRSTFIXINGLAG
			, IRS_KEY::FIXINGADVANCEORARREAR

			// These keys are for relaxing the prefix "ACCRUAL"	
			, IRS_KEY::BUSINESSDAYADJUSTMENT
			, IRS_KEY::CALENDAR
			, IRS_KEY::DAYCOUNT
			, IRS_KEY::FREQUENCY

			, IRS_KEY::FLOAT_SPREAD

            , OPTION_KEYS::OPTION_STYLE
			, OPTION_KEYS::STRIKE
			, OPTION_KEYS::SPOT_LAG
			, OPTION_KEYS::SPOT_BUSINESSDAYADJUSTMENT
			, OPTION_KEYS::SPOT_CALENDAR

		};
		std::vector<std::string> expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));
		return expectedKeys;
	}

	static std::vector<std::string> swaptionLVBKeys()
	{
		const std::string arr[] =
		{
            OBJECT_KEY::OBJECT_TYPE,
            OBJECT_KEY::TRADE_TYPE,
            SWAPTION_KEYS::OPTION_STYLE,
            SWAPTION_KEYS::LONG_SHORT,
            SWAPTION_KEYS::PAYER_RECEIVER,
            SWAPTION_KEYS::TENOR_DESCRIPTION,
            SWAPTION_KEYS::EFFECTIVE_DATE,
            SWAPTION_KEYS::EXPIRY,
            SWAPTION_KEYS::SETTLEMENT_TYPE,
            SWAPTION_KEYS::CURRENCY,
            SWAPTION_KEYS::NOTIONAL,
            SWAPTION_KEYS::LEVERAGE,
            SWAPTION_KEYS::STRIKE,
            SWAPTION_KEYS::SWAP_GENERATOR,
            SWAPTION_KEYS::SWAP_START,
            SWAPTION_KEYS::SWAP_END,
            SWAPTION_KEYS::NOTIFICATION_DAYS,
            SWAPTION_KEYS::DAYCOUNT,
            SWAPTION_KEYS::BUSINESSDAYADJUSTMENT,
            SWAPTION_KEYS::CALENDAR,
            SWAPTION_KEYS::FEE,
            SWAPTION_KEYS::FEE_DATE,
            SWAPTION_KEYS::FEE_PAY_RECEIVE
		};

		std::vector<std::string> expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));
		return expectedKeys;
	}

	static std::vector<std::string> volatilityLVBKeys()
	{
		const std::string arr[] =
		{
            OBJECT_KEY::OBJECT_TYPE,
			OPTION_KEYS::AS_OF_DATE, 
			IRS_KEY::CURRENCY,
            VOLATILITY_KEYS::VOL_DATA_SOURCE, 
			VOLATILITY_KEYS::VOL_TYPE,
			VOLATILITY_KEYS::VOLATILITY,
            VOLATILITY_KEYS::SHIFT_SIZE
		};
		std::vector<std::string> expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));
		return expectedKeys;
	}

	static std::vector<std::string> bondOptionLVBKeys()
	{
		const std::string arr[] =
		{
            OBJECT_KEY::OBJECT_TYPE,
            OPTION_KEYS::TRADE_TYPE,
			OPTION_KEYS::OPTION_STYLE,
            OPTION_KEYS::CALL_PUT,
			OPTION_KEYS::LONG_SHORT,
            OPTION_KEYS::STRIKE,
			OPTION_KEYS::EXPIRY,
            OPTION_KEYS::DELIVERY_DATE,
			OPTION_KEYS::BOND_NAME,
			OPTION_KEYS::FUTURE_NAME,
			IRS_KEY::CURRENCY,
			IRS_KEY::VALUATION_CURRENCY,
			IRS_KEY::NOTIONAL,
			IRS_KEY::LEVERAGE,
			IRS_KEY::DAYCOUNT

		};
		std::vector<std::string> expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));
		return expectedKeys;
	}

    /* @brief			Create a Option Pointer based on the Option's Label Value Blocks
	*  @param [in]		objectName      Option cached name
	*  @param [in]		tradeLVB		Option Description Label Value Block
	*  @param [in]		validateKeys    Validate the bond and schedule LVB keys, defaults to true
	*  @return			Option Pointer
	*/
	OptionPtr createOption(const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys )
    {
        MLIB_REQUIRE( !tradeLVB.isEmpty() , "The trade Label Value Block is empty or contains errors" )

        // Validate Object Type
        const std::string inputLVB = "tradeLVB";
        CachedObjectEnum objectType = toCachedObjectEnum( tradeLVB.getCompulsoryValueAsString( OBJECT_KEY::OBJECT_TYPE, inputLVB ) );
        MLIB_REQUIRE( objectType == OPTION, "Invalid Object Type - 'OPTION' type required" )

        // Determine the trade type can call the appropriate constructor
        OptionTradeTypeEnum optionTradeType = toOptionTradeTypeEnum( tradeLVB.getCompulsoryValueAsString( OPTION_KEYS::TRADE_TYPE, inputLVB ) );
        
        switch ( optionTradeType )
        {
            case CAP_FLOOR_TRADE:
            {
                return createCapFloor( objectName, tradeLVB, validateKeys );
                break;
            }
            case EUROPEAN_SWAPTION_TRADE:
            {
                return createEuropeanSwaption( objectName, tradeLVB, validateKeys );
                break;
            }
            case BOND_OPTION_TRADE:
            {
                return createBondOption( objectName, tradeLVB, validateKeys );
                break;
            }
            default:
			    throw ETradingException("#Error: Invalid Trade Type, must be 'CAPFLOOR', 'EUROPEAN_SWAPTION' or 'BONDOPTION' ");
			    break;
		}
    }

    /* @brief			Create a Option Pointer based on the Option's Label Value Blocks
	*  @param [in]		objectName      Option cached name
	*  @param [in]		tradeLVB		Option Description Label Value Block
	*  @param [in]		validateKeys    Validate the bond and schedule LVB keys, defaults to true
	*  @return			Option Pointer
	*/
	OptionPtr createCapFloor(const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys)
    {
        MLIB_REQUIRE( !tradeLVB.isEmpty(), "The trade Label Value Block is empty or contains errors" )
        
        // Validate Keys
        const std::string inputLVB = "CapFloorTradeLVB";
        validateKeysForLVB( capFloorLVBKeys(), tradeLVB.getKeys(), validateKeys, inputLVB );

        // CAP/FLOOR - Compulsory Keys enforced from within the capFloor constructor
        CapFloorOption capFloor( objectName, tradeLVB );
        return std::make_shared<CapFloorOption>(capFloor);
    }


    /* @brief			Create a Option Pointer based on the Option's Label Value Blocks
	*  @param [in]		objectName      Option cached name
	*  @param [in]		tradeLVB		Option Description Label Value Block
	*  @param [in]		validateKeys    Validate the bond and schedule LVB keys, defaults to true
	*  @return			Option Pointer
	*/
	OptionPtr createEuropeanSwaption(const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys)
    {
        MLIB_REQUIRE( !tradeLVB.isEmpty(), "The trade Label Value Block is empty or contains errors" )
        
        // Validate Keys
        const std::string inputLVB = "swaptionLVB";
        validateKeysForLVB( swaptionLVBKeys(), tradeLVB.getKeys(), validateKeys );

        // EUROPEAN SWAPTION - Compulsory Keys enforced from within the swaption constructor
		SwaptionTrade swaption( objectName, tradeLVB );
        return std::make_shared<SwaptionTrade>( swaption );	
    }


	/* @brief			Create a Option Pointer based on the Option's Label Value Blocks
	*  @param [in]		objectName      Option cached name
	*  @param [in]		tradeLVB		Option Description Label Value Block
	*  @param [in]		validateKeys    Validate the bond and schedule LVB keys, defaults to true
	*  @return			Option Pointer
	*/
	OptionPtr createBondOption(const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys)
	{
        MLIB_REQUIRE( !tradeLVB.isEmpty(), "The trade Label Value Block is empty or contains errors" )

        // Validate Keys
        const std::string inputLVB = "BondOptionTradeLVB";
		validateKeysForLVB( etrading::bondOptionLVBKeys(), tradeLVB.getKeys(), validateKeys, inputLVB );
		
        // BOND OPTION - Compulsory Keys are enforced within the BondOption Constructor
        BondOption bondOption(objectName, tradeLVB);
		return std::make_shared<BondOption>(bondOption);
	}


	/* @brief			Create a Volatility Pointer based on the Volatility's Label Value Blocks
	*  @param [in]		objectName      Volatility cached name
	*  @param [in]		volLVB			Volatility Description Label Value Block
	*  @param [in]		validateKeys    Validate the bond and schedule LVB keys, defaults to true
	*  @return			Option Pointer
	*/
	std::shared_ptr<Volatility> createVolatility(const std::string& objectName, const LabelValueBlock& volLVB, const bool& validateKeys)
	{
        MLIB_REQUIRE( !volLVB.isEmpty(), "The volatility Label Value Block is empty or contains errors" )
		
        // Validate Keys
        const std::string inputLVB = "VolatilityLVB";
        validateKeysForLVB( volatilityLVBKeys(), volLVB.getKeys(), validateKeys, inputLVB );
		
        // VOLATILITY OBJECT
        Volatility volObject(volLVB, objectName);
		return std::make_shared<Volatility>(volObject);
	}

	AnyTypeMatrix viewGreeks(const BlackScholesGreeks& greeks, const bool& showColumnHeaders) 
	{
		AnyTypeMatrix result;

		if (showColumnHeaders)
		{
			AnyTypeVector headers(5);
			headers[0] = (std::string)"Delta";
			headers[1] = (std::string)"Gamma";
			headers[2] = (std::string)"Vega";
			headers[3] = (std::string)"Theta";
			headers[4] = (std::string)"Rho";

			result.push_back(headers);
		}

		AnyTypeVector body(5);
		body[0] = greeks.deltaSpot;
		body[1] = greeks.gamma;
		body[2] = greeks.vega;
		body[3] = greeks.theta;
		body[4] = greeks.rho;

		result.push_back(body);

		return result;
	}


}