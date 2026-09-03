#pragma once

#include <string>

/* @brief			 swig interface for the mirGetBusinessDays method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		calendar		Calendar
*  @return			Number of business days between fromDate and toDate 
*/
int mirGetBusinessDays(const std::string& fromDate, 
					const std::string& toDate,
					const std::string& calendar);
