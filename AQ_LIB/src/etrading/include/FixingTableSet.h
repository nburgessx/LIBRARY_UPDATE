#pragma once

#include <map>
#include <utility>
#include "FixingTable.h"
#include "HasInstance.h"
#include "CoreEnumerations.h"




namespace etrading
{
    /*
    	This class contains a set of fixing tables accessible
    	by currency and curve tenor 	OR by the tableName
    	(note that one could overwrite the other)
    */
    class FixingTableSet : public HasName
    {
    public:
        FixingTableSet( const std::string& FixingTableName = "FIXING_TABLES" );
        const bool has( const std::string& tableName ) const;
        const bool has( const etrading::CCY, const etrading::CurveTenorEnum ) const;
        const FixingTable access( const etrading::CCY, const etrading::CurveTenorEnum ) const;
        const FixingTable access( const std::string& tableName ) const;
        bool insert( const FixingTable& );
    private:
        std::map< std::pair< etrading::CCY, etrading::CurveTenorEnum > , std::string > combiToName_;
        std::map< std::string , FixingTable > fixingTablesMap_;
    };

}
