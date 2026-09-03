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
    class FixedStaticData : public LegStaticData
    {
    public:

        FixedStaticData( const LabelValueBlock& marketDataLVB );
        FixedStaticData(const FixedStaticData& rhs);
        virtual ~FixedStaticData() {};

        LegStaticDataPtr clone();

    };

}
