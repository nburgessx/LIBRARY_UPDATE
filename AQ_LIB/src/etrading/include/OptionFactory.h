#pragma once

#include <string>
#include "LabelValueBlock.h"
#include "CapFloorPricer.h"
#include "Volatility.h"
#include "Bond.h"

namespace etrading
{
    /* @brief			Create a Option Pointer based on the Option's Label Value Blocks
	*  @param [in]		objectName      Option cached name
	*  @param [in]		tradeLVB		Option Description Label Value Block
	*  @param [in]		validateKeys    Validate the bond and schedule LVB keys, defaults to true
	*  @return			Option Pointer
	*/
	OptionPtr createOption(const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys = true);

    /* @brief			Create a Option Pointer based on the Option's Label Value Blocks
	*  @param [in]		objectName      Option cached name
	*  @param [in]		tradeLVB		Option Description Label Value Block
	*  @param [in]		validateKeys    Validate the bond and schedule LVB keys, defaults to true
	*  @return			Option Pointer
	*/
	OptionPtr createCapFloor(const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys = true);

    /* @brief			Create a Option Pointer based on the Option's Label Value Blocks
	*  @param [in]		objectName      Option cached name
	*  @param [in]		tradeLVB		Option Description Label Value Block
	*  @param [in]		validateKeys    Validate the bond and schedule LVB keys, defaults to true
	*  @return			Option Pointer
	*/
	OptionPtr createEuropeanSwaption(const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys = true);

	/* @brief			Create a Option Pointer based on the Option's Label Value Blocks
	*  @param [in]		objectName      Option cached name
	*  @param [in]		tradeLVB		Option Description Label Value Block
	*  @param [in]		validateKeys    Validate the bond and schedule LVB keys, defaults to true
	*  @return			Option Pointer
	*/
	OptionPtr createBondOption(const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys = true);

	/* @brief			Create a Volatility Pointer based on the Volatility's Label Value Blocks
	*  @param [in]		objectName      Volatility cached name
	*  @param [in]		volLVB			Volatility Description Label Value Block
	*  @param [in]		validateKeys    Validate the bond and schedule LVB keys, defaults to true
	*  @return			Option Pointer
	*/
	std::shared_ptr<Volatility> createVolatility(const std::string& objectName, const LabelValueBlock& volLVB, const bool& validateKeys = true);

	AnyTypeMatrix viewGreeks(const BlackScholesGreeks& greeks, const bool& showColumnHeaders);

}

