#include "aqToolSetup.h"
#include "tryAqToolSetup.h"
#include "InitializeETrading.h"		// etrading::InitializeETrading
#include "FolderConfig.h"			// etrading::FolderConfig
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

namespace
{
	// Shared message-building tail for aqToolInitialize/aqToolReset: reports which Calendar.csv
	// the library resolved to, once the underlying validation call has already run (and thrown, if
	// the load failed and checks were requested).
	std::string buildInitializedMessage( const std::string& prefix )
	{
		std::string message = prefix;

		const AQLString* resolvedCalendarPath = etrading::FolderConfig::calendar_path();
		if ( resolvedCalendarPath == nullptr || resolvedCalendarPath->size() == 0 )
		{
			message += "Holiday calendars failed to load";
		}
		else
		{
			// LAMBDA FUNCION: Convert forward slashes to backslashes for display on Windows
			auto toNativeSeparators = []( std::string path )
			{
				std::replace( path.begin(), path.end(), '/', '\\' );
				return path;
			};

			message += " Holiday calendars loaded from: ";
			message += toNativeSeparators( resolvedCalendarPath->getCString() );
		}

		return message;
	}
}


/* @brief			swig interface for the aqToolLoadCalendarFile method
*  @return			A notification string
*/
std::string aqToolClearEntityPool()
{
	AQ_API_START
	const std::string ret = validation::tryAqToolClearEntityPool().getCString();
    return ret;
	AQ_API_END
}

/* @brief			swig interface for the aqObjectClearCache method
*  @return	A string showing current version
*/
std::string aqObjectClearCache()
{
    AQ_API_START
    const std::string ret = validation::tryAqObjectClearCache().getCString();
    return ret;
	AQ_API_END
}

/* @brief			swig interface for the aqToolLoadCalendarFile method
*  @param [in]		filepath		The full name of the calendar file
*  @return			A notification string
*/
std::string aqToolLoadCalendarFile(const std::string& filepath)
{
	AQ_API_START
	AQLString tmp_filepath(filepath.c_str());
	const std::string ret = validation::tryAqToolLoadCalendarFile(tmp_filepath).getCString();
    return ret;
	AQ_API_END
}

/* @brief			swig interface for the aqToolLoadStaticData method
*  @param [in]		filepath The full name of the static data file
*  @return			A notification string
*/
std::string aqToolLoadStaticData(const std::string& filepath)
{
	AQ_API_START
	AQLString tmp_filepath(filepath.c_str());
	const std::string ret = validation::tryAqToolLoadStaticData(tmp_filepath).getCString();
    return ret;
	AQ_API_END
}

  
/* @brief			swig interface for the aqToolVersion method
*  @return	A string showing current version
*/
std::string aqToolVersion()
{
    AQ_API_START
	
    // TODO: Clean-up these dummy Excel Default Parameters
    int dummyExpiryMonth = 0;
    int dummyExpiryYear = 0;
    bool dummyShowExpiryDate = false;

	const std::string ret = validation::tryAqToolVersion(dummyExpiryMonth, dummyExpiryYear, dummyShowExpiryDate);
    return ret;

    AQ_API_END
}

// Method to enable/disable Parllel Mode
std::string aqToolParallelModeEnable(const bool enable)
{
    AQ_API_START
    
    const std::string result = validation::tryAqToolParallelModeEnable(enable);
    return result;
    
    AQ_API_END
}

// Method to get the Parallel Mode Status
std::string aqToolParallelModeStatus()
{
    AQ_API_START

    const std::string result = validation::tryAqToolParallelModeStatus();
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqToolLoadConfigurationFiles function. Reloads the configuration
*                   files (calendars, static data, startup config) from the resolved config folder.
*  @return			A notification string
*/
std::string aqToolLoadConfigurationFiles()
{
    AQ_API_START

    const std::string ret = validation::tryAqToolLoadConfigurationFiles().getCString();
    return ret;

    AQ_API_END
}

/* @brief			swig interface for the aqToolInitialize function. Loads the configuration
*                   files (calendars, static data, startup config) from the resolved config folder.
*                   IDEMPOTENT: a no-op (every argument ignored) if AlgoQuantLib is already
*                   initialized -- call aqToolReset() to force a reload with different settings.
*  @param [in]		configFolder (Optional)		Folder containing Calendar.csv / CBSchedule.csv /
*                   startup.conf / ir.properties. If empty, each file falls through to its own
*                   default resolution chain.
*  @param [in]		calendarPath (Optional)			Full-path override for the calendar file. Wins over configFolder.
*  @param [in]		cbSchedulePath (Optional)		Full-path override for the central-bank-schedule file. Wins over configFolder.
*  @param [in]		startupConfigPath (Optional)	Full-path override for the startup.conf file. Wins over configFolder.
*  @param [in]		irPropsPath (Optional)			Full-path override for the ir.properties file. Wins over configFolder.
*  @return			A notification string
*/
std::string aqToolInitialize( const std::string& configFolder, const std::string& calendarPath, const std::string& cbSchedulePath,
                               const std::string& startupConfigPath, const std::string& irPropsPath )
{
    AQ_API_START

	// checkStaticDataLoaded = true, checkCalendarLoaded = true -> throw, with the offending path,
	// if the calendars or static data did not load. No-ops (arguments ignored) if already initialized.
	validation::tryAqToolInitialize( AQLString( configFolder ), AQLString( calendarPath ),
	                                  AQLString( cbSchedulePath ), AQLString( startupConfigPath ),
	                                  AQLString( irPropsPath ), true, true );

	return buildInitializedMessage( "AlgoQuantLib initialised." );
	AQ_API_END
}

/* @brief			swig interface for the aqToolReset function. Forces a clean reload: always tears
*                   down and rebuilds from scratch first, even if the library is already initialized.
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
std::string aqToolReset( const std::string& configFolder, const std::string& calendarPath, const std::string& cbSchedulePath,
                          const std::string& startupConfigPath, const std::string& irPropsPath )
{
    AQ_API_START

	// checkStaticDataLoaded = true, checkCalendarLoaded = true -> throw, with the offending path,
	// if the calendars or static data did not load.
	validation::tryAqToolReset( AQLString( configFolder ), AQLString( calendarPath ),
	                             AQLString( cbSchedulePath ), AQLString( startupConfigPath ),
	                             AQLString( irPropsPath ), true, true );

	return buildInitializedMessage( "AlgoQuantLib reset and reinitialised." );
	AQ_API_END
}

/* @brief			swig interface for the aqToolTearDown function. Clears the AQObj object cache,
*                   the curve/swap/credit results containers and the object pool, then destroys
*                   the AlgoQuantLib data-instance singleton. Call before the process exits.
*  @return			A notification string
*/
std::string aqToolTearDown()
{
    AQ_API_START
    const std::string ret = validation::tryAqToolTearDown();
    return ret;
    AQ_API_END
}
