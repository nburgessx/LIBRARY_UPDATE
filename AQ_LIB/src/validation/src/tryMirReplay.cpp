// tryMirReplay.cpp

#include "tryMirReplay.h"
#include "Replay.h"
#include "StructuredExceptionHandler.h"
#include "LACoreAppError.h"

namespace validation
{
    /* @brief			Function to load and replay a test file
    *  @param [in]		filepath		Full file path to the test csv file
    */
    const LAString tryMirReplay( const LAString& filepath )
    {
        // --------------------------------------------------------------------------------
        // Note: Functions such as this one that replay tests do not need recording features
        // --------------------------------------------------------------------------------

        // START: Structured Exception Handler
        VALID_EXCEPTION_START


        // Validation
        if ( filepath.size() == 0 || filepath.isDefined() == false )
        {
            throw LACoreInvalidData( "#Error: Filepath must be Provided.", __FILE__, __LINE__ );
        }


        // Call underlying function and return result
        LAString result = etrading::replay( filepath );
        return result;

        // END: Structured Exception Handler
        VALID_EXCEPTION_END

    }
}