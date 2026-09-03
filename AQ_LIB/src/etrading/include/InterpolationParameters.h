#pragma once

#include <vector>


#include "CoreEnumerations.h"
#include "HasInstance.h"
#include "HasDataSchemaGenerator.h"
#include "HasSchemaObjectConverter.h"

namespace etrading
{
    struct InterpolationParameters : public HasDataSchemaGenerator, public HasSchemaObjectConverter
    {
        virtual const InterpolationEnum getType() const = 0;
    };

    struct MonotoneConvexParameters : public InterpolationParameters
    {
        MonotoneConvexParameters( const std::vector<double>& fdiscrete,
                                  const std::vector<double>& f,
                                  const unsigned int size );
        const std::vector<double> fdiscrete_;//!< Valarray to save the fdiscrete at index
        const std::vector<double> f_;		//!< Valarray to save the f at index
        const unsigned int size_;	//!< initial data numbers
        const InterpolationEnum getType() const;
        virtual const SchemaObject toSchemaObject() const;
        virtual DataSchema generateDataSchema() const;
    };

}