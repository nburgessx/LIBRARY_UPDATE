#pragma once


#include <vector>
#include <memory>
#include <string>
#include <utility>

#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>

#include <rapidjson/document.h>

#include "CoreEnumerations.h"
#include "DataHolder.h"
#include "SerializationResult.h"
#include "ETradingException.h"
#include "StaticStructureStore.h"
#include "DataSchema.h"
#include "ContainerUtilities.h"
#include "SchemaObject.h"
#include "HasSchemaObjectConverter.h"
#include "SerializeContainedData.h"
#include "SerializationResultWrapper.h"

using rapidjson::Document;
using rapidjson::Value;
using rapidjson::StringRef;

namespace etrading
{

    template<typename T>
    SerializationResult  serializeInstance(
        const T& instance,
        const serialize::SerializationMethodEnum methodType,
        const serialize::SerializationTargetEnum targetType,
        const std::string& targetInfo,
        const std::vector<std::string> variableNames,
        const std::vector<Variant> variableValues,
        const std::vector<std::shared_ptr<etrading::SchemaObject>>* containedObjects = nullptr )
    {

        BOOST_STATIC_ASSERT( boost::is_base_of<Cacheable, T>::value );

        const SchemaObject schemaObject = instance.toSchemaObject(); // everything is now in one

        bool allowRecursive = !( containedObjects == nullptr );

        if( methodType == serialize::JSON )
        {
            std::string startingString = createJSONStringFromSchemaObject( schemaObject, false, allowRecursive );
            if( !variableNames.empty() && variableValues.size() == variableNames.size() )
            {
                for( unsigned int varCounter = 0u; varCounter < variableNames.size(); varCounter++ )
                {
                    std::string toAddHere = ", \"" + variableNames[ varCounter ] + "\" : ";
                    const Variant& cv = variableValues[ varCounter ];
                    toAddHere += cv.toString( true );
                    startingString += toAddHere;
                }
            }

            if( !allowRecursive )
            {
                startingString += "}";
            }
            else
            {
                startingString += ", \"CONTAINED_OBJECTS\" : [";
                std::for_each( containedObjects->cbegin(), containedObjects->cend(),
                               [&startingString] ( const std::shared_ptr<SchemaObject>& schemaObjectPtr )
                {
                    if( schemaObjectPtr != nullptr )
                    {
                        std::string jsonString = createJSONStringFromSchemaObject( ( *schemaObjectPtr.get() ), true );
                        startingString += jsonString;
                    }
                } );
                startingString += "] } ";
            }

            SerializationResultWrapper wrap;
            wrap.jsonDocument = std::shared_ptr<Document>( new Document() );
            wrap.jsonDocument.get()->SetObject();
            wrap.jsonDocument.get()->Parse<0>( startingString.c_str() );

            return std::move( SerializationResult( methodType, targetType, wrap, targetInfo ) );
        }
        { std::ostringstream aqMsg25;
aqMsg25 << "serializeContainedData not implemented for serializaiton method: " << StaticStructureStore::getInstance().toString( methodType ).c_str(); AQ_THROW( aqMsg25.str() ); }
    };


}