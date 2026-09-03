#include "ConstantMaturitySwap.h"
#include "CMSLeg.h"
#include "CoreEnumerations.h"
#include "SwapUtilities.h"
#include "SettingsValidation.h"

namespace etrading
{

	ConstantMaturitySwap::ConstantMaturitySwap( const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB) : Swap(instanceName, CONSTANT_MATURITY_SWAP, swapPropertiesLVB ) 
	{
		validateLegs( leg1, leg2 );

        //Single Currency
        if ( leg1->getStaticData()->getCurrency() != leg2->getStaticData()->getCurrency() )
        {
  		    MLIB_THROW( "Only single currency swaps are supported." );
        }

        if (boost::math::isnan( leg1->getSchedule()->getNotional()) || boost::math::isnan(leg2->getSchedule()->getNotional() ))
        {
    		MLIB_THROW( "Notional not provided." );
        }

        inputParameters_ = swapPropertiesLVB;
        
        addToLegCollection( leg1 );
		addToLegCollection( leg2 );

    }

    ConstantMaturitySwap::ConstantMaturitySwap( const ConstantMaturitySwap& rhs ) : Swap( rhs )
    {}

    std::shared_ptr<Swap> ConstantMaturitySwap::clone()
    {
        SwapPtr swap = SwapPtr( new ConstantMaturitySwap( *this ));
        return swap;
    }

	/* @brief Calculates the total PV of all of the Constant Maturity Swap legs.
	*
	* @param[in]	valuationSettingsLVB		A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	convexityAdjustment		The convexity adjustment, to add to the PV.
	* @param[in]	fixingTableNames		Fixing table object names
	* @param[in]	legName					Optionally calculate the PV of the specified leg only.
	* @returns	The calculated PV value
	*/
	double ConstantMaturitySwap::pvUsingConvexityAdjustment( const LabelValueBlock& valuationSettingsLVB, const double convexityAdjustment, const LabelValueBlock& fixingTableNames, const LAString& legName) const
	{
		double pv = 0.0;

        if (legName.size() == 0)
        {
			// Calc PV of all legs
			for ( size_t i = 0; i < legs_.size(); ++i )
			{
				auto leg = legs_.get( i );

				DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, fixingTableNames, leg->getLegName()));

				// if CMS leg, call the alternative PV method with convexity adj.
				// If standard leg (fixed etc) call the
				if ( leg->getType() == CMS_SCHEDULE_TYPE )
				{
					// OK to use static_pointer_cast here because we know the leg type for sure (the ConstantMaturitySwap created it).
					std::shared_ptr<CMSLeg> cmsLeg = std::static_pointer_cast<CMSLeg> ( leg );

					pv += cmsLeg->pvUsingConvexityAdjustment( dataProvider, convexityAdjustment );

				}
				else
				{
					// regular leg type
					pv += leg->pv( dataProvider, false );
				}
			}
        }
		else
		{
			// Search for the specified leg and calculate the PV of that leg only.
			bool legFound = false;
			for (size_t i = 0; i < legs_.size(); ++i)
			{
				auto leg = legs_.get(i);
				if ( same( leg->getLegName(), legName ))
				{
					legFound = true;

					DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, fixingTableNames, leg->getLegName()));

					if ( leg->getType() == CMS_SCHEDULE_TYPE )
					{
						// OK to use static_pointer_cast here because we know the leg type for sure (the ConstantMaturitySwap created it).
						std::shared_ptr<CMSLeg> cmsLeg = std::static_pointer_cast<CMSLeg> ( leg );
						pv = cmsLeg->pvUsingConvexityAdjustment(dataProvider, convexityAdjustment );
					}
					else
					{
						// Regular leg type
						 pv = leg->pv( dataProvider, false );
					}

					break;
				}
			}
			if ( ! legFound )
			{
				MLIB_THROW( "Specified legName '" + legName + "' does not exist in the ConstantMaturitySwap." );
			}
		}

		return pv;
	}

	/* @brief Calculates the Constant Maturity Swap Par Rate
	*
	* @param[in]	valuationSettingsLVB		A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	convexityAdjustment		The convexity adjustment, to add to the PV.
	* @param[in]	legName					Optionally calculate the PV of the specified leg only.
	* @param[in]	fixingTableNames			Fixing table object names
	* @returns	The calculated Par Rate value
	*/
	double ConstantMaturitySwap::parRateUsingConvexityAdjustment( const LabelValueBlock& valuationSettingsLVB, double convexityAdjustment, const LabelValueBlock& fixingTableNames )
	{
		LAString fixedLeg;
		LAString cmsLeg;
		LAString validatedFixedLegName = validateLegName( fixedLeg, FIXED_SCHEDULE_TYPE, legs_ );
		LAString validatedCMSLegName   = validateLegName( cmsLeg, CMS_SCHEDULE_TYPE, legs_ );

		const double cmsLegPV = pvUsingConvexityAdjustment( valuationSettingsLVB, convexityAdjustment, fixingTableNames, validatedCMSLegName);

		const double fixedLegAnnuity = annuity(valuationSettingsLVB, validatedFixedLegName );

		const double parRate = cmsLegPV / fixedLegAnnuity;

		return parRate;
	}


}

