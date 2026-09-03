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

