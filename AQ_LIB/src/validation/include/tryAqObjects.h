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
#include "tryAqObjToolsGrid.h"

namespace validation
{

    bool tryAqObjExists( const std::string& typeAsString,
                         const std::string& objectName );

    // String Output - Get the object type, returns the first possible object type found
    std::string tryAqObjTypeAsString( const std::string& objectName );

    // Vector Output - Get the object type, returns a vector of possible object types
    std::vector<std::string> tryAqObjType( const std::string& objectName );

    std::vector<std::string> tryAqObjList( const std::string& typeAsString );

    bool tryAqObjDelete( const std::string& typeAsString, const std::string& objectName );

    int tryAqObjDeleteAll( const std::string& typeAsString );

    // Helper Function:
    std::string getObjectName( const std::tuple<std::string, etrading::CachedObjectEnum> & aqoLoadResultTuple );
    
	std::tuple<std::string, etrading::CachedObjectEnum> tryAqObjLoadAndReturnTupleResults( const std::string& fileName, const etrading::FileTypeEnum fileType = etrading::JSON, etrading::Environment& env = etrading::Environment::defaultEnv() );
    
    // Load a Single AQO object given the full file path
    std::string tryAqObjLoad( const std::string& fileName,
                              const etrading::FileTypeEnum fileType = etrading::JSON,
                              etrading::Environment& env = etrading::Environment::defaultEnv() );

    // Loads Multiple AQO objects of the Same Type by Referencing the Names and Folder
    std::vector<std::string> tryAqObjQuickLoad( const std::vector<std::string>& objectNames,
                                                const std::string& folder,
                                                const etrading::FileTypeEnum fileType = etrading::JSON,
                                                etrading::Environment& env = etrading::Environment::defaultEnv() );

    // Save a Single AQO object givent the full file path
	std::string tryAqObjSave( const std::string& objectName,
		                      const std::string& objectTypeString,
                              const std::string& fileName,
                              const etrading::FileTypeEnum fileType = etrading::JSON,
		                      etrading::Environment& env = etrading::Environment::defaultEnv() );

    // Loads Multiple AQO objects of the Same Type by Referencing the Names and Folder
    std::vector<std::string> tryAqObjQuickSave( const std::vector<std::string>& objectNames,
                                                const std::string& folder,
                                                const etrading::FileTypeEnum fileType = etrading::JSON,
		                                        etrading::Environment& env = etrading::Environment::defaultEnv() );

    std::tuple<std::string, etrading::CachedObjectEnum> tryAqObjLoadFromString( const std::string& jsonString, etrading::Environment& env = etrading::Environment::defaultEnv() );




}