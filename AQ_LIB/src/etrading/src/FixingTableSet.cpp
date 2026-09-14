
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
            { std::ostringstream aqMsg139;
aqMsg139 << "Cannot find the FixingTable for currency " << toString( ccy ) << " and tenor " << toString( curveTenor ) << " in the set named " << getRefToName(); AQ_THROW( aqMsg139.str() ); }
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
            { std::ostringstream aqMsg140;
aqMsg140 << "Cannot find the FixingTable with name " << tableName.c_str() << " in the set named " << getRefToName(); AQ_THROW( aqMsg140.str() ); }
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


