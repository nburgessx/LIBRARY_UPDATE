// BondFactory.h

/*
 * @brief			Bond Factory to Create Bond LWO Objects
 * @Created:		3rd February 2017
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <string>
#include "LabelValueBlock.h"
#include "Bond.h"

namespace etrading
{
    
    /* @brief			Create a Bond based on the Bond Label Value Blocks and Schedule
    *  @param [in]		bondObjectName          Bond cached name
    *  @param [in]		bondDescriptionLVB      Bond Description Label Value Block
    *  @param [in]		bondScheduleLVB         Bond Schedule Label Value Block
    *  @param [in]		validateKeys            Validate the bond and schedule LVB keys, defaults to true
    *  @return			Bond Pointer
    */
	BondPtr createBond( const std::string& bondObjectName, const LabelValueBlock& bondLVB, const LabelValueBlock& scheduleLVB, const bool& validateKeys = true );

	/* @brief			Create a Bond based on the Bond Label Value Blocks and Schedule
	*  @param [in]		bondObjectName          Bond cached name
	*  @param [in]		bondDescriptionLVB      Bond Description Label Value Block
	*  @param [in]		validateKeys            Validate the bond and schedule LVB keys, defaults to true
	*  @return			Bond Pointer
	*/
	BondPtr createBondFromSingleLVB(const std::string& bondObjectName, const LabelValueBlock& bondLVB, const bool& validateKeys = true);
	
	/* @brief			Create a Bond based on a BondGenerator
    *  @param [in]		bondObjectName          Bond cached name to use
	*  @param [in]		bondGeneratorName       Bond Generator name to use
    *  @param [in]		bondExpressionLVB       A Label Value Block containing dataValues specific to this bond
    *  @param [in]		validateKeys            Validate the bond and schedule LVB keys, defaults to true
    *  @return			Bond Pointer
    */
	BondPtr createBondFromGenerator( const std::string& bondObjectName, const std::string& bondGeneratorName, const LabelValueBlock& bondExpressionLVB, const bool& validateKeys = true );
}

