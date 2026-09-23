#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "aqObject.h"
#include "tryAqObject.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "CoreEnumerations.h"       // etrading::toString( CachedObjectEnum )


 /* @brief			swig interface for aqObjectSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjectSave( const std::string& objectName, const std::string& objectType, const std::string& filePath )
{
    AQ_API_START
    std::string result = validation::tryAqObjectSave( objectName, objectType, filePath );
	return result;
    AQ_API_END
}


/* @brief			swig interface for aqObjectLoad function
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjectLoad( const std::string& filePath )
{
    AQ_API_START
    std::string result = validation::tryAqObjectLoad( filePath );
	return result;
    AQ_API_END
}


/* @brief			swig interface for the aqObjectExists method
*/
bool aqObjectExists( const std::string& typeAsString, const std::string& objectName )
{
    AQ_API_START
    bool result = validation::tryAqObjectExists( typeAsString, objectName );
    return result;
    AQ_API_END
}


/* @brief			swig interface for the aqObjectTypeAsString method
*/
std::string aqObjectTypeAsString( const std::string& objectName )
{
    AQ_API_START
    std::string result = validation::tryAqObjectTypeAsString( objectName );
    return result;
    AQ_API_END
}


/* @brief			swig interface for the aqObjectType method
*/
std::vector<std::string> aqObjectType( const std::string& objectName )
{
    AQ_API_START
    std::vector<std::string> result = validation::tryAqObjectType( objectName );
    return result;
    AQ_API_END
}


/* @brief			swig interface for the aqObjectList method.
*					An empty typeAsString lists every cached object of every type (validation::tryAqObjectList()).
*/
std::vector<std::string> aqObjectList( const std::string& typeAsString )
{
    AQ_API_START
    std::vector<std::string> result = typeAsString.empty()
        ? validation::tryAqObjectList()
        : validation::tryAqObjectList( typeAsString );
    return result;
    AQ_API_END
}


/* @brief			swig interface for the aqObjectDelete method
*/
bool aqObjectDelete( const std::string& typeAsString, const std::string& objectName )
{
    AQ_API_START
    bool stillExists = validation::tryAqObjectDelete( typeAsString, objectName );
    return !stillExists;
    AQ_API_END
}


/* @brief			swig interface for the aqObjectDeleteAll method.
*					An empty typeAsString deletes every cached object of every type (validation::tryAqObjectDeleteAll()).
*/
int aqObjectDeleteAll( const std::string& typeAsString )
{
    AQ_API_START
    int result = typeAsString.empty()
        ? validation::tryAqObjectDeleteAll()
        : validation::tryAqObjectDeleteAll( typeAsString );
    return result;
    AQ_API_END
}


/* @brief			swig interface for the aqObjectLoadAndReturnTupleResults method
*/
std::vector<std::string> aqObjectLoadAndReturnTupleResults( const std::string& fileName )
{
    AQ_API_START

    std::tuple<std::string, etrading::CachedObjectEnum> loaded = validation::tryAqObjectLoadAndReturnTupleResults( fileName );

    // Marshall Output(s) - [object handle, cached object type], matching the AQ_XLL 2-row column shape
    std::vector<std::string> result;
    result.push_back( std::get<0>( loaded ) );
    result.push_back( etrading::toString( std::get<1>( loaded ) ) );
    return result;

    AQ_API_END
}


/* @brief			swig interface for the aqObjectLoadFromString method
*/
std::string aqObjectLoadFromString( const std::string& jsonString )
{
    AQ_API_START

    std::tuple<std::string, etrading::CachedObjectEnum> loaded = validation::tryAqObjectLoadFromString( jsonString );
    std::string result = std::get<0>( loaded );
    return result;

    AQ_API_END
}


/* @brief			swig interface for the aqObjectQuickLoad method
*/
std::vector<std::string> aqObjectQuickLoad( const std::vector<std::string>& objectNames, const std::string& folder )
{
    AQ_API_START
    std::vector<std::string> result = validation::tryAqObjectQuickLoad( objectNames, folder );
    return result;
    AQ_API_END
}


/* @brief			swig interface for the aqObjectQuickSave method
*/
std::vector<std::string> aqObjectQuickSave( const std::vector<std::string>& objectNames, const std::string& folder )
{
    AQ_API_START
    std::vector<std::string> result = validation::tryAqObjectQuickSave( objectNames, folder );
    return result;
    AQ_API_END
}