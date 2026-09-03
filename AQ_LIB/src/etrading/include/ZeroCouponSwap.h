/*
 * @brief			Class the defines the fixed fixed interest rate swap instrument
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <string>
#include "Swap.h"

namespace etrading
{

    class ZeroCouponSwap : public Swap
    {
    public:
        //ZeroCouponSwap(const std::string& instanceName, const SwapTypeEnum swapType, const LabelValueBlock& swapPropertiesLVB);
        ZeroCouponSwap(const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB);
   		ZeroCouponSwap(const ZeroCouponSwap& rhs);
        virtual ~ZeroCouponSwap() {}
        std::shared_ptr<Swap> clone();

        double parRate(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTable);

    };

}
