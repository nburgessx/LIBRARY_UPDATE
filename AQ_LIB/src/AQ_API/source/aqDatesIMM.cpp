#include "TypeUtilities.h"
#include "aqDatesIMM.h"
#include "tryAqDatesIMM.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"


/* @brief			swig interface for aqDatesIMMFromMonth. 
*  @param [in]		year			The year of the IMM date
*  @param [in]		month			The month of the IMM date
*  @param [in]		calendar		Calendar
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The IMM date on the given month
*/
std::string aqDatesIMMFromMonth(int year, 
							int month, 
							const std::string& calendar, 
							const std::string& businessDayAdj)
{
	AQ_API_START
    std::string ret;
	
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());

	ret = validation::tryAqDatesIMMFromMonth(year, month, tmp_calendar, tmp_businessDayAdj).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}


/* @brief			swig interface for aqDatesNthIMM 
*  @param [in]		year			The year of the IMM date
*  @param [in]		nth				The n'th IMM date of the year
*  @param [in]		calendar		Calendar
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The nth IMM date of the year
*/
std::string aqDatesNthIMM(int year, 
						int nth, 
						const std::string& calendar, 
						const std::string& businessDayAdj)
{
    AQ_API_START
	std::string ret;
	
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());

	ret = validation::tryAqDatesNthIMM(year, nth, tmp_calendar, tmp_businessDayAdj).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}


/* @brief			swig interface for aqDatesNthIMMFromStartDate
*  @param [in]		startDate		The start (reference) date
*  @param [in]		nth				The nth IMM date from the start date, starting from the one closest to the start date
*  @param [in]		calendar		Calendar
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The nth IMM date from the base date, starting from the one closest to the base date
*/
std::string aqDatesNthIMMFromStartDate(const std::string& startDate, 
									int nth, 
									const std::string& calendar, 
									const std::string& businessDayAdj)
{
    AQ_API_START
	std::string ret;

	AQLDate tmp_startDate( etrading::stringToDate( startDate ) );
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());

	ret = validation::tryAqDatesNthIMMFromStartDate(tmp_startDate, nth, tmp_calendar, tmp_businessDayAdj).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}