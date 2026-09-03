//
// @File: SchemaObject.cpp
// @Description: Implementation of SchemaObject
// @Created: 04 April 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.

#include <boost/format.hpp>
#include <rapidjson/document.h>

#include "ObjectUtilities.h"
#include "SchemaObject.h"
#include "CoreEnumerations.h"
#include "ContainerUtilities.h"
#include "SerializeContainedData.h"
#include "SerializationResultWrapper.h"
#include "SerializationUtilities.h"


namespace etrading
{

    SchemaObject::SchemaObject( const etrading::CachedObjectEnum enumValue, const std::string& name )
        :	DataHolder( enumValue, name )
    {};

    SchemaObject::SchemaObject( const SchemaObject& rhs )
        : DataHolder( rhs.getEnumType(), rhs.getRefToName() ), nestedSchemaObjects_( rhs.nestedSchemaObjects_ )
    {
        assignData( rhs );
    };

    SchemaObject::SchemaObject( const DataHolder& data, const std::string& name )
        : DataHolder( data.getEnumType(), name ) 
    {
        assignData( data );
    }

    SerializationResult SchemaObject::serialize( const serialize::SerializationMethodEnum method,
                                               const serialize::SerializationTargetEnum target,
                                               const std::string& targetInfo,
                                               std::vector<std::string>& variableNames,
                                               std::vector<Variant>& variableValues ) const
    {
        // we can set the name here
        variableNames.emplace_back( SchemaObject::OBJECT_NAME_LABEL );
        variableValues.emplace_back( Variant( getRefToName() ) );

        if( method == serialize::JSON )
        {
            // TODO: make it more efficient by just using rapidjson objects ...
            std::string startingString = createJSONStringFromSchemaObject( *this, false, false );

			if ( ! nestedSchemaObjects_.empty() )
			{
				// Insert a JSON array element "NESTED_OBJECTS": [ ... ]
				// and populate by calling serialize() on each nested schema object
				std::string nestedString = ", \"" +  std::string( NESTED_OBJECTS ) + "\" : [ ";

				for ( size_t i=0; i < nestedSchemaObjects_.size(); i++ )
				{
					SerializationResult nestedResult = nestedSchemaObjects_[i].serialize( method, serialize::STRING );
					nestedString += nestedResult;

					if(  ( i + 1 ) < nestedSchemaObjects_.size() )
					{
						// Place a comma between each nested JSON document, apart from the final document
						nestedString += ",";
					}
				}
				// Close off the NESTED_OBJECTS array
				nestedString += " ] ";

				startingString += nestedString;
			}

            if( !variableNames.empty() && variableValues.size() == variableNames.size() )
            {
                for( unsigned int varCounter = 0u; varCounter < variableNames.size(); varCounter++ )
                {
                    std::string toAddHere = ( boost::format( ", \"%s\" : " ) % variableNames[varCounter].c_str() ).str();
                    const Variant& cv = variableValues[ varCounter ];
                    toAddHere += cv.toString( true );
                    startingString += toAddHere;
                }
            }
            startingString += "}";

            SerializationResultWrapper wrap;
            wrap.jsonDocument = std::shared_ptr<rapidjson::Document>( new rapidjson::Document() );
            wrap.jsonDocument.get()->SetObject();
            wrap.jsonDocument.get()->Parse<0>( startingString.c_str() );
            
            // Ensure the file extension is included in the targetInfo file name.
            // ------------------------------------------------------------------
            std::string filenameWithExtension = targetInfo;
            etrading::appendFileExtension( filenameWithExtension, etrading::JSON );            

            // TODO: make this move constructed in C++11
            return std::move( SerializationResult( method, target, wrap, filenameWithExtension ) );
        }
        else
        {
            throw ETradingException( ( boost::format( "serializeContainedData not implemented for serialization method: %s" )
                                       % StaticStructureStore::getInstance().toString( method ).c_str() ).str() );
        }
    };

    void SchemaObject::adoptDataSchema( const SchemaObject& to )
    {
        for( size_t schemaCounter = 0u;  schemaCounter < to.numberOfSchemas(); schemaCounter++ )
        {
            const auto& schemaToAdopt = to.viewSchema( schemaCounter );
            const std::string schemaNameToAbsorbFrom = schemaToAdopt.getName();
            const auto schemaIdxInThis = schemaIdx( schemaNameToAbsorbFrom );
            if( schemaIdxInThis < 0 )
            {
                // add to the existing dataSchema
                this->addDataSchema( schemaToAdopt );
            }

            const std::vector<int> columnsToSet = columnsOfSchema( schemaNameToAbsorbFrom );
            const std::vector<int> columnsToReadFrom = to.columnsOfSchema( schemaNameToAbsorbFrom );

            if( columnsToSet.size() != columnsToReadFrom.size() )
            {
                throw ETradingException( ( boost::format( "Size of DataSchema %s in absorbing object (%i) is different from same schema in absorbed object (%i)" )
                                           % schemaNameToAbsorbFrom.c_str() % columnsToSet.size() % columnsToReadFrom.size() ).str() );
            }

            for( unsigned int colCounter = 0; colCounter < columnsToSet.size(); colCounter++ )
            {
                const int colIdxToSet = columnsToSet[ colCounter ];
                const etrading::ContainedTypeEnum targetType = getDataFormat( colIdxToSet ).second;
                setDataVector( colIdxToSet, to.viewData( columnsToReadFrom[ colCounter ] ), targetType );
            }
        };
    };

    void SchemaObject::absorbDataSchema( const SchemaObject& to, std::vector<std::string>* schemaNames )
    {
        const bool absorbAllPossible = ( schemaNames == nullptr );
        for( size_t schemaCounter = 0u;  schemaCounter < to.numberOfSchemas(); schemaCounter++ )
        {
            const std::string schemaNameToAbsorbFrom = to.viewSchema( schemaCounter ).getName();
            if( absorbAllPossible || getIndexOf( schemaNameToAbsorbFrom, *schemaNames ) >= 0  )
            {
                const auto schemaIdxInThis = schemaIdx( schemaNameToAbsorbFrom );
                if( schemaIdxInThis >= 0 )
                {
                    const std::vector<int> columnsToSet = columnsOfSchema( schemaNameToAbsorbFrom );
                    const std::vector<int> columnsToReadFrom = to.columnsOfSchema( schemaNameToAbsorbFrom );

                    if( columnsToSet.size() != columnsToReadFrom.size() )
                    {
                        throw ETradingException( ( boost::format( "Size of DataSchema %s in absorbing object (%i) is different from same schema in absorbed object (%i)" )
                                                   % schemaNameToAbsorbFrom.c_str() % columnsToSet.size() % columnsToReadFrom.size() ).str() );
                    }

                    for( unsigned int colCounter = 0; colCounter < columnsToSet.size(); colCounter++ )
                    {
                        const int colIdxToSet = columnsToSet[ colCounter ];
                        const etrading::ContainedTypeEnum targetType = getDataFormat( colIdxToSet ).second;
                        setDataVector( colIdxToSet, to.viewData( columnsToReadFrom[ colCounter ] ), targetType );
                    }
                }
                else
                {
                    throw ETradingException( ( boost::format( "SchemaObject::absorbDataSchema was requested to absorb a data schema that was not available in the current object (%s)" ) % schemaNameToAbsorbFrom.c_str() ).str() );
                }
            }
        }
    };

	// Adds a dependent SchemaObject to our vector of nested Schema Objects
	void SchemaObject::addNestedSchemaObject( const SchemaObject& nestedObject )
	{
		nestedSchemaObjects_.push_back( nestedObject );

	}

	const SchemaObject SchemaObject::toSchemaObject() const
	{
		return SchemaObject(*this);
	}

};


