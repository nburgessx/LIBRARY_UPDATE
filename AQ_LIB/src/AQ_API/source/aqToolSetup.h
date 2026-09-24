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

/* @brief			swig interface for the aqToolLoadConfigurationFiles function. Reloads the configuration
*                   files (calendars, static data, startup config) from the resolved config folder.
*  @return			A notification string
*/
std::string aqToolLoadConfigurationFiles();

/* @brief			swig interface for the aqToolInitialize function. Loads the configuration
*                   files (calendars, static data, startup config) from the resolved config folder.
*                   IDEMPOTENT: if AlgoQuantLib is already initialized, this is a no-op and every
*                   argument below is ignored -- object handles from an earlier call remain valid.
*                   Call aqToolReset() to force a clean tear-down and reload with different settings.
*  @param [in]		configFolder (Optional)		Folder containing Calendar.csv / CBSchedule.csv /
*                   startup.conf / ir.properties. If empty, each file falls through to its own
*                   default resolution chain. Ignored if already initialized.
*  @param [in]		calendarPath (Optional)			Full-path override for the calendar file. Wins over configFolder. Ignored if already initialized.
*  @param [in]		cbSchedulePath (Optional)		Full-path override for the central-bank-schedule file. Wins over configFolder. Ignored if already initialized.
*  @param [in]		startupConfigPath (Optional)	Full-path override for the startup.conf file. Wins over configFolder. Ignored if already initialized.
*  @param [in]		irPropsPath (Optional)			Full-path override for the ir.properties file. Wins over configFolder. Ignored if already initialized.
*  @return			A notification string
*/
std::string aqToolInitialize( const std::string& configFolder = "",
                               const std::string& calendarPath = "",
                               const std::string& cbSchedulePath = "",
                               const std::string& startupConfigPath = "",
                               const std::string& irPropsPath = "" );

/* @brief			swig interface for the aqToolReset function. Forces a clean reload: always tears
*                   down and rebuilds from scratch first, even if the library is already initialized
*                   -- so any object handle returned by an earlier call is no longer valid afterwards.
*                   Use this -- not aqToolInitialize -- to pick up a different config location or an
*                   edited config file on an already-initialized library.
*  @param [in]		configFolder (Optional)		Folder containing Calendar.csv / CBSchedule.csv /
*                   startup.conf / ir.properties. If empty, each file falls through to its own
*                   default resolution chain.
*  @param [in]		calendarPath (Optional)			Full-path override for the calendar file. Wins over configFolder.
*  @param [in]		cbSchedulePath (Optional)		Full-path override for the central-bank-schedule file. Wins over configFolder.
*  @param [in]		startupConfigPath (Optional)	Full-path override for the startup.conf file. Wins over configFolder.
*  @param [in]		irPropsPath (Optional)			Full-path override for the ir.properties file. Wins over configFolder.
*  @return			A notification string
*/
std::string aqToolReset( const std::string& configFolder = "",
                          const std::string& calendarPath = "",
                          const std::string& cbSchedulePath = "",
                          const std::string& startupConfigPath = "",
                          const std::string& irPropsPath = "" );

/* @brief			swig interface for the aqToolTearDown function. Clears the AQObj object cache,
*                   the curve/swap/credit results containers and the object pool, then destroys
*                   the AlgoQuantLib data-instance singleton. Call before the process exits.
*  @return			A notification string
*/
std::string aqToolTearDown();
