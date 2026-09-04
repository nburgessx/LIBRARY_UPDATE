#include "tryMirGetTerm.h"
#include "AQLDateScheduleHelpers.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"

namespace validation
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the mirGetTerm method
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		dayCount		Day count convention
    *  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day
    *  @return			Term between fromDate and toDate
    */
    double tryMirGetTerm( const AQLDate& fromDate,
                          const AQLDate& toDate,
                          const AQLString& dayCount,
                          bool includeLast )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetTerm_inputs" );
            file.write( "generatorFunction", "tryMirGetTerm" );
            file.write( "fromDate", fromDate );
            file.write( "toDate", toDate );
            file.write( "dayCount", dayCount );
            file.write( "includeLast", includeLast );
        }

		AQLString dayCountCopy( dayCount );
        double ret = etrading::AQLDateScheduleHelpers::getTerm( fromDate, toDate, dayCountCopy, includeLast );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetTerm_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }
}

