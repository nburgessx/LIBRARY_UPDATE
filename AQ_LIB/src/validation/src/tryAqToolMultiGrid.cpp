
#include <algorithm>
#include <utility>
#include <tuple>
#include <string>
#include <sstream>

#include "tryAqToolMultiGrid.h"
#include "DataSchema.h"
#include "EnvironmentUtilities.h"
#include "FreeObject.h"
#include "DateUtilities.h"
#include "FileUtilities.h"
#include "ContainerUtilities.h"
#include "EnvironmentPool.h"
#include "ObjectUtilities.h"


namespace validation
{
    std::string tryAqToolObjectMultiGridCreate(	const std::string& objectName,
                                            const std::vector<std::string>& gridNames,
                                            const std::vector<TableInfo>& infoBlocks,
                                            const bool allowJaggedData )
    {
        AQ_THROW_IF( infoBlocks.size() <= 0, "No Grid Information provided to tryAqToolObjectMultiGridCreate" );

        if( gridNames.size() != infoBlocks.size() )
        {
			std::ostringstream msg;
			msg << "Number of Grid names (" << gridNames.size() << ") does not match number of grid data ranges (" << infoBlocks.size() << ").";
			AQ_THROW( msg.str() );
        }

        const bool hasAnEmptyName = std::any_of( gridNames.cbegin(),
                                    gridNames.cend(),
                                    []( const std::string & gridName ) -> bool
        {
            return ( gridName.empty() || gridName == "" );
        } );
        if( hasAnEmptyName )
        {
			std::ostringstream msg;
			msg << "One of the individual grid names is empty or invalid (" << etrading::containerAsString( gridNames ) << ")";
			AQ_THROW( msg.str() );
        }

        // TODO:  encapsulate this in another function to clean up interface
        const std::vector<std::string>& columnNames = std::get<0>( infoBlocks[ 0 ] );
        const std::vector<etrading::ContainedTypeEnum>& colTypes = std::get<1>( infoBlocks[ 0 ] );
        const FlexibleData& rangeData = std::get<2>( infoBlocks[ 0 ] );
        etrading::FreeObject fo = createFreeObjectFromGrid( objectName, columnNames, colTypes, rangeData, gridNames[ 0 ], allowJaggedData );

        for( unsigned int gridCounter = 1; gridCounter < infoBlocks.size(); gridCounter++ )
        {
            const std::vector<std::string>& columnNames = std::get<0>( infoBlocks[ gridCounter ] );
            const std::vector<etrading::ContainedTypeEnum>& colTypes = std::get<1>( infoBlocks[ gridCounter ]) ;
            const FlexibleData& rangeData = std::get<2>( infoBlocks[ gridCounter ] );
            fo += createFreeObjectFromGrid( objectName, columnNames, colTypes, rangeData, gridNames[ gridCounter ], allowJaggedData );
        }

        etrading::copyToCache<etrading::FreeObject>( fo );
        return objectName;

        //std::ostringstream msg;
        //msg << objectName << " available as range (Environment: " << etrading::Environment::DEFAULT_ENV_NAME << "  Time: " << etrading::getCurrentDateTime() << ") ";
        //return msg.str();

    };

    std::pair<const FlexibleData, std::vector<std::string>>  tryAqToolObjectMultiGridDisplay(
                const std::string& objectName,
                const std::string& gridName )
    {

        auto& freeObjectStore = etrading::getObjectStore<etrading::FreeObject>( etrading::Environment::DEFAULT_ENV_NAME );
        if( freeObjectStore.has( objectName ) )
        {
            auto objectWithData = freeObjectStore.get( objectName );
            if( objectWithData )
            {
                // extract just the data for the schema
                const FlexibleData& allDataView = objectWithData->viewAllData();
				FlexibleData infoBlock;
				try
				{
					const auto columnIndices = objectWithData->columnsOfSchema( gridName );
					std::for_each( columnIndices.cbegin(), columnIndices.cend(), [&infoBlock, &allDataView]( const int idx )
					{
						infoBlock.push_back( allDataView[ idx ] );
					} );
				}
				catch (std::out_of_range e)
				{
					std::ostringstream msg;
					msg << "Unable to retrieve GridName \"" << gridName << "\" from the MultiGrid object.";
					AQ_THROW( msg.str() );
				}

                // extract the column name of the gridName
                const int index = objectWithData->schemaIdx( gridName );
                const std::vector<std::string>& columnNames = objectWithData->viewSchema( index ).getColumnNames();
                return std::make_pair( infoBlock, columnNames );

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
    tryAqToolObjectMultiGridSubNames( const std::string&  objectName )
    {

        auto& freeObjectStore = etrading::getObjectStore<etrading::FreeObject>( etrading::Environment::DEFAULT_ENV_NAME );
        if( freeObjectStore.has( objectName ) )
        {
            auto objectWithData = freeObjectStore.get( objectName );
            if( objectWithData )
            {
                std::vector<std::string> gridNames;
                const auto& schemas = objectWithData->snapshotOfSchemas();
                std::for_each( schemas.cbegin(), schemas.cend(), [&gridNames] ( const etrading::DataSchema & schema )
                {
                    gridNames.push_back( schema.getName() );
                } );
                return gridNames;
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

}