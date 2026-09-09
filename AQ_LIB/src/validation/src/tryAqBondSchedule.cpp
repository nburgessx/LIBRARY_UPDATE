#include "tryAqBondSchedule.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

#include "SwapUtilities.h"

namespace validation
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;
	using etrading::Schedule;

    /* @brief			return a set of expected keys for bond leg schedule label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryAqBondScheduleLVBKeys()
    {
		return etrading::getScheduleLVBKeys();
    }

    /* @brief			validation interface for the tryAqBondSchedule method
    *  @param [in]		bondScheduleLVB		A label value block defining the bond.
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
	*  @param [in]		showColumnHeaders	True to include the column headers in the output. Default to True
    *  @return			A matrix of floating leg/fixing leg schedules
    */
    AQLStringMatrix tryAqBondSchedule( const LabelValueBlock& bondScheduleLVB, bool validateKeys, bool showColumnHeaders, const std::vector<std::string>& columnList )
    {
        VALID_EXCEPTION_START

        const std::string inputLVB = "BondScheduleLVB";

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( bondScheduleLVB, validateKeys, showColumnHeaders, columnList );

        etrading::validateKeysForLVB( tryAqBondScheduleLVBKeys(), bondScheduleLVB.getKeys(), validateKeys );

       	Schedule schedule(bondScheduleLVB,"schedule");
		
		AQLStringMatrix result = schedule.display(showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList));

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

}
