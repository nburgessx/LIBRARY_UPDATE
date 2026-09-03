/* 
 * @brief			Swig interface to Java for meUtility setup related functions
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "meUtilitySetup.h"
#include "tryMeUtilitySetup.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for the meUtilityLoadCalendarFile method
*  @return			A notification string
*/
std::string meUtilityClearEntityPool()
{
	MLIB_API_START
	const std::string ret = validation_api::tryMeUtilityClearEntityPool().getCString();
    return ret;
	MLIB_API_END
}

/* @brief			swig interface for the meUtilityClearLWOCache method
*  @return	A string showing current version
*/
std::string meUtilityClearLWOCache()
{
    MLIB_API_START
    const std::string ret = validation_api::tryMeUtilityClearLWOCache().getCString();
    return ret;
	MLIB_API_END
}

/* @brief			swig interface for the meUtilityLoadCalendarFile method
*  @param [in]		filepath		The full name of the calendar file
*  @return			A notification string
*/
std::string meUtilityLoadCalendarFile(const std::string& filepath)
{
	MLIB_API_START
	LAString tmp_filepath(filepath.c_str());
	const std::string ret = validation_api::tryMeUtilityLoadCalendarFile(tmp_filepath).getCString();
    return ret;
	MLIB_API_END
}

/* @brief			swig interface for the meUtilityLoadStaticData method
*  @param [in]		filepath The full name of the static data file
*  @return			A notification string
*/
std::string meUtilityLoadStaticData(const std::string& filepath)
{
	MLIB_API_START
	LAString tmp_filepath(filepath.c_str());
	const std::string ret = validation_api::tryMeUtilityLoadStaticData(tmp_filepath).getCString();
    return ret;
	MLIB_API_END
}

  
/* @brief			swig interface for the meUtilityVersion method
*  @return	A string showing current version
*/
std::string meUtilityVersion()
{
    MLIB_API_START
	
    // TODO: Clean-up these dummy Excel Default Parameters
    int dummyExpiryMonth = 0;
    int dummyExpiryYear = 0;
    bool dummyShowExpiryDate = false;

	const std::string ret = validation_api::tryMeUtilityVersion(dummyExpiryMonth, dummyExpiryYear, dummyShowExpiryDate);
    return ret;

    MLIB_API_END
}

// Method to enable/disable Parllel Mode
std::string meUtilityParallelModeEnable(const bool enable)
{
    MLIB_API_START
    
    const std::string result = validation_api::tryMeUtilityParallelModeEnable(enable);
    return result;
    
    MLIB_API_END
}

// Method to get the Parallel Mode Status
std::string meUtilityParallelModeStatus()
{
    MLIB_API_START

    const std::string result = validation_api::tryMeUtilityParallelModeStatus();
    return result;

    MLIB_API_END
}
