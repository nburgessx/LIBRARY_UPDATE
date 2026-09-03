#pragma once

#include <string>
#include <vector>
#include "Swap.h"


namespace etrading
{

    class ConstantMaturitySwap : public Swap 
    {
	public:
		ConstantMaturitySwap( const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB );
   		ConstantMaturitySwap( const ConstantMaturitySwap& rhs );

		virtual ~ConstantMaturitySwap() {}
        
        std::shared_ptr<Swap> clone();

		/* @brief Calculates the total PV of all of the Constant Maturity Swap legs.
		*
		* @param[in]	valuationSettingsLVB		A label value block containing a single collection name or a curveCollection per leg
		* @param[in]	convexityAdjustment		The convexity adjustment, to add to the PV.
		* @param[in]	fixingTableNames		Fixing table object names
		* @param[in]	legName					Optionally calculate the PV of the specified leg only.
		* @returns	The calculated PV value
		*/
		double pvUsingConvexityAdjustment( const LabelValueBlock& valuationSettingsLVB, const double convexityAdjustment, const LabelValueBlock& fixingTableNames, const AQLString& legName="") const;

		/* @brief Calculates the Constant Maturity Swap Par Rate
		*
		* @param[in]	valuationSettingsLVB		A label value block containing a single collection name or a curveCollection per leg
		* @param[in]	convexityAdjustment		The convexity adjustment, to add to the PV.
		* @param[in]	legName					Optionally calculate the PV of the specified leg only.
		* @param[in]	fixingTableNames		Fixing table object names
		* @returns	The calculated Par Rate value
		*/
		double parRateUsingConvexityAdjustment( const LabelValueBlock& valuationSettingsLVB, double convexityAdjustment, const LabelValueBlock& fixingTableNames );

	private:

	};

}

