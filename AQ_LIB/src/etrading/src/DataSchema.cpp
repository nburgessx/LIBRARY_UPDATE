//
// @Description: Implementation of DataSchema


#include <locale>
#include <algorithm>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "DataSchema.h"
#include "ETradingException.h"


namespace etrading
{
    DataSchema::DataSchema()  { }; // STL needs default CTOR for empty containers...
    DataSchema::DataSchema(		const std::string& akeyName,
                                const int aNumberOfColumns,
                                const std::vector<etrading::ContainedTypeEnum>& aColumnTypes ,
                                const std::vector<std::string>& aColumnNames )
        : _numberOfColumns( aNumberOfColumns ),
          _keyName( boost::to_upper_copy<std::string>( akeyName ) )
    {
        if( _numberOfColumns < 1 )
        {
            throw ETradingException( "#Error: Cannot Create DataSchema with less than 1 column" );
        }
        if( aColumnTypes.size()  != _numberOfColumns )
        {
            throw ETradingException( "#Error: DataSchema number of columns does correspond ot the number of Column Types" );
        }
        setColumnNames( aColumnNames );
        _columnTypes = aColumnTypes;
    };

    const std::string& DataSchema::getName() const
    {
        return _keyName;
    };

    const bool DataSchema::hasColumnNames() const
    {
        return _columnNames.size() > 0;
    };

    void DataSchema::setName( const std::string& newname )
    {
        _keyName = newname;
    }

    const int DataSchema::getNumberOfColumns() const
    {
        return _numberOfColumns;
    };

    void DataSchema::setColumnNames( const std::vector<std::string>& aColumnNames )
    {
        int sizeOfNames = aColumnNames.size();
        if( sizeOfNames == _numberOfColumns )
        {
            _columnNames = aColumnNames;
        }
        else if( sizeOfNames != 0 )
        {
            std::string errString = 	std::string( "#Error: Number of Column Names: " ) + boost::lexical_cast<std::string>( sizeOfNames ) +
                                        std::string( " does not correspond with number of columns requested: " ) + boost::lexical_cast<std::string>( sizeOfNames ) ;
            throw ETradingException( errString.c_str() );
        }
    };

    const std::vector<std::string> DataSchema::getColumnNames() const
    {
        if( hasColumnNames() )
        {
            return _columnNames ;
        }
        else
        {
            return std::vector<std::string>();
        }
    };

    const std::vector<std::string>& DataSchema::getColumnNamesRef() const
    {
        return _columnNames;
    };

    const etrading::ContainedTypeEnum DataSchema::getColumnType( const int idx ) const
    {
        return this->_columnTypes[idx];
    }

    bool DataSchema::operator==( const DataSchema& rhs ) const
    {
        if( &rhs != this )
        {
            return ( _keyName == rhs._keyName ); // only need to verify  the name because we want to avoid duplicate names even if their underlying structure is different
        }
        else
        {
            return true;
        }
    }

}

