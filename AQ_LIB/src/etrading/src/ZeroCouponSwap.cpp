#include "ZeroCouponSwap.h"
#include "LACurvePricingObject.h"
#include "CurveValidation.h"
#include "SwapCalculation.h"

namespace etrading
{

    ZeroCouponSwap::ZeroCouponSwap(const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB) : Swap(instanceName, ZERO_COUPON_SWAP, swapPropertiesLVB)
	{

        validateLegs(leg1, leg2);

        //Single Currency
        if ( leg1->getStaticData()->getCurrency() != leg2->getStaticData()->getCurrency() )
        {
  		    throw AQLCoreInvalidData( "#Error: It is not a single currency Swap", __FILE__, __LINE__ );
        }

        setNotionalFromFutureValueNotional(leg1, leg2);

        addToLegCollection(leg1);
		addToLegCollection(leg2);

	}

    ZeroCouponSwap::ZeroCouponSwap(const ZeroCouponSwap& rhs) : Swap(rhs)
    {}

    std::shared_ptr<Swap> ZeroCouponSwap::clone()
    {
        SwapPtr swap = SwapPtr(new ZeroCouponSwap(*this));
        return swap;
    }

    double ZeroCouponSwap::parRate(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTables)
    {
        validateCollectionSize(valuationSettingsLVB);

        double ret = calculateIRR(valuationSettingsLVB, legs_, fixingTables);
        return ret;
	}

}

