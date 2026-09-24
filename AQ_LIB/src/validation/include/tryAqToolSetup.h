#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
	/* @brief			The single funnel every AlgoQuantLib consumer (GTEST, AQ_XLL, AQ_API)
	*                   initializes through. IDEMPOTENT: if AlgoQuantLib is already initialized,
	*                   this is a no-op that returns immediately -- it does not re-resolve paths,
	*                   does not re-apply any of the arguments below, and does not touch anything
	*                   already loaded. Use tryAqToolReset() to force a clean tear-down and reload,
	*                   e.g. to pick up a different config location or an edited config file on an
	*                   already-initialized library.
	*                   On the first call (nothing initialized yet), resolves the calendar /
	*                   central-bank-schedule / startup-config / ir-properties file paths, applies
	*                   any overrides given, builds the AlgoQuantLib data instance and loads the
	*                   optional startup-config generators (SWAP/BOND/CURVE_GENERATOR etc.).
	*  @param [in]		configFolder (optional)		Folder containing Calendar.csv / CBSchedule.csv /
	*                   startup.conf / ir.properties. If empty, each file falls through to its own
	*                   default resolution chain (see FolderConfig). Ignored if already initialized.
	*  @param [in]		calendarPath (optional)			Full-path override for the calendar file. Wins over configFolder. Ignored if already initialized.
	*  @param [in]		cbSchedulePath (optional)		Full-path override for the central-bank-schedule file. Wins over configFolder. Ignored if already initialized.
	*  @param [in]		startupConfigPath (optional)	Full-path override for the startup.conf file. Wins over configFolder. Ignored if already initialized.
	*  @param [in]		irPropsPath (optional)			Full-path override for the ir.properties file. Wins over configFolder. Ignored if already initialized.
	*  @param [in]		checkStaticDataLoaded (optional)	Throw if the ir static data failed to load. Default true. Ignored if already initialized.
	*  @param [in]		checkCalendarLoaded (optional)		Throw if the holiday calendars failed to load. Default true. Ignored if already initialized.
	*  @return			A notification string
	*/
	const std::string tryAqToolInitialize( const AQLString& configFolder = AQLString(),
	                                        const AQLString& calendarPath = AQLString(),
	                                        const AQLString& cbSchedulePath = AQLString(),
	                                        const AQLString& startupConfigPath = AQLString(),
	                                        const AQLString& irPropsPath = AQLString(),
	                                        bool checkStaticDataLoaded = true,
	                                        bool checkCalendarLoaded = true );

	/* @brief			Forces a clean reload: always tears down first (clears the AQObj object
	*                   cache, results containers and object pool, destroys the data-instance
	*                   singleton) regardless of whether AlgoQuantLib is already initialized, then
	*                   performs the same resolve-paths / rebuild / load-generators sequence as
	*                   tryAqToolInitialize's first call. Use this -- not tryAqToolInitialize -- when
	*                   the intent is to reinitialize with different settings or reload edited config
	*                   on an already-running library. Same parameter contract as tryAqToolInitialize.
	*  @return			A notification string
	*/
	const std::string tryAqToolReset( const AQLString& configFolder = AQLString(),
	                                   const AQLString& calendarPath = AQLString(),
	                                   const AQLString& cbSchedulePath = AQLString(),
	                                   const AQLString& startupConfigPath = AQLString(),
	                                   const AQLString& irPropsPath = AQLString(),
	                                   bool checkStaticDataLoaded = true,
	                                   bool checkCalendarLoaded = true );

	/* @brief			Tear-down counterpart to tryAqToolInitialize/tryAqToolReset: clears the AQObj
	*                   object cache, the curve/swap/credit results containers, the object pool and
	*                   the volatility manager, then destroys the AlgoQuantLib data-instance singleton.
	*  @return			A notification string
	*/
	const std::string tryAqToolTearDown();

    /* @brief	Clear the object pool
     *  @return	A notification string
     */
    AQLString tryAqToolClearEntityPool();

    /* @brief	Clear the object pool and the AQObj object cache
     *  @return	A notification string
     */
    AQLString tryAqObjectClearCache();


    /* @brief			validation interface for the aqToolLoadCalendarFile function
    *  @param [in]		filepath		The full name of the calendar file
    *  @return	A notification string
    */
    AQLString tryAqToolLoadCalendarFile( const AQLString& filepath );

    /* @brief			validation interface for the aqToolLoadStaticData function
    *  @param [in]		filepath The full name of the static data file
    *  @return			A notification string
    */
    AQLString tryAqToolLoadStaticData( const AQLString& filepath );
	
    /* @brief			validation interface for loading the optional AQObj configuration files
    *  @param [in]		configPath (optional)		The full path for the config file startup.conf. If empty, the default path will be used  
    *  @return			A notification string
    */
	AQLString tryAqToolLoadConfigurationFiles( const AQLString& configPath = "" );

    /* @brief			validation interface for the aqToolVersion function
    *  @param [in]		expiryMonth			An integer representing license expiry month
    *  @param [in]		expiryYear			An integer representing license expiry year
    *  @param [in]		showLicenceExpiry	Boolean to show licence expiry date. Defaults to false.
    *  @return	A string showing current version
    */
    std::string tryAqToolVersion( int& expiryMonth, int& expiryYear, bool showLicenceExpiry = false );

    // Method to enable OMP Parallelization Mode and OMP Threaded Methods
    std::string tryAqToolParallelModeEnable( const bool enable );

    // Method to get the OMP Parallelization Mode Status
    std::string tryAqToolParallelModeStatus();

}