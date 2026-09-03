#include "tryMirGetNextCBDate.h"
#include "LADateHelpers.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"

namespace validation_api
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the mirGetNextCBDate method
    *  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
    *  @param [in]		baseDate		Base date
    *  @param [in]		strictlyAfter	True if strictly after the base date
    *  @return			Date of the next central bank meeting on/after the base date
    */
    LADate tryMirGetNextCBDate( const LAString& centralBankId,
                                const LADate& baseDate,
                                bool strictlyAfter )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetNextCBDate_inputs" );
            file.write( "generatorFunction", "tryMirGetNextCBDate" );
            file.write( "centralBankId", centralBankId );
            file.write( "baseDate", baseDate );
            file.write( "strictlyAfter", strictlyAfter );
        }

        LADate ret = etrading::LADateHelpers::getNextCBDate( centralBankId, baseDate, strictlyAfter );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetNextCBDate_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}
