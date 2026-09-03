#pragma once

//
// @File: HasSchemaObjectConverter.h
// @Description: This Class wraps around a boost::spirit::hold_any value to represent a data structure similar to an excel cell.
//  This class allows a set of different value types to be set and accessed from the same class instance. The types are as defined
// in etrading::ContainedEnumType.
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of Mizuho International.


namespace etrading
{

    class SchemaObject;

    class HasToSchemaObjectMethod
    {
    public:
        virtual const SchemaObject toSchemaObject() const = 0;
    };

    class HasSchemaObjectConverter : public HasToSchemaObjectMethod
    {
    public:
        HasSchemaObjectConverter( const SchemaObject& );
        // using HasSchemaObjectConverter::HasSchemaObjectConverter in derived forces the CTOR in the derived, no need for an implementation here...
    protected:
        HasSchemaObjectConverter() {};
    };

};
