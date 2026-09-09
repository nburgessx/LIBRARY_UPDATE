#include "TypeUtilities.h"
#include "aqDateIMM.h"
#include "tryAqDateIMM.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"


/* @brief			swig interface for aqDateIMMFromMonth. 
*  @param [in]		year			The year of the IMM date
*  @param [in]		month			The month of the IMM date
*  @param [in]		calendar		Calendar
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The IMM date on the given month
*/
std::string aqDateIMMFromMonth(int year, 
							int month, 
							const std::string& calendar, 
							const std::string& businessDayAdj)
{
	AQ_API_START
    std::string ret;
	
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());

	ret = validation::tryAqDateIMMFromMonth(year, month, tmp_calendar, tmp_businessDayAdj).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}


/* @brief			swig interface for aqDateNthIMM 
*  @param [in]		year			The year of the IMM date
*  @param [in]		nth				The n'th IMM date of the year
*  @param [in]		calendar		Calendar
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The nth IMM date of the year
*/
std::string aqDateNthIMM(int year, 
						int nth, 
						const std::string& calendar, 
						const std::string& businessDayAdj)
{
    AQ_API_START
	std::string ret;
	
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());

	ret = validation::tryAqDateNthIMM(year, nth, tmp_calendar, tmp_businessDayAdj).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}


/* @brief			swig interface for aqDateNthIMMFromStartDate
*  @param [in]		startDate		The start (reference) date
*  @param [in]		nth				The nth IMM date from the start date, starting from the one closest to the start date
*  @param [in]		calendar		Calendar
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The nth IMM date from the base date, starting from the one closest to the base date
*/
std::string aqDateNthIMMFromStartDate(const std::string& startDate, 
									int nth, 
									const std::string& calendar, 
									const std::string& businessDayAdj)
{
    AQ_API_START
	std::string ret;

	AQLDate tmp_startDate( etrading::stringToDate( startDate ) );
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());

	ret = validation::tryAqDateNthIMMFromStartDate(tmp_startDate, nth, tmp_calendar, tmp_businessDayAdj).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}