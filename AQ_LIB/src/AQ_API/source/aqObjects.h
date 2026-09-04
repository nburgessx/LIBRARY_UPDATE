#pragma once
#include <string>


/* @brief			swig interface for aqObjSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjSave( const std::string& objectName, const std::string& objectType, const std::string& filePath );

/* @brief			swig interface for aqObjLoad function
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjLoad( const std::string& filePath );
