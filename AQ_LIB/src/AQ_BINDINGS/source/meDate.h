/* 
 * @brief			Swig interface to Java for function meDate functions
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for meDateFromTenor
*  @param [in]		startDate			The start date
*  @param [in]		tenor				Tenor added to the from date
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar			Calendar
*  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
*  @return			The end date
*/
std::string meDateFromTenor(const std::string& startDate, 
							const std::string& tenor, 
							const std::string& businessDayAdj, 
							const std::string& calendar, 
							const std::string& rollConvention);

/* @brief			swig interface for meDateFromTenor
*  @param [in]		startDate			The start date
*  @param [in]		tenor				Tenor added to the from date
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar			Calendar
*  @return			The end date
*/
std::string meDateFromTenor(const std::string& startDate, 
							const std::string& tenor, 
							const std::string& businessDayAdj, 
							const std::string& calendar);

/* @brief			swig interface for meDateFromTenor
*  @param [in]		startDate			The start date
*  @param [in]		tenor				Tenor added to the from date
*  @return			The end date
*/
std::string meDateFromTenor(const std::string& startDate, 
							const std::string& tenor);

/* @brief			swig interface for meDateFromTenor
*  @param [in]		startDates			The list of start dates
*  @param [in]		tenor				Tenor added to the from date
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar			Calendar
*  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
*  @return			A vector of end dates
*/
std::vector<std::string> meDateFromTenor(const std::vector<std::string>& startDates, 
										const std::string& tenor, 
										const std::string& businessDayAdj, 
										const std::string& calendar, 
										const std::string& rollConvention);

/* @brief			swig interface for meDateFromYearFraction
*  @param [in]		startDate			Start date
*  @param [in]		yearFraction		Year fraction
*  @param [in]		dayCount			Day count convention
*  @return			The end date derived from the FromDate and given term
*/
const std::string meDateFromYearFraction(const std::string& startDate, 
									double yearFraction, 
									const std::string& dayCount);


/* @brief			swig interface for the meDateBusinessDays method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		calendar		Calendar
*  @return			Number of business days between fromDate and toDate 
*/
int meDateBusinessDays(const std::string& fromDate, 
					const std::string& toDate,
					const std::string& calendar); 


/* @brief			swig interface for the meDateYearFraction method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		dayCount		Day count convention
*  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day 
*  @return			Term between fromDate and toDate 
*/
double meDateYearFraction( const std::string& fromDate, 
						   const std::string& toDate,
						   const std::string& dayCount,
						   bool includeLast);

/* @brief			swig interface for the meDateYearFraction method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		dayCount		Day count convention
*  @return			Term between fromDate and toDate 
*/
double meDateYearFraction( const std::string& fromDate, 
						   const std::string& toDate,
						   const std::string& dayCount ); 


/* @brief			swig interface for the meLWOSwapSpotDate method
*  @param [in]		asOfDate			The asOf- or valuation-date
*  @param [in]		spotLag				The spot lag
*  @param [in]		fixingCalendar		Fixing Calendar
*  @param [in]		paymentCalendar		Payment Calendar
*  @param [in]		businessDayAdj		Business Day Adjustment
*  @param [in]		rollConvention		Roll Convention
*  @return			Swap Spot Date 
*/
std::string meLWOSwapUSDSpotDate( const std::string & asOfDate,
						          const std::string & spotLag,
						          const std::string & fixingCalendar,
						          const std::string & paymentCalendar,
						          const std::string & businessDayAdj,
						          const std::string & rollConvention ); 

/* @brief			Method to get the curve spot date by applying multiple date shifts
*  @param [in]		asOfDate		        The curve asOfDate
*  @param [in]		fixingLag			    Tenor added to the from date
*  @param [in]		fixingCalendar		    Fixing Calendar
*  @param [in]		fixingBusDayAdj		    Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		paymentLag			    Tenor added to the from date
*  @param [in]		paymentCalendar		    Fixing Calendar
*  @param [in]		paymentBusDayAdj        Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The curve spot date
*/
std::string meDateShiftedSpotDate( const std::string& asOfDate,
						           const std::string& fixingLag,
						           const std::string& fixingCalendar,
                                   const std::string& fixingBusDayAdj,
                                   const std::string& paymentLag,
						           const std::string& paymentCalendar,
                                   const std::string& paymentBusDayAdj );
