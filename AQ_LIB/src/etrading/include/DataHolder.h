//
// @File: DataHolder.h
// @Description: This is a base class that handles the core data of derived base classes (e.g. serialization/deserialization)
// by default none of its functionality should be exposed on classes using this (either through protected/private inheritance or
// aggregation). The exception in this library is FreeObject as it requires full data flexibility.
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of Mizuho International.

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <utility>
#include <iostream>
#include <algorithm>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/array.hpp>
#include <boost/optional.hpp>
#include <boost/assign.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "Cacheable.h"
#include "DataSchema.h"
#include "CoreEnumerations.h"
#include "ETradingException.h"
#include "StaticStructureStore.h"
#include "Variant.h"
#include "HasInstance.h"
#include "TypeName.h"
#include "IsLWOObject.h"

namespace etrading
{
    class DataHolder : public IsLWOObject 
    {
    public:
        DataHolder( const etrading::CachedObjectEnum enumInput , const std::string& name);
        void setDataSchemas( const std::vector<DataSchema>& sks );
        void addDataSchema( const DataSchema& dataSchema );
        // for allowing the addition of objects that don't always have the same form

        void clearAll();
        void clearData();

        template <typename S>
        void setColumnData( const std::string& schemaName,
                            const std::string& columnName,
                            const std::vector<S>& inputColumnData )
        {
            std::pair<int, etrading::ContainedTypeEnum> columnInfo = getColumnInfo( schemaName, columnName );
            int columnIndex = columnInfo.first;
            if( columnIndex >= 0 )
            {
                const etrading::ContainedTypeEnum targetType = columnInfo.second;
                setDataVector( columnIndex, inputColumnData, targetType );
            }
            else
            {
                throw ETradingException( ( boost::format( "Unable to find Column Name %s For Data Set called %s" )
                                           % columnName.c_str()
                                           % schemaName.c_str() ).str().c_str() );
            }
        }

        template <typename S>
        void setColumnData( const std::string& schemaName,
                            const int columnIndexInKey,
                            const std::vector<S>& inputColumnData )
        {
            auto cit = std::find_if( _theKeys.cbegin(), _theKeys.cend(), [&schemaName] ( const DataSchema & sk )
            {
                return sk.getName() == schemaName;
            } );
            if( cit != _theKeys.end() )
            {
                auto targetType = cit->getColumnType( columnIndexInKey );
                int columnIndex = _theKeyColumnMapping.at( cit->getName() )[columnIndexInKey];
                setDataVector( columnIndex, inputColumnData, targetType );
            }
            else
            {
                throw ETradingException( ( boost::format( "#Error: No Schema with name %s exists" ) % schemaName.c_str() ).str().c_str() );
            }
        }

        void setDataForSchema(	const std::string& schemaName,
                                const std::vector<std::vector<Variant>>& inputData,
                                const std::vector<ContainedTypeEnum>& types = std::vector<ContainedTypeEnum>( 0 ) );

        void setDataForSchemaWithMap( const std::string& schemaName, const std::map<std::string, Variant>& inputMap );
        bool assignData( const DataHolder& rhs ); // no assignment operator - keep the const enum in the base class

        Variant getValueCopy( const std::string& keyName, const unsigned int rowIndex, const unsigned int columnIndex ) const;
        std::vector<Variant> getValuesCopy( const std::string& keyName, const unsigned int index, bool isColumn = true ) const;
        std::pair<int, etrading::ContainedTypeEnum> getDataFormat( const unsigned int index ) const;

        const std::vector<DataSchema> snapshotOfSchemas() const;
        const std::vector<int> columnsOfSchema( const std::string& skName ) const;
        const bool doesKeyExist( const std::string& keyName ) const;
        const std::vector<std::vector<Variant>>& viewAllData() const;
        const DataSchema& viewSchema( const int ) const;
        const int schemaIdx( const std::string& skName ) const;
        const size_t numberOfSchemas() const;
        const std::vector<std::string> keyNames() const;

        static const std::string OBJECT_NAME_LABEL;

    protected:
        const std::vector<Variant>& viewData( const unsigned int idx ) const;

        std::vector<DataSchema> _theKeys;
        std::vector<std::vector<Variant>> _theData;

        template <typename S>
        void setDataVector( const int columnIndex, const std::vector<S>& inputColumnData, etrading::ContainedTypeEnum targetType )
        {
            if( Variant::isConvertibleVector( inputColumnData, targetType ) )
            {
                _theData[columnIndex ].clear();
                _theData[columnIndex ] = Variant::createVariantVector( inputColumnData, targetType );
            }
            else
            {
                // _theColumnNameMapping
                std::string columnNameInfo  = ( _theColumnNameMapping.count( columnIndex ) > 0 ) ? _theColumnNameMapping[ columnIndex ] + " column name" : "NO column name";
                throw ETradingException( ( boost::format( "#Error: DataSchema column %s with column name %s requires type %s but input data is of type %s" )
                                           % boost::lexical_cast<std::string>( columnIndex ).c_str()
                                           % columnNameInfo.c_str()
                                           % toString( targetType )
                                           % TypeName::get<S>() ).str().c_str() );
            }
        };

    private:
        std::map<std::string, std::vector<int> > _theKeyColumnMapping;
        std::map< int, std::string > _theColumnNameMapping; // this cannot be a bimap because there could be duplicate column names

        static std::string createUnableToFindKey( const std::string& keyName );
        static unsigned int calculateColumnsInDataSchemas( const std::vector<DataSchema>& sks );

        std::pair<int, etrading::ContainedTypeEnum> getColumnInfo( const std::string& keyName, const std::string& columnName ) const;
        std::string constructColOverflowString( int index ) const;

    };

}
