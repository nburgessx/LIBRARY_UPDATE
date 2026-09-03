/*
 * @brief			validation interface for meDate method(s)
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "tryMeDate.h"
#include "LADateScheduleHelpers.h"
#include "RecordMacros.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "CurveUtilities.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "LAPriceDataCalendar.h"
#include "CoreEnumerations.h"
#include "DateUtilities.h"
#include "FuturesDates.h"
#include "CoreEnumerations.h"

namespace validation_api
{
    using etrading::CreateDataFile;

    // Function to get today's date using the system date
    LADate tryMeDateToday()
    {
        return etrading::todaysDate();
    }

    /* @brief			validation method for meDateFromTenor
    *  @param [in]		startDate			The from date
    *  @param [in]		tenor				Tenor added to the from date
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			The end date
    */
    LADate tryMeDateFromTenor( const LADate& startDate,
                               const LAString& tenor,
                               const LAString& businessDayAdj,
                               const LAString& calendar,
                               const LAString& rollConvention )
    {
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD

        DateVector startDateVector;
        startDateVector.push_back( startDate );
        DateVector ret = tryMeDateFromTenor( startDateVector, tenor, businessDayAdj, calendar, rollConvention );
        return ret[0];

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for meDateFromTenor
    *  @param [in]		startDates			A vector of from dates
    *  @param [in]		tenor				Tenor added to the from date
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			A vector of end dates
    */
    DateVector tryMeDateFromTenor( const DateVector& startDates,
                                   const LAString& tenor,
                                   const LAString& businessDayAdj,
                                   const LAString& calendar,
                                   const LAString& rollConvention )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateFromTenor_inputs" );
            file.write( "generatorFunction",    "tryMeDateFromTenor" );
            file.write( "startDates",		    startDates );
            file.write( "tenor",				tenor );
            file.write( "businessDayAdj",	    businessDayAdj );
            file.write( "calendar",			    calendar );
            file.write( "rollConvention",		rollConvention );
        }
        
        DateVector ret = etrading::getDateFromTenor(startDates,
                                                    tenor,
                                                    businessDayAdj,
                                                    calendar,
                                                    rollConvention);
        
        if ( CreateDataFile::recordEnabled() )
        {

            CreateDataFile file( "tryMeDateFromTenor_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

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
	LADate tryMeDateShiftedSpotDate( const LADate& asOfDate,
						             const LAString& fixingLag,			    // TODO: Convert to StandardString
						             const LAString& fixingCalendar,	    // TODO: Convert to StandardString
                                     const LAString& fixingBusDayAdj,	    // TODO: Convert to StandardString
                                     const LAString& paymentLag,	        // TODO: Convert to StandardString
						             const LAString& paymentCalendar,       // TODO: Convert to StandardString
                                     const LAString& paymentBusDayAdj )	    // TODO: Convert to StandardString
    {
        VALID_EXCEPTION_START
		
		RECORD_INPUTS( asOfDate, fixingLag, fixingCalendar, fixingBusDayAdj, paymentLag, paymentCalendar, paymentBusDayAdj )
        
        // Parameter Validation
        LAString fixingLag_                 = fixingLag;
        LAString fixingBusDayAdj_           = fixingBusDayAdj;
        LAString paymentLag_                = paymentLag;
        LAString paymentBusDayAdj_          = paymentBusDayAdj;
        if ( fixingLag == "" )              fixingLag_           = "0D";
        if ( fixingBusDayAdj == "" )        fixingBusDayAdj_     = "NO_CHANGE";
        if ( paymentLag == "" )             paymentLag_          = "0D";
        if ( paymentBusDayAdj == "" )       paymentBusDayAdj_    = "NO_CHANGE";

        const LADate spotDate = etrading::getShiftedSpotDate( asOfDate, fixingLag_, fixingCalendar, fixingBusDayAdj_, paymentLag_, paymentCalendar, paymentBusDayAdj_ );
        
        RECORD_OUTPUTS_AND_RETURN_RESULT( spotDate )

        VALID_EXCEPTION_END
    }

	/* @param [in]		curveAsOfDate		The curve asOfDate
    *  @param [in]		spotLag				Tenor added to the from date
    *  @param [in]		fixingCalendar		Fixing Calendar
	*  @param [in]		paymentCalendar		Payment Calendar
	*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			The curve spot date
    */
	LADate tryMeCurveUSDSpotDate( const LADate& curveAsOfDate,
							      const LAString& spotLag,               // TODO: Convert to StandardString
							      const LAString& fixingCalendar,        // TODO: Convert to StandardString
							      const LAString& paymentCalendar,       // TODO: Convert to StandardString
							      const LAString& businessDayAdj,        // TODO: Convert to StandardString
							      const LAString& rollConvention )       // TODO: Convert to StandardString
	{
		VALID_EXCEPTION_START
		
		RECORD_INPUTS( curveAsOfDate, spotLag, fixingCalendar, paymentCalendar, businessDayAdj, rollConvention )
        
        // Parameter Validation
        LAString spotLag_               = spotLag;
        LAString businessDayAdj_        = businessDayAdj;
        if ( spotLag == "" )            spotLag_ = "0D";
        if ( businessDayAdj == "" )     businessDayAdj_ = "NO_CHANGE";
            
        const LADate spotDate = etrading::getCurveUSDSpotDate( curveAsOfDate, spotLag_, fixingCalendar, paymentCalendar, businessDayAdj_, rollConvention );
        
        RECORD_OUTPUTS_AND_RETURN_RESULT( spotDate )

        VALID_EXCEPTION_END
	}

    /* @brief			validation interface for meDateFromYearFraction
    *  @param [in]		startDate			Start date
    *  @param [in]		yearFraction		Year fraction
    *  @param [in]		dayCount			Day count convention
    *  @return			The end date derived from the FromDate and given year fraction
    */
    LADate tryMeDateFromYearFraction( const LADate& startDate,
                                      double yearFraction,
                                      const LAString& dayCount )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateFromYearFraction_inputs" );
            file.write( "generatorFunction", "tryMeDateFromYearFraction" );
            file.write( "startDate",	    startDate );
            file.write( "yearFraction",		yearFraction );
            file.write( "dayCount",			dayCount );
        }

        LADate ret;
        const etrading::DayCountEnum dayCounter = etrading::toDayCountEnum( dayCount.getCString() );

        if( dayCounter == etrading::ACT_ACT_DAYCOUNT )
        {
            auto date = etrading::toGregorianDateFromYYYYMMDD( startDate.stringWithFormat().getCString() );
            ret = LADate( etrading::toYYYYMMDDFromGregorianDate( etrading::addYearFraction( date, yearFraction ) ).c_str() );
        }
        else
        {
            ret = etrading::getDateFromYearFraction( startDate, yearFraction, etrading::toDayCountEnum(dayCount.getCString()) );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateFromYearFraction_outputs" );
            file.write( "output", ret );
        }
        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meDateYearFraction method
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		dayCount		Day count convention
    *  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day
    *  @return			Year fraction between fromDate and toDate
    */
    double tryMeDateYearFraction( const LADate& fromDate,
                                  const LADate& toDate,
                                  const LAString& dayCount,
                                  bool includeLast )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateYearFraction_inputs" );
            file.write( "generatorFunction", "tryMeDateYearFraction" );
            file.write( "fromDate", fromDate );
            file.write( "toDate", toDate );
            file.write( "dayCount", dayCount );
            file.write( "includeLast", includeLast );
        }

        double ret = etrading::getYearFraction( fromDate, toDate, etrading::toDayCountEnum(dayCount.getCString()), includeLast );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateYearFraction_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the meDateBusinessDays method
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		calendar		Calendar
    *  @return			Number of business days between fromDate and toDate
    */
    int tryMeDateBusinessDays( const LADate& fromDate,
                               const LADate& toDate,
                               const LAString& calendar )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateBusinessDays_inputs" );
            file.write( "generatorFunction", "tryMeDateBusinessDays" );
            file.write( "fromDate", fromDate );
            file.write( "toDate", toDate );
            file.write( "calendar", calendar );
        }

        LAPriceDataCalendar cal;
        cal.convertFromString( calendar );
        LADate tmpdate = fromDate;

        int ret = 0;
        do
        {
            tmpdate = cal.getBusinessDay( fromDate, ret );
            ret = ret + 1;
        }
        while ( tmpdate < toDate );

        ret = ret - 1;

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateBusinessDays_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meDateIsWorkingDay method
    *  @param [in]		date	        date
    *  @param [in]		calendar		holidayCentre(s)
    *  @return			returns a boolean to indicate if the date specified is a working day
    */
    bool tryMeDateIsWorkingDay( const LADate& date,
                                const LAString& holidayCentre )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateIsWorkingDay_inputs" );
            file.write( "generatorFunction", "tryMeDateIsWorkingDay" );
            file.write( "date",             date );
            file.write( "holidayCentre",    holidayCentre );
        }

        DateVector startDateVector;
        startDateVector.push_back( date );

        // Calculate the adjusted date taking into account holidays
        DateVector adjustedDate = etrading::LADateScheduleHelpers::getMultiDate( startDateVector,     // StartDateVector
                                                                               "0D",                // Tenor
                                                                               "FOLLOWING",         // BusinessDayAdjustment
                                                                               holidayCentre,       // Calendar
                                                                               nullptr );           // RollConvention
        
        if ( adjustedDate.size() < 1 )
        {
            throw LACoreInvalidData( "#Error: Unable to verify if the date is a holiday. Please check if holiday centre is correct.", __FILE__, __LINE__ );
        }

        // If the adjusted date is not equal to our initial start date then we have identified a holiday
        bool isWorkingDay = true;
        if ( date != adjustedDate[0] )
        {
            isWorkingDay = false;
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateIsWorkingDay_outputs" );
            file.write( "output", isWorkingDay );
        }

        return isWorkingDay;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the meDateIsHoliday method
    *  @param [in]		date	        date
    *  @param [in]		calendar		holidayCentre(s)
    *  @return			returns a boolean to indicate if the date specified is a holday
    */
    bool tryMeDateIsHoliday( const LADate& date,
                             const LAString& holidayCentre )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateIsHoliday_inputs" );
            file.write( "generatorFunction", "tryMeDateIsHoliday" );
            file.write( "date",             date );
            file.write( "holidayCentre",    holidayCentre );
        }

        bool isHoliday = false;

        // Check if the original date specified is a weekend
        const bool isOriginalDateWeekend = ( date.dayOfWeek() == 0 || date.dayOfWeek() == 6 ); // Sunday = 0, Saturday = 6

        // If our original date is a weekend it is not considered a holiday
        if ( !isOriginalDateWeekend )
        {
            DateVector startDateVector;
            startDateVector.push_back( date );

            // Calculate the adjusted date taking into account holidays
            DateVector adjustedDate = etrading::LADateScheduleHelpers::getMultiDate( startDateVector,     // StartDateVector
                                                                                   "0D",                // Tenor
                                                                                   "FOLLOWING",         // BusinessDayAdjustment
                                                                                   holidayCentre,       // Calendar
                                                                                   nullptr );           // RollConvention
        
            if ( adjustedDate.size() < 1 )
            {
                throw LACoreInvalidData( "#Error: Unable to verify if the date is a holiday. Please check if holiday centre is correct.", __FILE__, __LINE__ );
            }

            // If the adjusted date is not equal to our initial start date then we have identified a holiday, provided the original date is not a weekend
            if ( date != adjustedDate[0] && !isOriginalDateWeekend )
            {
                isHoliday = true;
            }
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateIsHoliday_outputs" );
            file.write( "output", isHoliday );
        }

        return isHoliday;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meDateIsWeekend method
    *  @param [in]		date	        date
    *  @return			returns a boolean to indicate if the date specified falls on a weekend
    */
    bool tryMeDateIsWeekend( const LADate& date )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateIsWeekend_inputs" );
            file.write( "generatorFunction", "tryMeDateIsWeekend" );
            file.write( "date",             date );
        }

        // Check if the original date specified is a weekend
        const bool dateIsWeekend = ( date.dayOfWeek() == 0 || date.dayOfWeek() == 6 ); // Sunday = 0, Saturday = 6

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateIsWeekend_outputs" );
            file.write( "output", dateIsWeekend );
        }

        return dateIsWeekend;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meDateIsWeekday method
    *  @param [in]		date	        date
    *  @return			returns a boolean to indicate if the date specified is a weekday
    */
    bool tryMeDateIsWeekday( const LADate& date )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateIsWeekday_inputs" );
            file.write( "generatorFunction", "tryMeDateIsWeekday" );
            file.write( "date",             date );
        }

        // Check if the original date specified is a weekend
        const bool dateIsWeekday = ( date.dayOfWeek() != 0 && date.dayOfWeek() != 6 ); // Sunday = 0, Saturday = 6

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateIsWeekday_outputs" );
            file.write( "output", dateIsWeekday );
        }

        return dateIsWeekday;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meDateFuturesContract method
    *  @param [in]		LAString        future's ticker
    *  @return			returns a date representing the future's start date
    */
    LADate tryMeDateFuturesContract( const LAString& futuresTicker )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateFuturesContract_inputs" );
            file.write( "generatorFunction", "tryMeDateFutureStart" );
            file.write( "futuresTicker", futuresTicker );
        }

        // Get the futures start date for the specified futures contract
        const LADate futureStartDate = etrading::futureStartDate( futuresTicker );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateFuturesContract_outputs" );
            file.write( "output", futureStartDate );
        }

        return futureStartDate;

        VALID_EXCEPTION_END
    }

}



