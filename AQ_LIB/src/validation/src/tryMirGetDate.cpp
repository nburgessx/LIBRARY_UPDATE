#include "tryMirGetDate.h"

#include "LADateScheduleHelpers.h"

#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"


namespace validation
{
    using etrading::CreateDataFile;

    /* @brief			validation method for mirGetDate
    *  @param [in]		baseDate			The from date
    *  @param [in]		term				Term added to the from date
    *  @param [in]		slidingRule			Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			The end date
    */
    LAString tryMirGetDate( const LADate& baseDate,
                            const LAString& term,
                            const LAString& slidingRule,
                            const LAString& calendar,
                            const LAString& rollConvention )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback

        if ( CreateDataFile::recordEnabled() )
        {

            CreateDataFile file( "tryMirGetDate_inputs" );
            file.write( "generatorFunction", "tryMirGetDate" );
            file.write( "baseDate",			baseDate );
            file.write( "term",				term );
            file.write( "slidingRule",		slidingRule );
            file.write( "calendar",			calendar );
            file.write( "rollConvention",	rollConvention );
        }

        LADate mbReturnDate = etrading::LADateScheduleHelpers::getDate( baseDate, term, slidingRule, calendar );
        LAString ret = mbReturnDate.stringWithFormat( "YYYYMMDD" );
        if ( CreateDataFile::recordEnabled() )
        {

            CreateDataFile file( "tryMirGetDate_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for mirGetDate
    *  @param [in]		baseDate			A vector of from dates
    *  @param [in]		term				Term added to the from date
    *  @param [in]		slidingRule			Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			A vector of end dates
    */
    DateVector tryMirGetDate( const DateVector& baseDates,
                              const LAString& term,
                              const LAString& slidingRule,
                              const LAString& calendar,
                              const LAString& rollConvention )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback

        if ( CreateDataFile::recordEnabled() )
        {

            CreateDataFile file( "tryMirGetDate_inputs" );
            file.write( "generatorFunction", "tryMirGetDate" );
            file.write( "baseDates",			baseDates );
            file.write( "term",				term );
            file.write( "slidingRule",		slidingRule );
            file.write( "calendar",			calendar );
            file.write( "rollConvention",	rollConvention );
        }

        DateVector ret = etrading::LADateScheduleHelpers::getMultiDate( baseDates,
                                                                      term,
                                                                      slidingRule,
                                                                      calendar,
                                                                      rollConvention.size() == 0 ? NULL : &rollConvention );

        if ( CreateDataFile::recordEnabled() )
        {

            CreateDataFile file( "tryMirGetDate_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


}



