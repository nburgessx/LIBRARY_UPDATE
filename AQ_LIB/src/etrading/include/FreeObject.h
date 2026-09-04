#pragma once

//
// @File: FreeObject.h
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
#include "SerializeInterface.h"
#include "HasInstance.h"
#include "Variant.h"

namespace etrading
{

    // @FreeObject: this object exposes all the methods of the implementation object (DataHolder)
    // thus allowing for complete freedom in data control
    class FreeObject
        : public DataHolder
    {
    public:
        FreeObject( const std::string& name );
        FreeObject( const FreeObject& rhs );
        FreeObject& operator+=( const FreeObject& rhs );
        
		// implementation of SerializeInterface
		// Bring in the overloaded versions of serialize from IsAQObject
		using IsAQObject::serialize;
        virtual SerializationResult serialize(	const serialize::SerializationMethodEnum method,
                                                const serialize::SerializationTargetEnum target,
                                                const std::string& targetInfo,
                                                std::vector<std::string>& variableNames,
                                                std::vector<Variant>& variableValues ) const;
		virtual const SchemaObject toSchemaObject() const; // quirk of inheritance 
    private:
        FreeObject& operator=( const FreeObject& rhs ) = delete;
    };

};