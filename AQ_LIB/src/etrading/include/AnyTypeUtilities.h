// AnyTypeUtilities.h

/*
 * @brief			Utility file for Boost Variant or Any Type Helpers
 */

#pragma once
#include "AQLCoreTemplateType.h"

namespace etrading
{
    // Method to cast boost any variant type to a string
    // anyValue             The anyType variant to cast
    // doublePrecision      The double precision to use when casting from double AnyTypes to string - defualts to 10 d.p.
    std::string fromAnyTypeToString( const AnyType& anyValue, const int doublePrecision = 10 );
}
