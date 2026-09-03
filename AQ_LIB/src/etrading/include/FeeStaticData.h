#pragma once

#include <string>
#include "LegStaticData.h"

namespace etrading
{
    class FeeStaticData : public LegStaticData
    {
    public:

        FeeStaticData( const LabelValueBlock& marketDataLVB );
        FeeStaticData(const FeeStaticData& rhs);
        virtual ~FeeStaticData() {};

        LegStaticDataPtr clone();

        static std::vector<std::string> lvbKeys()
        {
            const std::string arr[] =
            {
	            IRS_KEY::LEG_TYPE
                , MARKET_KEY::DISCOUNT_CURVE
                , IRS_KEY::VALUATION_CURRENCY
                , IRS_KEY::CURRENCY
            };
            std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
            return expectedKeys;
        }
    };

}
