#include "tryMirGetIMMDate.h"

#include "LADateScheduleHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"

using etrading::CreateDataFile;

namespace validation
{
    /* @brief			validation interface for mirGetIMMDate1.
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		month			The month of the IMM date
    *  @param [in]		calendar		Calendar
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The IMM date on the given month
    */
    LAString tryMirGetIMMDate1( int year,
                                int month,
                                const LAString& calendar,
                                const LAString& slidingRule )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetIMMDate1_inputs" );
            file.write( "generatorFunction", "tryMirGetIMMDate1" );
            file.write( "year",				year );
            file.write( "month",				month );
            file.write( "calendar",			calendar );
            file.write( "slidingRule",		slidingRule );
        }

		LAString calendarCopy( calendar );
		LAString slidingRuleCopy( slidingRule );
        LADate immDate = etrading::LADateScheduleHelpers::getIMMDate1( year, month, calendarCopy, slidingRuleCopy );
        LAString ret = immDate.stringWithFormat( "YYYYMMDD" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetIMMDate1_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for mirGetIMMDate2
    *  @param [in]		year			The year of the IMM date
    *  @param [in]		number			The number'th IMM date of the year
    *  @param [in]		calendar		Calendar
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The nth IMM date of the year
    */
    LAString tryMirGetIMMDate2( int year,
                                int number,
                                const LAString& calendar,
                                const LAString& slidingRule )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetIMMDate2_inputs" );
            file.write( "generatorFunction", "tryMirGetIMMDate2" );
            file.write( "year",				year );
            file.write( "number",				number );
            file.write( "calendar",			calendar );
            file.write( "slidingRule",		slidingRule );
        }

		LAString calendarCopy( calendar );
		LAString slidingRuleCopy( slidingRule );
        LADate immDate = etrading::LADateScheduleHelpers::getIMMDate2( year, number, calendarCopy, slidingRuleCopy );
        LAString ret = immDate.stringWithFormat( "YYYYMMDD" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetIMMDate2_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for mirGetIMMDate3
    *  @param [in]		baseDate		The base (reference) date
    *  @param [in]		number			The (number-1)th IMM date from the base date, starting from the one closest to the base date
    *  @param [in]		calendar		Calendar
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The nth IMM date from the base date, starting from the one closest to the base date
    */
    LAString tryMirGetIMMDate3( const LADate& baseDate,
                                int number,
                                const LAString& calendar,
                                const LAString& slidingRule )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetIMMDate3_inputs" );
            file.write( "generatorFunction", "tryMirGetIMMDate3" );
            file.write( "baseDate",				baseDate );
            file.write( "number",				number );
            file.write( "calendar",				calendar );
            file.write( "slidingRule",			slidingRule );
        }

		LAString calendarCopy( calendar );
		LAString slidingRuleCopy( slidingRule );
        LADate immDate = etrading::LADateScheduleHelpers::getIMMDate3( baseDate, number, calendarCopy, slidingRuleCopy );
        LAString ret = immDate.stringWithFormat( "YYYYMMDD" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetIMMDate3_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}


