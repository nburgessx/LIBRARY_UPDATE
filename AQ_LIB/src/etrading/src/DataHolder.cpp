//
// @File: DataHolder.cpp
// @Description: Implementation of DataHolder
// @Created: 04 April 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of Mizuho International.

#include <utility>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/assign.hpp>

#include "DataHolder.h"
#include "StaticStructureStore.h"
#include "ContainerUtilities.h"

namespace etrading
{

    /*static*/ const std::string DataHolder::OBJECT_NAME_LABEL = "NAME";

    DataHolder::DataHolder( const etrading::CachedObjectEnum enumInput , const std::string& name )  : IsLWOObject(name , enumInput)
    {
        if( enumInput != FREE_OBJECT )
        {
            setDataSchemas( StaticStructureStore::getInstance().getCachedObjectStructure( enumInput ) ); // the constructor sets the schema
        }
    };

    const std::vector<std::string> DataHolder::keyNames() const
    {
        std::vector<std::string> keyNames;
        std::for_each(	   _theKeys.cbegin(),
                           _theKeys.cend(),
                           [&keyNames]( const DataSchema & sk )
        {
            keyNames.emplace_back( sk.getName() );
        } );
        return keyNames;
    }

    void DataHolder::setDataSchemas( const std::vector<DataSchema>& sks )
    {
        std::vector<std::string> keyNames;
        std::for_each(	   sks.cbegin(),
                           sks.cend(),
                           [&keyNames]( const DataSchema & sk )
        {
            keyNames.emplace_back( sk.getName() );
        } );

        if( !is_unique( keyNames ) )
        {
            throw ETradingException( ( boost::format( "DataSchemas do not have unique names: %s" ) % containerAsString( keyNames ) ).str() );
        }

        // re-initialize the object
        clearAll();
        _theKeys = sks;
        int colCounter = 0;

        const unsigned int totalNumberOfColumns = calculateColumnsInDataSchemas( sks );
        _theData =  std::vector<std::vector<Variant>>( totalNumberOfColumns, std::vector<Variant>( 0 ) );

        // TODO: C++11 object for loops
        for( unsigned int skCounter = 0u; skCounter < _theKeys.size(); skCounter++ )
        {
            _theData[colCounter] = std::vector<Variant>( 0 );
            const DataSchema strucKey = _theKeys[ skCounter ];
            std::vector<int> columnIdxs;
            int endOfIdx = colCounter + strucKey.getNumberOfColumns();
            boost::push_back( columnIdxs, boost::irange( colCounter, endOfIdx ) );
            colCounter = endOfIdx;
            _theKeyColumnMapping[strucKey.getName()] =  columnIdxs;
            bool hasColumnNames = strucKey.hasColumnNames();
            if( hasColumnNames )
            {
                const std::vector<std::string> columnNamesOfKey = strucKey.getColumnNames();
                for( unsigned int smallColCounter = 0u; smallColCounter < columnIdxs.size(); smallColCounter++ )
                {
					_theColumnNameMapping[columnIdxs[ smallColCounter ]] = columnNamesOfKey[ smallColCounter ];
                }
            }
        }
    };

    void DataHolder::addDataSchema( const DataSchema& dataSchema )
    {
        const auto keyNames = this->keyNames();
        if( std::find( keyNames.cbegin(), keyNames.cend(), dataSchema.getName() ) != keyNames.cend() )
        {
            throw ETradingException( ( boost::format( "%s creation: %s already exists as a DataSchema in %s" )
                                       % toString( getEnumType() ) % dataSchema.getName() % containerAsString( keyNames ) ).str() );
        }
        _theKeys.push_back( dataSchema );
        auto dataColumnsToConcatenate = std::vector<std::vector<Variant>>( dataSchema.getNumberOfColumns(), std::vector<Variant>( 0 ) );
        _theData.insert( _theData.end(), dataColumnsToConcatenate.begin(), dataColumnsToConcatenate.end() );
        std::vector<int> matchingColumnIdxs;
        boost::push_back( matchingColumnIdxs, boost::irange( _theData.size() - dataSchema.getNumberOfColumns(), _theData.size() ) );
        _theKeyColumnMapping[dataSchema.getName()] =  matchingColumnIdxs;
        bool hasColumnNames = dataSchema.hasColumnNames();
        if( hasColumnNames )
        {
            const std::vector<std::string> columnNamesOfKey = dataSchema.getColumnNames();
            for( unsigned int smallColCounter = 0u; smallColCounter < matchingColumnIdxs.size(); smallColCounter++ )
            {
                _theColumnNameMapping[matchingColumnIdxs[ smallColCounter ]] = columnNamesOfKey[ smallColCounter ];
            }
        }
    };

    void DataHolder::clearAll()
    {
        _theKeys.clear();
        _theData.clear();
        _theKeyColumnMapping.clear();
        _theColumnNameMapping.clear();
    };

    void DataHolder::clearData()
    {
        _theData.clear();
        unsigned int totalNumberOfColumns = calculateColumnsInDataSchemas( _theKeys );
        _theData =  std::vector<std::vector<Variant>>( totalNumberOfColumns, std::vector<Variant>( 0 ) );
    };

    // given a key name and a column name in that key, give me the type of the column and column index in _theData
    std::pair<int, etrading::ContainedTypeEnum> DataHolder::getColumnInfo( const std::string& schemaName, const std::string& columnName ) const
    {
        // verify if the schemaName is present in this object, if not return a no find
        const bool keyNameExists = 	( _theKeyColumnMapping.find( schemaName ) != _theKeyColumnMapping.end() );
        if( !keyNameExists )
        {
            return std::make_pair( -1, VARIANT_VALUE );
        }

        // go through the data columns that correspond to the schema and see if you can find a column name that will match columnName, if yes get that column index in all the data.
        auto columnsToConsider = _theKeyColumnMapping.at( schemaName );
        auto it = std::find_if( columnsToConsider.cbegin(), columnsToConsider .cend(),
                                [this, &columnName] ( const int x ) -> bool
        {
            const bool aColumnNameExistsForColumn = 	( _theColumnNameMapping.find( x ) != _theColumnNameMapping.end() );
            if( !aColumnNameExistsForColumn ) return false;
            return ( _theColumnNameMapping.at( x ) == columnName );
        } );

        // if we could not find the column name return a no find else
        // get the schema with the corresponding schemaName, look through all its column names and
        // find the matching column name then get the corresponding index in the contained type vector
        if( it != columnsToConsider.cend() )
        {
            auto cit = std::find_if( _theKeys.cbegin(), _theKeys.cend(), [&schemaName] ( const DataSchema & sk )
            {
                return sk.getName() == schemaName ;
            } );
            if( cit != _theKeys.cend() )
            {
                const std::vector<std::string>& columnNamesHere = cit->getColumnNamesRef();
                auto citColName = std::find( columnNamesHere.cbegin(), columnNamesHere.cend(), columnName );
                if( citColName == columnNamesHere.cend() )
                {
                    throw ETradingException( ( boost::format( "#Error: Unable to find Column Name %s inside the Schema %s despite presence in Column Name Mapping" ) % columnName.c_str() % schemaName.c_str() ).str().c_str() );
                }
                size_t idx = citColName - columnNamesHere.cbegin();
                return std::make_pair( ( *it ), cit->getColumnType( idx ) );
            }
            else
            {
                throw ETradingException( ( boost::format( "#Error: Unable to find the Schema called %s despite existing mapping to column indices " ) % schemaName.c_str() ).str().c_str() );
            }
            // return std::make_pair(-1, etrading::ContainedTypeEnum::VARIANT);   // should never be called ...
        }
        else
        {
            return std::make_pair( -1, VARIANT_VALUE );
        }
    };

    // grab the data from another object, note that the object has to be compatible with the current enumeration
    bool DataHolder::assignData( const DataHolder& rhs )
    {
        if( &rhs != this )
        {
            const etrading::CachedObjectEnum typeOfObject = getEnumType() ;
            // only set the data if we have compatability with the current enumeration
            if( typeOfObject == FREE_OBJECT || typeOfObject == rhs.getEnumType() )
            {
                clearAll();
                _theKeys = rhs._theKeys;
                _theData = rhs._theData;
                _theKeyColumnMapping = rhs._theKeyColumnMapping;
                _theColumnNameMapping = rhs._theColumnNameMapping;
                return true;
            }
        }
        return false;
    }

    // given a key name give me the value in a certain row and column
    Variant DataHolder::getValueCopy( const std::string& schemaName,
                                      const unsigned int rowIndex,
                                      const unsigned int columnIndex ) const
    {
        const bool keyNameExists = 	( _theKeyColumnMapping.find( schemaName ) != _theKeyColumnMapping.end() );
        if( !keyNameExists )
        {
            throw ETradingException( createUnableToFindKey( schemaName ).c_str() );
        }

        const std::vector<int> relevantColumns = _theKeyColumnMapping.at( schemaName );
        const unsigned int numOfColumns = relevantColumns.size();
        if( numOfColumns < ( columnIndex + 1 ) )
        {
            throw ETradingException( ( boost::format( "#Error: Schema %s has %i columns, requested column was %i " ) % schemaName.c_str() % numOfColumns % columnIndex ).str().c_str() );
        }

        const unsigned int relevantColumn = relevantColumns[ columnIndex ];
        const std::vector<Variant>& relData =  _theData[ relevantColumn ];
        const unsigned int numOfRows = relData.size();
        if( numOfRows < ( rowIndex + 1 ) )
        {
            throw ETradingException( ( boost::format( "#Error: Schema %s has %i rows, requested row was %i " ) % schemaName.c_str() % numOfRows % rowIndex ).str().c_str() );
        }

        return relData[ rowIndex ];
    }

    // given an index in _theData, give me the type of the column and the number of rows in the column
    std::pair<int, etrading::ContainedTypeEnum> DataHolder::getDataFormat( const unsigned int index ) const
    {
        if( index > ( _theData.size() - 1 ) )
        {
            throw ETradingException( constructColOverflowString( index ).c_str() );
        }

        const std::vector<Variant>& testVec = _theData[ index ];
        int sizeOfVector = testVec.size();

        if( sizeOfVector > 0 )
        {
            etrading::ContainedTypeEnum defaultEnum = testVec[ 0 ].getType();
            if( std::find_if(	testVec.cbegin(), testVec.cend(), [&defaultEnum] ( const Variant & cv )
        {
            return cv.getType() != defaultEnum;
            } ) == testVec.cend() )
            {
                defaultEnum = VARIANT_VALUE;
            }
            return std::make_pair( sizeOfVector, defaultEnum );
        }
        else
        {
            return std::make_pair( 0, VARIANT_VALUE );
        }
    }

    // given a key name get me all the values in a column with a certain index (if isColumn is true) or get me all the values in a row (if isColumn is false)
    std::vector<Variant> DataHolder::getValuesCopy(
        const std::string& schemaName,
        const unsigned int index,
        bool isColumn ) const
    {
        const bool keyNameExists = 	( _theKeyColumnMapping.find( schemaName ) != _theKeyColumnMapping.end() );
        if( !keyNameExists )
        {
            throw ETradingException( createUnableToFindKey( schemaName ).c_str() );
        }

        const std::vector<int> relevantColumns = _theKeyColumnMapping.at( schemaName );
        const unsigned int numOfColumns = relevantColumns.size();
        if( isColumn )
        {
            if( numOfColumns < ( index + 1 ) )
            {
                throw ETradingException( ( boost::format( "#Error: Schema %s has %i columns, requested column was %i " ) % schemaName.c_str() % numOfColumns % index ).str().c_str() );
            }
            return _theData[ relevantColumns[index] ];
        }
        else
        {
            std::vector<Variant> retValues;
            BOOST_FOREACH( const int& colIdx, relevantColumns )
            {
                const std::vector<Variant>& relData =  _theData[ colIdx ];
                const unsigned int numOfRows = relData.size();
                if( numOfRows < ( index + 1 ) )
                {
                    throw ETradingException( ( boost::format( "#Error: Schema %s has %i rows, requested row was %i " ) % schemaName.c_str() % numOfRows % index ).str().c_str() );
                }
                retValues.push_back( _theData[ colIdx ][ index ] );
            }
            return retValues;
        }
    }

    // helper function given a string saying it is unable to find a certain key
    std::string DataHolder::createUnableToFindKey( const std::string& schemaName )
    {
        return std::string(  ( boost::format( "#Error: Unable to find Schema called %s despite existing mapping to column indices " ) % schemaName.c_str() ).str() );
    }

    // given a series for schema, tell me how many columns this would translate to
    unsigned int DataHolder::calculateColumnsInDataSchemas( const std::vector<DataSchema>& sks )
    {
        unsigned int numberOfRequiredColumns = 0u;
        std::for_each(  sks.cbegin(), sks.cend(),
                        [&numberOfRequiredColumns]( const DataSchema & sk )
        {
            numberOfRequiredColumns += sk.getNumberOfColumns();
        } );
        return numberOfRequiredColumns;
    }

    // given an index of the _theData, let me look at the data
    const std::vector<Variant>& DataHolder::viewData( const unsigned int idx ) const
    {
        if( idx >= _theData.size() )
        {
            throw ETradingException( constructColOverflowString( idx ).c_str() );
        }
        return _theData[ idx ];
    }

    const std::vector<std::vector<Variant>>& DataHolder::viewAllData() const
    {
        return _theData;
    }

    const DataSchema& DataHolder::viewSchema( const int idx ) const
    {
        if( size_t(idx) >= _theKeys.size() || idx < 0 )
        {
            throw ETradingException( ( boost::format( "#Error: Object has %i Schema, requested key index was %i " ) % _theKeys.size() % idx ).str() );
        }
        return _theKeys[ idx ];
    }

    const size_t DataHolder::numberOfSchemas() const
    {
        return _theKeys.size();
    }

    const std::vector<DataSchema> DataHolder::snapshotOfSchemas() const
    {
        std::vector<DataSchema> snapshotOfStructure;
        boost::push_back( snapshotOfStructure, _theKeys );
        return snapshotOfStructure;
    }

    const std::vector<int> DataHolder::columnsOfSchema( const std::string& skName ) const
    {
		std::vector<int> columns;
		try
		{
			columns = _theKeyColumnMapping.at( skName );
		}
		catch (std::out_of_range&)
		{
			throw LACoreInvalidData( ( boost::format( "#Error: Could not find required property key '%s' in object with name '%s'." )  
						% skName % getRefToName() ).str().c_str(), __FILE__, __LINE__ );
		}
        return columns;
    }

    const bool DataHolder::doesKeyExist( const std::string& keyName ) const
    {
        bool isKeyFound = true;
        if ( _theKeyColumnMapping.find( keyName ) == _theKeyColumnMapping.end() ) 
        {
            isKeyFound = false;
        }
        return isKeyFound;
    }

    const int DataHolder::schemaIdx( const std::string& skName ) const
    {
        auto cit = std::find_if(	_theKeys.cbegin(),
                                    _theKeys.cend(),
                                    [&skName] ( const DataSchema & sk )
        {
            return sk.getName() == skName;
        } );
        return  (int)( cit != _theKeys.cend() ? cit - _theKeys.cbegin() : -1 );
    }

    // helper function giving an error string
    std::string DataHolder::constructColOverflowString( int index ) const
    {
        return std::string( ( boost::format( "Data matrix has %i columns, requested information column was %i " ) % _theData.size() % index ).str() );
    }

    void DataHolder::setDataForSchema( const std::string& schemaName,
									   const std::vector<std::vector<Variant>>& inputData,
									   const std::vector<ContainedTypeEnum>& types )
    {
        auto cit = std::find_if( _theKeys.cbegin(),
                                 _theKeys.cend(),
                                 [&schemaName] ( const DataSchema & sk )
        {
            return sk.getName() == schemaName;
        } );

        if( cit != _theKeys.end() )
        {
            const std::vector<int>& theColumnsToSet = _theKeyColumnMapping[ cit->getName() ];
            std::vector<ContainedTypeEnum> usedTypes = types;
            if( usedTypes.size() != inputData.size() ) // not set or invalid types ...
            {
                usedTypes = Variant::getContainedTypeInfo( inputData );
            }
            
			const size_t number_of_columns = usedTypes.size();
            if( number_of_columns != cit->getNumberOfColumns() )
            {
                throw ETradingException( ( boost::format( "#Error: DataSchema %s expects %i columns, however input data has %s columns" )
                                           % schemaName.c_str()
                                           % cit->getNumberOfColumns()
                                           % number_of_columns ).str() );
            }

			// Check Used Types Match Schema ... Except ...
			// ------------------------------------------------
			// 1.	No need to validate empty types
			// 2.	Data of size 1 can never be a variant, see Variant Class getContainedTypeInfo.
			//		Therefore we can not try to validate data where variant type is expected
			// 3.	Variant types allow any input so it makes no sense to validate variant data types
			for( size_t colCounter = 0; colCounter < number_of_columns; ++colCounter )
			{
				// Skip if empty & also skip validation of variant data since it can be any any type
				if ( cit->getColumnType(colCounter) == etrading::VARIANT_VALUE || usedTypes[colCounter] == etrading::EMPTY_VALUE )
				{
					continue;
				}

				if( cit->getColumnType( colCounter ) != usedTypes[ colCounter ] )
				{
					throw ETradingException( ( boost::format( "#Error: DataSchema %s expects column %i with type %s, however input data column %i has type %s" )
												% schemaName.c_str()
												% ( colCounter + 1 )
												% toString( cit->getColumnType( colCounter ) ).c_str()
												% ( colCounter + 1 )
												% toString( usedTypes[ colCounter ] ).c_str() ).str() );
				}
			}

            // We use a double loop to avoid setting anything if there is a mismatch
            for( size_t colCounter = 0; colCounter < number_of_columns; ++colCounter )
            {
                size_t columnIndex = theColumnsToSet[ colCounter ];
				if(_theData.size() >= (columnIndex + 1))
				{ 
					_theData[ columnIndex ].clear(); 
				}	
                _theData[ columnIndex ] = inputData[ colCounter ];
            }
        }
        else
        {
            throw ETradingException( ( boost::format( "#Error: No Schema with name %s exists" ) % schemaName.c_str() ).str().c_str() );
        }
    };

    void DataHolder::setDataForSchemaWithMap( const std::string& schemaName, const std::map<std::string, Variant>& inputMap )
    {
        std::vector<std::string> keys;
        boost::copy( inputMap | boost::adaptors::map_keys, std::back_inserter( keys ) );
        std::vector<Variant> values;
        boost::copy( inputMap | boost::adaptors::map_values, std::back_inserter( values ) );

        std::vector<etrading::ContainedTypeEnum> types = boost::assign::list_of( STRING_VALUE )( VARIANT_VALUE );

        VariantMatrix inputData;
        inputData.push_back( Variant::createVariantVector( keys, STRING_VALUE ) );
        inputData.push_back( values );

        setDataForSchema( schemaName, inputData, types );
    };

}