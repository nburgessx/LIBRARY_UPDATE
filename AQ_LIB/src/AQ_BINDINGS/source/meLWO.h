/* 
 * @brief			Swig interface for meLWO... functions
 * @Created:		17 May 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include <string>


/* @brief			swig interface for meLWOSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
const std::string meLWOSave( const std::string& objectName, const std::string& objectType, const std::string& filePath );

/* @brief			swig interface for meLWOLoad function
*  @param [in]		filePath		    The object file path
*/
const std::string meLWOLoad( const std::string& filePath );
