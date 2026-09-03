#include "tryMirGetParRate.h"

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

    /* @brief			swig interface for mirGetDateFromTerm
    *  @param [in]		fromDate			The curve collection ID
    *  @param [in]		termY				Year fraction
    *  @param [in]		dayCount			Day count convention
    *  @param [in]		includeLast			True if include the last day, not used in the core function
    *  @return			The end date derived from the FromDate and given term
    */
    LAString tryMirGetDateFromTerm( const LADate& fromDate,
                                    double termY,
                                    const LAString& dayCount,
                                    bool includeLast )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetDateFromTerm_inputs" );
            file.write( "generatorFunction", "tryMirGetDateFromTerm" );
            file.write( "fromDate",			fromDate );
            file.write( "termY",				termY );
            file.write( "dayCount",			dayCount );
            file.write( "includeLast",		includeLast );
        }

		LADate fromDateCopy( fromDate );
		LAString dayCountCopy( dayCount );
        LADate mbReturnDate = etrading::LADateScheduleHelpers::getDateFromTerm( fromDateCopy, termY, dayCountCopy, includeLast );
        LAString ret = mbReturnDate.stringWithFormat( "YYYYMMDD" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetDateFromTerm_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


}


