#include "aqToolSetup.h"
#include "tryAqToolSetup.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


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
