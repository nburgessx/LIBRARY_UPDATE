
#include <algorithm>
#include <utility>
#include <tuple>
#include <string>
#include <boost/format.hpp>


#include "tryAqObjectsMultiGrid.h"
#include "ETradingException.h"
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
    std::string tryAqObjectsMultiGridCreate(	const std::string& objectName,
                                            const std::vector<std::string>& gridNames,
                                            const std::vector<TableInfo>& infoBlocks,
                                            const bool allowJaggedData )
    {
        if( infoBlocks.size() <= 0 )
        {
			throw AQLCoreInvalidData( "#Error: No Grid Information provided to tryAqObjectsMultiGridCreate", __FILE__, __LINE__ );
        }

        if( gridNames.size() != infoBlocks.size() )
        {
			throw AQLCoreInvalidData( ( boost::format( "#Error: Number of Grid names (%i) does not match number of grid data ranges (%i)." )
                                   % gridNames.size()
                                   % infoBlocks.size() ).str().c_str(), __FILE__, __LINE__ );
        }

        const bool hasAnEmptyName = std::any_of( gridNames.cbegin(),
                                    gridNames.cend(),
                                    []( const std::string & gridName ) -> bool
        {
            return ( gridName.empty() || gridName == "" );
        } );
        if( hasAnEmptyName )
        {
			throw AQLCoreInvalidData( ( boost::format( "#Error: One of the individual grid names is empty or invalid (%s)" )
                                   % etrading::containerAsString( gridNames ).c_str() ).str().c_str(), __FILE__, __LINE__ );
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

        //return (	boost::format( "%s available as range (Environment: %s  Time: %s) " )
        //            %  objectName
        //            % etrading::Environment::DEFAULT_ENV_NAME
        //            % etrading::getCurrentDateTime() ).str();

    };

    std::pair<const FlexibleData, std::vector<std::string>>  tryAqObjectsMultiGridDisplay(
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
					throw AQLCoreInvalidData (  ( boost::format("#Error: Unable to retrieve GridName \"%s\" from the MultiGrid object." ) 
											 % gridName.c_str()).str().c_str(), __FILE__, __LINE__);
				}

                // extract the column name of the gridName
                const int index = objectWithData->schemaIdx( gridName );
                const std::vector<std::string>& columnNames = objectWithData->viewSchema( index ).getColumnNames();
                return std::make_pair( infoBlock, columnNames );

            }
            else
            {
				throw AQLCoreInvalidData(	( boost::format( "#Error: Found object \"%s\", but data is NULL" )
										  % objectName.c_str() ).str().c_str(), __FILE__, __LINE__ );
            }
        }
        else
        {
			throw AQLCoreInvalidData(	( boost::format( "#Error: Unable to find object with name \"%s\"" )
                                      % objectName.c_str() ).str().c_str(), __FILE__, __LINE__ );
        }
    };

    std::vector<std::string>
    tryAqObjectsMultiGridSubNames( const std::string&  objectName )
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
				throw AQLCoreInvalidData(	( boost::format( "#Error: Found object \"%s\", but data is NULL" )
										  % objectName.c_str() ).str().c_str(), __FILE__, __LINE__ );
            }
        }
        else
        {
			throw AQLCoreInvalidData(	( boost::format( "#Error: Unable to find object with name \"%s\"" )
                                      % objectName.c_str() ).str().c_str(), __FILE__, __LINE__ );
        }

    };

}