#include "LACoreTemplateType.h"
#include "TypeUtilities.h"
#include "meLWO.h"
#include "tryMeLWO.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


 /* @brief			swig interface for meLWOSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
const std::string meLWOSave( const std::string& objectName, const std::string& objectType, const std::string& filePath )
{
    AQ_API_START
    std::string result = validation::tryMeLWOSave( objectName, objectType, filePath );
	return result;
    AQ_API_END
}


/* @brief			swig interface for meLWOLoad function
*  @param [in]		filePath		    The object file path
*/
const std::string meLWOLoad( const std::string& filePath )
{
    AQ_API_START
    std::string result = validation::tryMeLWOLoad( filePath );
	return result;
    AQ_API_END
}