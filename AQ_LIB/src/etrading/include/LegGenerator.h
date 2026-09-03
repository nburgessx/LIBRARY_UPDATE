/*
 * @brief			Class the defines the LegGenerator
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include "LabelValueBlock.h"
#include "CommonConstants.h"
#include "CoreEnumerations.h"
#include "Variant.h"
#include "DataSchema.h"
#include "SchemaObject.h"

namespace etrading
{
    class LegGenerator
    {
	public:

        LegGenerator();
        LegGenerator(const LabelValueBlock& LegGeneratorLVB);
		LegGenerator(const LegGenerator& rhs);
		virtual ~LegGenerator() {}

        const DataSchema generateDataSchema(const std::string& schemaName="") const;
        const SchemaObject toSchemaObject() const;
        void toSchemaObject(SchemaObject& schemaObject, const std::string& schemaName) const;
        std::map<std::string, Variant> getDataMap() const;

        LabelValueBlock getInputParameters() const;

	protected:
   		LabelValueBlock inputParameters_;
	};

}
