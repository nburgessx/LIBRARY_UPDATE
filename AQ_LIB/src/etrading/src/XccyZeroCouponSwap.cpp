#include "XccyZeroCouponSwap.h"
#include "LACurvePricingObject.h"
#include "CurveValidation.h"
#include "SwapCalculation.h"

namespace etrading
{

    XccyZeroCouponSwap::XccyZeroCouponSwap(const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB) 
                    : CrossCurrencySwap(instanceName, XCCY_ZERO_COUPON_SWAP, swapPropertiesLVB)
	{
		validateLegs(leg1, leg2);

        //Cross Currency
        if (leg1->getStaticData()->getCurrency() == leg2->getStaticData()->getCurrency())
        {
  		    throw AQLCoreInvalidData( "#Error: Invalid XCCY Swap: Xccy Swap legs cannot have the same currency", __FILE__, __LINE__ );
        }

        //Handle XccySwap properties
        populateAndValidateXccySwapStaticDataObject(leg1, leg2, swapPropertiesLVB);

        auto notionalAdjustedLeg = legs_.findLegByName(notionalResetLegName_);

        if (isMTM_ && notionalAdjustedLeg != nullptr && notionalAdjustedLeg->getSchedule()->isPaymentFreqEnumAtMaturity())
        {
            throw AQLCoreInvalidData("#Error: For MTM XCCY Swap, NotionalAdjustedLeg's payment frequency cannot be 'AT_MATURITY'.",__FILE__,__LINE__);
        }

        addToLegCollection(leg1);
		addToLegCollection(leg2);
	}

    XccyZeroCouponSwap::XccyZeroCouponSwap(const XccyZeroCouponSwap& rhs) : CrossCurrencySwap(rhs)
    {}

    std::shared_ptr<Swap> XccyZeroCouponSwap::clone()
    {
        SwapPtr swap = SwapPtr(new XccyZeroCouponSwap(*this));
        return swap;
    }

	LabelValueBlock XccyZeroCouponSwap::preCalculate(const LabelValueBlock& valuationSettingsLVB)
	{

		validateCollectionSize(valuationSettingsLVB);

		auto asOfDateFxRate = calculateAsOfDateFxRate(valuationSettingsLVB, valuationCurrency_, legs_.get(0), legs_.get(1));
		LabelValueBlock valSettingLVB(valuationSettingsLVB, VALUATION_SETTING_KEYS::FX_AS_OF_DATE_RATE, boost::lexical_cast<std::string>(asOfDateFxRate));

		setNotionalFromFutureValueNotional(legs_.get(0), legs_.get(1), valuationCurrency_, asOfDateFxRate);

		updateMTMXccyNotionalResetByFxLeg(valSettingLVB, legs_.get(0), legs_.get(1), notionalResetLegName_, isMTM_, "");

		return valSettingLVB;
	}


    double XccyZeroCouponSwap::parRate(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
    {
		auto valSettingLVB = preCalculate(valuationSettingsLVB);

        double ret = calculateIRR(valSettingLVB, legs_, fixingTableNames);
        return ret;
	}

}

