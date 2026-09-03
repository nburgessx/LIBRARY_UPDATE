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
#include <vector>
#include "SwigTypes.h"	// For SWIG_STRINGMATRIX which manages the LAStringMatrix idiosyncrasies type for R API

/* @brief			swig interface for meLWOSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
SWIG_STRINGMATRIX meLWOGridDisplay( const std::string& objectName, const bool& displayColumnNames = false );

