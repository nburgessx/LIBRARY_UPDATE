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

