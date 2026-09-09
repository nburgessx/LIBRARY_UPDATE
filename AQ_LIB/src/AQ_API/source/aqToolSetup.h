#pragma once

#include <string>

/* @brief			swig interface for the aqToolLoadCalendarFile method
*  @return			A notification string
*/
std::string aqToolClearEntityPool();


/* @brief			swig interface for the aqObjectClearCache method
*  @return	A string showing current version
*/
std::string aqObjectClearCache();

/* @brief			swig interface for the aqToolLoadCalendarFile method
*  @param [in]		filepath		The full name of the calendar file
*  @return			A notification string
*/
std::string aqToolLoadCalendarFile(const std::string& filepath);

/* @brief			swig interface for the aqToolLoadStaticData method
*  @param [in]		filepath The full name of the static data file
*  @return			A notification string
*/
std::string aqToolLoadStaticData(const std::string& filepath);

/* @brief			swig interface for the aqToolVersion method
*  @return	A string showing current version
*/
std::string aqToolVersion();

/* @brief			swig interface for the aqToolVersion method
*  @return	A string showing current version
*/
std::string aqToolVersion();

// Method to enable/disable Parllel Mode
std::string aqToolParallelModeEnable(const bool enable);

// Method to get the Parallel Mode Status
std::string aqToolParallelModeStatus();
