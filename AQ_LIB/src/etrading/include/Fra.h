#pragma once

#include "FraSchedule.h"
#include "FloatLeg.h"

namespace etrading
{
    class Fra : public FloatLeg 
    {
    public:

        Fra(const std::string& instanceName);
		Fra(const std::string& instanceName, const LabelValueBlock& fraLVB, const SchedulePtr& schedule={});

        Fra(const Fra& rhs);
		virtual ~Fra() {}

        LegPtr clone();

		ScheduleTypeEnum getType() const;

		//Return the fixed strikeRate that make the FRA PV zero
		double fraRate(const LabelValueBlock& valuationSettingsLVB);

		//Return Future Price from FRA rate based on the meanReversion and volatility (convexity adjustment is calculated internally)
		double fraRateToFuturePrice(const LabelValueBlock& valuationSettingsLVB, const double& meanReversion, const double& volatility);

		//Return Future Price from FRA rate based on the given convexity adjustment
		double fraRateToFuturePrice(const LabelValueBlock& valuationSettingsLVB, const double& convexityAdjustment);

		// These keys are not mandatory, however keys not in this list will be rejected and result in error messages 
		static std::vector<std::string> descriptionLVBKeys()
		{
			const std::string arr[] =
			{
				IRS_KEY::LEG_TYPE
				, MARKET_KEY::FORECAST_CURVE 
				, MARKET_KEY::DISCOUNT_CURVE
				, IRS_KEY::CURRENCY

		        // fields for float 
				, IRS_KEY::FIRSTFIXING
                , IRS_KEY::LASTFIXING
                , IRS_KEY::FIRSTSTUBCURVEINDEX
                , IRS_KEY::LASTSTUBCURVEINDEX
				, IRS_KEY::IS_FWD_INTER //This is to allow user to change isFwdInter to true or false

			};
        
			std::vector< std::string > expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
			return expectedKeys;
		}

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

		// These keys are not mandatory, however keys not in this list will be rejected and result in error messages 
		static std::vector<std::string> lvbKeys()
		{
			std::vector<std::string> expectedKeys = descriptionLVBKeys();
			std::vector<std::string> scheduleKeys = FraSchedule::lvbKeys();
			expectedKeys.insert(expectedKeys.end(), scheduleKeys.begin(), scheduleKeys.end());

			return expectedKeys;
		}

	};

}


