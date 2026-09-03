/* 
 * @brief			Swig interface for meLWO... functions
 * @Created:		17 May 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#include "LACoreTemplateType.h"
#include "TypeUtilities.h"
#include "meLWO.h"
#include "tryMeLWO.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


 /* @brief			swig interface for meLWOSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
const std::string meLWOSave( const std::string& objectName, const std::string& objectType, const std::string& filePath )
{
    MLIB_API_START
    std::string result = validation_api::tryMeLWOSave( objectName, objectType, filePath );
	return result;
    MLIB_API_END
}


/* @brief			swig interface for meLWOLoad function
*  @param [in]		filePath		    The object file path
*/
const std::string meLWOLoad( const std::string& filePath )
{
    MLIB_API_START
    std::string result = validation_api::tryMeLWOLoad( filePath );
	return result;
    MLIB_API_END
}