//
// @File: SerializeContainedData.h
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.



#pragma once

#include <vector>
#include <memory>
#include <string>
#include <utility>

#include <rapidjson/document.h>

#include "CoreEnumerations.h"
#include "DataHolder.h"
#include "SerializationResult.h"
#include "ETradingException.h"
#include "DataSchema.h"
#include "ContainerUtilities.h"

namespace etrading
{
    class SchemaObject;
    typedef std::vector<std::vector<Variant>> ContainedDataType;

    rapidjson::Value VariantToJSONValue( const Variant& cv, rapidjson::Document::AllocatorType& allocator );
    rapidjson::Document createJSONFromString( const std::string& inputData );
    rapidjson::Document createJSONFromFile( const std::string& fileName );
	std::string createStringFromJSON( const rapidjson::Document& doc );
	std::string createStringFromJSONValue( const rapidjson::Value& val );

    // TODO: add horizontal (by row) processing instead of by column?
    std::string serializeDataSchemaToJSONString(	const DataSchema& sk,
            const ContainedDataType& cd,
            const std::vector<int>& relevantIdxs );

    std::pair<DataSchema, std::vector<std::vector<Variant>>> createDataSchemaWithDataFromJSON( const rapidjson::Value& inputValue );

    std::vector<Variant> createVariantVector( const rapidjson::Value& inputValue, const etrading::ContainedTypeEnum targetType );

    class SchemaObject; // fwd declare for include in class
    std::string createJSONStringFromSchemaObject(	const SchemaObject& co,
            bool addClosure = false,
            bool allowRecursive = false );

    struct CurveBuildProperties;
    CurveBuildProperties createCurveBuildPropertiesFromJsonArray( const rapidjson::Value& skVariableValuesArray, const rapidjson::Value* interpolationParameters, const std::string& cbp_name );

    std::vector<double> populateDoubleVector( const rapidjson::Value& skDoubleArray );
    std::vector<boost::gregorian::date> populateDateVector( const rapidjson::Value& skDateArray );
    std::vector<std::string> populateStringVector( const rapidjson::Value& skStringArray );

}



