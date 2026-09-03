/*
 * @brief			Class the defines the float leg schedule 
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once

#include "Schedule.h"

namespace etrading
{
    class FraSchedule : public Schedule
    {
    public:
		FraSchedule(const std::string& instanceName);
		FraSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName);
		FraSchedule(const FraSchedule& rhs);
		virtual ~FraSchedule() {};
        SchedulePtr clone();

		double getFixedRate() const;

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

		// These keys are not mandatory, however keys not in this list will be rejected and result in error messages 
		static std::vector<std::string> lvbKeys()
		{
			const std::string arr[] =
			{
				IRS_KEY::NOTIONAL
                , IRS_KEY::NOTIONAL_EXCHANGE
				, IRS_KEY::LEVERAGE

				, IRS_KEY::PAY_RECEIVE

				, IRS_KEY::EFFECTIVE_DATE
				, IRS_KEY::MATURITY_DATE

				, IRS_KEY::FREQUENCY
				, IRS_KEY::DAYCOUNT
				, IRS_KEY::BUSINESSDAYADJUSTMENT
				, IRS_KEY::CALENDAR
				, IRS_KEY::FIXINGLAG
                , IRS_KEY::STRIKE_RATE
				, IRS_KEY::FIRSTSTUBDATE
				, IRS_KEY::LASTSTUBDATE
				, IRS_KEY::ROLLDAY
				, IRS_KEY::STUBTYPE
				, IRS_KEY::FRA_STYLE

				// allow more keys to be more flexible
				, IRS_KEY::ACCRUALFREQUENCY
				, IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT
				, IRS_KEY::ACCRUALCALENDAR
				, IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT
				, IRS_KEY::FIXINGCALENDAR
				, IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT
				, IRS_KEY::FIXINGCALENDAR

				// support FLOAT prefix				
				, IRS_KEY::FLOAT_FREQUENCY
				, IRS_KEY::FLOAT_DAYCOUNT
				, IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT
				, IRS_KEY::FLOAT_CALENDAR
				, IRS_KEY::FLOAT_FIXINGLAG
				, IRS_KEY::FLOAT_FIRSTSTUBDATE
				, IRS_KEY::FLOAT_LASTSTUBDATE
				, IRS_KEY::FLOAT_ROLLDAY
				, IRS_KEY::FLOAT_STUBTYPE
				, IRS_KEY::FLOAT_ACCRUALFREQUENCY
				, IRS_KEY::FLOAT_ACCRUALBUSINESSDAYADJUSTMENT
				, IRS_KEY::FLOAT_ACCRUALCALENDAR
				, IRS_KEY::FLOAT_FIXINGBUSINESSDAYADJUSTMENT
				, IRS_KEY::FLOAT_FIXINGCALENDAR
				, IRS_KEY::FLOAT_FIXINGBUSINESSDAYADJUSTMENT
				, IRS_KEY::FLOAT_FIXINGCALENDAR

			};

			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

    protected:
        void createCashflows();
 
    private:
        double strikeRate_;
        double floatRate_;
		FraStyleEnum fraStyle_;

	};

}

