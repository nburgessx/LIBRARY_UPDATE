#pragma once

#include <string>
#include <utility>
#include <tuple>
#include <vector>

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "Variant.h"
#include "Environment.h"
#include "FileUtilities.h"
#include "CoreEnumerations.h"
#include "UserUtilities.h"
#include "tryAqToolGrid.h"

namespace validation
{

    bool tryAqObjectExists( const std::string& typeAsString,
                         const std::string& objectName );

    // String Output - Get the object type, returns the first possible object type found
    std::string tryAqObjectTypeAsString( const std::string& objectName );

    // Vector Output - Get the object type, returns a vector of possible object types
    std::vector<std::string> tryAqObjectType( const std::string& objectName );

    std::vector<std::string> tryAqObjectList( const std::string& typeAsString );

    // Every cached object name, across every object type. Backs aqObjectList
    // / aqObjectDeleteAll when no type is given (2026-09-12, Nicholas).
    std::vector<std::string> tryAqObjectList();

    bool tryAqObjectDelete( const std::string& typeAsString, const std::string& objectName );

    int tryAqObjectDeleteAll( const std::string& typeAsString );

    // Delete every cached object of every type. Backs aqObjectDeleteAll with
    // no ObjectType argument (2026-09-12, Nicholas) - the same
    // etrading::deleteAllObjects(Environment&) call tryAqObjectClearCache
    // already uses to clear every category, exposed here on its own so
    // aqObjectDeleteAll can report a count without also resetting the
    // config/entity-pool state that a full ClearCache does.
    int tryAqObjectDeleteAll();

    // Helper Function:
    std::string getObjectName( const std::tuple<std::string, etrading::CachedObjectEnum> & aqObjLoadResultTuple );
    
	std::tuple<std::string, etrading::CachedObjectEnum> tryAqObjectLoadAndReturnTupleResults( const std::string& fileName, const etrading::FileTypeEnum fileType = etrading::JSON, etrading::Environment& env = etrading::Environment::defaultEnv() );
    
    // Load a Single AQObj object given the full file path
    std::string tryAqObjectLoad( const std::string& fileName,
                              const etrading::FileTypeEnum fileType = etrading::JSON,
                              etrading::Environment& env = etrading::Environment::defaultEnv() );

    // Loads Multiple AQObj objects of the Same Type by Referencing the Names and Folder
    std::vector<std::string> tryAqObjectQuickLoad( const std::vector<std::string>& objectNames,
                                                const std::string& folder,
                                                const etrading::FileTypeEnum fileType = etrading::JSON,
                                                etrading::Environment& env = etrading::Environment::defaultEnv() );

    // Save a Single AQObj object givent the full file path
	std::string tryAqObjectSave( const std::string& objectName,
		                      const std::string& objectTypeString,
                              const std::string& fileName,
                              const etrading::FileTypeEnum fileType = etrading::JSON,
		                      etrading::Environment& env = etrading::Environment::defaultEnv() );

    // Loads Multiple AQObj objects of the Same Type by Referencing the Names and Folder
    std::vector<std::string> tryAqObjectQuickSave( const std::vector<std::string>& objectNames,
                                                const std::string& folder,
                                                const etrading::FileTypeEnum fileType = etrading::JSON,
		                                        etrading::Environment& env = etrading::Environment::defaultEnv() );

    std::tuple<std::string, etrading::CachedObjectEnum> tryAqObjectLoadFromString( const std::string& jsonString, etrading::Environment& env = etrading::Environment::defaultEnv() );




}