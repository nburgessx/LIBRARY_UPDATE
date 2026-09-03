#pragma once

#include <string>

/* @brief			swig interface for the meUtilityLoadCalendarFile method
*  @return			A notification string
*/
std::string meUtilityClearEntityPool();


/* @brief			swig interface for the meUtilityClearLWOCache method
*  @return	A string showing current version
*/
std::string meUtilityClearLWOCache();

/* @brief			swig interface for the meUtilityLoadCalendarFile method
*  @param [in]		filepath		The full name of the calendar file
*  @return			A notification string
*/
std::string meUtilityLoadCalendarFile(const std::string& filepath);

/* @brief			swig interface for the meUtilityLoadStaticData method
*  @param [in]		filepath The full name of the static data file
*  @return			A notification string
*/
std::string meUtilityLoadStaticData(const std::string& filepath);

/* @brief			swig interface for the meUtilityVersion method
*  @return	A string showing current version
*/
std::string meUtilityVersion();

/* @brief			swig interface for the meUtilityVersion method
*  @return	A string showing current version
*/
std::string meUtilityVersion();

// Method to enable/disable Parllel Mode
std::string meUtilityParallelModeEnable(const bool enable);

// Method to get the Parallel Mode Status
std::string meUtilityParallelModeStatus();
