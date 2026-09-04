#pragma once
#include <string>


/* @brief			swig interface for aqObjectsSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjectsSave( const std::string& objectName, const std::string& objectType, const std::string& filePath );

/* @brief			swig interface for aqObjectsLoad function
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjectsLoad( const std::string& filePath );
