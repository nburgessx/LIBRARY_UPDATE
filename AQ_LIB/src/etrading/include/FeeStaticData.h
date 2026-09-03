/*
 * @brief			Class the defines the leg market data
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

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
