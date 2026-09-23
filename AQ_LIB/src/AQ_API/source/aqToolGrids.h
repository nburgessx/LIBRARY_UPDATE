#pragma once

#include <string>
#include <vector>
#include "SwigTypes.h"	// For SWIG_STRINGMATRIX which manages the AQLStringMatrix idiosyncrasies type for R API

/* @brief			swig interface for aqObjectSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
SWIG_STRINGMATRIX aqGridObjectDisplay( const std::string& objectName, const bool& displayColumnNames = false );

// Each grid is an arbitrary-shape data block (no fixed column count); see the matching note in
// aqToolData.h -- excluded from the R binding rather than guessed at.
#if (!defined(SWIG_R)) && (!defined(SWIGR))
/* @brief			swig interface for aqToolObjectMultiGridCreate. Create and store a multi-grid from up to three named grids.
*  @param [in]		objectName			Name for the multi-grid object
*  @param [in]		gridNames			The name of each grid within the multi-grid
*  @param [in]		grids				The grids, one string matrix per name
*  @param [in]		allowJaggedData		Optional. Default FALSE. Allow columns of differing length
*  @return			The multi-grid object handle
*/
std::string aqToolObjectMultiGridCreate( const std::string& objectName,
                                          const std::vector<std::string>& gridNames,
                                          const std::vector<SWIG_STRINGMATRIX>& grids,
                                          const bool allowJaggedData = false );
#endif

/* @brief			swig interface for aqToolObjectMultiGridDisplay. Display one named grid of a multi-grid as a matrix.
*  @param [in]		objectName		A multi-grid handle
*  @param [in]		gridName		The grid within the multi-grid to display
*  @return			A string matrix of the grid's data
*/
SWIG_STRINGMATRIX aqToolObjectMultiGridDisplay( const std::string& objectName, const std::string& gridName );

/* @brief			swig interface for aqToolObjectMultiGridSubNames. The sub-grid names held by a multi-grid.
*  @param [in]		objectName		A multi-grid handle
*  @return			The sub-grid names
*/
std::vector<std::string> aqToolObjectMultiGridSubNames( const std::string& objectName );

