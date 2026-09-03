#pragma once

#include <string>

#include "Cacheable.h"
#include "HasSchemaObjectConverter.h"
#include "HasInstance.h"
#include "SerializationResult.h"
#include "CoreEnumerations.h"

namespace etrading
{

    struct IsLWOObject : public Cacheable, public HasSchemaObjectConverter, public HasName
    {
        IsLWOObject( const std::string& objectName, const CachedObjectEnum enumValue );
        virtual SerializationResult serialize(	const serialize::SerializationMethodEnum method,
                                                const serialize::SerializationTargetEnum target,
                                                const std::string& targetInfo,
                                                std::vector<std::string>& variableNames,
                                                std::vector<Variant>& variableValues ) const;

		virtual SerializationResult serialize( const serialize::SerializationMethodEnum method,
											   const serialize::SerializationTargetEnum target,
											   const std::string& targetInfo = "" ) const;

    };

}

