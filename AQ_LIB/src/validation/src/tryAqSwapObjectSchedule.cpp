#include "tryAqSwapObjectSchedule.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

#include "SwapUtilities.h"
#include "AQObjUtilities.h"
#include "ObjectUtilities.h"

namespace validation
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;
	using etrading::Schedule;

	/* @brief			validation interface for the aqSwapObjectScheduleCreate method
	*  @param [in]		scheduleName		Schedule name
	*  @param [in]		swapScheduleLVB		Schedule label value block 
	*  @param [in]		legName				legName name
	*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
    *  @return			The schedule name
	*/
	std::string tryAqSwapObjectScheduleCreate(const std::string& scheduleName, const LabelValueBlock& swapScheduleLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( scheduleName.c_str(), "", scheduleName, swapScheduleLVB, validateKeys );

		AQLString scheduleType = swapScheduleLVB.getOptionalValueAsAQLString( etrading::IRS_KEY::SCHEDULE_TYPE);
		etrading::validateKeysForLVB(etrading::getScheduleLVBKeys(scheduleType), swapScheduleLVB.getKeys(), validateKeys );

		std::shared_ptr<Schedule> mySchedule = etrading::createSchedule(scheduleName, swapScheduleLVB);

		etrading::registerToCache<Schedule>( mySchedule);

		std::string ret = scheduleName;

		AQ_RECORD_DECORATED_OUTPUTS( scheduleName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqSwapSchedule method
	*  @param [in]		swapName		        Swap name
	*  @param [in]		showBespokeProperties   True to show bespoke schedule properties, default to false
	*  @param [in]		showColumnHeaders		True to show column headers, default to true
	*  @param [in]  	columnList              Column header names to show specified columns. Default to empty list showing all columns.
    *  @return			Schedule display
    */
    AnyTypeMatrix tryAqSwapObjectScheduleDisplay( const std::string& scheduleName, bool showBespokeProperties, bool showColumnHeaders, const std::vector<std::string>& columnList)
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( scheduleName, showBespokeProperties, showColumnHeaders, columnList );

		auto schedule = etrading::getSchedule(scheduleName);

		auto headerBodyPair = schedule->view(showBespokeProperties, showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList));

		auto result = etrading::mergeHeaderAndBodyPair(headerBodyPair);

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the aqSwapObjectScheduleCreateBespoke method
	*  @param [in]		scheduleName					Bespoke schedule name
	*  @param [in]		bespokeScheduleProperties		Bespoke schedule properties label value block
	*  @param [in]		bespokeScheduleLVB				Bespoke schedule cashflow label value block matrix
	*  @param [in]		validateKeys		            True to validate the all keys provided are valid for the bespokeScheduleProperties. Default to True
    *  @return			The schedule name
    */
	std::string tryAqSwapObjectScheduleCreateBespoke(const std::string& scheduleName, const LabelValueBlock& bespokeScheduleProperties, const AQLStringMatrix& bespokeScheduleLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( scheduleName.c_str(), "", scheduleName, bespokeScheduleProperties, bespokeScheduleLVB, validateKeys );

		etrading::validateKeysForLVB(Schedule::bespokeLVBKeys(etrading::BESPOKE_SCHEDULE_WITH_PROPERTIES), bespokeScheduleProperties.getKeys(), validateKeys );
        std::vector<LabelValueBlock> cashflowLVBs = etrading::buildMultiLabelValueBlock(bespokeScheduleLVB);
		auto mySchedule = etrading::createScheduleBespoke(scheduleName, bespokeScheduleProperties, cashflowLVBs, etrading::BESPOKE_SCHEDULE_WITH_PROPERTIES);

		etrading::registerToCache<Schedule>( mySchedule);

		std::string ret = scheduleName;

		AQ_RECORD_DECORATED_OUTPUTS( scheduleName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqSwapObjectScheduleCreateBespokeFromCashflows method
	*  @param [in]		scheduleObjectName				Bespoke schedule name
	*  @param [in]		bespokeScheduleProperties		Bespoke schedule properties label value block
	*  @param [in]		bespokeCashflowsLVB				Bespoke schedule cashflow label value block matrix
	*  @param [in]		validateKeys		            True to validate the all keys provided are valid for the bespokeScheduleProperties. Default to True
	*  @return			The schedule name
	*/
	std::string tryAqSwapObjectScheduleCreateBespokeFromCashflows(const std::string& scheduleObjectName, const LabelValueBlock& bespokeScheduleProperties, const AQLStringMatrix& bespokeCashflowsLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( scheduleObjectName.c_str(), "", scheduleObjectName, bespokeScheduleProperties, bespokeCashflowsLVB, validateKeys );

		std::vector<LabelValueBlock> cashflowLVBs = etrading::buildMultiLabelValueBlock(bespokeCashflowsLVB);
		etrading::validateKeysForLVB(Schedule::bespokeLVBKeys(etrading::BESPOKE_SCHEDULE), bespokeScheduleProperties.getKeys(), validateKeys);

		etrading::validateKeysForLVB(etrading::Cashflow::bespokeCashflowLVBKeys(), cashflowLVBs.at(0).getKeys(), validateKeys);

		auto mySchedule = etrading::createScheduleBespoke(scheduleObjectName, bespokeScheduleProperties, cashflowLVBs, etrading::BESPOKE_SCHEDULE);

		etrading::registerToCache<Schedule>(mySchedule);

		std::string ret = scheduleObjectName;

		AQ_RECORD_DECORATED_OUTPUTS( scheduleObjectName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

   	/* @brief			validation interface for the aqCreditObjectFeeScheduleCreate method
	*  @param [in]		scheduleName				Fee schedule name
	*  @param [in]		feeScheduleLVB				Fee schedule cashflow label value block
	*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
    *  @return			The schedule name
    */
	std::string tryAqCreditObjectFeeScheduleCreate(const std::string& scheduleName, const AQLStringMatrix& feeScheduleLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( scheduleName.c_str(), "", scheduleName, feeScheduleLVB, validateKeys );

        AQLStringVector keys;
        for (size_t i = 0; i < feeScheduleLVB.size(); ++i)
        {
            keys.push_back(feeScheduleLVB[i][0]);
        }

        etrading::validateKeysForLVB(etrading::getScheduleLVBKeys("fee"), keys, validateKeys );
        
        std::vector<LabelValueBlock> cashflowLVBs = etrading::buildMultiLabelValueBlock(feeScheduleLVB);

		auto mySchedule = etrading::createFeeSchedule(scheduleName, cashflowLVBs);

		etrading::registerToCache<Schedule>( mySchedule);

		std::string ret = scheduleName;

		AQ_RECORD_DECORATED_OUTPUTS( scheduleName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}


	/* @brief			validation interface for the aqToolSwapScheduleTemplate method
	*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @param [in]	    columnList          Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			A matrix of floating leg/fixing leg schedules
	*/
	AnyTypeMatrix tryAqToolSwapScheduleTemplate(bool showColumnHeaders, const LabelValueBlock& swapScheduleLVB, bool validateKeys, const std::vector<std::string>& columnList)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(showColumnHeaders, swapScheduleLVB, validateKeys, columnList);

		const std::string inputLVB = "SwapScheduleLVB";

		AQLString scheduleType = swapScheduleLVB.getOptionalValueAsAQLString(etrading::IRS_KEY::SCHEDULE_TYPE, "");
		etrading::validateKeysForLVB(etrading::getScheduleLVBKeys(scheduleType), swapScheduleLVB.getKeys(), validateKeys);

		auto schedule = etrading::createSchedule("schedule", swapScheduleLVB);
		auto headerBody = schedule->view(false /** showBespokeProperties */, showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList));
		auto result = etrading::mergeHeaderAndBodyPair(headerBody);

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}


}
