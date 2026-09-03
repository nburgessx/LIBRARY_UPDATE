#pragma once

#include "DataSchema.h"
#include "SchemaObject.h"

namespace etrading
{

    class HasDataSchemaGenerator
    {
    public:
        virtual DataSchema generateDataSchema() const = 0;
    };

}