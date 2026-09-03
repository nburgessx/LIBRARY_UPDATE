/* 
 * @brief			Swig interface to Java for mirGetTerm methods
 * @Created:		04 April 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>

/* @brief			swig interface for the mirGetTerm method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		dayCount		Day count convention
*  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day 
*  @return			Term between fromDate and toDate 
*/
double mirGetTerm(const std::string& fromDate, 
			const std::string& toDate,
			const std::string& dayCount,
			bool includeLast) throw(std::exception);
