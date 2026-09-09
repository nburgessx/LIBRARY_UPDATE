#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Swap.h"

using etrading::LabelValueBlock;

namespace validation
{

    /* @brief			validation interface for the aqSwapObjectLegCreate method
	*  @param [in]		legObjectName	Leg object name
	*  @param [in]		legLVB			Leg label value block with legs
	*  @param [in]		legObjectName	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqSwapObjectLegCreate(const std::string& legObjectName, const LabelValueBlock& legLVB, bool validateKeys=true);

    /* @brief			validation interface for the aqSwapObjectCreate method
	*  @param [in]		legObjectName		    Fee Leg object name
	*  @param [in]		feeProperties			Fee properties label value block
	*  @param [in]		feeScheduleLVB			Fee schedule label value block matrix
	*  @param [in]		validateKeys	        True to validate the all keys provided are valid. Default to True
	*  @return			legObjectName
	*/
	std::string tryAqCreditObjectFeeLegCreate(const std::string& legObjectName, const LabelValueBlock& feeProperties, const AQLStringMatrix& feeScheduleLVB, bool validateKeys=true);


    /* @brief			validation interface for the aqSwapObjectLegCreateFromSchedule method
	*  @param [in]		legObjectName	Leg object name
	*  @param [in]		scheduleName	Schedule name
	*  @param [in]		legLVB			Leg label value block 
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqSwapObjectLegCreateFromSchedule(const std::string& legObjectName, const std::string& scheduleName, const LabelValueBlock& legLVB, bool validateKeys=true);

    /* @brief			validation interface for the aqSwapObjectLegPV method
	*  @param [in]		legObjectName		Leg object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
    *  @param [in]		valuationDate	    Valuation Date (AssetSwap's Bond SettleDate)
	*  @return			Leg PV
	*/
	double tryAqSwapObjectLegPV(const std::string& legObjectName, const LabelValueBlock& valuationSettingsLVB, const std::string& fixingTableName="");

    /* @brief			validation interface for the aqSwapObjectLegDisplay method, which display the INPUT parameters of the cached swap
	*  @param [in]		legObjectName		Leg object name
	*  @return			Leg display of the input parameters
	*/
	AQLStringMatrix tryAqSwapObjectLegDisplay(const std::string& legObjectName);

   	/* @brief			validation interface for the aqSwapObjectLegDisplayCashflows method 
	*  @param [in]		legObjectName		Leg object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @param [in] 	    columnList          Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Leg display of leg output
	*/
	AnyTypeMatrix tryAqSwapObjectLegDisplayCashflows(const std::string& legObjectName, const LabelValueBlock& valuationSettingsLVB, const std::string& fixingTableName="", bool showColumnHeaders=true,
                                             const std::vector<std::string>& columnList=std::vector<std::string>());


}

