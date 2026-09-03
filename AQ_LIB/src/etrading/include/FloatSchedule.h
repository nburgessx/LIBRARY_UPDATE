#pragma once

#include "Schedule.h"

namespace etrading
{
    class FloatSchedule : public Schedule
    {
    public:
		FloatSchedule(const std::string& instanceName);
		FloatSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName);
        //Bespoke Schedule
        FloatSchedule(const std::string& instanceName, const LabelValueBlock& bespokeScheduleProperties, const std::vector<LabelValueBlock>& cashflowLVBs, const BespokeScheduleTypeEnum& bespokeScheduleType);

		FloatSchedule(const FloatSchedule& rhs);
		virtual ~FloatSchedule() {};
        SchedulePtr clone();

        const std::string getCashflowSchemaName() const;
        const DataSchema generateCashflowSchema(const std::string& schemaName="") const;
        std::map<std::string, std::vector<std::string>> getCashflowDataMap() const;

		double getSpread() const;
        bool isVariableSpread() const;

		//Override
        void updateCashflowsCompoundingCoupons( const DataProvider& dataProvider );

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

		// These keys are not mandatory, however keys not in this list will be rejected and result in error messages 
		static std::vector<std::string> lvbKeys()
		{
			const std::string arr[] =
			{
				IRS_KEY::FLOAT_FREQUENCY
				, IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT
				, IRS_KEY::FLOAT_CALENDAR
                , IRS_KEY::FLOAT_DAYCOUNT
				, IRS_KEY::FLOAT_ACCRUALFREQUENCY
				, IRS_KEY::FLOAT_ACCRUALBUSINESSDAYADJUSTMENT
				, IRS_KEY::FLOAT_ACCRUALCALENDAR
				, IRS_KEY::FLOAT_ACCRUALDAYCOUNT
				, IRS_KEY::FLOAT_PAYMENTFREQUENCY
				, IRS_KEY::FLOAT_PAYMENTBUSINESSDAYADJUSTMENT
				, IRS_KEY::FLOAT_PAYMENTCALENDAR
				, IRS_KEY::FLOAT_PAYMENTLAG
				, IRS_KEY::FLOAT_FIRSTSTUBDATE
				, IRS_KEY::FLOAT_LASTSTUBDATE
				, IRS_KEY::FLOAT_ROLLDAY
				, IRS_KEY::FLOAT_STUBTYPE
				, IRS_KEY::FLOAT_FIXINGBUSINESSDAYADJUSTMENT
				, IRS_KEY::FLOAT_FIXINGCALENDAR
				, IRS_KEY::FLOAT_FIXINGLAG
				, IRS_KEY::FLOAT_FIRSTFIXINGLAG
				, IRS_KEY::FLOAT_FIXINGADVANCEORARREAR

			};
			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			//The coreKeys are without FLOAT prefix, should accept keys with or without prefix
			std::vector<std::string> coreKeys = Schedule::lvbKeys();
			expectedKeys.insert(expectedKeys.end(), coreKeys.begin(), coreKeys.end());

			return expectedKeys;
		}

    protected:
		// @brief: A helper method which initialises the FloatSchedule from a label-value block.
		void initialise( const LabelValueBlock& scheduleLVB );

		void populateNotionalAndPaymentFreqEnum(const LabelValueBlock& scheduleLVB=LabelValueBlock());
        void createUpfrontCashflow(const LADate& paymentDate, double leverage);
        virtual void createCashflows();
       	void createBespokeCashflows(const std::vector<LabelValueBlock>& cashflowLVBs); 
 
    private:
        double spread_;

	};

}

