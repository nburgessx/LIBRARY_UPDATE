/*
* @brief			Class the defines the BondOption Trade
* @Created:			30 July 2018
* @Author:			Yongyan Zheng
* @Department:		Quant Research & Analytics
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/

#include "BondOption.h"
#include "ParameterValidation.h"
#include "LWOUtilities.h"
#include "SettingsValidation.h"

namespace etrading
{
	// Main constructor
	BondOption::BondOption(const std::string& objectName, const LabelValueBlock& dealLVB) : Option(objectName)
	{
		// Validate Object Type
        const std::string inputLVB = "BondOptionTradeLVB";
        CachedObjectEnum objectType = toCachedObjectEnum( dealLVB.getCompulsoryValueAsString( OBJECT_KEY::OBJECT_TYPE, inputLVB ) );
        MLIB_REQUIRE( objectType == OPTION, "Invalid Object Type - 'OPTION' type required" )
		
        // Parse the input parameter block
		inputParameters_ = dealLVB;

		// Option Style: European, Bermudan, American
		optionStyle_ = toOptionStyleEnum(dealLVB.getCompulsoryValueAsString(OPTION_KEYS::OPTION_STYLE, inputLVB));
		callPut_ = toCallOrPutEnum(dealLVB.getCompulsoryValueAsString(OPTION_KEYS::CALL_PUT, inputLVB));
		
        longShort_ = toLongShortPositionEnum( dealLVB.getCompulsoryValueAsString(OPTION_KEYS::LONG_SHORT, inputLVB));

		strike_ = dealLVB.getCompulsoryValueAsDoubleFromKeys(IRS_KEY::STRIKE_RATE, OPTION_KEYS::STRIKE, inputLVB);

		// Unadjusted expiry Date
		const std::string expiryDt = dealLVB.getCompulsoryValueAsString(OPTION_KEYS::EXPIRY, inputLVB);
		expiryDate_ = stringToDate(expiryDt.c_str(), "#Error: Invalid Expiry Date");

		// Underlying Bond
		const std::string underlyingBondObjectName = getKeyFromValuationSettings(OPTION_KEYS::BOND_NAME, dealLVB).getCString();
		const std::string futureName = dealLVB.getOptionalValueAsString(OPTION_KEYS::FUTURE_NAME, "");

		MLIB_REQUIRE( !(underlyingBondObjectName.empty() && futureName.empty()) , "Either BondName or FutureName must be provided as the Option Underlying.")
		MLIB_REQUIRE( (underlyingBondObjectName.empty() || futureName.empty()), "Cannot provide both BondName and FutureName for the Option Underlying.")

		// Populate the underlyingBond pointer if the bondName is provided, otherwise it 
		if (!underlyingBondObjectName.empty())
		{
			underlying_ = getBond(underlyingBondObjectName);
		}
	
		const std::string underlyingDeliveryDt = dealLVB.getCompulsoryValueAsString(OPTION_KEYS::DELIVERY_DATE, inputLVB);
		underlyingDeliveryDate_ = stringToDate(underlyingDeliveryDt.c_str(), "#Error: Invalid Underlying Delivery Date");

		const std::string inputCurrency = dealLVB.getCompulsoryValueAsString(IRS_KEY::CURRENCY, inputLVB);
		currency_ = toCCYEnum(inputCurrency);
		valuationCurrency_ = toCCYEnum(dealLVB.getOptionalValueAsString(IRS_KEY::VALUATION_CURRENCY, inputCurrency));
		
		notional_ = dealLVB.getCompulsoryValueAsDouble(IRS_KEY::NOTIONAL );
		leverage_ = dealLVB.getOptionalValueAsDouble(IRS_KEY::LEVERAGE, 1.);

		dayCount_ = toDayCountEnum(dealLVB.getCompulsoryValueAsString(OPTION_KEYS::DAYCOUNT, inputLVB));
	}

	// Copy constructor
	BondOption::BondOption(const BondOption& rhs) : Option(rhs),
													optionStyle_( rhs.optionStyle_ ),
													callPut_(rhs.callPut_),
													longShort_( rhs.longShort_ ),
													strike_( rhs.strike_ ),
													expiryDate_( rhs.expiryDate_),
													underlying_(rhs.underlying_),
													underlyingDeliveryDate_(rhs.underlyingDeliveryDate_),
													currency_( rhs.currency_),
													valuationCurrency_( rhs.valuationCurrency_ ),
													notional_( rhs.notional_ ),
													leverage_( rhs.leverage_ ),
													dayCount_(rhs.dayCount_)
	{}

	// Return true if the underlying is Bond, return false if the underlying is Bond Future
	bool BondOption::isUnderlyingBond() const
	{
		return (underlying_ != nullptr);
	}

	std::shared_ptr<Option> BondOption::clone()
	{
		BondOption temp(*this);
		return std::make_shared<BondOption>(temp);
	}

	const SchemaObject BondOption::toSchemaObject() const
	{
		SchemaObject schemaObject = Option::toSchemaObject();

		if (underlying_ != nullptr)
		{
			underlying_->toSchemaObject(schemaObject);
		}
	
		return schemaObject;
	}

}
