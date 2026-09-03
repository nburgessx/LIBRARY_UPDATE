#include "tryMirGetBusinessDays.h"
#include "LAPriceDataCalendar.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"

namespace validation
{

    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the mirGetBusinessDays method
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		calendar		Calendar
    *  @return			Number of business days between fromDate and toDate
    */
    int tryMirGetBusinessDays( const LADate& fromDate,
                               const LADate& toDate,
                               const LAString& calendar )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetBusinessDays_inputs" );
            file.write( "generatorFunction", "tryMirGetBusinessDays" );
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
            CreateDataFile file( "tryMirGetBusinessDays_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}
