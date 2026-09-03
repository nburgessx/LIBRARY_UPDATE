#pragma once

#include "LACoreTemplateType.h"
#include "CommonConstants.h"
#include "CoreEnumerations.h"
#include "LabelValueBlock.h"
#include "Option.h"
#include "Bond.h"

namespace etrading
{
	class BondOption : public Option
	{
	public:
		// Constructors
		BondOption(const std::string& objectName, const LabelValueBlock& dealLVB);
		BondOption( const BondOption& rhs );
		virtual ~BondOption() {};

		std::shared_ptr<Option> clone();

		//Override the abstract class:
		const SchemaObject toSchemaObject() const;

		// Return true if the underlying is Bond, return false if the underlying is Bond Future
		bool isUnderlyingBond() const;

		// Getter methods
		OptionStyleEnum getOptionStyle() const { return optionStyle_; };
		CallOrPutEnum getCallPut() const { return callPut_; };
		LongShortPositionEnum getLongShort() const { return longShort_; };
		double getStrike() const { return strike_; };
		LADate getExpiryDate() const { return expiryDate_; };
		LADate getUnderlyingDeliveryDate() const { return underlyingDeliveryDate_; };
		BondPtr getUnderlying() const { return underlying_; };

		CCY getCurrency() const { return currency_; };
		CCY getValuationCurrency() const { return valuationCurrency_; };
		double getNotional() const { return notional_; };
		double getLeverage() const { return leverage_; };

		DayCountEnum getDayCount() const { return dayCount_; };

	private:

		// Member data
		OptionStyleEnum optionStyle_;					// European, Bermudan, American
		CallOrPutEnum callPut_;							// Call / Put
		LongShortPositionEnum longShort_;			    // Long / Short

		double strike_;
		LADate expiryDate_;
		LADate underlyingDeliveryDate_;

		BondPtr underlying_;

		CCY currency_;
		CCY valuationCurrency_;

		double notional_;
		double leverage_;
		DayCountEnum dayCount_;

	};

}


