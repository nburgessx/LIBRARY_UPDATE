#include "tryMeDateIMM.h"

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
    /* @brief			validation interface for meDateIMMFromMonth.
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		month			The month of the IMM date
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The IMM date on the given month
    */
    AQLDate tryMeDateIMMFromMonth( int year,
                                  int month,
                                  const AQLString& calendar,
                                  const AQLString& businessDayAdj )
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
        AQLString bdAdj( etrading::getDefaultValueForEmptyString( businessDayAdj, "FOLLOWING" ) );

		AQLString calendarCopy( calendar );
        AQLDate ret = etrading::AQLDateScheduleHelpers::getIMMDate1( year, month, calendarCopy, bdAdj );

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
    AQLDate tryMeDateNthIMM( int year,
                            int nth,
                            const AQLString& calendar,
                            const AQLString& businessDayAdj )
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
        AQLString bdAdj( etrading::getDefaultValueForEmptyString( businessDayAdj, "FOLLOWING" ) );
		AQLString calendarCopy( calendar );
        AQLDate ret = etrading::AQLDateScheduleHelpers::getIMMDate2( year, nth, calendarCopy, bdAdj );

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
    AQLDate tryMeDateNthIMMFromStartDate( const AQLDate& startDate,
                                         int nth,
                                         const AQLString& calendar,
                                         const AQLString& businessDayAdj )
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
        AQLString bdAdj( etrading::getDefaultValueForEmptyString( businessDayAdj, "FOLLOWING" ) );
		AQLString calendarCopy( calendar );
        AQLDate ret = etrading::AQLDateScheduleHelpers::getIMMDate3( startDate, nth, calendarCopy, bdAdj );

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
    AQLDate tryMeDateIMMCurrent( const AQLDate & valuationDate, const bool includeToday )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( valuationDate, includeToday );

        // Calculation
        const AQLDate result = etrading::currentIMMDate( valuationDate, includeToday );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }
    
    /* @brief			validation interface for tryMeDateIMMNext
    *  @param [in]		referenceDate	The IMM reference date
    *  @return			The next IMM start date relative to the reference date
    */
    AQLDate tryMeDateIMMNext( const AQLDate & referenceDate )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( referenceDate );

        // Calculation
        const AQLDate result = etrading::nextIMMDate( referenceDate );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }
    
    /* @brief			validation interface for tryMeDateIMMPrevious
    *  @param [in]		referenceDate	The IMM reference date
    *  @return			The previous IMM start date relative to the reference date
    */
    AQLDate tryMeDateIMMPrevious( const AQLDate & referenceDate )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( referenceDate );

        // Calculation
        const AQLDate result = etrading::previousIMMDate( referenceDate );

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
    AQLDate tryMeDateIMMNth( const AQLDate & valuationDate, const int nthIMM, const bool includeToday )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( valuationDate, nthIMM, includeToday );

        // Calculation
        const AQLDate result = etrading::nthIMMDate( valuationDate, nthIMM, includeToday );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

}


