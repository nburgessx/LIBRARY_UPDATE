#include "tryMirGetBusinessDays.h"
#include "LADateHelpers.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"

namespace validation
{

    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the mirGetECBStartDate method
    *  @param [in]		ecbDate ECB(European Central Bank) date
    *  @return			The start date of an ECB Swap based on the ecb date
    */
    AQLDate tryMirGetECBStartDate( const AQLDate& ecbDate )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetECBStartDate_inputs" );
            file.write( "generatorFunction", "tryMirGetECBStartDate" );
            file.write( "ecbDate", ecbDate );
        }

        AQLDate ret = etrading::LADateHelpers::getECBStartDate( ecbDate );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetECBStartDate_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }
}

