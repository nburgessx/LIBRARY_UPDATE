#include "VanillaIRSwap.h"

namespace etrading
{

	VanillaIRSwap::VanillaIRSwap(const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB) : Swap(instanceName, VANILLA_SWAP, swapPropertiesLVB) 
	{

        validateLegs(leg1, leg2);

        //Single Currency
        if ( leg1->getStaticData()->getCurrency() != leg2->getStaticData()->getCurrency() )
        {
  		    throw AQLCoreInvalidData( "#Error: It is not a single currency Swap", __FILE__, __LINE__ );
        }

        if (boost::math::isnan(leg1->getSchedule()->getNotional()) || boost::math::isnan(leg2->getSchedule()->getNotional()))
        {
    		throw AQLCoreInvalidData( "#Error: Please provide Notional", __FILE__, __LINE__ );
        }

        inputParameters_ = swapPropertiesLVB;
        
        addToLegCollection(leg1);
		addToLegCollection(leg2);

    }

    VanillaIRSwap::VanillaIRSwap(const VanillaIRSwap& rhs) : Swap(rhs)
    {}

    std::shared_ptr<Swap> VanillaIRSwap::clone()
    {
        SwapPtr swap = SwapPtr(new VanillaIRSwap(*this));
        return swap;
    }


}

