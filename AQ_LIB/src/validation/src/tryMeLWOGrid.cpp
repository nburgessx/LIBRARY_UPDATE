#include <boost/format.hpp>


#include "tryMeLWOGrid.h"
#include "ETradingException.h"
#include "DataSchema.h"
#include "ObjectUtilities.h"
#include "EnvironmentUtilities.h"
#include "FreeObject.h"
#include "DateUtilities.h"
#include "FileUtilities.h"
#include "EnvironmentPool.h"
#include "StructuredExceptionHandler.h"
#include "CreateDataFile.h"
#include "Environment.h"
#include "SerializationUtilities.h"

using etrading::CreateDataFile;


namespace validation
{
    std::string tryMeLWOGridCreate(
        const std::string& objectName,
        const TableInfo& tableInfo,
        const bool allowJaggedData )
    {
        const std::vector<std::string>& columnNames = std::get<0>( tableInfo );
        const std::vector<etrading::ContainedTypeEnum>& colTypes = std::get<1>( tableInfo );
        const FlexibleData& rangeData = std::get<2>( tableInfo );
        const int numberOfColumns = colTypes.size();

        if( rangeData.size() <= 0 )
        {
            throw LACoreInvalidData( "Empty data table was supplied to tryMeLWOGridCreate", __FILE__, __LINE__ );
        }

        if( columnNames.size() != rangeData.size() )
        {
            throw LACoreInvalidData( ( boost::format( "Number of Column Names (%i) does not match number of data columns (%i)." )
                                   % columnNames.size()
                                   % rangeData.size() ).str().c_str(), __FILE__, __LINE__ );
        }

        if( numberOfColumns != columnNames.size() )
        {
            throw LACoreInvalidData( ( boost::format( "Number of Column Names (%i) does not match number of column types (%i)." )
                                   % columnNames.size()
                                   % colTypes.size() ).str().c_str() , __FILE__, __LINE__ );
        }

        etrading::FreeObject fo = createFreeObjectFromGrid( objectName, columnNames, colTypes, rangeData, objectName, allowJaggedData );

        etrading::copyToCache<etrading::FreeObject>( fo );

        return objectName;

        //return (	boost::format( "%s available as grid object (Environment: %s  Time: %s) " )
        //            %  objectName
        //            % etrading::Environment::DEFAULT_ENV_NAME
        //            % etrading::getCurrentDateTime() ).str();
    }

    std::pair<const FlexibleData, std::vector<std::string>>  tryMeLWOGridDisplay(
                const std::string& objectName )
    {
        auto& freeObjectStore = etrading::getObjectStore<etrading::FreeObject>( etrading::Environment::DEFAULT_ENV_NAME );
        if( freeObjectStore.has( objectName ) )
        {
            auto objectWithData = freeObjectStore.get( objectName );
            if( objectWithData )
            {
                const FlexibleData& dataView = objectWithData->viewAllData();
                const std::vector<std::string>& columnNames = objectWithData->viewSchema( 0 ).getColumnNames();
                return std::make_pair( dataView, columnNames );
            }
            else
            {
				throw LACoreInvalidData(	( boost::format( "#Error: Found object \"%s\", but data is NULL" )
										  % objectName.c_str() ).str().c_str(), __FILE__, __LINE__ );
            }
        }
        else
			throw LACoreInvalidData(	( boost::format( "#Error: Unable to find object with name \"%s\"" )
                                      % objectName.c_str() ).str().c_str(), __FILE__, __LINE__ );
    };

    std::pair<const bool, std::string> tryMeLWOGridLoad(	const std::string& fileName )
    {
        // Append the file extension if missing
        std::string filenameWithExtension = etrading::appendFileExtension( fileName, etrading::JSON );

        if( !etrading::fileExists( filenameWithExtension ) )
        {
            throw LACoreInvalidData(	( boost::format( "#Error: File %s does not exist" )  % filenameWithExtension.c_str() ).str().c_str(), __FILE__, __LINE__ );
        }

        auto cacheInfoOnDeserialization = etrading::deSerializeFromJSON( etrading::serialize::FILE, filenameWithExtension );
        const std::string objectName = cacheInfoOnDeserialization.first;
        return std::make_pair( true, objectName );
    };

    std::string tryMeLWOGridSave( const std::string& objectName,
                                 const std::string& fileNameToWriteTo )
    {
        auto& freeObjectStore = etrading::getObjectStore<etrading::FreeObject>( etrading::Environment::DEFAULT_ENV_NAME );
        if( freeObjectStore.has( objectName ) )
        {
            auto objectWithData = freeObjectStore.get( objectName );
            if( objectWithData )
            {
                // Append the file extension if missing
                std::string filenameWithExtension = etrading::appendFileExtension( fileNameToWriteTo, etrading::JSON );

                objectWithData->serialize( etrading::serialize::JSON, etrading::serialize::FILE, filenameWithExtension );
                if( !etrading::fileExists( filenameWithExtension ) )
                {
					throw LACoreAppError( ( boost::format( "#Error: Unable to write existing %s object to file %s (check permission, directory, etc.)" )
                                        % objectName
                                        % filenameWithExtension ).str().c_str(), __FILE__, __LINE__ );
                }
                else
                {
                    return ( boost::format( "Object %s was written to file %s" ) % objectName.c_str() % filenameWithExtension.c_str() ).str();
                }
            }
            else
            {
				throw LACoreInvalidData(	( boost::format( "#Error: Found object \"%s\", but data is NULL" )
                                          % objectName.c_str() ).str().c_str(), __FILE__, __LINE__ );
            }
        }
        else
			throw LACoreInvalidData(	( boost::format( "#Error: Unable to find object with name \"%s\"" )
                                      % objectName.c_str() ).str().c_str(), __FILE__, __LINE__ );
    };

    std::vector<std::string>
    tryMeLWOGridObjectNames()
    {
        auto& freeObjectStore = etrading::getObjectStore<etrading::FreeObject>( etrading::Environment::DEFAULT_ENV_NAME );
        return freeObjectStore.keys();
    };


    const bool tryMeLWOGridClearOne( const std::string& objectName )
    {
        auto& freeObjectStore = etrading::getObjectStore<etrading::FreeObject>( etrading::Environment::DEFAULT_ENV_NAME );

        if( freeObjectStore.has( objectName ) )
        {
            freeObjectStore.erase( objectName );
            return !freeObjectStore.has( objectName );
        }
        else
        {
			throw LACoreInvalidData(	( boost::format( "#Error: Unable to find object with name \"%s\"" )
                                      % objectName.c_str() ).str().c_str(), __FILE__, __LINE__ );
            return false;  // not hit
        }
    };


    const bool tryMeLWOGridClearAll()
    {
        auto& freeObjectStore = etrading::getObjectStore<etrading::FreeObject>( etrading::Environment::DEFAULT_ENV_NAME );
        freeObjectStore.clear();
        return ( freeObjectStore.keys().size() == 0 );
    }


};
