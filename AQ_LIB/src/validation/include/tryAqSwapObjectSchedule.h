#pragma once

#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{

	/* @brief			validation interface for the aqSwapObjectScheduleCreate method
	*  @param [in]		scheduleName		Schedule name
	*  @param [in]		swapScheduleLVB		Schedule label value block 
	*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
    *  @return			The scheduleName showing the schedule has been cached.
	*/
	std::string tryAqSwapObjectScheduleCreate(const std::string& scheduleName, const LabelValueBlock& swapScheduleLVB, bool validateKeys = true);

	/* @brief			validation interface for the aqSwapSchedule method
	*  @param [in]		swapName		        Swap name
	*  @param [in]		showBespokeProperties   True to show bespoke schedule properties, default to false
	*  @param [in]		showColumnHeaders		True to show column headers, default to true
	*  @param [in] 	    columnList              Column header names to show specified columns. Default to empty list showing all columns.
    *  @return			Schedule display
    */
    AnyTypeMatrix tryAqSwapObjectScheduleDisplay( const std::string& scheduleName, bool showBespokeProperties=false, bool showColumnHeaders=true,
                                           const std::vector<std::string>& columnList=std::vector<std::string>());

	/* @brief			validation interface for the aqSwapObjectScheduleCreateBespoke method
	*  @param [in]		scheduleName					Bespoke schedule name
	*  @param [in]		bespokeScheduleProperties		Bespoke schedule properties label value block
	*  @param [in]		bespokeScheduleLVB				Bespoke schedule cashflow label value block
	*  @param [in]		validateKeys		            True to validate the all keys provided are valid for the bespokeScheduleProperties. Default to True
    *  @return			The schedule name
    */
	std::string tryAqSwapObjectScheduleCreateBespoke(const std::string& scheduleName, const LabelValueBlock& bespokeScheduleProperties, const AQLStringMatrix& bespokeScheduleLVB, bool validateKeys = true);

	/* @brief			validation interface for the aqSwapObjectScheduleCreateBespokeFromCashflows method
	*  @param [in]		scheduleObjectName				Bespoke schedule name
	*  @param [in]		bespokeScheduleProperties		Bespoke schedule properties label value block
	*  @param [in]		bespokeCashflowsLVB				Bespoke schedule cashflow label value block matrix
	*  @param [in]		validateKeys		            True to validate the all keys provided are valid for the bespokeScheduleProperties. Default to True
	*  @return			The schedule name
	*/
	std::string tryAqSwapObjectScheduleCreateBespokeFromCashflows(const std::string& scheduleObjectName, const LabelValueBlock& bespokeScheduleProperties, const AQLStringMatrix& bespokeCashflowsLVB, bool validateKeys = true);

   	/* @brief			validation interface for the aqCreditObjectFeeScheduleCreate method
	*  @param [in]		scheduleName				Fee schedule name
	*  @param [in]		feeScheduleLVB				Fee schedule cashflow label value block
	*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
    *  @return			The schedule name
    */
	std::string tryAqCreditObjectFeeScheduleCreate(const std::string& scheduleName, const AQLStringMatrix& feeScheduleLVB, bool validateKeys);

	/* @brief			validation interface for the aqToolSwapScheduleTemplate method
	*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @param [in]	    columnList          Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			A matrix of floating leg/fixing leg schedules
	*/
	AnyTypeMatrix tryAqToolSwapScheduleTemplate(bool showColumnHeaders, const LabelValueBlock& swapScheduleLVB, bool validateKeys = true, const std::vector<std::string>& columnList = std::vector<std::string>());

}
