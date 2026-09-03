
#include "ExampleObjects.h"
#include "SchemaObject.h"

namespace etrading
{
    StandAlone::StandAlone( const std::string& objectName ) :
        IsLWOObject( objectName, etrading::EXAMPLE_STAND_ALONE ),
        d_( 10.0 )
    {};

    const SchemaObject StandAlone::toSchemaObject() const
    {
        SchemaObject schemaObject( EXAMPLE_STAND_ALONE, getRefToName() );
        const std::string FIRST_SCHEMA_NAME = schemaObject.viewSchema( 0 ).getName();
        std::vector<double> data;
        data.push_back( this->d_ );
        schemaObject.setColumnData( FIRST_SCHEMA_NAME, 0, data );
        return schemaObject;
    };


}