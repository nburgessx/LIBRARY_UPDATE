/*
 * @brief			Class the defines the fee leg schedule 
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
    class FeeSchedule : public Schedule
    {
    public:
		FeeSchedule(const std::string& instanceName);
		FeeSchedule(const std::string& instanceName, const std::vector<LabelValueBlock>& cashflowLVBs);
		FeeSchedule(const FeeSchedule& rhs);
		virtual ~FeeSchedule() {};

        SchedulePtr clone();

        //data serialization related
        const std::string getCashflowSchemaName() const;
        const DataSchema generateDataSchema(const std::string& schemaName="") const;
        const DataSchema generateCashflowSchema(const std::string& schemaName="") const;
        std::map<std::string, std::vector<std::string>> getCashflowDataMap() const;

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

		static std::vector<std::string> lvbKeys()
		{
			const std::string arr[] =
			{
                CASHFLOW_KEY::PAYMENT_DATE
				,FEE_KEY::AMOUNT
				,IRS_KEY::PAY_RECEIVE
			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
			return expectedKeys;
		}

	protected:
		void createBespokeCashflows(const std::vector<LabelValueBlock>& cashflowLVBs); 

    };

}

