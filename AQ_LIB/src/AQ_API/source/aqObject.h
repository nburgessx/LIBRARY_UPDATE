#pragma once
#include <string>


/* @brief			swig interface for aqObjectSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjectSave( const std::string& objectName, const std::string& objectType, const std::string& filePath );

/* @brief			swig interface for aqObjectLoad function
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjectLoad( const std::string& filePath );
