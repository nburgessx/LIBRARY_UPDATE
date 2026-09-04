//
// @Description: A virtual base class for the serialization

#pragma once

#include <string>
#include <vector>

#include "CoreEnumerations.h"
#include "Variant.h"
#include "SerializationResult.h"

namespace etrading
{
    // despite the (minor) performance loss we make this a virtual function because we are looking for
    // virtual calls on the nested elements
    struct SerializeInterface
    {
        /*
        @brief
        method = method of serialization (currently only JSON)
        target = target  of serialization (currently either STRING or FILE)
        targetInfo = information required to create the target (e.g. file name)
        variableNames = names of optional additional variables to serialize
        variableValues = values of optional additional variables to serialize
        SerializationResult  (cfr. SerializationResult .h)
        */
        virtual SerializationResult serialize(	const serialize::SerializationMethodEnum method,
                                                const serialize::SerializationTargetEnum target,
                                                const std::string& targetInfo,
                                                std::vector<std::string>& variableNames,
                                                std::vector<Variant>& variableValues ) const  = 0;
    };

}


