#pragma once

#include <string>

/* @brief			swig interface for the aqToolsLoadCalendarFile method
*  @return			A notification string
*/
std::string aqToolsClearEntityPool();


/* @brief			swig interface for the aqObjClearCache method
*  @return	A string showing current version
*/
std::string aqObjClearCache();

/* @brief			swig interface for the aqToolsLoadCalendarFile method
*  @param [in]		filepath		The full name of the calendar file
*  @return			A notification string
*/
std::string aqToolsLoadCalendarFile(const std::string& filepath);

/* @brief			swig interface for the aqToolsLoadStaticData method
*  @param [in]		filepath The full name of the static data file
*  @return			A notification string
*/
std::string aqToolsLoadStaticData(const std::string& filepath);

/* @brief			swig interface for the aqToolsVersion method
*  @return	A string showing current version
*/
std::string aqToolsVersion();

/* @brief			swig interface for the aqToolsVersion method
*  @return	A string showing current version
*/
std::string aqToolsVersion();

// Method to enable/disable Parllel Mode
std::string aqToolsParallelModeEnable(const bool enable);

// Method to get the Parallel Mode Status
std::string aqToolsParallelModeStatus();
