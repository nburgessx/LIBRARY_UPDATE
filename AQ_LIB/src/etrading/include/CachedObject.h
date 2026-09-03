
#pragma once


//
// @File: CachedObject.h
// @Description: This is a base class that handles the core data of derived base classes (e.g. serialization/deserialization)
// by default none of its functionality should be exposed on classes using this (either through protected/private inheritance or
// aggregation). The exception in this library is FreeObject as it requires full data flexibility.
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.


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

#include "DataSchema.h"
#include "CoreEnumerations.h"
#include "ETradingException.h"
#include "StaticStructureStore.h"
#include "Variant.h"
#include "HasInstance.h"
#include "TypeName.h"
#include "SerializationResult.h"
#include "SerializeContainedData.h"
#include "SerializeInterface.h"
#include "SerializationResult.h"

namespace etrading
{

    // @CachedObject: This class is the implementation class of a flexible data store
    // Data Structures are described through StructuredKeys, the actual data itself is set the _theData member variable
    // Accounting from data structures (structured keys) to actual data is done through _theKeyColumnMapping and _theColumnNameMapping
    class CachedObject : private HasConstInstance<etrading::CachedObjectEnum>, public SerializeInterface
    {
    public:

        CachedObject( const etrading::CachedObjectEnum enumInput );
        void setStructuredKeys( const std::vector<DataSchema>& sks );
        const etrading::CachedObjectEnum getEnumType() const;
        void clearAll();
        void clearData();
        template <typename S>
        void setStructuredKeyColumnData( const std::string& keyName,
                                         const std::string& columnName,
                                         const std::vector<S> inputColumnData )
        {
            std::pair<int, etrading::ContainedTypeEnum> columnInfo = getColumnInfo( keyName, columnName );
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
                                           % keyName.c_str() ).str().c_str() );
            }
        }

        template <typename S>
        void setStructuredKeyColumnData( const std::string& keyName,
                                         const int columnIndexInKey,
                                         const std::vector<S> inputColumnData )
        {
            auto cit = std::find_if( _theKeys.cbegin(), _theKeys.cend(), [&keyName] ( const DataSchema & sk )
            {
                return sk.getName() == keyName;
            } );
            if( cit != _theKeys.end() )
            {
                auto targetType = cit->getColumnType( columnIndexInKey );
                int columnIndex = _theKeyColumnMapping.at( cit->getName() ).at( columnIndexInKey );
                setDataVector( columnIndex, inputColumnData, targetType );
            }
            else
            {
                throw ETradingException( ( boost::format( "No Structured key with name %s exists" ) % keyName.c_str() ).str().c_str() );
            }
        }

        void setStructuredKeyData(	const std::string& keyName,
                                    const std::vector<std::vector<Variant>>& inputData,
                                    const std::vector<ContainedTypeEnum>& types = std::vector<ContainedTypeEnum>( 0 ) );

        bool assignData( const CachedObject& rhs ); // no assignment operator - keep the const enum in the base class
        Variant getValueCopy( const std::string& keyName, const unsigned int rowIndex, const unsigned int columnIndex ) const;
        std::vector<Variant> getValuesCopy( const std::string& keyName, const unsigned int index, bool isColumn = true ) const;
        std::pair<int, etrading::ContainedTypeEnum> getDataFormat( const unsigned int index ) const;

        virtual SerializationResult serialize(	const serialize::SerializationMethodEnum method,
                                                const serialize::SerializationTargetEnum target,
                                                const std::string& targetInfo,
                                                const std::vector<std::string> variableNames,
                                                const std::vector<Variant> variableValues ) const;

        const std::vector<const DataSchema> snapshotOfKeys() const;
        const std::vector<int> keyColumns( const std::string& skName ) const;
        const std::vector<std::vector<Variant>>& viewAllData() const;
        const DataSchema& viewKey( const int ) const;
        const int keyIdx( const std::string& skName ) const;

        static const std::string OBJECT_NAME_LABEL;

    protected:
        const std::vector<Variant>& viewData( const unsigned int idx ) const;

        template<typename T>
        static SerializationResult  serializeContainedData(
            const T& instance,
            const serialize::SerializationMethodEnum methodType,
            const serialize::SerializationTargetEnum targetType,
            const std::string& targetInfo,
            const std::vector<std::string> variableNames,
            const std::vector<Variant> variableValues,
            const std::vector<std::shared_ptr<etrading::CachedObject>>& containedObjects =  std::vector<std::shared_ptr<etrading::CachedObject>>( 0, nullptr )
        )
        {
            BOOST_STATIC_ASSERT( boost::is_base_of<CachedObject, T>::value );

            const CachedObject& co = static_cast<const CachedObject&>( instance );

            bool allowRecursive = !( containedObjects.empty() );

            if( methodType == serialize::JSON )
            {
                // TODO: make it more efficient by just using rapidjson objects ...
                std::string startingString = createJSONStringFromCachedObject( co, false, allowRecursive );
                if( !variableNames.empty() && variableValues.size() == variableNames.size() )
                {
                    for( unsigned int varCounter = 0u; varCounter < variableNames.size(); varCounter++ )
                    {
                        std::string toAddHere = ( boost::format( ", \"%s\" : " ) % variableNames.at( varCounter ).c_str() ).str();
                        const Variant& cv = variableValues.at( varCounter );
                        toAddHere += cv.toString( true );
                        startingString += toAddHere;
                    }
                }

                if( !allowRecursive )
                {
                    startingString += "}";
                }
                else
                {
                    startingString += ", \"CONTAINED_OBJECTS\" : [";
                    std::for_each( containedObjects.cbegin(), containedObjects.cend(),
                                   [&startingString] ( const std::shared_ptr<CachedObject>& cachedObjectPtr )
                    {
                        if( cachedObjectPtr != nullptr )
                        {
                            std::string jsonString = createJSONStringFromCachedObject( ( *cachedObjectPtr.get() ), true );
                            startingString += jsonString;
                        }
                    } );
                    startingString += "] } ";
                }

                SerializationResultWrapper wrap;
                wrap.jsonDocument = std::shared_ptr<Document>( new Document() );
                wrap.jsonDocument.get()->SetObject();
                wrap.jsonDocument.get()->Parse<0>( startingString.c_str() );

                //wrap.jsonDocument.get()->AddMember("CachedObjectType",cachedObjectType,wrap.jsonDocument.get()->GetAllocator());
                // TODO: make this move constructed in C++11
                return SerializationResult( methodType, targetType, wrap, targetInfo );
            }
            throw ETradingException( ( boost::format( "serializeContainedData not implemented for serializaiton method: %s" )
                                       % StaticStructureStore::getInstance().toString( methodType ).c_str() ).str() );
        };


        std::vector<DataSchema> _theKeys;
        std::vector<std::vector<Variant>> _theData;
    private:
        std::map<std::string, std::vector<int> > _theKeyColumnMapping;
        std::map< int, std::string > _theColumnNameMapping; // this cannot be a bimap because there could be duplicate column names

        static std::string createUnableToFindKey( const std::string& keyName );
        static unsigned int calculateColumnsInStructuredKeys( const std::vector<DataSchema>& sks );

        std::pair<int, etrading::ContainedTypeEnum> getColumnInfo( const std::string& keyName, const std::string& columnName ) const;
        std::string constructColOverflowString( int index ) const;

        template <typename S>
        void setDataVector( const int columnIndex, const std::vector<S>& inputColumnData, etrading::ContainedTypeEnum targetType )
        {
            if( Variant::isConvertible( inputColumnData, targetType ) )
            {
                _theData.at( columnIndex ).clear();
                _theData.at( columnIndex ) = Variant::createVariantVector( inputColumnData, targetType );
            }
            else
            {
                // _theColumnNameMapping
                std::string columnNameInfo  = ( _theColumnNameMapping.count( columnIndex ) > 0 ) ? _theColumnNameMapping.at( columnIndex ) + " column name" : "NO column name";
                throw ETradingException( ( boost::format( "Repository Data Column %s (%s) requires type %s but the input vector is of type %s" )
                                           % boost::lexical_cast<std::string>( columnIndex ).c_str()
                                           % columnNameInfo.c_str()
                                           % StaticStructureStore::getInstance().toString( targetType ).c_str()
                                           % TypeName::get<S>() ).str().c_str() );
            }
        };

    };

};


