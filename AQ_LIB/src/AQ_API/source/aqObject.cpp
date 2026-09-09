#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "aqObject.h"
#include "tryAqObject.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


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