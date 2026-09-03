/*
 * @brief			validation interface for the meProductSwapPV method
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Swap.h"

using etrading::LabelValueBlock;

namespace validation_api
{

    std::vector<std::string> tryMeLWOSwapLVBKeys();

	/* @brief			validation interface for the meLWOSwapCreateFromMutipleLegs method
	*  @param [in]		swapName		Swap name to be cached
	*  @param [in]		legObjectNames	Cached leg object names
	*  @param [in]		swapPropertiesLVB	Swap level properties
	*  @param [in]		isXccySwap	    True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryMeLWOSwapCreateFromLegs(const std::string& swapName, const std::vector<std::string>& legObjectNames, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys=true);

    /* @brief			validation interface for the meLWOSwapCreateFromLegLVBs method
	*  @param [in]		swapName		Swap name
	*  @param [in]		leg1LVB			Leg1 label value block 
	*  @param [in]		leg2LVB			Leg2 label value block 
	*  @param [in]		swapPropertiesLVB	Swap level properties
	*  @param [in]		isXccySwap	    True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryMeLWOSwapCreateFromLegLVBs(const std::string& swapName, const LabelValueBlock& leg1LVB, const LabelValueBlock& leg2LVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys=true);

	/* @brief			validation interface for the meLWOSwapCreate method
	*  @param [in]		swapName		Swap name
	*  @param [in]		swapLVB			Swap label value block with legs
	*  @param [in]		swapPropertiesLVB	Swap level properties
	*  @param [in]		isXccySwap	    True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryMeLWOSwapCreate(const std::string& swapName, const LAStringMatrix& swapLVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys=true);

	/* @brief			validation interface for the meLWOSwapCreateFromLegs method
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
	std::string tryMeLWOSwapCreateFromSchedule(const std::string& swapName, const std::string& schedule1Name, const std::string& schedule2Name, const LabelValueBlock& leg1LVB, const LabelValueBlock& leg2LVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys=true);

	/* @brief			validation interface for the meLWOSwapCreate method
	*  @param [in]		swapName		Swap name
	*  @param [in]		swapLVB			Swap label value block with legs
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryMeLWOSwapCreateBackToBack(const std::string& fromSwapName, const std::string& toSwapName);

    /* @brief			validation interface for the meLWOSwapAddLeg method
	*  @param [in]		swapName		The cached swap name
	*  @param [in]		legObjectName   The cached leg name
	*  @return			Swap name
	*/
	std::string tryMeLWOSwapAddLeg(const std::string& swapName, const std::string& legObjectName);

    /* @brief			validation interface for the meLWOSwapAddFee method, same as meLWOSwapAddLeg() except for the extra Fee leg check
	*  @param [in]		swapName		The cached swap name
	*  @param [in]		feeName         The cached fee leg name
	*  @return			Swap name
	*/
	std::string tryMeLWOSwapAddFee(const std::string& swapName, const std::string& feeName);

    /* @brief			validation interface for the meLWOSwapGeneratorCreate method
	*  @param [in]		swapGeneratorName		Swap generator name
	*  @param [in]		swapGeneratorLVB	    Swap generator label value block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapGeneratorName
	*/
	std::string tryMeLWOSwapGeneratorCreate(const std::string& swapGeneratorName, const LAStringMatrix& swapGeneratorLVB, bool validateKeys=true);

    /* @brief			validation interface for the meLWOSwapCreateFromGenerator method
	*  @param [in]		swapName		        Swap name
	*  @param [in]		swapGeneratorName		Swap generator name
	*  @param [in]		expressionLVB		    Expression label value block to customize the swap
	*  @param [in]		swapPropertiesLVB	    Swap level properties
	*  @param [in]		isXccySwap	            True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	        True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryMeLWOSwapCreateFromGenerator(const std::string& swapName, const std::string& swapGeneratorName, const LabelValueBlock& expressionLVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys=true);

    /* @brief			validation interface for the meLWOSwapDisplay method, which display the INPUT parameters of the cached swap
	*  @param [in]		swapName		Swap name
	*  @return			Swap display of the input parameters
	*/
	std::vector<AnyTypeMatrix> tryMeLWOSwapDisplay(const std::string& swapName);

    /* @brief			validation interface for the meLWOSwapGeneratorDisplay method, which display the INPUT parameters of the cached swap generator
	*  @param [in]		swapGeneratorName		Swap generator name
	*  @return			Swap generator display of the input parameters
	*/
	LAStringMatrix tryMeLWOSwapGeneratorDisplay(const std::string& swapGeneratorName);


}

