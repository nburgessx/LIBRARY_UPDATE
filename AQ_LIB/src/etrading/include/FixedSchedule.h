/*
 * @brief			Class the defines the fixed leg schedule 
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#pragma once

#include "Schedule.h"

namespace etrading
{
    class FixedSchedule : public Schedule
    {
    public:
		FixedSchedule(const std::string& instanceName);
		FixedSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName);
        //Bespoke Schedule
        FixedSchedule(const std::string& instanceName, const LabelValueBlock& bespokeScheduleProperties, const std::vector<LabelValueBlock>& cashflowLVBs , const BespokeScheduleTypeEnum& bespokeScheduleType);

		FixedSchedule(const FixedSchedule& rhs);
		virtual ~FixedSchedule() {};
        SchedulePtr clone();

        const std::string getCashflowSchemaName() const;
        const DataSchema generateCashflowSchema(const std::string& schemaName="") const;
        std::map<std::string, std::vector<std::string>> getCashflowDataMap() const;

		double getFixedRate() const;

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

        static std::vector<std::string> lvbKeys()
		{
			const std::string arr[] =
			{
                IRS_KEY::FIXED_FREQUENCY
				, IRS_KEY::FIXED_BUSINESSDAYADJUSTMENT
				, IRS_KEY::FIXED_CALENDAR
                , IRS_KEY::FIXED_DAYCOUNT
				, IRS_KEY::FIXED_ACCRUALFREQUENCY
				, IRS_KEY::FIXED_ACCRUALBUSINESSDAYADJUSTMENT
				, IRS_KEY::FIXED_ACCRUALCALENDAR
				, IRS_KEY::FIXED_ACCRUALDAYCOUNT
				, IRS_KEY::FIXED_PAYMENTFREQUENCY
				, IRS_KEY::FIXED_PAYMENTBUSINESSDAYADJUSTMENT
				, IRS_KEY::FIXED_PAYMENTCALENDAR
				, IRS_KEY::FIXED_PAYMENTLAG
				, IRS_KEY::FIXED_FIRSTSTUBDATE
				, IRS_KEY::FIXED_LASTSTUBDATE
				, IRS_KEY::FIXED_ROLLDAY
				, IRS_KEY::FIXED_STUBTYPE
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			//The coreKeys are without FIXED prefix, should accept keys with or without prefix
			std::vector<std::string> coreKeys = Schedule::lvbKeys();
			expectedKeys.insert(expectedKeys.end(), coreKeys.begin(), coreKeys.end());

			return expectedKeys;
		}

	protected:
		void populateNotionalAndPaymentFreqEnum(const LabelValueBlock& scheduleLVB=LabelValueBlock());
		void createUpfrontCashflow(const LADate& paymentDate, double leverage);
        virtual void createCashflows();
		void createBespokeCashflows(const std::vector<LabelValueBlock>& cashflowLVBs); 

		void initialize(const LabelValueBlock& scheduleLVB);

	private:
        double fixedRate_;
	};

}

