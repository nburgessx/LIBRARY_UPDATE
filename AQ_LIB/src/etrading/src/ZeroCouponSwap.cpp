/*
 * @brief			Class the defines the fixed float interest rate swap instrument
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

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
  		    throw LACoreInvalidData( "#Error: It is not a single currency Swap", __FILE__, __LINE__ );
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

