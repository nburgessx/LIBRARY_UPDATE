/*
 * @brief			validation interface for the meLWO object utility methods
 * @Created:		11 April 2016
 * @Author:			Hans Roggeman
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <utility>
#include <tuple>
#include <vector>

#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include "Variant.h"
#include "Environment.h"
#include "FileUtilities.h"
#include "CoreEnumerations.h"
#include "UserUtilities.h"
#include "tryMeLWOGrid.h"

namespace validation_api
{

    bool tryMeLWOExists( const std::string& typeAsString,
                         const std::string& objectName );

    // String Output - Get the object type, returns the first possible object type found
    std::string tryMeLWOTypeAsString( const std::string& objectName );

    // Vector Output - Get the object type, returns a vector of possible object types
    std::vector<std::string> tryMeLWOType( const std::string& objectName );

    std::vector<std::string> tryMeLWOList( const std::string& typeAsString );

    bool tryMeLWODelete( const std::string& typeAsString, const std::string& objectName );

    int tryMeLWODeleteAll( const std::string& typeAsString );

    // Helper Function:
    std::string getObjectName( const std::tuple<std::string, etrading::CachedObjectEnum> & lwoLoadResultTuple );
    
	std::tuple<std::string, etrading::CachedObjectEnum> tryMeLWOLoadAndReturnTupleResults( const std::string& fileName, const etrading::FileTypeEnum fileType = etrading::JSON, etrading::Environment& env = etrading::Environment::defaultEnv() );
    
    // Load a Single LWO Object given the full file path
    std::string tryMeLWOLoad( const std::string& fileName,
                              const etrading::FileTypeEnum fileType = etrading::JSON,
                              etrading::Environment& env = etrading::Environment::defaultEnv() );

    // Loads Multiple LWO Objects of the Same Type by Referencing the Names and Folder
    std::vector<std::string> tryMeLWOQuickLoad( const std::vector<std::string>& objectNames,
                                                const std::string& folder,
                                                const etrading::FileTypeEnum fileType = etrading::JSON,
                                                etrading::Environment& env = etrading::Environment::defaultEnv() );

    // Save a Single LWO Object givent the full file path
	std::string tryMeLWOSave( const std::string& objectName,
		                      const std::string& objectTypeString,
                              const std::string& fileName,
                              const etrading::FileTypeEnum fileType = etrading::JSON,
		                      etrading::Environment& env = etrading::Environment::defaultEnv() );

    // Loads Multiple LWO Objects of the Same Type by Referencing the Names and Folder
    std::vector<std::string> tryMeLWOQuickSave( const std::vector<std::string>& objectNames,
                                                const std::string& folder,
                                                const etrading::FileTypeEnum fileType = etrading::JSON,
		                                        etrading::Environment& env = etrading::Environment::defaultEnv() );

    std::tuple<std::string, etrading::CachedObjectEnum> tryMeLWOLoadFromString( const std::string& jsonString, etrading::Environment& env = etrading::Environment::defaultEnv() );




}