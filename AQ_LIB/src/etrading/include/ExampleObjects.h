#pragma once

#include <string>

#include "IsLWOObject.h"
#include "SchemaObject.h"

namespace etrading
{

    struct StandAlone : public IsLWOObject
    {
        StandAlone( const std::string& objectName );
        virtual const SchemaObject toSchemaObject() const;
        double d_;
    };

    struct BaseObject : public IsLWOObject
    {
        BaseObject( const std::string& objectName ) : IsLWOObject( objectName, EXAMPLE_BASE )
        {};
        virtual const SchemaObject toSchemaObject() const
        {
            SchemaObject schemaObject( EXAMPLE_BASE, getRefToName() );
            return schemaObject;
        };
        virtual operator std::string() const
        {
            return "BaseObject";
        };
    };

    struct DerivedObject : public BaseObject
    {
        DerivedObject( const std::string& objectName ) : BaseObject( objectName )
        {};
        virtual const SchemaObject toSchemaObject() const
        {
            SchemaObject schemaObject( EXAMPLE_BASE, getRefToName() );
            return schemaObject;
        };
        virtual operator std::string() const
        {
            return "DerivedObject";
        };
    };

}