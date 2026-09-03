#pragma once

#include "Variant.h"
#include "DataSchema.h"

namespace etrading
{

    class HasVariantMatrixAccess
    {
    public:
        virtual VariantMatrix getVariantMatrix() const = 0;
    };

	struct CanGenerateDataSchema
	{
		virtual DataSchema generateDataSchema() const = 0;
	};

}