#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "aqObjects.h"
#include "tryAqObjects.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


 /* @brief			swig interface for aqObjSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjSave( const std::string& objectName, const std::string& objectType, const std::string& filePath )
{
    AQ_API_START
    std::string result = validation::tryAqObjSave( objectName, objectType, filePath );
	return result;
    AQ_API_END
}


/* @brief			swig interface for aqObjLoad function
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjLoad( const std::string& filePath )
{
    AQ_API_START
    std::string result = validation::tryAqObjLoad( filePath );
	return result;
    AQ_API_END
}