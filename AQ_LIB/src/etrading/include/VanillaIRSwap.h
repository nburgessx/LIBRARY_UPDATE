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
