#pragma once

#include <string>
#include <vector>

#include "LegGenerator.h"
#include "IsAQObject.h"
#include "SchemaObject.h"
#include "CoreEnumerations.h"

namespace etrading
{

    class SwapGenerator : public IsAQObject 
    {
	public:
		SwapGenerator(const std::string& instanceName, const LegGenerator& leg1, const LegGenerator& leg2);
   		SwapGenerator(const SwapGenerator& rhs);
		virtual ~SwapGenerator() {}

        const SchemaObject toSchemaObject() const;
        AQLStringMatrix viewInputParameters() const;
        LegGenerator getLegGenerator(size_t i) const;

        static std::vector<std::string> lvbKeys()
        {
            const std::string arr[] =
            {
                IRS_KEY::LEG_TYPE
                , IRS_KEY::LEVERAGE
                , MARKET_KEY::FORECAST_CURVE //Optional for fixed leg, compulsory for float leg
                , IRS_KEY::COMPOUND_METHOD //Optional for both fixed leg and float leg
                , IRS_KEY::NOTIONAL_EXCHANGE
                , MARKET_KEY::DISCOUNT_CURVE
                , IRS_KEY::VALUATION_CURRENCY
                , IRS_KEY::CURRENCY

                , IRS_KEY::ACCRUALFREQUENCY
                , IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT
                , IRS_KEY::ACCRUALCALENDAR
                , IRS_KEY::ACCRUALDAYCOUNT

                , IRS_KEY::PAYMENTFREQUENCY
                , IRS_KEY::PAYMENTBUSINESSDAYADJUSTMENT
                , IRS_KEY::PAYMENTCALENDAR
                , IRS_KEY::PAYMENTLAG

                , IRS_KEY::FIRSTSTUBDATE
                , IRS_KEY::LASTSTUBDATE
                , IRS_KEY::ROLLDAY
                , IRS_KEY::STUBTYPE
                , IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT
                , IRS_KEY::FIXINGCALENDAR
                , IRS_KEY::FIXINGLAG
                , IRS_KEY::FIXINGADVANCEORARREAR
                , IRS_KEY::IS_FWD_INTER

                ,IRS_KEY::FIRSTSTUBCURVEINDEX
                ,IRS_KEY::LASTSTUBCURVEINDEX

				,CMS_KEY::CMS_GENERATOR_NAME1
				,CMS_KEY::CMS_INDEX_MATURITY1
				,CMS_KEY::CMS_INDEX_MULTIPLIER1

				,CMS_KEY::CMS_GENERATOR_NAME2
				,CMS_KEY::CMS_INDEX_MATURITY2
				,CMS_KEY::CMS_INDEX_MULTIPLIER2

                // These keys are for relaxing the prefix "ACCRUAL"    
                , IRS_KEY::FREQUENCY
                , IRS_KEY::BUSINESSDAYADJUSTMENT
                , IRS_KEY::CALENDAR
                , IRS_KEY::DAYCOUNT

				// For credit risky legs
				, IRS_KEY::PAYMENT_TRIGGER

				// Keys specific to Inflation instruments
				, IRS_KEY::INFLATION_RESET_TYPE

				// fields for Bond Schedules
				, BOND_KEY::CALCULATION_TYPE
				, BOND_KEY::EX_DIVIDEND_TENOR
				, BOND_KEY::EX_DIVIDEND_BUSINESSDAYADJUSTMENT

			};

            std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
            return expectedKeys;
        }


        static std::vector<std::string> expressionLvbKeys()
        {
            const std::string arr[] =
            {
                IRS_KEY::EFFECTIVE_DATE
                ,IRS_KEY::MATURITY_DATE
                ,SWAP_EXPRESSION_KEY::PAY_RECEIVE1
                ,SWAP_EXPRESSION_KEY::PAY_RECEIVE2
                ,SWAP_EXPRESSION_KEY::NOTIONAL1
                ,SWAP_EXPRESSION_KEY::NOTIONAL2
                ,SWAP_EXPRESSION_KEY::RATE_OR_SPREAD1
                ,SWAP_EXPRESSION_KEY::RATE_OR_SPREAD2
                ,SWAP_EXPRESSION_KEY::FIRSTFIXING1
                ,SWAP_EXPRESSION_KEY::LASTFIXING1
                ,SWAP_EXPRESSION_KEY::FIRSTFIXING2
                ,SWAP_EXPRESSION_KEY::LASTFIXING2
                ,SWAP_EXPRESSION_KEY::IS_FWD_INTER1
                ,SWAP_EXPRESSION_KEY::IS_FWD_INTER2

                ,IRS_KEY::PAY_RECEIVE
                ,IRS_KEY::NOTIONAL
                ,IRS_KEY::FIRSTFIXING
                ,IRS_KEY::LASTFIXING
                ,IRS_KEY::IS_FWD_INTER

				,CMS_KEY::CMS_GENERATOR_NAME1
				,CMS_KEY::CMS_INDEX_MATURITY1
				,CMS_KEY::CMS_INDEX_MULTIPLIER1

				,CMS_KEY::CMS_GENERATOR_NAME2
				,CMS_KEY::CMS_INDEX_MATURITY2
				,CMS_KEY::CMS_INDEX_MULTIPLIER2

				,TRS_KEY::BOND_NAME
				,TRS_KEY::ASSET_PERFORMANCE
            };
            std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
            return expectedKeys;
        }

	protected:
		std::vector<LegGenerator> legGenerators_;

	};

	typedef std::shared_ptr< SwapGenerator > SwapGeneratorPtr;
}
