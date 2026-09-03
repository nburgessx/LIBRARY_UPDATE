/*
 * @brief			Class the defines the fixed leg
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once

#include "Leg.h"
#include "FixedSchedule.h"
#include "FixedStaticData.h"

namespace etrading
{
    class FixedLeg : public Leg
    {
    public:

		FixedLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule={});
        FixedLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule);
		FixedLeg(const FixedLeg& rhs);
		virtual ~FixedLeg() {}
        
        LegPtr clone();

        LabelValueBlock getInputParameters() const;
		double pv( DataProvider& dataProvider, bool nativeCurrencyPV=false, bool updateCurveData=true);

		//double dv01(const LAString& curveCollection);
        ScheduleTypeEnum getType() const;

		/*	@brief	Initialise the DataProvider object from the supplied curveCollection and fixngtable
		*	@param[out]		dataProvider		A reference to the DataProvider object which will be populated
		*	@param[in]		updateCurveData		Whether to update the curve data inside the dataProvider
		*/
		virtual void initializeDataProvider( DataProvider& dataProvider, bool updateCurveData=true);

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;
		
		static std::vector<std::string> legLVBWithoutScheduleKeys()
		{
			std::vector<std::string> expectedKeys = Leg::lvbKeys();
			return expectedKeys;
		}

		static std::vector<std::string> lvbKeys()
		{
			std::vector<std::string> expectedKeys = FixedLeg::legLVBWithoutScheduleKeys();
			std::vector<std::string> scheduleKeys = FixedSchedule::lvbKeys();
			expectedKeys.insert(expectedKeys.end(), scheduleKeys.begin(), scheduleKeys.end());
			return expectedKeys;
		}

	protected:

		const double pvCalc( const DataProvider& dataProvider, bool nativeCurrencyPV ) const;

	};


}

