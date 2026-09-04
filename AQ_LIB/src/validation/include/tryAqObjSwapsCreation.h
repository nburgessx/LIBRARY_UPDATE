#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Swap.h"

using etrading::LabelValueBlock;

namespace validation
{

    std::vector<std::string> tryAqObjSwapsLVBKeys();

	/* @brief			validation interface for the aqObjSwapsCreateFromMutipleLegs method
	*  @param [in]		swapName		Swap name to be cached
	*  @param [in]		legObjectNames	Cached leg object names
	*  @param [in]		swapPropertiesLVB	Swap level properties
	*  @param [in]		isXccySwap	    True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqObjSwapsCreateFromLegs(const std::string& swapName, const std::vector<std::string>& legObjectNames, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys=true);

    /* @brief			validation interface for the aqObjSwapsCreateFromLegLVBs method
	*  @param [in]		swapName		Swap name
	*  @param [in]		leg1LVB			Leg1 label value block 
	*  @param [in]		leg2LVB			Leg2 label value block 
	*  @param [in]		swapPropertiesLVB	Swap level properties
	*  @param [in]		isXccySwap	    True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqObjSwapsCreateFromLegLVBs(const std::string& swapName, const LabelValueBlock& leg1LVB, const LabelValueBlock& leg2LVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys=true);

	/* @brief			validation interface for the aqObjSwapsCreate method
	*  @param [in]		swapName		Swap name
	*  @param [in]		swapLVB			Swap label value block with legs
	*  @param [in]		swapPropertiesLVB	Swap level properties
	*  @param [in]		isXccySwap	    True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqObjSwapsCreate(const std::string& swapName, const AQLStringMatrix& swapLVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys=true);

	/* @brief			validation interface for the aqObjSwapsCreateFromLegs method
	*  @param [in]		swapName		Swap name
	*  @param [in]		schedule1Name	Schedule1 name
	*  @param [in]		schedule2Name	Schedule2 name
	*  @param [in]		leg1LVB			Leg1 label value block 
	*  @param [in]		leg2LVB			Leg2 label value block 
	*  @param [in]		swapPropertiesLVB	Swap level properties
	*  @param [in]		isXccySwap	    True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqObjSwapsCreateFromSchedule(const std::string& swapName, const std::string& schedule1Name, const std::string& schedule2Name, const LabelValueBlock& leg1LVB, const LabelValueBlock& leg2LVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys=true);

	/* @brief			validation interface for the aqObjSwapsCreate method
	*  @param [in]		swapName		Swap name
	*  @param [in]		swapLVB			Swap label value block with legs
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqObjSwapsCreateBackToBack(const std::string& fromSwapName, const std::string& toSwapName);

    /* @brief			validation interface for the aqObjSwapsAddLeg method
	*  @param [in]		swapName		The cached swap name
	*  @param [in]		legObjectName   The cached leg name
	*  @return			Swap name
	*/
	std::string tryAqObjSwapsAddLeg(const std::string& swapName, const std::string& legObjectName);

    /* @brief			validation interface for the aqObjSwapsAddFee method, same as aqObjSwapsAddLeg() except for the extra Fee leg check
	*  @param [in]		swapName		The cached swap name
	*  @param [in]		feeName         The cached fee leg name
	*  @return			Swap name
	*/
	std::string tryAqObjSwapsAddFee(const std::string& swapName, const std::string& feeName);

    /* @brief			validation interface for the aqObjSwapsGeneratorCreate method
	*  @param [in]		swapGeneratorName		Swap generator name
	*  @param [in]		swapGeneratorLVB	    Swap generator label value block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapGeneratorName
	*/
	std::string tryAqObjSwapsGeneratorCreate(const std::string& swapGeneratorName, const AQLStringMatrix& swapGeneratorLVB, bool validateKeys=true);

    /* @brief			validation interface for the aqObjSwapsCreateFromGenerator method
	*  @param [in]		swapName		        Swap name
	*  @param [in]		swapGeneratorName		Swap generator name
	*  @param [in]		expressionLVB		    Expression label value block to customize the swap
	*  @param [in]		swapPropertiesLVB	    Swap level properties
	*  @param [in]		isXccySwap	            True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	        True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqObjSwapsCreateFromGenerator(const std::string& swapName, const std::string& swapGeneratorName, const LabelValueBlock& expressionLVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys=true);

    /* @brief			validation interface for the aqObjSwapsDisplay method, which display the INPUT parameters of the cached swap
	*  @param [in]		swapName		Swap name
	*  @return			Swap display of the input parameters
	*/
	std::vector<AnyTypeMatrix> tryAqObjSwapsDisplay(const std::string& swapName);

    /* @brief			validation interface for the aqObjSwapsGeneratorDisplay method, which display the INPUT parameters of the cached swap generator
	*  @param [in]		swapGeneratorName		Swap generator name
	*  @return			Swap generator display of the input parameters
	*/
	AQLStringMatrix tryAqObjSwapsGeneratorDisplay(const std::string& swapGeneratorName);


}

