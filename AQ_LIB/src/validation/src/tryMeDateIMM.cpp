/*
 * @brief			validation interface for meDateIMM method(s)
 * @Created:		18 March 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "tryMeDateIMM.h"

#include "LADateScheduleHelpers.h"
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

namespace validation_api
{
    /* @brief			validation interface for meDateIMMFromMonth.
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		month			The month of the IMM date
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The IMM date on the given month
    */
    LADate tryMeDateIMMFromMonth( int year,
                                  int month,
                                  const LAString& calendar,
                                  const LAString& businessDayAdj )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateIMMFromMonth_inputs" );
            file.write( "generatorFunction", "tryMeDateIMMFromMonth" );
            file.write( "year",				year );
            file.write( "month",				month );
            file.write( "calendar",			calendar );
            file.write( "businessDayAdj",		businessDayAdj );
        }

        etrading::validateStringEmptiness( calendar, "#Error: 'Calendar' must be specified." );
        LAString bdAdj( etrading::getDefaultValueForEmptyString( businessDayAdj, "FOLLOWING" ) );

		LAString calendarCopy( calendar );
        LADate ret = etrading::LADateScheduleHelpers::getIMMDate1( year, month, calendarCopy, bdAdj );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateIMMFromMonth_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for meDateNthIMM
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		nth				The n'th IMM date of the year
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The nth IMM date of the year
    */
    LADate tryMeDateNthIMM( int year,
                            int nth,
                            const LAString& calendar,
                            const LAString& businessDayAdj )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateNthIMM_inputs" );
            file.write( "generatorFunction", "tryMeDateNthIMM" );
            file.write( "year",				year );
            file.write( "nth",				nth );
            file.write( "calendar",			calendar );
            file.write( "businessDayAdj",		businessDayAdj );
        }

        etrading::validateStringEmptiness( calendar, "#Error: 'Calendar' must be specified." );
        LAString bdAdj( etrading::getDefaultValueForEmptyString( businessDayAdj, "FOLLOWING" ) );
		LAString calendarCopy( calendar );
        LADate ret = etrading::LADateScheduleHelpers::getIMMDate2( year, nth, calendarCopy, bdAdj );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateNthIMM_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for meDateNthIMMFromStartDate
    *  @param [in]		startDate		The start (reference) date
    *  @param [in]		nth				The nth IMM date from the start date, starting from the one closest to the start date
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The nth IMM date from the start date, starting from the one closest to the start date
    */
    LADate tryMeDateNthIMMFromStartDate( const LADate& startDate,
                                         int nth,
                                         const LAString& calendar,
                                         const LAString& businessDayAdj )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateNthIMMFromStartDate_inputs" );
            file.write( "generatorFunction", "tryMeDateNthIMMFromStartDate" );
            file.write( "startDate",				startDate );
            file.write( "nth",				    nth );
            file.write( "calendar",				calendar );
            file.write( "businessDayAdj",			businessDayAdj );
        }

        etrading::validateStringEmptiness( calendar, "#Error: 'Calendar' must be specified." );
        LAString bdAdj( etrading::getDefaultValueForEmptyString( businessDayAdj, "FOLLOWING" ) );
		LAString calendarCopy( calendar );
        LADate ret = etrading::LADateScheduleHelpers::getIMMDate3( startDate, nth, calendarCopy, bdAdj );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateNthIMMFromStartDate_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for tryMeDateIMMCurrent
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @return			The current IMM start date, which may be in the past
    */
    LADate tryMeDateIMMCurrent( const LADate & valuationDate, const bool includeToday )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( valuationDate, includeToday );

        // Calculation
        const LADate result = etrading::currentIMMDate( valuationDate, includeToday );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }
    
    /* @brief			validation interface for tryMeDateIMMNext
    *  @param [in]		referenceDate	The IMM reference date
    *  @return			The next IMM start date relative to the reference date
    */
    LADate tryMeDateIMMNext( const LADate & referenceDate )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( referenceDate );

        // Calculation
        const LADate result = etrading::nextIMMDate( referenceDate );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }
    
    /* @brief			validation interface for tryMeDateIMMPrevious
    *  @param [in]		referenceDate	The IMM reference date
    *  @return			The previous IMM start date relative to the reference date
    */
    LADate tryMeDateIMMPrevious( const LADate & referenceDate )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( referenceDate );

        // Calculation
        const LADate result = etrading::previousIMMDate( referenceDate );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for tryMeDateIMMNth
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		nthIMM	        The nth IMM start date to generate
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @return			The nth IMM date relative to the valuation date
    */
    LADate tryMeDateIMMNth( const LADate & valuationDate, const int nthIMM, const bool includeToday )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( valuationDate, nthIMM, includeToday );

        // Calculation
        const LADate result = etrading::nthIMMDate( valuationDate, nthIMM, includeToday );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

}


