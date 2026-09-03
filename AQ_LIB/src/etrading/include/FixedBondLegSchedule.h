/*
 * @brief			Class the defines the fixed leg schedule 
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#pragma once

#include "BondSchedule.h"

namespace etrading
{
    class FixedBondLegSchedule : public BondSchedule
    {
    public:
		FixedBondLegSchedule(const std::string& instanceName);
		FixedBondLegSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName);
		FixedBondLegSchedule(const FixedBondLegSchedule& rhs);
		virtual ~FixedBondLegSchedule() {};
        SchedulePtr clone();

		//Override
		double getFixedRate() const { return fixedRate_; };
		//Override
		//Override
		double calculateAccruedInterest(const DataProvider& dataProvider, bool nativeCurrencyPV) const;

		//Override
		void calculateScheduleDates();

   	protected:

		//Override
		void createUpfrontCashflow(const LADate& paymentDate, double leverage);
		//Override
        void createCashflows();
		//Override
		void initialize(const LabelValueBlock& scheduleLVB);

		//Override
		void populateAccrualStartDates(const LabelValueBlock& scheduleLVB);

	private:
		double fixedRate_;

		BondYieldParameters bondYieldParameters_;

	};

}

