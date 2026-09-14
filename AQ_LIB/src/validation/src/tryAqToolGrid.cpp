#include <sstream>

#include "tryAqToolGrid.h"
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
    std::string tryAqToolObjectGridCreate(
        const std::string& objectName,
        const TableInfo& tableInfo,
        const bool allowJaggedData )
    {
        const std::vector<std::string>& columnNames = std::get<0>( tableInfo );
        const std::vector<etrading::ContainedTypeEnum>& colTypes = std::get<1>( tableInfo );
        const FlexibleData& rangeData = std::get<2>( tableInfo );
        const int numberOfColumns = colTypes.size();

        AQ_THROW_IF( rangeData.size() <= 0, "Empty data table was supplied to tryAqToolObjectGridCreate" );

        if( columnNames.size() != rangeData.size() )
        {
            std::ostringstream msg;
            msg << "Number of Column Names (" << columnNames.size() << ") does not match number of data columns (" << rangeData.size() << ").";
            AQ_THROW( msg.str() );
        }

        if( numberOfColumns != columnNames.size() )
        {
            std::ostringstream msg;
            msg << "Number of Column Names (" << columnNames.size() << ") does not match number of column types (" << colTypes.size() << ").";
            AQ_THROW( msg.str() );
        }

        etrading::FreeObject fo = createFreeObjectFromGrid( objectName, columnNames, colTypes, rangeData, objectName, allowJaggedData );

        etrading::copyToCache<etrading::FreeObject>( fo );

        return objectName;

        //std::ostringstream msg;
        //msg << objectName << " available as grid object (Environment: " << etrading::Environment::DEFAULT_ENV_NAME << "  Time: " << etrading::getCurrentDateTime() << ") ";
        //return msg.str();
    }

    std::pair<const FlexibleData, std::vector<std::string>>  tryAqToolObjectGridDisplay(
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
				std::ostringstream msg;
				msg << "Found object \"" << objectName << "\", but data is NULL";
				AQ_THROW( msg.str() );
            }
        }
        else
        {
			std::ostringstream msg;
			msg << "Unable to find object with name \"" << objectName << "\"";
			AQ_THROW( msg.str() );
        }
    };

    std::pair<const bool, std::string> tryAqToolObjectGridLoad(	const std::string& fileName )
    {
        // Append the file extension if missing
        std::string filenameWithExtension = etrading::appendFileExtension( fileName, etrading::JSON );

        if( !etrading::fileExists( filenameWithExtension ) )
        {
            std::ostringstream msg;
            msg << "File " << filenameWithExtension << " does not exist";
            AQ_THROW( msg.str() );
        }

        auto cacheInfoOnDeserialization = etrading::deSerializeFromJSON( etrading::serialize::FILE, filenameWithExtension );
        const std::string objectName = cacheInfoOnDeserialization.first;
        return std::make_pair( true, objectName );
    };

    std::string tryAqToolObjectGridSave( const std::string& objectName,
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
					std::ostringstream msg;
					msg << "Unable to write existing " << objectName << " object to file " << filenameWithExtension << " (check permission, directory, etc.)";
					AQ_THROW( msg.str() );
                }
                else
                {
                    std::ostringstream msg;
                    msg << "Object " << objectName << " was written to file " << filenameWithExtension;
                    return msg.str();
                }
            }
            else
            {
				std::ostringstream msg;
				msg << "Found object \"" << objectName << "\", but data is NULL";
				AQ_THROW( msg.str() );
            }
        }
        else
        {
			std::ostringstream msg;
			msg << "Unable to find object with name \"" << objectName << "\"";
			AQ_THROW( msg.str() );
        }
    };

    std::vector<std::string>
    tryAqToolObjectGridObjectNames()
    {
        auto& freeObjectStore = etrading::getObjectStore<etrading::FreeObject>( etrading::Environment::DEFAULT_ENV_NAME );
        return freeObjectStore.keys();
    };


    const bool tryAqToolObjectGridClearOne( const std::string& objectName )
    {
        auto& freeObjectStore = etrading::getObjectStore<etrading::FreeObject>( etrading::Environment::DEFAULT_ENV_NAME );

        if( freeObjectStore.has( objectName ) )
        {
            freeObjectStore.erase( objectName );
            return !freeObjectStore.has( objectName );
        }
        else
        {
			std::ostringstream msg;
			msg << "Unable to find object with name \"" << objectName << "\"";
			AQ_THROW( msg.str() );
            return false;  // not hit
        }
    };


    const bool tryAqToolObjectGridClearAll()
    {
        auto& freeObjectStore = etrading::getObjectStore<etrading::FreeObject>( etrading::Environment::DEFAULT_ENV_NAME );
        freeObjectStore.clear();
        return ( freeObjectStore.keys().size() == 0 );
    }


};
