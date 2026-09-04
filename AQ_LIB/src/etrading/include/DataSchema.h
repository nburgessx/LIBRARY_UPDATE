#pragma once
//
// @Description: This is a test program




#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

#include "ETradingException.h"
#include "CoreEnumerations.h"

namespace etrading
{
    // @DataSchema this class represents the layout of a data structure
    // the schema has a name, a number of colunmns, a series of column types and optional column names.
    class DataSchema
    {
    public:
        DataSchema(); // TODO: implement a const std::string& akeyName and get rid of the default CTOR

        DataSchema(	const std::string& akeyName,
                    const int aNumberOfColumns,
                    const std::vector<etrading::ContainedTypeEnum>& aColumnTypes ,
                    const std::vector<std::string>& aColumnNames = std::vector<std::string>( 0 ) );

        const std::string& getName() const;
        const bool hasColumnNames() const;

        const std::vector<std::string> getColumnNames() const; // copy
        const std::vector<std::string>& getColumnNamesRef() const; // ref

        const int getNumberOfColumns() const;

        void setName( const std::string& newname );
        void setColumnNames( const std::vector<std::string>& newColumnNames );

        const etrading::ContainedTypeEnum getColumnType( const int idx ) const;

        bool operator==( const DataSchema& rhs ) const;

    private:
        std::string _keyName;
        int _numberOfColumns;
        std::vector<std::string> _columnNames;
        std::vector<etrading::ContainedTypeEnum> _columnTypes;
    };

}
