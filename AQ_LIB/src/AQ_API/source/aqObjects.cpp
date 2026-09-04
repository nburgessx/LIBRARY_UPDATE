#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "aqObjects.h"
#include "tryAqObjects.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


 /* @brief			swig interface for aqObjectsSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjectsSave( const std::string& objectName, const std::string& objectType, const std::string& filePath )
{
    AQ_API_START
    std::string result = validation::tryAqObjectsSave( objectName, objectType, filePath );
	return result;
    AQ_API_END
}


/* @brief			swig interface for aqObjectsLoad function
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjectsLoad( const std::string& filePath )
{
    AQ_API_START
    std::string result = validation::tryAqObjectsLoad( filePath );
	return result;
    AQ_API_END
}