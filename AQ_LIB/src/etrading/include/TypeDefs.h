// TypeDefs.h

#ifndef ETRADING_TYPEDEFS_TYPEDEFS_H
#define ETRADING_TYPEDEFS_TYPEDEFS_H

// Includes: Standard Library
#include <string>
#include <vector>

#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>

#include "NamedEntity.h"
#include "CoreEnumerations.h"

namespace ETrading
{
    namespace TypeDefs
    {

        typedef std::vector< std::vector< std::string > > MATRIX_STRING;
        typedef std::vector< std::string > VECTOR_STRING;


    }
}

#endif ETRADING_TYPEDEFS_TYPEDEFS_H