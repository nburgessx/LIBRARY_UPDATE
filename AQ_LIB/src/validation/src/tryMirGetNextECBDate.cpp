/*
 * @brief			validation interface for mirGetNextECBDate method(s)
 * @Created:		04 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#include "tryMirGetNextECBDate.h"
#include "LADateHelpers.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"

namespace validation_api
{

    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the mirGetNextECBDate method
    *  @param [in]		baseDate		Base date
    *  @param [in]		strictlyAfter	True if strictly after the base date
    *  @return			Date of the next ECB (European Central Bank) meeting on/after the base date
    */
    LADate tryMirGetNextECBDate( const LADate& baseDate,
                                 bool strictlyAfter )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetNextECBDate_inputs" );
            file.write( "generatorFunction", "tryMirGetNextECBDate" );
            file.write( "baseDate", baseDate );
            file.write( "strictlyAfter", strictlyAfter );
        }

        LADate ret = etrading::LADateHelpers::getNextECBDate( baseDate, strictlyAfter );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetNextECBDate_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }
}
