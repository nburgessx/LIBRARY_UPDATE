#pragma once

#include "Leg.h"
#include "FeeSchedule.h"
#include "FeeStaticData.h"

namespace etrading
{
    class FeeLeg : public Leg
    {
    public:

		FeeLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule={});
        FeeLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule);
		FeeLeg(const FeeLeg& rhs);
		virtual ~FeeLeg() {}

        LegPtr clone();

        double annuityWithNotional(DataProvider& dataProvider);
		double pv( DataProvider& dataProvider, bool nativeCurrencyPV=false, bool updateCurveData=true);
		//double dv01(const LAString& curveCollection);

        ScheduleTypeEnum getType() const;

		/*	@brief	Initialise the DataProvider object from the supplied curveCollection and fixngtable
		*	@param[out]		dataProvider		A reference to the DataProvider object which will be populated
		*	@param[in]		updateCurveData		Whether to update the curve data inside the dataProvider
		*/
		void initializeDataProvider( DataProvider& dataProvider, bool updateCurveData=true);

        void flipPayerReceiver();

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

		static std::vector<std::string> legLVBWithoutScheduleKeys()
		{
			std::vector<std::string> expectedKeys = FeeStaticData::lvbKeys();
			return expectedKeys;
		}

		static std::vector<std::string> lvbKeys()
		{
			std::vector<std::string> expectedKeys = FeeStaticData::lvbKeys();
			std::vector<std::string> scheduleKeys = FeeSchedule::lvbKeys();
			expectedKeys.insert(expectedKeys.end(), scheduleKeys.begin(), scheduleKeys.end());

            return expectedKeys;
		}

    };


}

