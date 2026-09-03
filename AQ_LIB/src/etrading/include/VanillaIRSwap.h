/*
 * @brief			Class the defines the interest rate swap instrument
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>
#include "Swap.h"

namespace etrading
{

    class VanillaIRSwap : public Swap 
    {
	public:
		VanillaIRSwap(const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB);
   		VanillaIRSwap(const VanillaIRSwap& rhs);

        virtual ~VanillaIRSwap() {}
        
        std::shared_ptr<Swap> clone();
    };

}
