#include <algorithm>

#include "tryMeLWO.h"
#include "LWOUtilities.h"
#include "EnvironmentUtilities.h"
#include "CoreEnumerations.h"
#include "ObjectUtilities.h"

#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "ContainerUtilities.h"
#include "SerializationUtilities.h"


using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation_api
{

    bool tryMeLWOExists( const std::string& typeAsString, const std::string& objectName )
    {
        bool doesObjectExist = etrading::doesLWOExist( objectName, typeAsString );
        return doesObjectExist;
    }

    // Get the object type, returns the first possible object type found
    std::string tryMeLWOTypeAsString( const std::string& objectName )
    {
        const std::vector<std::string> listOfPossibleTypes = tryMeLWOType( objectName );
        MLIB_REQUIRE( listOfPossibleTypes.size() > 0, "Invalid Object: Object does not exist" )
        const std::string result = listOfPossibleTypes[0];
        return result;
    }

    std::vector<std::string> tryMeLWOType( const std::string& objectName )
    {
        auto objectTypes = etrading::availableObjectTypes( objectName );
        std::vector<std::string> availableTypesAsStrings;
        std::transform( objectTypes.cbegin(), objectTypes.cend(), std::back_inserter( availableTypesAsStrings ),
                        []( const etrading::CachedObjectEnum enumValue )
        {
            return etrading::toString( enumValue );
        } );
        return availableTypesAsStrings;
    }


    std::vector<std::string> tryMeLWOList( const std::string& typeAsString )
    {
        const etrading::CachedObjectEnum objEnum = etrading::toCachedObjectEnum( etrading::trim_to_upper( typeAsString.c_str() ) );
        return etrading::Environment::defaultEnv().getObjectNames( objEnum );
    }


    bool tryMeLWODelete( const std::string& typeAsString, const std::string& objectName )
    {
        if ( !etrading::doesLWOExist( objectName, typeAsString ) )
        {
            MLIB_THROW( ( boost::format( "Object %s does not exist." ) % objectName.c_str() ).str().c_str() );
        }
        const etrading::CachedObjectEnum objEnum = etrading::toCachedObjectEnum( etrading::trim_to_upper( typeAsString.c_str() ) );
        return etrading::Environment::defaultEnv().deleteObject( objectName, objEnum );
    }

    
    int tryMeLWODeleteAll( const std::string& typeAsString )
    {
        const etrading::CachedObjectEnum objEnum = etrading::toCachedObjectEnum( etrading::trim_to_upper( typeAsString.c_str() ) );
        return etrading::Environment::defaultEnv().deleteAllObjects( objEnum );
    }

    
	// Helper Function
    // Function to get the object name from the meLWOLoad function which returns a tuple
    std::string getObjectName( const std::tuple<std::string, etrading::CachedObjectEnum> & lwoLoadResultTuple )
    {
        // Return the string name from the tuple in position 0
        return std::get<0>( lwoLoadResultTuple );
    }

    std::tuple<std::string, etrading::CachedObjectEnum> tryMeLWOLoadAndReturnTupleResults( const std::string& fileName, const etrading::FileTypeEnum fileType, etrading::Environment& env )
	{
        VALID_EXCEPTION_START

        // Append the file extension if missing
        std::string filenameWithExtension = etrading::appendFileExtension( fileName, fileType );

        // Check the input file exists
        etrading::checkFileExists( filenameWithExtension, fileType );

        // Deserialize and Cache the the input file
        auto cacheInfoOnDeserialization = etrading::deSerializeFromJSON( etrading::serialize::FILE, filenameWithExtension, env );
        
        std::string objectName = cacheInfoOnDeserialization.first;
		const etrading::CachedObjectEnum typeLoaded = cacheInfoOnDeserialization.second;
		
        return std::make_tuple( objectName, typeLoaded );

        VALID_EXCEPTION_END
	}


    std::string tryMeLWOLoad( const std::string& fileName, const etrading::FileTypeEnum fileType, etrading::Environment& env )
	{
        // No thread guard needed here since the underlying function 'tryMeLWOLoadAndReturnTupleResults' owns the thread guard
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD
        return getObjectName( tryMeLWOLoadAndReturnTupleResults( fileName, fileType, env ) );
        VALID_EXCEPTION_END
	}

    // Loads Multiple LWO Objects of the Same Type by Referencing the Names and Folder
    std::vector<std::string> tryMeLWOQuickLoad( const std::vector<std::string>& objectNames,
                                                const std::string& folder,
                                                const etrading::FileTypeEnum fileType,
                                                etrading::Environment& env )
    {
        // No thread guard needed here since the underlying function 'tryMeLWOLoadAndReturnTupleResults' owns the thread guard
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD
        
        std::vector<std::string> objectFilePaths;
        std::vector<std::string> cleansedObjectNames;
        
        // Check Folder Specified Correctly
        MLIB_REQUIRE( folder.size() > 0, "No Folder Specified")
        std::string folderWithForwardSlash = folder;
        
        if ( folder[ folder.size()-1 ] != '\\' && folder[ folder.size()-1 ] != '/' )
        {
            folderWithForwardSlash = folderWithForwardSlash + "/";
        }

        // Create the Object File Paths - Removing Blank Object Names
        for ( size_t n = 0; n < objectNames.size(); ++n )
        {
            std::string thisObjectName = objectNames[n];
            if ( !thisObjectName.empty() && thisObjectName != "" )
            {
                objectFilePaths.push_back( folderWithForwardSlash + thisObjectName + "." + toString(fileType) );
                cleansedObjectNames.push_back( thisObjectName );
            }
        }
            
        MLIB_REQUIRE( objectFilePaths.size() > 0, "Unable to Load: No Object Names Provided" )
        std::vector<std::string> resultsVector( objectFilePaths.size() );

        for ( size_t i = 0; i < objectFilePaths.size(); ++i )
        {
            // Iterate over and try to load each object. Don't throw if one object fails to load, ...
            // ... rather try to load all objects and report failures in the output results vector.
            MLIB_SET_VARIABLE_OR_OVERRIDE_ON_FAILURE( 
                resultsVector[i],
                tryMeLWOLoad( objectFilePaths[i], fileType, env ),
                std::string( "Not Loaded: Object '" + cleansedObjectNames[i] + "' does not exist in folder specified") );
        }

        return resultsVector;

        VALID_EXCEPTION_END
    }

	std::string tryMeLWOSave( const std::string& objectName, const std::string& objectTypeString, const std::string& fileName, const etrading::FileTypeEnum fileType, etrading::Environment& env )
	{
        VALID_EXCEPTION_START

        // Get object type
        std::string objectTypeUsed = objectTypeString;
        std::vector<std::string> objectTypes = tryMeLWOType( objectName );
        
        // Use object type string if provided otherwise determine from the object type method
        if ( objectTypeString.empty() || objectTypeString.size() == 0 )
        {
            MLIB_REQUIRE( objectTypes.size() == 1, "Unable to save object - Object does not exist or duplicate object types with the same name. If a duplicate please specify object type" )
            objectTypeUsed = objectTypes[0];
        }
        
		const etrading::CachedObjectEnum objectType = etrading::toCachedObjectEnum(objectTypeUsed);
		
        MLIB_REQUIRE( env.hasObject( objectName, objectType ), "Object '" + objectName + "' with type '" + toString(objectType) + " ' does not exist")
		
		// typename etrading::to_cached_object_type<objectType>::type  // this does not work because it is not a constant expression for compilation
		// so we should try to code to an interface
		auto lwoPtr = env.accessObjectInterface( objectName , objectType); // etrading::getLWOCurve(lwoCurveName);
		
        // For Intel Linux Compiler use nullptr = {}
        MLIB_REQUIRE( lwoPtr != nullptr, "Object '" + objectName + " ' with type '" + toString(objectType) + "' does not exist" )

        // Append the file extension if missing
        std::string filenameWithExtension = etrading::appendFileExtension( fileName, fileType );

        // Serialize
		lwoPtr->serialize( etrading::serialize::JSON, etrading::serialize::FILE, filenameWithExtension );
			
        // Check the Serialization file was created
        etrading::checkFileExists( filenameWithExtension, fileType );
			
        // Create the Result String
        std::string resultString;
        MLIB_SET_VARIABLE( resultString, "Saved: Object '" + objectName + "' saved to file " + filenameWithExtension )
        
        return resultString;
		
        VALID_EXCEPTION_END
	}

    // Loads Multiple LWO Objects of the Same Type by Referencing the Names and Folder
    std::vector<std::string> tryMeLWOQuickSave( const std::vector<std::string>& objectNames,
                                                const std::string& folder,
                                                const etrading::FileTypeEnum fileType,
		                                        etrading::Environment& env )
    {
	    // No thread guard needed here since the underlying function 'tryMeLWOSave' owns the thread guard
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD
        
        std::vector<std::string> objectFilePaths;
        std::vector<std::string> cleansedObjectNames;

        // Check Folder Specified Correctly
        MLIB_REQUIRE( folder.size() > 0, "No Folder Specified")
        std::string folderWithForwardSlash = folder;
        
        if ( folder[ folder.size()-1 ] != '\\' && folder[ folder.size()-1 ] != '/' )
        {
            folderWithForwardSlash = folderWithForwardSlash + "/";
        }

        // Create the Object File Paths - Removing Blank Object Names
        for ( size_t n = 0; n < objectNames.size(); ++n )
        {
            std::string thisObjectName = objectNames[n];
            if ( !thisObjectName.empty() && thisObjectName != "" )
            {
                objectFilePaths.push_back( folderWithForwardSlash + thisObjectName + "." + toString(fileType) );
                cleansedObjectNames.push_back( thisObjectName );
            }
        }
        
        // Dimension & Access Violation Checks
        MLIB_REQUIRE( objectFilePaths.size() > 0, "Unable to Save: No Object Names Provided" )
        MLIB_REQUIRE( objectFilePaths.size() == cleansedObjectNames.size(), "Unable to Save: Inconsistent number of Object names and filepaths" )

        std::vector<std::string> resultsVector( objectFilePaths.size() );

        for ( size_t i = 0; i < objectFilePaths.size(); ++i )
        {
            std::string thisObjectType;

            // Iterate over and try to get the type of each object. Don't throw if we can't find one object type, ...
            // ... rather report failures in the output results vector.
            MLIB_SET_VARIABLE_OR_OVERRIDE_ON_FAILURE(
                thisObjectType,
                tryMeLWOTypeAsString( cleansedObjectNames[i] ),
                std::string("UNKNOWN_TYPE") );

            // Iterate over and try to load each object. Don't throw if one object fails to load, ...
            // ... rather try to load all objects and report failures in the output results vector.
            MLIB_SET_VARIABLE_OR_OVERRIDE_ON_FAILURE( 
                resultsVector[i],
                tryMeLWOSave( cleansedObjectNames[i], thisObjectType, objectFilePaths[i], fileType, env ),
                std::string( "Not Saved: Invalid Folder or Object '" + cleansedObjectNames[i] + " ' of type '" + thisObjectType + "' does not exist" ) );
        }

        return resultsVector;
        
        VALID_EXCEPTION_END
    }

    std::tuple<std::string, etrading::CachedObjectEnum> tryMeLWOLoadFromString( const std::string& jsonString, etrading::Environment& env )
    {
        VALID_EXCEPTION_START

        auto cacheInfoOnDeserialization = etrading::deSerializeFromJSON( etrading::serialize::STRING, jsonString, env );
        std::string objectName = cacheInfoOnDeserialization.first;
		const etrading::CachedObjectEnum typeLoaded = cacheInfoOnDeserialization.second;
		return std::make_tuple(objectName ,typeLoaded);
 
        VALID_EXCEPTION_END            
    }

}