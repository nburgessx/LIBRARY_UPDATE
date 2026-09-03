/* 
 * @brief			Swig interface to Java for function mirGetDate
 * @Created:		16 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for mirGetDate
*  @param [in]		BaseDate		The from date
*  @param [in]		Term			Term added to the from date
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		Calendar		Calendar
*  @param [in]		RollConvention	Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
*  @return			The end date
*/
std::string mirGetDate(const std::string& BaseDate, 
						const std::string& Term, 
						const std::string& SlidingRule, 
						const std::string& Calendar, 
						const std::string& RollConvention) throw(std::exception);


/* @brief			swig interface for mirGetDate
*  @param [in]		BaseDates		A vector of from dates
*  @param [in]		Term			Term added to the from date
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		Calendar		Calendar
*  @param [in]		RollConvention	Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
*  @return			A vector of end dates
*/
std::vector<std::string> mirGetDate(const std::vector<std::string>& BaseDates, 
									const std::string& Term, 
									const std::string& SlidingRule, 
									const std::string& Calendar, 
									const std::string& RollConvention) throw(std::exception);