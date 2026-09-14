

#include <utility>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include "Cacheable.h"
#include "StaticStructureStore.h"

// #include "SerializationResult.h"

namespace etrading
{
    Cacheable::Cacheable( const etrading::CachedObjectEnum enumInput )
        : HasConstInstance<etrading::CachedObjectEnum>( enumInput )
    {};

    const etrading::CachedObjectEnum Cacheable::getEnumType() const
    {
        return getRefToInstance();
    };
}