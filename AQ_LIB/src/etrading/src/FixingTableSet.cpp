#include <boost/format.hpp>

#include "FixingTableSet.h"
#include "ETradingException.h"


namespace etrading
{

    FixingTableSet::FixingTableSet( const std::string& fixingTableName ) : HasName( fixingTableName )
    {};

    const bool FixingTableSet::has( const std::string& tableName ) const
    {
        return ( fixingTablesMap_.count( tableName ) > 0 );
    };

    const bool FixingTableSet::has( const etrading::CCY ccy , const etrading::CurveTenorEnum curveTenor ) const
    {
        return ( combiToName_.count( std::make_pair( ccy, curveTenor ) ) > 0 );
    };

    const FixingTable FixingTableSet::access( const etrading::CCY ccy, const etrading::CurveTenorEnum curveTenor ) const
    {
        auto iterator = combiToName_.find( std::make_pair( ccy, curveTenor ) );
        if( iterator != combiToName_.end() )
        {
            return ( iterator->second );
        }
        else
        {
            throw ETradingException( ( boost::format( "Cannot find the FixingTable for currency %s and tenor %s in the set named %s" )
                                       % toString( ccy ) % toString( curveTenor ) % getRefToName() ).str() );
        }
    };

    const FixingTable FixingTableSet::access( const std::string& tableName ) const
    {
        auto iterator = fixingTablesMap_.find( tableName );
        if( iterator != fixingTablesMap_.end() )
        {
            return ( iterator->second );
        }
        else
        {
            throw ETradingException( ( boost::format( "Cannot find the FixingTable with name %s in the set named %s" )
                                       % tableName.c_str() % getRefToName() ).str() );
        }
    };

    bool FixingTableSet::insert( const FixingTable& fixingTable )
    {
        bool inCombi = has( fixingTable.getCurrency(), fixingTable.getCurveTenor() ) ;
        bool inName = has( fixingTable.getName() );
        const bool alreadyInMap = ( inCombi || inName );

        combiToName_[std::make_pair( fixingTable.getCurrency(), fixingTable.getCurveTenor() )] = fixingTable.getName();
        fixingTablesMap_[fixingTable.getName()] = fixingTable;
        return alreadyInMap;
    }

}


