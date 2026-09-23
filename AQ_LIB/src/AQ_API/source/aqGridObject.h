// aqGridObject.h

/*
 * @brief			Swig interface for aqGridObject... lifecycle functions - create,
 *					save, load, list and clear a cached object grid (a cached
 *					rectangular data block). Display (aqGridObjectDisplay) and the
 *					related multi-grid functions already live in aqToolGrids.h.
 *					See src/AQ_XLL/src/xllTool.cpp for the marshalling reference.
 */

#pragma once
#include "SwigTypes.h"
#include "AQLCoreTemplateType.h"     // Data TypeDefs
#include <string>
#include <vector>


/* @brief			Create and store an object grid (a cached rectangular data block); returns its handle.
*  @param [in]		objectName			Name for the grid object
*  @param [in]		data				The range to store
*  @param [in]		allowJaggedData		Optional. Default FALSE. Allow columns of differing length
*  @returns			The grid object handle
*/
// data is an arbitrary-shape range (no fixed column count); see the matching note in aqToolData.h --
// excluded from the R binding rather than guessed at.
#if (!defined(SWIG_R)) && (!defined(SWIGR))
std::string aqGridObjectCreate( const std::string& objectName,
                                 const SWIG_STRINGMATRIX& data,
                                 const bool allowJaggedData = false );
#endif

/* @brief			Save an object grid to a file. Returns a status string.
*  @param [in]		objectName			A grid handle
*  @param [in]		fileNameToWriteTo	Full path to write the grid to
*/
std::string aqGridObjectSave( const std::string& objectName, const std::string& fileNameToWriteTo );

/* @brief			Load an object grid from a file. Returns a status string.
*  @param [in]		fileName			Full path to the grid file
*/
std::string aqGridObjectLoad( const std::string& fileName );

/* @brief			The names of every cached object grid.
*  @returns			The object grid names
*/
std::vector<std::string> aqGridObjectNames();

/* @brief			Remove one object grid from the cache. Returns TRUE on success.
*  @param [in]		objectName			A grid handle
*/
bool aqGridObjectClearOne( const std::string& objectName );

/* @brief			Remove every object grid from the cache. Returns TRUE on success.
*/
bool aqGridObjectClearAll();
