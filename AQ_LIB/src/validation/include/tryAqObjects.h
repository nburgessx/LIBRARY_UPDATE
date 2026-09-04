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
#include "tryAqObjectsGrid.h"

namespace validation
{

    bool tryAqObjectsExists( const std::string& typeAsString,
                         const std::string& objectName );

    // String Output - Get the object type, returns the first possible object type found
    std::string tryAqObjectsTypeAsString( const std::string& objectName );

    // Vector Output - Get the object type, returns a vector of possible object types
    std::vector<std::string> tryAqObjectsType( const std::string& objectName );

    std::vector<std::string> tryAqObjectsList( const std::string& typeAsString );

    bool tryAqObjectsDelete( const std::string& typeAsString, const std::string& objectName );

    int tryAqObjectsDeleteAll( const std::string& typeAsString );

    // Helper Function:
    std::string getObjectName( const std::tuple<std::string, etrading::CachedObjectEnum> & lwoLoadResultTuple );
    
	std::tuple<std::string, etrading::CachedObjectEnum> tryAqObjectsLoadAndReturnTupleResults( const std::string& fileName, const etrading::FileTypeEnum fileType = etrading::JSON, etrading::Environment& env = etrading::Environment::defaultEnv() );
    
    // Load a Single AQO object given the full file path
    std::string tryAqObjectsLoad( const std::string& fileName,
                              const etrading::FileTypeEnum fileType = etrading::JSON,
                              etrading::Environment& env = etrading::Environment::defaultEnv() );

    // Loads Multiple AQO objects of the Same Type by Referencing the Names and Folder
    std::vector<std::string> tryAqObjectsQuickLoad( const std::vector<std::string>& objectNames,
                                                const std::string& folder,
                                                const etrading::FileTypeEnum fileType = etrading::JSON,
                                                etrading::Environment& env = etrading::Environment::defaultEnv() );

    // Save a Single AQO object givent the full file path
	std::string tryAqObjectsSave( const std::string& objectName,
		                      const std::string& objectTypeString,
                              const std::string& fileName,
                              const etrading::FileTypeEnum fileType = etrading::JSON,
		                      etrading::Environment& env = etrading::Environment::defaultEnv() );

    // Loads Multiple AQO objects of the Same Type by Referencing the Names and Folder
    std::vector<std::string> tryAqObjectsQuickSave( const std::vector<std::string>& objectNames,
                                                const std::string& folder,
                                                const etrading::FileTypeEnum fileType = etrading::JSON,
		                                        etrading::Environment& env = etrading::Environment::defaultEnv() );

    std::tuple<std::string, etrading::CachedObjectEnum> tryAqObjectsLoadFromString( const std::string& jsonString, etrading::Environment& env = etrading::Environment::defaultEnv() );




}