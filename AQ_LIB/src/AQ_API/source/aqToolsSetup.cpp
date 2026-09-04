#include "aqToolsSetup.h"
#include "tryAqToolsSetup.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for the aqToolsLoadCalendarFile method
*  @return			A notification string
*/
std::string aqToolsClearEntityPool()
{
	AQ_API_START
	const std::string ret = validation::tryAqToolsClearEntityPool().getCString();
    return ret;
	AQ_API_END
}

/* @brief			swig interface for the meUtilityClearLWOCache method
*  @return	A string showing current version
*/
std::string meUtilityClearLWOCache()
{
    AQ_API_START
    const std::string ret = validation::tryMeUtilityClearLWOCache().getCString();
    return ret;
	AQ_API_END
}

/* @brief			swig interface for the aqToolsLoadCalendarFile method
*  @param [in]		filepath		The full name of the calendar file
*  @return			A notification string
*/
std::string aqToolsLoadCalendarFile(const std::string& filepath)
{
	AQ_API_START
	AQLString tmp_filepath(filepath.c_str());
	const std::string ret = validation::tryAqToolsLoadCalendarFile(tmp_filepath).getCString();
    return ret;
	AQ_API_END
}

/* @brief			swig interface for the aqToolsLoadStaticData method
*  @param [in]		filepath The full name of the static data file
*  @return			A notification string
*/
std::string aqToolsLoadStaticData(const std::string& filepath)
{
	AQ_API_START
	AQLString tmp_filepath(filepath.c_str());
	const std::string ret = validation::tryAqToolsLoadStaticData(tmp_filepath).getCString();
    return ret;
	AQ_API_END
}

  
/* @brief			swig interface for the aqToolsVersion method
*  @return	A string showing current version
*/
std::string aqToolsVersion()
{
    AQ_API_START
	
    // TODO: Clean-up these dummy Excel Default Parameters
    int dummyExpiryMonth = 0;
    int dummyExpiryYear = 0;
    bool dummyShowExpiryDate = false;

	const std::string ret = validation::tryAqToolsVersion(dummyExpiryMonth, dummyExpiryYear, dummyShowExpiryDate);
    return ret;

    AQ_API_END
}

// Method to enable/disable Parllel Mode
std::string aqToolsParallelModeEnable(const bool enable)
{
    AQ_API_START
    
    const std::string result = validation::tryAqToolsParallelModeEnable(enable);
    return result;
    
    AQ_API_END
}

// Method to get the Parallel Mode Status
std::string aqToolsParallelModeStatus()
{
    AQ_API_START

    const std::string result = validation::tryAqToolsParallelModeStatus();
    return result;

    AQ_API_END
}
