/*
 * @brief			validation interface for the CapFloor analytics 
 * @Created:		18th January 2017
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"
#include "JSONInfoBlock.h"   // JSON Info Blocks

using etrading::LabelValueBlock; 

namespace validation_api
{

    /* @brief			validation interface for the meLWOVolatilityCreate method
	*  @param [in]		objectName	    Volatility object name
	*  @param [in]		volLVB			Volatility Label Value Block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			CapFloor Handle
	*/
	std::string tryMeLWOVolatilityCreate( const std::string& objectName, const LabelValueBlock& volLVB, bool validateKeys=true );

	/* @brief			validation interface for the meLWOSabrMarketDataCreate method
	* @param [in] objectName        The name of the SABR Market Data object to create
	* @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	* @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	* @param [out]                  The objectName
	*/
	std::string tryMeLWOSabrMarketDataCreate(const std::string& objectName, const std::vector<std::string>& dataBlockNames, const etrading::JSONInfoBlockTuples& infoBlocks);
		
	/* @brief			validation interface for the meLWOSabrModelCalibrate method
	 * @param [in] objectName        The name of the SABR Model object, calibrated to market data
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
	std::string tryMeLWOSabrModelCalibrate(const std::string& objectName, const std::vector<std::string>& dataBlockNames, const etrading::JSONInfoBlockTuples& infoBlocks);

	/* @brief			validation interface for meLWOSabrVolatility
	*  @param [in]		volSurfaceName	Vol surface name
	*  @param [in]		expiry			Option expiry
	*  @param [in]		tenor			Tenor
	*  @param [in]		strike			Strike
	*  @param [in]		forward			Forward
	*/
	double tryMeLWOSabrVolatility(const std::string& volSurfaceName, const std::string& expiry, const std::string& tenor, const double strike, const double forward);

	/* @brief			validation interface for meLWOSabrParameter
	*  @param [in]		volSurfaceName	Vol surface name
	*  @param [in]		expiry			Option expiry
	*  @param [in]		tenor			Tenor
	*  @param [in]		paramName		Parameter name, i.e. alpha, beta, nu, rho
	*/
	double tryMeLWOSabrParameter(const std::string& volSurfaceName, const std::string& expiry, const std::string& tenor, const std::string& paramName);

	/* @brief validation interface for meLWOSabrMarketDataDisplay
	* @param [in] marketDataObjectName	The market data object you wish to display
	* @param [in] marketDataKey			The name of the marketData label-value block that you wish to display
	* @param [out]						A VariantMatrix containing a LabelValue block of properties
	*/
	etrading::VariantMatrix tryMeLWOSabrMarketDataDisplay(const std::string& marketDataObjectName, const std::string& marketDataKey);

	/* @brief validation interface for meLWOSabrModelDisplay
	* @param [in] modelObjectName	The model object you wish to display
	* @param [in] modelKey			The name of the model label-value block that you wish to display
	* @param [out]						A VariantMatrix containing a LabelValue block of properties
	*/
	etrading::VariantMatrix tryMeLWOSabrModelDisplay(const std::string& modelObjectName, const std::string& modelKey);


}
