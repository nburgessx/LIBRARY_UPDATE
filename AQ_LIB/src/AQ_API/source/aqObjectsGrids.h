#pragma once

#include <string>
#include <vector>
#include "SwigTypes.h"	// For SWIG_STRINGMATRIX which manages the AQLStringMatrix idiosyncrasies type for R API

/* @brief			swig interface for aqObjectsSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
SWIG_STRINGMATRIX aqObjectsGridDisplay( const std::string& objectName, const bool& displayColumnNames = false );

