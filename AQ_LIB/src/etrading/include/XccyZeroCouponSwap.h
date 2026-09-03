#pragma once

#include <string>
#include "CrossCurrencySwap.h"
#include "ZeroCouponSwap.h"

namespace etrading
{

    class XccyZeroCouponSwap : public CrossCurrencySwap
    {
    public:

        XccyZeroCouponSwap(const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB);
   		XccyZeroCouponSwap(const XccyZeroCouponSwap& rhs);
        virtual ~XccyZeroCouponSwap() {}
        std::shared_ptr<Swap> clone();

        double parRate(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTable);

	protected:

		//override the parent method
		LabelValueBlock preCalculate(const LabelValueBlock& valuationSettingsLVB);

    };

}
