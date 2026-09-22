#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqDateFromTenor
*  @param [in]		startDate			The start date
*  @param [in]		tenor				Tenor added to the from date
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar			Calendar
*  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
*  @return			The end date
*/
std::string aqDateFromTenor(const std::string& startDate, 
							const std::string& tenor, 
							const std::string& businessDayAdj, 
							const std::string& calendar, 
							const std::string& rollConvention);

/* @brief			swig interface for aqDateFromTenor
*  @param [in]		startDate			The start date
*  @param [in]		tenor				Tenor added to the from date
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar			Calendar
*  @return			The end date
*/
std::string aqDateFromTenor(const std::string& startDate, 
							const std::string& tenor, 
							const std::string& businessDayAdj, 
							const std::string& calendar);

/* @brief			swig interface for aqDateFromTenor
*  @param [in]		startDate			The start date
*  @param [in]		tenor				Tenor added to the from date
*  @return			The end date
*/
std::string aqDateFromTenor(const std::string& startDate, 
							const std::string& tenor);

/* @brief			swig interface for aqDateFromTenor
*  @param [in]		startDates			The list of start dates
*  @param [in]		tenor				Tenor added to the from date
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar			Calendar
*  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
*  @return			A vector of end dates
*/
std::vector<std::string> aqDateFromTenor(const std::vector<std::string>& startDates, 
										const std::string& tenor, 
										const std::string& businessDayAdj, 
										const std::string& calendar, 
										const std::string& rollConvention);

/* @brief			swig interface for aqDateFromYearFraction
*  @param [in]		startDate			Start date
*  @param [in]		yearFraction		Year fraction
*  @param [in]		dayCount			Day count convention
*  @return			The end date derived from the FromDate and given term
*/
const std::string aqDateFromYearFraction(const std::string& startDate, 
									double yearFraction, 
									const std::string& dayCount);


/* @brief			swig interface for the aqDateBusinessDays method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		calendar		Calendar
*  @return			Number of business days between fromDate and toDate 
*/
int aqDateBusinessDays(const std::string& fromDate, 
					const std::string& toDate,
					const std::string& calendar); 


/* @brief			swig interface for the aqDateYearFraction method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		dayCount		Day count convention
*  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day 
*  @return			Term between fromDate and toDate 
*/
double aqDateYearFraction( const std::string& fromDate, 
						   const std::string& toDate,
						   const std::string& dayCount,
						   bool includeLast);

/* @brief			swig interface for the aqDateYearFraction method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		dayCount		Day count convention
*  @return			Term between fromDate and toDate 
*/
double aqDateYearFraction( const std::string& fromDate, 
						   const std::string& toDate,
						   const std::string& dayCount ); 


/* @brief			swig interface for the aqObjSwapsSpotDate method
*  @param [in]		asOfDate			The asOf- or valuation-date
*  @param [in]		spotLag				The spot lag
*  @param [in]		fixingCalendar		Fixing Calendar
*  @param [in]		paymentCalendar		Payment Calendar
*  @param [in]		businessDayAdj		Business Day Adjustment
*  @param [in]		rollConvention		Roll Convention
*  @return			Swap Spot Date 
*/
std::string aqCurveUSDSpotDate( const std::string & asOfDate,
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
std::string aqDateShiftedSpotDate( const std::string& asOfDate,
						           const std::string& fixingLag,
						           const std::string& fixingCalendar,
                                   const std::string& fixingBusDayAdj,
                                   const std::string& paymentLag,
						           const std::string& paymentCalendar,
                                   const std::string& paymentBusDayAdj );

/* @brief			swig interface for the aqDateToday method
*  @return			Today's date, from the system clock
*/
std::string aqDateToday();

/* @brief			swig interface for the aqDateIsWorkingDay method
*  @param [in]		date			The date to test
*  @param [in]		holidayCentre	Holiday centre(s)
*  @return			TRUE if the date is a working day for the given holiday centre
*/
bool aqDateIsWorkingDay( const std::string& date,
                         const std::string& holidayCentre );

/* @brief			swig interface for the aqDateIsHoliday method
*  @param [in]		date			The date to test
*  @param [in]		holidayCentre	Holiday centre(s)
*  @return			TRUE if the date is a holiday for the given holiday centre
*/
bool aqDateIsHoliday( const std::string& date,
                      const std::string& holidayCentre );

/* @brief			swig interface for the aqDateIsWeekend method
*  @param [in]		date	The date to test
*  @return			TRUE if the date falls on a Saturday or Sunday
*/
bool aqDateIsWeekend( const std::string& date );

/* @brief			swig interface for the aqDateIsWeekday method
*  @param [in]		date	The date to test
*  @return			TRUE if the date falls on a Monday to Friday
*/
bool aqDateIsWeekday( const std::string& date );

/* @brief			swig interface for the aqDateFuturesContract method
*  @param [in]		futuresTicker	The futures ticker, e.g. EDZ25, FFF26
*  @return			The futures contract's expiry (start) date
*/
std::string aqDateFuturesContract( const std::string& futuresTicker );

/* @brief			swig interface for the aqDateIsRegularSwapSchedule method
*  @param [in]		swapStart			Swap start date
*  @param [in]		swapMaturity		Swap end date. Tenors are typically adjusted and end dates are not
*  @param [in]		isMaturityAdjusted	Swap end date business day adjusted. Maturities derived from Tenors are adjusted, whereas explicit maturity dates are unadjusted
*  @param [in]		frequency			Swap floating frequency
*  @param [in]		busDayAdj			Swap business date adjustment convention
*  @param [in]		calendar			Swap calendar
*  @param [in]		rollDay				Roll day
*  @param [in]		rollConvention		Optional. Swap roll convention e.g. IMM, EOM
*  @return			TRUE if the swap schedule is regular (with no stub) and FALSE otherwise
*/
bool aqDateIsRegularSwapSchedule( const std::string& swapStart,
                                  const std::string& swapMaturity,
                                  const bool isMaturityAdjusted,
                                  const std::string& frequency,
                                  const std::string& busDayAdj,
                                  const std::string& calendar,
                                  const int rollDay,
                                  const std::string& rollConvention = "" );
