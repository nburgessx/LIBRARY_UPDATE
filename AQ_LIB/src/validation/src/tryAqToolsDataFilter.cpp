// tryAqToolsDataFilter.cpp

/*
 * @brief			Function to filter a vector of input data and remove duplicates
 */

#include "tryAqToolsDataFilter.h"
#include "ExceptionMacros.h"
#include "DataUtilities.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

namespace validation
{

    using etrading::CreateDataFile;
    using etrading::decorateFilename;


    /* @brief			Function to filter a vector of input data and remove duplicates
    *  @param [in]		dataVector		            input vector data
    *  @param [in]      displayByRow		        True = Display by Row, False = Display by Column, Default is False
    *  @return			Returns a filtered data list with duplicates removed
    */
    const VariantMatrix tryAqToolsDataFilter( const VariantVector & dataVector, const bool displayByRow )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( dataVector, displayByRow );

        VariantMatrix result = etrading::dataFilter( dataVector, displayByRow );

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

}