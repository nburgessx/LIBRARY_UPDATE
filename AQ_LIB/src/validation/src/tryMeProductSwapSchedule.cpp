#include "tryMeProductSwapSchedule.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

#include "SwapUtilities.h"
#include "CoreEnumerations.h"

namespace
{
	/* @brief Returns a set of default column headings for a fixed schedule
	*/
	const std::unordered_set<etrading::CashflowHeaderEnum, etrading::EnumClassHash> getDefaultFixedScheduleColumnNames()
	{
		std::unordered_set<etrading::CashflowHeaderEnum, etrading::EnumClassHash> defaultFixedScheduleColumnNames;
		defaultFixedScheduleColumnNames.insert( etrading::ACCRUAL_START_HEADER );
		defaultFixedScheduleColumnNames.insert( etrading::ACCRUAL_END_HEADER );
		defaultFixedScheduleColumnNames.insert( etrading::ACCRUAL_DAYS_HEADER );
		defaultFixedScheduleColumnNames.insert( etrading::ACCRUAL_YEAR_FRACTIONS_HEADER );
		defaultFixedScheduleColumnNames.insert( etrading::PAYMENT_DATE_HEADER );

		return defaultFixedScheduleColumnNames;
	}

	/* @brief Returns a set of default column headings for a float schedule
	*/
	const std::unordered_set<etrading::CashflowHeaderEnum, etrading::EnumClassHash> getDefaultFloatScheduleColumnNames()
	{
		std::unordered_set<etrading::CashflowHeaderEnum, etrading::EnumClassHash> defaultFloatScheduleColumnNames;
		defaultFloatScheduleColumnNames.insert( etrading::FIXING_DATE_HEADER );
		defaultFloatScheduleColumnNames.insert( etrading::ACCRUAL_START_HEADER );
		defaultFloatScheduleColumnNames.insert( etrading::ACCRUAL_END_HEADER );
		defaultFloatScheduleColumnNames.insert( etrading::ACCRUAL_DAYS_HEADER );
		defaultFloatScheduleColumnNames.insert( etrading::ACCRUAL_YEAR_FRACTIONS_HEADER );
		defaultFloatScheduleColumnNames.insert( etrading::PAYMENT_DATE_HEADER );

		return defaultFloatScheduleColumnNames;
	}
}

namespace validation
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;
	using etrading::Schedule;

    /* @brief			This method is Deprecated, return a set of expected keys for swap leg schedule label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductSwapScheduleLVBKeys()
    {
		return etrading::getScheduleLVBKeys();
    }

    /* @brief			validation interface for the tryMeProductSwapSchedule method
    *  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
	*  @param [in]		swapScheduleLVB		A LabelValueBlock containing the schedule configuration parameters
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
	*  @param [in]	    columnList          Column header names to show specified columns. Default to empty list 
    *  @return			A matrix containing the swap schedule
    */
    AQLStringMatrix tryMeProductSwapSchedule( bool showColumnHeaders, const LabelValueBlock& swapScheduleLVB, bool validateKeys, const std::vector<std::string>& columnList, const bool& convertDatesToExcelFormat )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( showColumnHeaders, swapScheduleLVB, validateKeys, columnList );

        const std::string inputLVB = "SwapScheduleLVB";

   		AQLString scheduleType = swapScheduleLVB.getOptionalValueAsAQLString( etrading::IRS_KEY::SCHEDULE_TYPE, "");
		etrading::validateKeysForLVB(etrading::getScheduleLVBKeys(scheduleType), swapScheduleLVB.getKeys(), validateKeys );

        // TODO: Make the schedule pure virtual so that the cashflows can be pure virtual

        auto schedule = etrading::createSchedule("schedule", swapScheduleLVB);
       	AQLStringMatrix result = schedule->display(showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList), convertDatesToExcelFormat);

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the tryMeProductSwapScheduleFixed method
    *  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
	*  @param [in]		swapScheduleLVB		A LabelValueBlock containing the schedule configuration parameters
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
	*  @param [in]	    columnList          Column header names to show specified columns. Default to empty list 
    *  @return			A matrix containing the swap schedule
    */
    AQLStringMatrix tryMeProductSwapScheduleFixed( bool showColumnHeaders, const LabelValueBlock& swapScheduleLVB, bool validateKeys, const std::vector<std::string>& columnList, const bool& convertDatesToExcelFormat )
	{
	    VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( showColumnHeaders, swapScheduleLVB, validateKeys, columnList );

		// Enrich the provided schedule information with dummy default values.
		StandardStringVector addKeys(6);
        addKeys[0] = etrading::IRS_KEY::SCHEDULE_TYPE;
        addKeys[1] = etrading::IRS_KEY::PAY_RECEIVE;
        addKeys[2] = etrading::IRS_KEY::FIXED_RATE;
        addKeys[3] = etrading::IRS_KEY::NOTIONAL;

        StandardStringVector addValues(6);
        addValues[0] = "FIXED";
        addValues[1] = "PAY";
        addValues[2] = "0.0";
        addValues[3] = "1.0";
        
        LabelValueBlock enrichedSwapScheduleLVB( swapScheduleLVB, addKeys, addValues );
		
		etrading::validateKeysForLVB(etrading::getScheduleLVBKeys("FIXED"), enrichedSwapScheduleLVB.getKeys(), validateKeys );

        auto schedule = etrading::createSchedule("schedule", enrichedSwapScheduleLVB);
		auto cashflowHeaderEnumSet = columnList.size() > 0 ? etrading::toCashflowHeaderEnumSet(columnList)
															: getDefaultFixedScheduleColumnNames();

       	AQLStringMatrix result = schedule->display(showColumnHeaders, cashflowHeaderEnumSet, convertDatesToExcelFormat );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the tryMeProductSwapScheduleFloat method
    *  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
	*  @param [in]		swapScheduleLVB		A LabelValueBlock containing the schedule configuration parameters
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
	*  @param [in]	    columnList          Column header names to show specified columns. Default to empty list 
    *  @return			A matrix containing the swap schedule
    */
    AQLStringMatrix tryMeProductSwapScheduleFloat( bool showColumnHeaders, const LabelValueBlock& swapScheduleLVB, bool validateKeys, const std::vector<std::string>& columnList, const bool& convertDatesToExcelFormat )
	{
	    VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( showColumnHeaders, swapScheduleLVB, validateKeys, columnList );

		// Enrich the provided schedule information with dummy default values.
		StandardStringVector addKeys(3);
        addKeys[0] = etrading::IRS_KEY::SCHEDULE_TYPE;
        addKeys[1] = etrading::IRS_KEY::PAY_RECEIVE;
        addKeys[2] = etrading::IRS_KEY::NOTIONAL;

        StandardStringVector addValues(3);
        addValues[0] = "FLOAT";
        addValues[1] = "PAY";
        addValues[2] = "1.0";
        
        LabelValueBlock enrichedSwapScheduleLVB( swapScheduleLVB, addKeys, addValues );

        auto schedule = etrading::createSchedule("schedule", enrichedSwapScheduleLVB);
		auto cashflowHeaderEnumSet = columnList.size() > 0 ? etrading::toCashflowHeaderEnumSet(columnList)
															: getDefaultFloatScheduleColumnNames();

       	AQLStringMatrix result = schedule->display(showColumnHeaders, cashflowHeaderEnumSet, convertDatesToExcelFormat );
		
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

}
