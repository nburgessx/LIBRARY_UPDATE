#include "tryAqToolReplay.h"
#include "Replay.h"
#include "StructuredExceptionHandler.h"
#include "AQLCoreAppError.h"
#include "ExceptionMacros.h"

namespace validation
{
    /* @brief			Function to load and replay a test file
    *  @param [in]		filepath		Full file path to the test csv file
    */
    const AQLString tryAqToolReplay( const AQLString& filepath )
    {
        // --------------------------------------------------------------------------------
        // Note: Functions such as this one that replay tests do not need recording features
        // --------------------------------------------------------------------------------

        // START: Structured Exception Handler
        VALID_EXCEPTION_START


        // Validation
        AQ_THROW_IF( filepath.size() == 0 || filepath.isDefined() == false, "Filepath must be Provided." );


        // Call underlying function and return result
        AQLString result = etrading::replay( filepath );
        return result;

        // END: Structured Exception Handler
        VALID_EXCEPTION_END

    }
}