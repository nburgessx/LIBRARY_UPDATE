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
