#include "tryAqDateIMM.h"

#include "AQLDateScheduleHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"
#include "FuturesDates.h"

using etrading::CreateDataFile;

namespace validation
{
    /* @brief			validation interface for aqDateIMMFromMonth.
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		month			The month of the IMM date
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The IMM date on the given month
    */
    AQLDate tryAqDateIMMFromMonth( int year,
                                  int month,
                                  const AQLString& calendar,
                                  const AQLString& businessDayAdj )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        AQ_RECORD_INPUTS( year, month, calendar, businessDayAdj );

        etrading::validateStringEmptiness( calendar, "#Error: 'Calendar' must be specified." );
        AQLString bdAdj( etrading::getDefaultValueForEmptyString( businessDayAdj, "FOLLOWING" ) );

		AQLString calendarCopy( calendar );
        AQLDate ret = etrading::AQLDateScheduleHelpers::getIMMDate1( year, month, calendarCopy, bdAdj );

        AQ_RECORD_OUTPUTS( ret );

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for aqDateNthIMM
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		nth				The n'th IMM date of the year
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The nth IMM date of the year
    */
    AQLDate tryAqDateNthIMM( int year,
                            int nth,
                            const AQLString& calendar,
                            const AQLString& businessDayAdj )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        AQ_RECORD_INPUTS( year, nth, calendar, businessDayAdj );

        etrading::validateStringEmptiness( calendar, "#Error: 'Calendar' must be specified." );
        AQLString bdAdj( etrading::getDefaultValueForEmptyString( businessDayAdj, "FOLLOWING" ) );
		AQLString calendarCopy( calendar );
        AQLDate ret = etrading::AQLDateScheduleHelpers::getIMMDate2( year, nth, calendarCopy, bdAdj );

        AQ_RECORD_OUTPUTS( ret );

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for aqDateNthIMMFromStartDate
    *  @param [in]		startDate		The start (reference) date
    *  @param [in]		nth				The nth IMM date from the start date, starting from the one closest to the start date
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The nth IMM date from the start date, starting from the one closest to the start date
    */
    AQLDate tryAqDateNthIMMFromStartDate( const AQLDate& startDate,
                                         int nth,
                                         const AQLString& calendar,
                                         const AQLString& businessDayAdj )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        AQ_RECORD_INPUTS( startDate, nth, calendar, businessDayAdj );

        etrading::validateStringEmptiness( calendar, "#Error: 'Calendar' must be specified." );
        AQLString bdAdj( etrading::getDefaultValueForEmptyString( businessDayAdj, "FOLLOWING" ) );
		AQLString calendarCopy( calendar );
        AQLDate ret = etrading::AQLDateScheduleHelpers::getIMMDate3( startDate, nth, calendarCopy, bdAdj );

        AQ_RECORD_OUTPUTS( ret );

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for tryAqDateIMMCurrent
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @return			The current IMM start date, which may be in the past
    */
    AQLDate tryAqDateIMMCurrent( const AQLDate & valuationDate, const bool includeToday )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( valuationDate, includeToday );

        // Calculation
        const AQLDate result = etrading::currentIMMDate( valuationDate, includeToday );

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }
    
    /* @brief			validation interface for tryAqDateIMMNext
    *  @param [in]		referenceDate	The IMM reference date
    *  @return			The next IMM start date relative to the reference date
    */
    AQLDate tryAqDateIMMNext( const AQLDate & referenceDate )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( referenceDate );

        // Calculation
        const AQLDate result = etrading::nextIMMDate( referenceDate );

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }
    
    /* @brief			validation interface for tryAqDateIMMPrevious
    *  @param [in]		referenceDate	The IMM reference date
    *  @return			The previous IMM start date relative to the reference date
    */
    AQLDate tryAqDateIMMPrevious( const AQLDate & referenceDate )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( referenceDate );

        // Calculation
        const AQLDate result = etrading::previousIMMDate( referenceDate );

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for tryAqDateIMMNth
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		nthIMM	        The nth IMM start date to generate
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @return			The nth IMM date relative to the valuation date
    */
    AQLDate tryAqDateIMMNth( const AQLDate & valuationDate, const int nthIMM, const bool includeToday )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( valuationDate, nthIMM, includeToday );

        // Calculation
        const AQLDate result = etrading::nthIMMDate( valuationDate, nthIMM, includeToday );

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

}


