#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"
#include "JSONInfoBlock.h"   // JSON Info Blocks

using etrading::LabelValueBlock; 

namespace validation
{

    /* @brief			validation interface for the aqObjectsVolatilityCreate method
	*  @param [in]		objectName	    Volatility object name
	*  @param [in]		volLVB			Volatility Label Value Block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			CapFloor Handle
	*/
	std::string tryAqObjectsVolatilityCreate( const std::string& objectName, const LabelValueBlock& volLVB, bool validateKeys=true );

	/* @brief			validation interface for the aqObjectsSabrMarketDataCreate method
	* @param [in] objectName        The name of the SABR Market Data object to create
	* @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	* @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	* @param [out]                  The objectName
	*/
	std::string tryAqObjectsSabrMarketDataCreate(const std::string& objectName, const std::vector<std::string>& dataBlockNames, const etrading::JSONInfoBlockTuples& infoBlocks);
		
	/* @brief			validation interface for the aqObjectsSabrModelCalibrate method
	 * @param [in] objectName        The name of the SABR Model object, calibrated to market data
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
	std::string tryAqObjectsSabrModelCalibrate(const std::string& objectName, const std::vector<std::string>& dataBlockNames, const etrading::JSONInfoBlockTuples& infoBlocks);

	/* @brief			validation interface for aqObjectsSabrVolatility
	*  @param [in]		volSurfaceName	Vol surface name
	*  @param [in]		expiry			Option expiry
	*  @param [in]		tenor			Tenor
	*  @param [in]		strike			Strike
	*  @param [in]		forward			Forward
	*/
	double tryAqObjectsSabrVolatility(const std::string& volSurfaceName, const std::string& expiry, const std::string& tenor, const double strike, const double forward);

	/* @brief			validation interface for aqObjectsSabrParameter
	*  @param [in]		volSurfaceName	Vol surface name
	*  @param [in]		expiry			Option expiry
	*  @param [in]		tenor			Tenor
	*  @param [in]		paramName		Parameter name, i.e. alpha, beta, nu, rho
	*/
	double tryAqObjectsSabrParameter(const std::string& volSurfaceName, const std::string& expiry, const std::string& tenor, const std::string& paramName);

	/* @brief validation interface for aqObjectsSabrMarketDataDisplay
	* @param [in] marketDataObjectName	The market data object you wish to display
	* @param [in] marketDataKey			The name of the marketData label-value block that you wish to display
	* @param [out]						A VariantMatrix containing a LabelValue block of properties
	*/
	etrading::VariantMatrix tryAqObjectsSabrMarketDataDisplay(const std::string& marketDataObjectName, const std::string& marketDataKey);

	/* @brief validation interface for aqObjectsSabrModelDisplay
	* @param [in] modelObjectName	The model object you wish to display
	* @param [in] modelKey			The name of the model label-value block that you wish to display
	* @param [out]						A VariantMatrix containing a LabelValue block of properties
	*/
	etrading::VariantMatrix tryAqObjectsSabrModelDisplay(const std::string& modelObjectName, const std::string& modelKey);


}
