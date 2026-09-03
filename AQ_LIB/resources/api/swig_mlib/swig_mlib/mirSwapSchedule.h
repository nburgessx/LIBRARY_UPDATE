/* 
 * @brief			Swig interface to Java for mirSwapSchedule methods
 * @Created:		04 April 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for the mirSwapSchedule method
*  @param [in]		accrualStartDate				Accrual start date
*  @param [in]		accrualEndDateOrTenor			Accrual end date or tenor
*  @param [in]		accrualDaycount					Accrual day count convention
*  @param[in]		accrualFrequency				Accrual frequencey
*  @param[in]		accrualSlidingRule				Accrual Roll Convention or Sliding Rule
*  @param[in]		accrualCalendar					Accrual calendar
*  @param[in]		paymentFrequency				Payment frequencey
*  @param[in]		paymentSlidingRule				Payment Roll Convention or Sliding Rule
*  @param[in]		paymentCalendar					Payment calendar
*  @param[in]		paymentLag						Payment Lag
*  @param[in]		fixingSlidingRule				Fixing Roll Convention or Sliding Rule
*  @param[in]		fixingCalendar					Fixing calendar
*  @param[in]		fixingLag						Fixing Lag
*  @param[in]		fixingAdvanceOrArrears	        Fixing in advance or arrears, default to fixing in advance
*  @param[in]		firstStub						First Odd Date, i.e. end date of the front stub period
*  @param[in]		lastStub						Last Odd Date, i.e. start date of the end stub period
*  @param[in]		paymentRollDay					Roll Day of Month (e.g. 9th of month ) 
*  @param[in]		showColumnHeaders				True to show column headers. 
*  @return			a matrix of floading leg/fixing leg schedules 
*/
std::vector<std::vector<std::string>> mirSwapSchedule(
						  const std::string& accrualStartDate,
                          const std::string& accrualEndDateOrTenor,
                          const std::string& accrualDaycount,
                          const std::string& accrualFrequency,
						  const std::string& accrualSlidingRule,            
                          const std::string& accrualCalendar,
                          const std::string& paymentFrequency,
						  const std::string& paymentSlidingRule,           
                          const std::string& paymentCalendar,
                          const std::string& paymentLag,
                          const std::string& fixingSlidingRule,
                          const std::string& fixingCalendar,
                          const std::string& fixingLag,
						  const std::string& fixingAdvanceOrArrears, 
						  const std::string& firstStub,                       
                          const std::string& lastStub,                        
                          const std::string& paymentRollDay,
						  bool showColumnHeaders) throw(std::exception);
