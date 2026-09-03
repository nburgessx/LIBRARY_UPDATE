#include "meUtilitySetup.h"
#include "tryMeUtilitySetup.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for the meUtilityLoadCalendarFile method
*  @return			A notification string
*/
std::string meUtilityClearEntityPool()
{
	AQ_API_START
	const std::string ret = validation_api::tryMeUtilityClearEntityPool().getCString();
    return ret;
	AQ_API_END
}

/* @brief			swig interface for the meUtilityClearLWOCache method
*  @return	A string showing current version
*/
std::string meUtilityClearLWOCache()
{
    AQ_API_START
    const std::string ret = validation_api::tryMeUtilityClearLWOCache().getCString();
    return ret;
	AQ_API_END
}

/* @brief			swig interface for the meUtilityLoadCalendarFile method
*  @param [in]		filepath		The full name of the calendar file
*  @return			A notification string
*/
std::string meUtilityLoadCalendarFile(const std::string& filepath)
{
	AQ_API_START
	LAString tmp_filepath(filepath.c_str());
	const std::string ret = validation_api::tryMeUtilityLoadCalendarFile(tmp_filepath).getCString();
    return ret;
	AQ_API_END
}

/* @brief			swig interface for the meUtilityLoadStaticData method
*  @param [in]		filepath The full name of the static data file
*  @return			A notification string
*/
std::string meUtilityLoadStaticData(const std::string& filepath)
{
	AQ_API_START
	LAString tmp_filepath(filepath.c_str());
	const std::string ret = validation_api::tryMeUtilityLoadStaticData(tmp_filepath).getCString();
    return ret;
	AQ_API_END
}

  
/* @brief			swig interface for the meUtilityVersion method
*  @return	A string showing current version
*/
std::string meUtilityVersion()
{
    AQ_API_START
	
    // TODO: Clean-up these dummy Excel Default Parameters
    int dummyExpiryMonth = 0;
    int dummyExpiryYear = 0;
    bool dummyShowExpiryDate = false;

	const std::string ret = validation_api::tryMeUtilityVersion(dummyExpiryMonth, dummyExpiryYear, dummyShowExpiryDate);
    return ret;

    AQ_API_END
}

// Method to enable/disable Parllel Mode
std::string meUtilityParallelModeEnable(const bool enable)
{
    AQ_API_START
    
    const std::string result = validation_api::tryMeUtilityParallelModeEnable(enable);
    return result;
    
    AQ_API_END
}

// Method to get the Parallel Mode Status
std::string meUtilityParallelModeStatus()
{
    AQ_API_START

    const std::string result = validation_api::tryMeUtilityParallelModeStatus();
    return result;

    AQ_API_END
}
