/*
 * @brief			Class the defines the interest rate swap instrument
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "VanillaIRSwap.h"

namespace etrading
{

	VanillaIRSwap::VanillaIRSwap(const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB) : Swap(instanceName, VANILLA_SWAP, swapPropertiesLVB) 
	{

        validateLegs(leg1, leg2);

        //Single Currency
        if ( leg1->getStaticData()->getCurrency() != leg2->getStaticData()->getCurrency() )
        {
  		    throw LACoreInvalidData( "#Error: It is not a single currency Swap", __FILE__, __LINE__ );
        }

        if (boost::math::isnan(leg1->getSchedule()->getNotional()) || boost::math::isnan(leg2->getSchedule()->getNotional()))
        {
    		throw LACoreInvalidData( "#Error: Please provide Notional", __FILE__, __LINE__ );
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

