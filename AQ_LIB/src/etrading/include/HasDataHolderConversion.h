#pragma once


#include "DataHolder.h"

namespace etrading
{

    /*
    The reason this class exists is that for non-public inheritance this operator is not available
    This happens usually for objects that keep their data in the DataHolder class and inherit from it in
    a private or protected way to block access to the methods of DataHolder. However these objects
    still go through a SchemaObject Object for serialization and need the conversion for creating the
    SchemaObject Object.
    */
    struct HasDataHolderConversion
    {
        operator const DataHolder& () const
        {
            return static_cast<const DataHolder&>( *this );
        };

        operator DataHolder& ()
        {
            return static_cast<DataHolder&>( *this );
        };

    };

}