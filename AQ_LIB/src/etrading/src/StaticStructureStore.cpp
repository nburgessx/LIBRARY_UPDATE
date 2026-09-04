//
// @File: StaticStructureStore.cpp
// @Description: Implementation of Singleton StaticStructureStore
// @Created: 04 April 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.

#include <vector>
#include <boost/assign.hpp>
#include <boost/bimap.hpp>
#include <boost/format.hpp>

#include "StaticStructureStore.h"
#include "CoreEnumerations.h"
#include "NamesBiMap.h"


namespace etrading
{
	// Helper method which assists in constructing our required bi-directional map.
	boost::bimap<serialize::SerializationMethodEnum, std::string>
	makeBimap()
	{
		typedef boost::bimap<serialize::SerializationMethodEnum, std::string> MyBimap;
		typedef MyBimap::value_type position;
		MyBimap bm;

		bm.insert( position ( serialize::SerializationMethodEnum::JSON, "JSON" ) );
		return bm;
	}

    const std::string StaticStructureStore::toString( const serialize::SerializationMethodEnum srEnum ) const
    {
        try
        {
            return SERIALIZEMETHOD_NAMES.getString( srEnum );
        }
        catch( ... )
        {
            throw ETradingException(  ( boost::format( "Unable to find matching String description for SerializationMethodEnum (%i)" ) % srEnum ).str()  );
        }
    }

    // given the type of Object show me its inner structure
    const std::vector<DataSchema> StaticStructureStore::getCachedObjectStructure( const etrading::CachedObjectEnum enumObject ) const
    {
        auto it = CACHEDOBJECT_STRUCTURES.find( enumObject );
        if ( it != CACHEDOBJECT_STRUCTURES.end() )
        {
            return ( *it ).second;
        }
        std::vector<DataSchema> emptyVector( 0 );
        emptyVector.clear(); // how useful is this?
        return emptyVector;
    }

    const std::vector<std::string> StaticStructureStore::getKeyColumns( const etrading::CachedObjectEnum enumObject, const std::string& skName ) const
    {
        std::vector<DataSchema> keys = getCachedObjectStructure( enumObject );
        auto cit = std::find_if( keys.cbegin(), keys.cend(), [&skName]( const DataSchema & sk )
        {
            return sk.getName() == skName;
        } );
        if( cit != keys.cend() )
        {
            return ( *cit ).getColumnNamesRef();
        }
        else
        {
            return std::vector<std::string>( 0, "" );
        }
    }


    StaticStructureStore::StaticStructureStore()  : SERIALIZEMETHOD_NAMES( makeBimap() )
    {

        // TODO: find a way to clean up this messy interface....
#define DEFINE_SCHEMA2(NAME,C1_TYPE,C2_TYPE,C1_NAME,C2_NAME) \
    #NAME, 2, boost::assign::list_of(C1_TYPE)(C2_TYPE), boost::assign::list_of(#C1_NAME)(#C2_NAME)  \

#define DEFINE_SCHEMA3(NAME,C1_TYPE,C2_TYPE,C3_TYPE,C1_NAME,C2_NAME,C3_NAME) \
    #NAME, 3, boost::assign::list_of(C1_TYPE)(C2_TYPE)(C3_TYPE), boost::assign::list_of(#C1_NAME)(#C2_NAME)(#C3_NAME)  \

        
    // Set up the DataSchema of a TableDateDouble
    DataSchema schemaTableDateDouble( DEFINE_SCHEMA2( TABLE, DATE_VALUE, DOUBLE_VALUE, DATE_VALUE, DOUBLE_VALUE ) );
    std::vector<DataSchema> schemaVectorForTableDateDouble = boost::assign::list_of( schemaTableDateDouble );

    // Set up the DataSchema of a TableDoubleDouble
    DataSchema schemaTableDoubleDouble( DEFINE_SCHEMA2( MATRIX, DOUBLE_VALUE, DOUBLE_VALUE, DOUBLE_VALUE, DOUBLE_VALUE ) );
    std::vector<DataSchema> schemaVectorForTableDoubleDouble = boost::assign::list_of( schemaTableDoubleDouble );

    // Set up the DataSchema of a CurveBuildProperties
    DataSchema schemaCurveBuildProperties( DEFINE_SCHEMA2( CURVE_BUILD_PROPERTIES, STRING_VALUE, VARIANT_VALUE, VARIABLE_NAME, VARIABLE_VALUE ) );
    std::vector<DataSchema> schemaVectorForCurveBuildProperties = boost::assign::list_of( schemaCurveBuildProperties );

    // Set up the DataSchema of a AQObjCurve
    DataSchema schemaRates( DEFINE_SCHEMA3( RATES, DATE_VALUE, DOUBLE_VALUE, DOUBLE_VALUE, DATE_VALUE, DISCOUNT_FACTORS, FORWARD_RATES ) );
    DataSchema schemaCurveConvention( DEFINE_SCHEMA2( CURVE_BUILD_PROPERTIES, STRING_VALUE, VARIANT_VALUE, VARIABLE_NAME, VARIABLE_VALUE ) );
    std::vector<DataSchema> schemaVectorForCurveConvention = boost::assign::list_of( schemaCurveConvention )( schemaRates );

    // Set up the DataSchema of a StandAlone
    DataSchema schemaStandalone( "THE_NUMBER", 1, boost::assign::list_of( DOUBLE_VALUE ), boost::assign::list_of( "COL_NAME" ) );
    std::vector<DataSchema> schemaVectorForStandAlone = boost::assign::list_of( schemaStandalone );

    // Set up the DataSchema for a FixingTable
    DataSchema schemaFixingTableFixings( DEFINE_SCHEMA2( FIXING_TABLE, DATE_VALUE, DOUBLE_VALUE, FIXING_DATES, FIXING_VALUES ) );
    DataSchema schemaFixingTableParameters( "FIXING_TABLE_PARAMETERS",                                              // DataSchemaName
                                             2,                                                                     // Number of Columns in Data Schema
                                             boost::assign::list_of( STRING_VALUE )( STRING_VALUE ),                            // Column Types
                                             boost::assign::list_of( "PARAMETER_NAME" )( "PARAMETER_VALUE" ) );     // Column Headers

    std::vector< DataSchema > schemaVectorForFixingTable = boost::assign::list_of( schemaFixingTableParameters )( schemaFixingTableFixings );

    // link the schemas to the enums so we can set them in the objects derived from CachedObject
	CACHEDOBJECT_STRUCTURES = makeMap( boost::assign::map_list_of( TABLE,                    schemaVectorForTableDateDouble )
																 ( MATRIX,                   schemaVectorForTableDoubleDouble )
																 ( CURVE_BUILD_PROPERTIES,   schemaVectorForCurveBuildProperties )
																 ( CURVE_DEPRECATED,         schemaVectorForCurveConvention )
																 ( EXAMPLE_STAND_ALONE,      schemaVectorForStandAlone )
																 ( FIXING_TABLE,             schemaVectorForFixingTable ));
    }

}
