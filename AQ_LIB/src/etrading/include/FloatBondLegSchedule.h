#pragma once

#include "FloatSchedule.h"

namespace etrading
{
	class FloatBondLegSchedule : public FloatSchedule
	{
	public:
		FloatBondLegSchedule(const std::string& instanceName);
		FloatBondLegSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName);
		FloatBondLegSchedule(const FloatBondLegSchedule& rhs);
		virtual ~FloatBondLegSchedule() {};
		SchedulePtr clone();

		double getQuotedMargin() const { return quotedMargin_; };
		void setQuotedMargin(const double& quotedMargin) { quotedMargin_ = quotedMargin; };


		//Override
		DateVector generateAccrualAndPaymentSchedules();

		//Override
		double calculateAccruedInterest(const DataProvider& dataProvider, bool nativeCurrencyPV) const;

	private:

		//Fixed spread added to the float leg
		double quotedMargin_;

	};

}

