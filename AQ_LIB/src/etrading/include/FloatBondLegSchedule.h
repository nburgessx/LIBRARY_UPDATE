/*
* @brief			Class the defines the fixed leg schedule
* @Created:		05 July 2016
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/
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

