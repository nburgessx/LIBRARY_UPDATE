#pragma once

#include "Leg.h"
#include "CMSSchedule.h"
#include "CMSStaticData.h"

namespace etrading
{
    class CMSLeg : public Leg
    {
    public:

        CMSLeg( const std::string& instanceName );
		CMSLeg( const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule={} );
        CMSLeg( const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule );
        CMSLeg( const CMSLeg& rhs);
		virtual ~CMSLeg() {}

        LegPtr clone();

        LabelValueBlock getInputParameters() const;

		double annuityWithNotional( DataProvider& dataProvider );

		/* @brief Calculates the PV of the CMSLeg, using the ConvexityAdjustment input
		*
		* @param[in]	swapName			Swap object name
		* @param[in]	dataProvider		The data provider
		* @param[in]	convexityAdjustment	The convexity adjustment which is added to the PV of the CMS leg.
		* @param[in]	updateCurveData		Optional flag, whether to update cashflows with curve data
		* @returns	The calculated PV value
		*/
		double pvUsingConvexityAdjustment(DataProvider& dataProvider, const double convexityAdjustment, bool updateCurveData=true );

        ScheduleTypeEnum getType() const;

		// Build the swap and update the float leg inside here?
		virtual void initializeDataProvider( DataProvider& dataProvider, bool updateCurveData=true);

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

		static std::vector<std::string> legLVBWithoutScheduleKeys()
		{
			std::vector<std::string> expectedKeys = Leg::lvbKeys();
			return expectedKeys;
		}

		static std::vector<std::string> lvbKeys()
		{
			std::vector<std::string> expectedKeys = CMSLeg::legLVBWithoutScheduleKeys();
			std::vector<std::string> scheduleKeys = CMSSchedule::lvbKeys();
			expectedKeys.insert( expectedKeys.end(), scheduleKeys.begin(), scheduleKeys.end() );
			return expectedKeys;
		}

	private:

        //Helper function to the public initializeDataProvider() functions
		void initializeDataProviderInternal( DataProvider& dataProvider, bool isFloatRateRequired, bool updateCurveData=true );
	};

}


