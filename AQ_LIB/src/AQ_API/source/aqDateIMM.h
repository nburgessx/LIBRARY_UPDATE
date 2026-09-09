#pragma once

#include <string>
#include <vector>

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
							const std::string& businessDayAdj);


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
						const std::string& businessDayAdj);


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
									const std::string& businessDayAdj);