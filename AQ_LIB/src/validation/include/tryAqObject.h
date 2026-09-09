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

    bool tryAqObjectDelete( const std::string& typeAsString, const std::string& objectName );

    int tryAqObjectDeleteAll( const std::string& typeAsString );

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