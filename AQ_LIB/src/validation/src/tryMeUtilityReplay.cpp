/*
 * @brief			validation interface for the meUtilityReplay function
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "tryMeUtilityReplay.h"
#include "Replay.h"
#include "StructuredExceptionHandler.h"
#include "LACoreAppError.h"

namespace validation_api
{
    /* @brief			Function to load and replay a test file
    *  @param [in]		filepath		Full file path to the test csv file
    */
    const LAString tryMeUtilityReplay( const LAString& filepath )
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