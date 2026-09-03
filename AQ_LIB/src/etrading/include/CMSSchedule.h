#pragma once

#include "FloatSchedule.h"
#include "CMSStaticData.h"

namespace etrading
{
    class CMSSchedule : public FloatSchedule
    {
    public:
		CMSSchedule( const std::string& instanceName );
		CMSSchedule( const LabelValueBlock& scheduleLVB, const std::string& instanceName );

		CMSSchedule( const CMSSchedule& rhs );
		virtual ~CMSSchedule() {};
        SchedulePtr clone();

		/* @brief		updates the cashflows in this CMS leg with the float rate calculated from swap index par rates
		*  @param[in]	cmsStaticData		Contains the CMS static data parameters ( swap index, maturity, multiplier )
		*/
		void initializeDataProviderWithCMSParameters( DataProvider& dataProvider, const std::shared_ptr<CMSStaticData>& cmsStaticData ) const;

    protected:
        virtual void createCashflows();
 
    private:

		/* @brief		A private helper which constructs a default swap expression LVB used for computing swap par-rates
		*/
		LabelValueBlock setupSwapExpressionLVBForCMS() const;

		/* @brief		A pivate helper which constructs a temporary interest rate swap (IRS) from the supplied SwapGeneratorName, and uses the swap to calculate a par rate.
		*  @param[in]	valuationSettingsLVB	The valuationSettingsLVB used to price the IRS
		*  @param[in]	swapGeneratorName	The name of the swapGenerator to use when constructing the IRS
		*  @param[in]	irsEffectiveDate	The effective (start) date of the IRS
		*  @param[in]	swapIndexMaturity	A string containing the IRS maturity tenor, e.g '5Y'
		*  @returns		The IRS par-rate
		*/
		double calculateUnderlyingParRate( const LabelValueBlock& valuationSettingsLVB, const AQLString& swapGeneratorName, const AQLDate& underlyingEffectiveDate, const AQLString& underlyingMaturity ) const;
	

	};

}

