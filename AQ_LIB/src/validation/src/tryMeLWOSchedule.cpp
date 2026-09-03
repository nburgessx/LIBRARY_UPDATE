#include "tryMeLWOSchedule.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

#include "SwapUtilities.h"
#include "LWOUtilities.h"
#include "ObjectUtilities.h"

namespace validation
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;
	using etrading::Schedule;

	/* @brief			validation interface for the meLWOScheduleCreate method
	*  @param [in]		scheduleName		Schedule name
	*  @param [in]		swapScheduleLVB		Schedule label value block 
	*  @param [in]		legName				legName name
	*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
    *  @return			The schedule name
	*/
	std::string tryMeLWOScheduleCreate(const std::string& scheduleName, const LabelValueBlock& swapScheduleLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOScheduleCreate_inputs", scheduleName.c_str()));
			file.write("generatorFunction", "tryMeLWOScheduleCreate");
			file.write("scheduleName", scheduleName);
			file.write("swapScheduleLVB", swapScheduleLVB);
			file.write("validateKeys", validateKeys);
		}

		LAString scheduleType = swapScheduleLVB.getOptionalValueAsLAString( etrading::IRS_KEY::SCHEDULE_TYPE);
		etrading::validateKeysForLVB(etrading::getScheduleLVBKeys(scheduleType), swapScheduleLVB.getKeys(), validateKeys );

		std::shared_ptr<Schedule> mySchedule = etrading::createSchedule(scheduleName, swapScheduleLVB);

		etrading::registerToCache<Schedule>( mySchedule);

		std::string ret = scheduleName;

		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOScheduleCreate_outputs", scheduleName.c_str()));
			file.write("output", ret);
		}

		return ret;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the meProductSwapSchedule method
	*  @param [in]		swapName		        Swap name
	*  @param [in]		showBespokeProperties   True to show bespoke schedule properties, default to false
	*  @param [in]		showColumnHeaders		True to show column headers, default to true
	*  @param [in]  	columnList              Column header names to show specified columns. Default to empty list showing all columns.
    *  @return			Schedule display
    */
    AnyTypeMatrix tryMeLWOScheduleDisplay( const std::string& scheduleName, bool showBespokeProperties, bool showColumnHeaders, const std::vector<std::string>& columnList)
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( scheduleName, showBespokeProperties, showColumnHeaders, columnList );

		auto schedule = etrading::getSchedule(scheduleName);

		auto headerBodyPair = schedule->view(showBespokeProperties, showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList));

		auto result = etrading::mergeHeaderAndBodyPair(headerBodyPair);

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the meLWOScheduleCreateBespoke method
	*  @param [in]		scheduleName					Bespoke schedule name
	*  @param [in]		bespokeScheduleProperties		Bespoke schedule properties label value block
	*  @param [in]		bespokeScheduleLVB				Bespoke schedule cashflow label value block matrix
	*  @param [in]		validateKeys		            True to validate the all keys provided are valid for the bespokeScheduleProperties. Default to True
    *  @return			The schedule name
    */
	std::string tryMeLWOScheduleCreateBespoke(const std::string& scheduleName, const LabelValueBlock& bespokeScheduleProperties, const LAStringMatrix& bespokeScheduleLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOScheduleCreateBespoke_inputs", scheduleName.c_str()));
			file.write("generatorFunction", "tryMeLWOScheduleCreateBespoke");
			file.write("scheduleName", scheduleName);
			file.write("bespokeScheduleProperties", bespokeScheduleProperties);
			file.write("bespokeScheduleLVB", bespokeScheduleLVB);
			file.write("validateKeys", validateKeys);
		}

		etrading::validateKeysForLVB(Schedule::bespokeLVBKeys(etrading::BESPOKE_SCHEDULE_WITH_PROPERTIES), bespokeScheduleProperties.getKeys(), validateKeys );
        std::vector<LabelValueBlock> cashflowLVBs = etrading::buildMultiLabelValueBlock(bespokeScheduleLVB);
		auto mySchedule = etrading::createScheduleBespoke(scheduleName, bespokeScheduleProperties, cashflowLVBs, etrading::BESPOKE_SCHEDULE_WITH_PROPERTIES);

		etrading::registerToCache<Schedule>( mySchedule);

		std::string ret = scheduleName;

		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOScheduleCreateBespoke_outputs", scheduleName.c_str()));
			file.write("output", ret);
		}

		return ret;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the meLWOScheduleCreateBespokeFromCashflows method
	*  @param [in]		scheduleObjectName				Bespoke schedule name
	*  @param [in]		bespokeScheduleProperties		Bespoke schedule properties label value block
	*  @param [in]		bespokeCashflowsLVB				Bespoke schedule cashflow label value block matrix
	*  @param [in]		validateKeys		            True to validate the all keys provided are valid for the bespokeScheduleProperties. Default to True
	*  @return			The schedule name
	*/
	std::string tryMeLWOScheduleCreateBespokeFromCashflows(const std::string& scheduleObjectName, const LabelValueBlock& bespokeScheduleProperties, const LAStringMatrix& bespokeCashflowsLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled())
		{
			CreateDataFile file(decorateFilename("tryMeLWOScheduleCreateBespokeFromCashflows_inputs", scheduleObjectName.c_str()));
			file.write("generatorFunction", "tryMeLWOScheduleCreateBespokeFromCashflows");
			file.write("scheduleObjectName", scheduleObjectName);
			file.write("bespokeScheduleProperties", bespokeScheduleProperties);
			file.write("bespokeCashflowsLVB", bespokeCashflowsLVB);
			file.write("validateKeys", validateKeys);
		}

		std::vector<LabelValueBlock> cashflowLVBs = etrading::buildMultiLabelValueBlock(bespokeCashflowsLVB);
		etrading::validateKeysForLVB(Schedule::bespokeLVBKeys(etrading::BESPOKE_SCHEDULE), bespokeScheduleProperties.getKeys(), validateKeys);

		etrading::validateKeysForLVB(etrading::Cashflow::bespokeCashflowLVBKeys(), cashflowLVBs.at(0).getKeys(), validateKeys);

		auto mySchedule = etrading::createScheduleBespoke(scheduleObjectName, bespokeScheduleProperties, cashflowLVBs, etrading::BESPOKE_SCHEDULE);

		etrading::registerToCache<Schedule>(mySchedule);

		std::string ret = scheduleObjectName;

		if (CreateDataFile::recordEnabled())
		{
			CreateDataFile file(decorateFilename("tryMeLWOScheduleCreateBespokeFromCashflows_outputs", scheduleObjectName.c_str()));
			file.write("output", ret);
		}

		return ret;

		VALID_EXCEPTION_END
	}

   	/* @brief			validation interface for the meLWOFeeScheduleCreate method
	*  @param [in]		scheduleName				Fee schedule name
	*  @param [in]		feeScheduleLVB				Fee schedule cashflow label value block
	*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
    *  @return			The schedule name
    */
	std::string tryMeLWOFeeScheduleCreate(const std::string& scheduleName, const LAStringMatrix& feeScheduleLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOFeeScheduleCreate_inputs", scheduleName.c_str()));
			file.write("generatorFunction", "tryMeLWOFeeScheduleCreate");
			file.write("scheduleName", scheduleName);
			file.write("feeScheduleLVB", feeScheduleLVB);
			file.write("validateKeys", validateKeys);
		}

        LAStringVector keys;
        for (size_t i = 0; i < feeScheduleLVB.size(); ++i)
        {
            keys.push_back(feeScheduleLVB[i][0]);
        }

        etrading::validateKeysForLVB(etrading::getScheduleLVBKeys("fee"), keys, validateKeys );
        
        std::vector<LabelValueBlock> cashflowLVBs = etrading::buildMultiLabelValueBlock(feeScheduleLVB);

		auto mySchedule = etrading::createFeeSchedule(scheduleName, cashflowLVBs);

		etrading::registerToCache<Schedule>( mySchedule);

		std::string ret = scheduleName;

		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOFeeScheduleCreate_outputs", scheduleName.c_str()));
			file.write("output", ret);
		}

		return ret;

		VALID_EXCEPTION_END
	}


	/* @brief			validation interface for the meUtilitySwapScheduleTemplate method
	*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @param [in]	    columnList          Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			A matrix of floating leg/fixing leg schedules
	*/
	AnyTypeMatrix tryMeUtilitySwapScheduleTemplate(bool showColumnHeaders, const LabelValueBlock& swapScheduleLVB, bool validateKeys, const std::vector<std::string>& columnList)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(showColumnHeaders, swapScheduleLVB, validateKeys, columnList);

		const std::string inputLVB = "SwapScheduleLVB";

		LAString scheduleType = swapScheduleLVB.getOptionalValueAsLAString(etrading::IRS_KEY::SCHEDULE_TYPE, "");
		etrading::validateKeysForLVB(etrading::getScheduleLVBKeys(scheduleType), swapScheduleLVB.getKeys(), validateKeys);

		auto schedule = etrading::createSchedule("schedule", swapScheduleLVB);
		auto headerBody = schedule->view(false /** showBespokeProperties */, showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList));
		auto result = etrading::mergeHeaderAndBodyPair(headerBody);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}


}
