/*
 * @brief			Class the defines the leg market data
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "FixedStaticData.h"

namespace etrading
{
    FixedStaticData::FixedStaticData( const LabelValueBlock& marketDataLVB ) : LegStaticData(marketDataLVB)
    {}

    LegStaticDataPtr FixedStaticData::clone()
    {
        LegStaticDataPtr data = LegStaticDataPtr(new FixedStaticData(*this));
        return data;
    }

    FixedStaticData::FixedStaticData(const FixedStaticData& rhs) : LegStaticData(rhs)
    {}


}

