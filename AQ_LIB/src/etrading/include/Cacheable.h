//
// @File: Cacheable.h
// @Description: This is a base class that handles the core data of derived base classes (e.g. serialization/deserialization)
// by default none of its functionality should be exposed on classes using this (either through protected/private inheritance or
// aggregation). The exception in this library is FreeObject as it requires full data flexibility.
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.

#pragma once

#include "HasInstance.h"
#include "CoreEnumerations.h"
#include "SerializeInterface.h"

namespace etrading
{

    class Cacheable
        : private HasConstInstance<etrading::CachedObjectEnum>
        , public SerializeInterface
    {
    public:
        Cacheable( const etrading::CachedObjectEnum enumInput );
        const etrading::CachedObjectEnum getEnumType() const;
        // TODO : add a DOWNCAST method based on the enum
    };

}