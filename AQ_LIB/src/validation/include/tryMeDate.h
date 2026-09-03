#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
    // Function to get today's date using the system date
    AQLDate tryMeDateToday();
    
    /* @brief			validation method for meDateFromTenor
    *  @param [in]		startDate			The from date
    *  @param [in]		tenor				Tenor added to the from date
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			The end date
    */
    AQLDate tryMeDateFromTenor( const AQLDate& startDate,
                               const AQLString& tenor,
                               const AQLString& businessDayAdj,
                               const AQLString& calendar,
                               const AQLString& rollConvention );


    /* @brief			validation method for meDateFromTenor
    *  @param [in]		startDates			A vector of from dates
    *  @param [in]		tenor				Tenor added to the from date
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			A vector of end dates
    */
    DateVector tryMeDateFromTenor( const DateVector& startDates,
                                   const AQLString& tenor,
                                   const AQLString& businessDayAdj,
                                   const AQLString& calendar,
                                   const AQLString& rollConvention );

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
	AQLDate tryMeDateShiftedSpotDate( const AQLDate& asOfDate,
						             const AQLString& fixingLag,			    // TODO: Convert to StandardString
						             const AQLString& fixingCalendar,	    // TODO: Convert to StandardString
                                     const AQLString& fixingBusDayAdj,	    // TODO: Convert to StandardString
                                     const AQLString& paymentLag,		    // TODO: Convert to StandardString
						             const AQLString& paymentCalendar,	    // TODO: Convert to StandardString
                                     const AQLString& paymentBusDayAdj );    // TODO: Convert to StandardString

	/* @param [in]		curveAsOfDate		The curve asOfDate
    *  @param [in]		spotLag				Tenor added to the from date
    *  @param [in]		fixingCalendar		Fixing Calendar
	*  @param [in]		paymentCalendar		Payment Calendar
	*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			The curve spot date
    */
	AQLDate tryMeCurveUSDSpotDate( const AQLDate& curveAsOfDate,
							      const AQLString& spotLag,               // TODO: Convert to StandardString
							      const AQLString& fixingCalendar,        // TODO: Convert to StandardString
							      const AQLString& paymentCalendar,       // TODO: Convert to StandardString
							      const AQLString& businessDayAdj,        // TODO: Convert to StandardString
							      const AQLString& rollConvention );      // TODO: Convert to StandardString

    /* @brief			validation interface for meDateFromYearFraction
    *  @param [in]		startDate			Start date
    *  @param [in]		yearFraction		Year fraction
    *  @param [in]		dayCount			Day count convention
    *  @return			The end date derived from the FromDate and given year fraction
    */
    AQLDate tryMeDateFromYearFraction( const AQLDate& startDate,
                                      double yearFraction,
                                      const AQLString& dayCount );

    /* @brief			validation interface for the meDateYearFraction method
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		dayCount		Day count convention
    *  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day
    *  @return			Year fraction between fromDate and toDate
    */
    double tryMeDateYearFraction( const AQLDate& fromDate,
                                  const AQLDate& toDate,
                                  const AQLString& dayCount,
                                  bool includeLast = true );


    /* @brief			validation interface for the meDateBusinessDays method
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		calendar		Calendar
    *  @return			Number of business days between fromDate and toDate
    */
    int tryMeDateBusinessDays( const AQLDate& fromDate,
                               const AQLDate& toDate,
                               const AQLString& calendar );

    /* @brief			validation interface for the meDateIsWorkingDay method
    *  @param [in]		date	        date
    *  @param [in]		calendar		holidayCentre(s)
    *  @return			returns a boolean to indicate if the date specified is a working day
    */
    bool tryMeDateIsWorkingDay( const AQLDate& date,
                                const AQLString& holidayCentre );

    /* @brief			validation interface for the meDateIsHoliday method
    *  @param [in]		date	        date
    *  @param [in]		calendar		holidayCentre(s)
    *  @return			returns a boolean to indicate if the date specified is a holday
    */
    bool tryMeDateIsHoliday( const AQLDate& date,
                             const AQLString& holidayCentre );

    /* @brief			validation interface for the meDateIsWeekend method
    *  @param [in]		date	        date
    *  @return			returns a boolean to indicate if the date specified falls on a weekend
    */
    bool tryMeDateIsWeekend( const AQLDate& date );

    /* @brief			validation interface for the meDateIsWeekday method
    *  @param [in]		date	        date
    *  @return			returns a boolean to indicate if the date specified is a weekday
    */
    bool tryMeDateIsWeekday( const AQLDate& date );

    /* @brief			validation interface for the meDateFuturesContract method
    *  @param [in]		AQLString        future's ticker
    *  @return			returns a date representing the future's start date
    */
    AQLDate tryMeDateFuturesContract( const AQLString& futuresTicker );
}
