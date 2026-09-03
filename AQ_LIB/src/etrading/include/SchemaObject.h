#pragma once

//
// @File: SchemaObject.h
// @Description: This object represents a fully exposed CachedObject. The aim of exposing this object is to allow fully flexible data manipulation by the client
// (e.g. changing data sizes/formats and number of arguments from the client)
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.


#include <vector>
#include <string>

#include "DataHolder.h"
#include "HasInstance.h"
#include "Variant.h"
#include "SerializationResult.h"

namespace etrading
{

    // @SchemaObject: this object exposes all the methods of the implementation object (DataHolder)
    // thus allowing for complete freedom in data control
    class SchemaObject : public DataHolder
    {
    public:
        SchemaObject( const etrading::CachedObjectEnum, const std::string& name );
        SchemaObject( const SchemaObject& );
        SchemaObject( const DataHolder& data, const std::string& name );
        // for the conversion of objects that hold data directly in the DataHolder

		// implementation of SerializeInterface
		// Bring in the overloaded versions of serialize from IsLWOObject
		using IsLWOObject::serialize;
        virtual SerializationResult serialize(	const serialize::SerializationMethodEnum method,
                                                const serialize::SerializationTargetEnum target,
                                                const std::string& targetInfo,
                                                std::vector<std::string>& variableNames,
                                                std::vector<Variant>& variableValues ) const;

        // only the shared schemaNames are absorbed into existing schema names with the same names
        // if attempted to absorb all schemas then all names must exist in the current SchemaObject object
        void absorbDataSchema( const SchemaObject&, std::vector<std::string>* schemaNames =  nullptr );

        // the current DataSchemas are expanded to include the SchemaObject's schemas
        // unless for schemas where the same name already exists in which case it will absorb (as above)
        void adoptDataSchema( const SchemaObject& );

		virtual const SchemaObject toSchemaObject() const; // quirk of inheritance - avoids the SchemaObject layer

		// Adds a dependent SchemaObject to our vector of nested Schema Objects
		virtual void addNestedSchemaObject( const SchemaObject& nestedObject );


    private:
		// Contains the nested SchemaObjects which this SchemaObject depends on
		std::vector<SchemaObject> nestedSchemaObjects_;
    };

};