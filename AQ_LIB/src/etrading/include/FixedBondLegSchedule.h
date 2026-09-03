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

