#include "tryMeUtilityRecord.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "CreateDataFile.h"

namespace validation
{
    /* @brief			validation interface for the meUtilityVersion function
    *  @param [in]		enable		True to enable the recording of tests
    *  @param [in]		folder		Output folder
    *  @param [in]		repeat		Allowed repeated tests
    *  @param [in]		startIndex	Start index for repeated tests
    *  @param [in]		maxIndex	Max index for repeated tests
    *  @return			A notification string
    */
    LAString tryMeUtilityRecord( bool enable, const LAString& folder, bool repeat, int startIndex, int maxIndex )
    {
        VALID_EXCEPTION_START

        // Set the Record Flag and Output Folder
        const bool record = etrading::CreateDataFile::setRecordFlag( enable );
        const LAString outputFolder = etrading::CreateDataFile::setOutputFolder( LAString( folder ) );

        // Ensure that test cases are not indexed with negative values
        if ( startIndex < 0 || maxIndex < 0 )
        {
            throw LACoreInvalidData( "#Error: Start- and EndIndex cannot be negative", __FILE__, __LINE__ );
        }

        std::string msg;
        if ( !record )
        {
            // Disable Test Recording
            msg = "Recording Disabled";
        }
        else if ( !repeat )
        {
            // Enable Test Recording ( Single Test Mode )
            const int r = etrading::CreateDataFile::endTestCount();
            msg = "Recording Enabled, Output Folder: " + std::string( outputFolder.getCString() );
        }
        else
        {
            // Enable Test Recording ( Repeat Mode )
            const int start = etrading::CreateDataFile::beginTestCount( startIndex );
            const int max = etrading::CreateDataFile::setMaxIndex( maxIndex );

            msg = "Repeated Recording Enabled, Start Index: "
                  + boost::lexical_cast<std::string>( startIndex )
                  + ", Max Index: "
                  + boost::lexical_cast<std::string>( maxIndex )
                  + ", Output Folder: "
                  + std::string( outputFolder.getCString() );
        }

        LAString ret( msg.c_str() );

        return ret;

        VALID_EXCEPTION_END
    }
}