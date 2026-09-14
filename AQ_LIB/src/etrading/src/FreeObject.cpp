//
// @Description: Implementation of FreeObject

#include <memory>

#include <boost/range/algorithm_ext/push_back.hpp>

#include "FreeObject.h"
#include "CoreEnumerations.h"
#include "ContainerUtilities.h"
#include "SerializeInstance.h"


namespace etrading
{

    FreeObject::FreeObject( const std::string& name )
        :	DataHolder( FREE_OBJECT , name )
		
    {};

    FreeObject::FreeObject( const FreeObject& rhs )
		: DataHolder( FREE_OBJECT , rhs.getRefToName() ) 
		
    {
        assignData( rhs );
    };

    FreeObject& FreeObject::operator+=( const FreeObject& rhs )
    {
        if( &rhs != this )
        {
            // TODO: template these paragraphs
            auto mergedKeys( _theKeys );
            std::vector<std::string> myKeyNames;
            std::for_each( mergedKeys.cbegin(),
                           mergedKeys.cend(),
                           [&myKeyNames]( const DataSchema & sk )
            {
                myKeyNames.push_back( sk.getName() );
            }  );

            auto copyOfOtherKeys( rhs.snapshotOfSchemas() );
            std::vector<std::string> theirKeyNames;
            std::for_each( copyOfOtherKeys.cbegin(),
                           copyOfOtherKeys.cend(),
                           [&theirKeyNames]( const DataSchema & sk )
            {
                theirKeyNames.push_back( sk.getName() );
            }  );

            std::vector<std::string> mergedKeyNames = myKeyNames;
            boost::push_back( mergedKeyNames, theirKeyNames );
            if( !is_unique( mergedKeyNames ) )
            {
                { std::ostringstream aqMsg141;
aqMsg141 << "Cannot duplicate Schema names when adding two FreeObjects (" << containerAsString( myKeyNames ).c_str() << ") and (" << containerAsString( theirKeyNames ).c_str() << ")"; AQ_THROW( aqMsg141.str() ); }
            }

            auto mergedData( this->viewAllData() );
            boost::push_back( mergedData, rhs.viewAllData() );

            boost::push_back( mergedKeys, copyOfOtherKeys );
            this->setDataSchemas( mergedKeys ); // also clears current data at the same time
            _theData = mergedData; // safe to do it here as we concatenate in the right order

            return ( *this );
        }
        else
        {
            AQ_THROW( "Cannot add a FreeObject to itself because it generates duplicate Schemas" );
        }
    };

    SerializationResult FreeObject::serialize( const serialize::SerializationMethodEnum method,
            const serialize::SerializationTargetEnum target,
            const std::string& targetInfo,
            std::vector<std::string>& variableNames,
            std::vector<Variant>& variableValues ) const
    {
        variableNames.emplace_back( DataHolder::OBJECT_NAME_LABEL );
        variableValues.emplace_back( Variant( getRefToName() ) );
        return serializeInstance( *this, method, target, targetInfo, variableNames, variableValues );
    };

	const SchemaObject FreeObject::toSchemaObject() const
	{
		return SchemaObject( static_cast<const DataHolder&>(*this), this->getRefToName());
	};

}