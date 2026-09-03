/* 
 * @brief			Swig interfaces to Java for functions mirGetIMMDateX
 * @Created:		16 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for mirGetIMMDate1. 
*  @param [in]		Year			The year of the IMM date
*  @param [in]		Month			The month of the IMM date
*  @param [in]		Calendar		Calendar
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The IMM date on the given month
*/
std::string mirGetIMMDate1(int Year, 
							int Month, 
							const std::string& Calendar, 
							const std::string& SlidingRule) throw(std::exception);


/* @brief			swig interface for mirGetIMMDate2 
*  @param [in]		Year			The year of the IMM date
*  @param [in]		Number			The number'th IMM date of the year
*  @param [in]		Calendar		Calendar
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The nth IMM date of the year
*/
std::string mirGetIMMDate2(int Year, 
							int Number, 
							const std::string& Calendar, 
							const std::string& SlidingRule) throw(std::exception);


/* @brief			swig interface for mirGetIMMDate3
*  @param [in]		BaseDate		The base (reference) date
*  @param [in]		Number			The (number-1)th IMM date from the base date, starting from the one closest to the base date
*  @param [in]		Calendar		Calendar
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The nth IMM date from the base date, starting from the one closest to the base date
*/
std::string mirGetIMMDate3(const std::string& BaseDate, 
							int Number, 
							const std::string& Calendar, 
							const std::string& SlidingRule) throw(std::exception);