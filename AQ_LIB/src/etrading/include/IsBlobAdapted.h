#pragma once

#include "Variant.h"


namespace etrading
{

    class isBlobAdapted
    {
    public:
        virtual DataBlob getDataBlob() const = 0;
    }

}
