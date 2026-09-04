#include "tryAqDates.h"
#include "AQLDateScheduleHelpers.h"
#include "RecordMacros.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "CurveUtilities.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "AQLPriceDataCalendar.h"
#include "CoreEnumerations.h"
#include "DateUtilities.h"
#include "FuturesDates.h"
#include "CoreEnumerations.h"

namespace validation
{
    using etrading::CreateDataFile;

    // Function to get today's date using the system date
    AQLDate tryAqDatesToday()
    {
        return etrading::todaysDate();
    }

    /* @brief			validation method for aqDatesFromTenor
    *  @param [in]		startDate			The from date
    *  @param [in]		tenor				Tenor added to the from date
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			The end date
    */
    AQLDate tryAqDatesFromTenor( const AQLDate& startDate,
                               const AQLString& tenor,
                               const AQLString& businessDayAdj,
                               const AQLString& calendar,
                               const AQLString& rollConvention )
    {
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD

        DateVector startDateVector;
        startDateVector.push_back( startDate );
        DateVector ret = tryAqDatesFromTenor( startDateVector, tenor, businessDayAdj, calendar, rollConvention );
        return ret[0];

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for aqDatesFromTenor
    *  @param [in]		startDates			A vector of from dates
    *  @param [in]		tenor				Tenor added to the from date
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			A vector of end dates
    */
    DateVector tryAqDatesFromTenor( const DateVector& startDates,
                                   const AQLString& tenor,
                                   const AQLString& businessDayAdj,
                                   const AQLString& calendar,
                                   const AQLString& rollConvention )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesFromTenor_inputs" );
            file.write( "generatorFunction",    "tryAqDatesFromTenor" );
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

            CreateDataFile file( "tryAqDatesFromTenor_outputs" );
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
	AQLDate tryAqDatesShiftedSpotDate( const AQLDate& asOfDate,
						             const AQLString& fixingLag,			    // TODO: Convert to StandardString
						             const AQLString& fixingCalendar,	    // TODO: Convert to StandardString
                                     const AQLString& fixingBusDayAdj,	    // TODO: Convert to StandardString
                                     const AQLString& paymentLag,	        // TODO: Convert to StandardString
						             const AQLString& paymentCalendar,       // TODO: Convert to StandardString
                                     const AQLString& paymentBusDayAdj )	    // TODO: Convert to StandardString
    {
        VALID_EXCEPTION_START
		
		RECORD_INPUTS( asOfDate, fixingLag, fixingCalendar, fixingBusDayAdj, paymentLag, paymentCalendar, paymentBusDayAdj )
        
        // Parameter Validation
        AQLString fixingLag_                 = fixingLag;
        AQLString fixingBusDayAdj_           = fixingBusDayAdj;
        AQLString paymentLag_                = paymentLag;
        AQLString paymentBusDayAdj_          = paymentBusDayAdj;
        if ( fixingLag == "" )              fixingLag_           = "0D";
        if ( fixingBusDayAdj == "" )        fixingBusDayAdj_     = "NO_CHANGE";
        if ( paymentLag == "" )             paymentLag_          = "0D";
        if ( paymentBusDayAdj == "" )       paymentBusDayAdj_    = "NO_CHANGE";

        const AQLDate spotDate = etrading::getShiftedSpotDate( asOfDate, fixingLag_, fixingCalendar, fixingBusDayAdj_, paymentLag_, paymentCalendar, paymentBusDayAdj_ );
        
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
	AQLDate tryAqCurvesUSDSpotDate( const AQLDate& curveAsOfDate,
							      const AQLString& spotLag,               // TODO: Convert to StandardString
							      const AQLString& fixingCalendar,        // TODO: Convert to StandardString
							      const AQLString& paymentCalendar,       // TODO: Convert to StandardString
							      const AQLString& businessDayAdj,        // TODO: Convert to StandardString
							      const AQLString& rollConvention )       // TODO: Convert to StandardString
	{
		VALID_EXCEPTION_START
		
		RECORD_INPUTS( curveAsOfDate, spotLag, fixingCalendar, paymentCalendar, businessDayAdj, rollConvention )
        
        // Parameter Validation
        AQLString spotLag_               = spotLag;
        AQLString businessDayAdj_        = businessDayAdj;
        if ( spotLag == "" )            spotLag_ = "0D";
        if ( businessDayAdj == "" )     businessDayAdj_ = "NO_CHANGE";
            
        const AQLDate spotDate = etrading::getCurveUSDSpotDate( curveAsOfDate, spotLag_, fixingCalendar, paymentCalendar, businessDayAdj_, rollConvention );
        
        RECORD_OUTPUTS_AND_RETURN_RESULT( spotDate )

        VALID_EXCEPTION_END
	}

    /* @brief			validation interface for aqDatesFromYearFraction
    *  @param [in]		startDate			Start date
    *  @param [in]		yearFraction		Year fraction
    *  @param [in]		dayCount			Day count convention
    *  @return			The end date derived from the FromDate and given year fraction
    */
    AQLDate tryAqDatesFromYearFraction( const AQLDate& startDate,
                                      double yearFraction,
                                      const AQLString& dayCount )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesFromYearFraction_inputs" );
            file.write( "generatorFunction", "tryAqDatesFromYearFraction" );
            file.write( "startDate",	    startDate );
            file.write( "yearFraction",		yearFraction );
            file.write( "dayCount",			dayCount );
        }

        AQLDate ret;
        const etrading::DayCountEnum dayCounter = etrading::toDayCountEnum( dayCount.getCString() );

        if( dayCounter == etrading::ACT_ACT_DAYCOUNT )
        {
            auto date = etrading::toGregorianDateFromYYYYMMDD( startDate.stringWithFormat().getCString() );
            ret = AQLDate( etrading::toYYYYMMDDFromGregorianDate( etrading::addYearFraction( date, yearFraction ) ).c_str() );
        }
        else
        {
            ret = etrading::getDateFromYearFraction( startDate, yearFraction, etrading::toDayCountEnum(dayCount.getCString()) );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesFromYearFraction_outputs" );
            file.write( "output", ret );
        }
        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqDatesYearFraction method
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		dayCount		Day count convention
    *  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day
    *  @return			Year fraction between fromDate and toDate
    */
    double tryAqDatesYearFraction( const AQLDate& fromDate,
                                  const AQLDate& toDate,
                                  const AQLString& dayCount,
                                  bool includeLast )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesYearFraction_inputs" );
            file.write( "generatorFunction", "tryAqDatesYearFraction" );
            file.write( "fromDate", fromDate );
            file.write( "toDate", toDate );
            file.write( "dayCount", dayCount );
            file.write( "includeLast", includeLast );
        }

        double ret = etrading::getYearFraction( fromDate, toDate, etrading::toDayCountEnum(dayCount.getCString()), includeLast );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesYearFraction_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the aqDatesBusinessDays method
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		calendar		Calendar
    *  @return			Number of business days between fromDate and toDate
    */
    int tryAqDatesBusinessDays( const AQLDate& fromDate,
                               const AQLDate& toDate,
                               const AQLString& calendar )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesBusinessDays_inputs" );
            file.write( "generatorFunction", "tryAqDatesBusinessDays" );
            file.write( "fromDate", fromDate );
            file.write( "toDate", toDate );
            file.write( "calendar", calendar );
        }

        AQLPriceDataCalendar cal;
        cal.convertFromString( calendar );
        AQLDate tmpdate = fromDate;

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
            CreateDataFile file( "tryAqDatesBusinessDays_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqDatesIsWorkingDay method
    *  @param [in]		date	        date
    *  @param [in]		calendar		holidayCentre(s)
    *  @return			returns a boolean to indicate if the date specified is a working day
    */
    bool tryAqDatesIsWorkingDay( const AQLDate& date,
                                const AQLString& holidayCentre )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesIsWorkingDay_inputs" );
            file.write( "generatorFunction", "tryAqDatesIsWorkingDay" );
            file.write( "date",             date );
            file.write( "holidayCentre",    holidayCentre );
        }

        DateVector startDateVector;
        startDateVector.push_back( date );

        // Calculate the adjusted date taking into account holidays
        DateVector adjustedDate = etrading::AQLDateScheduleHelpers::getMultiDate( startDateVector,     // StartDateVector
                                                                               "0D",                // Tenor
                                                                               "FOLLOWING",         // BusinessDayAdjustment
                                                                               holidayCentre,       // Calendar
                                                                               nullptr );           // RollConvention
        
        if ( adjustedDate.size() < 1 )
        {
            throw AQLCoreInvalidData( "#Error: Unable to verify if the date is a holiday. Please check if holiday centre is correct.", __FILE__, __LINE__ );
        }

        // If the adjusted date is not equal to our initial start date then we have identified a holiday
        bool isWorkingDay = true;
        if ( date != adjustedDate[0] )
        {
            isWorkingDay = false;
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesIsWorkingDay_outputs" );
            file.write( "output", isWorkingDay );
        }

        return isWorkingDay;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the aqDatesIsHoliday method
    *  @param [in]		date	        date
    *  @param [in]		calendar		holidayCentre(s)
    *  @return			returns a boolean to indicate if the date specified is a holday
    */
    bool tryAqDatesIsHoliday( const AQLDate& date,
                             const AQLString& holidayCentre )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesIsHoliday_inputs" );
            file.write( "generatorFunction", "tryAqDatesIsHoliday" );
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
            DateVector adjustedDate = etrading::AQLDateScheduleHelpers::getMultiDate( startDateVector,     // StartDateVector
                                                                                   "0D",                // Tenor
                                                                                   "FOLLOWING",         // BusinessDayAdjustment
                                                                                   holidayCentre,       // Calendar
                                                                                   nullptr );           // RollConvention
        
            if ( adjustedDate.size() < 1 )
            {
                throw AQLCoreInvalidData( "#Error: Unable to verify if the date is a holiday. Please check if holiday centre is correct.", __FILE__, __LINE__ );
            }

            // If the adjusted date is not equal to our initial start date then we have identified a holiday, provided the original date is not a weekend
            if ( date != adjustedDate[0] && !isOriginalDateWeekend )
            {
                isHoliday = true;
            }
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesIsHoliday_outputs" );
            file.write( "output", isHoliday );
        }

        return isHoliday;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqDatesIsWeekend method
    *  @param [in]		date	        date
    *  @return			returns a boolean to indicate if the date specified falls on a weekend
    */
    bool tryAqDatesIsWeekend( const AQLDate& date )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesIsWeekend_inputs" );
            file.write( "generatorFunction", "tryAqDatesIsWeekend" );
            file.write( "date",             date );
        }

        // Check if the original date specified is a weekend
        const bool dateIsWeekend = ( date.dayOfWeek() == 0 || date.dayOfWeek() == 6 ); // Sunday = 0, Saturday = 6

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesIsWeekend_outputs" );
            file.write( "output", dateIsWeekend );
        }

        return dateIsWeekend;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqDatesIsWeekday method
    *  @param [in]		date	        date
    *  @return			returns a boolean to indicate if the date specified is a weekday
    */
    bool tryAqDatesIsWeekday( const AQLDate& date )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesIsWeekday_inputs" );
            file.write( "generatorFunction", "tryAqDatesIsWeekday" );
            file.write( "date",             date );
        }

        // Check if the original date specified is a weekend
        const bool dateIsWeekday = ( date.dayOfWeek() != 0 && date.dayOfWeek() != 6 ); // Sunday = 0, Saturday = 6

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesIsWeekday_outputs" );
            file.write( "output", dateIsWeekday );
        }

        return dateIsWeekday;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqDatesFuturesContract method
    *  @param [in]		AQLString        future's ticker
    *  @return			returns a date representing the future's start date
    */
    AQLDate tryAqDatesFuturesContract( const AQLString& futuresTicker )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesFuturesContract_inputs" );
            file.write( "generatorFunction", "tryAqDatesFutureStart" );
            file.write( "futuresTicker", futuresTicker );
        }

        // Get the futures start date for the specified futures contract
        const AQLDate futureStartDate = etrading::futureStartDate( futuresTicker );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqDatesFuturesContract_outputs" );
            file.write( "output", futureStartDate );
        }

        return futureStartDate;

        VALID_EXCEPTION_END
    }

}



