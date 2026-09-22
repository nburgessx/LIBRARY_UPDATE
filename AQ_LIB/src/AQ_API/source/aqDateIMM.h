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

/* @brief			swig interface for aqDateIMMCurrent
*  @param [in]		valuationDate	The valuation date
*  @param [in]		includeToday	Optional. Default FALSE. TRUE lets an IMM date on the valuation date count as current
*  @return			The current (most recent) IMM start date, which may be in the past
*/
std::string aqDateIMMCurrent( const std::string& valuationDate,
                              const bool includeToday = false );

/* @brief			swig interface for aqDateIMMNext
*  @param [in]		referenceDate	The IMM reference date
*  @return			The next IMM start date relative to the reference date
*/
std::string aqDateIMMNext( const std::string& referenceDate );

/* @brief			swig interface for aqDateIMMPrevious
*  @param [in]		referenceDate	The IMM reference date
*  @return			The previous IMM start date relative to the reference date
*/
std::string aqDateIMMPrevious( const std::string& referenceDate );

/* @brief			swig interface for aqDateIMMNth
*  @param [in]		valuationDate	The valuation date
*  @param [in]		nthIMM			Which IMM date; positive is forward, negative is backward
*  @param [in]		includeToday	Optional. Default FALSE. TRUE lets an IMM date on the valuation date count
*  @return			The nth IMM date relative to the valuation date
*/
std::string aqDateIMMNth( const std::string& valuationDate,
                         const int nthIMM,
                         const bool includeToday = false );