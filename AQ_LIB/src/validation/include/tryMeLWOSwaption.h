#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"
#include "RateProviderExternal.h"


using etrading::LabelValueBlock; 

namespace validation
{

	/* @brief			validation interface for the meLWOSwaptionCreate method
	*  @param [in]		objectName		Swaption object name
	*  @param [in]		tradeLVB		CapFloor Label Value Block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			Swaption object Handle
	*/
	std::string tryMeLWOSwaptionCreate( const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys=true );

    /* @brief			validation interface for the meLWOSwaptionDisplay method, which displays the INPUT parameters of the cached option
	*  @param [in]		objectName		Swaption object name
	*  @return			Display of the swaption input parameters
	*/
	AnyTypeMatrix tryMeLWOSwaptionDisplay( const std::string& objectName );

	/* @brief			validation interface for the meLWOSwaptionPV method, which calculates the PV of the cached swaption
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption PV
	*/
	double tryMeLWOSwaptionPV(const std::string& objectName, const LAStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the meLWOSwaptionImplivedVol method
	*  @param [in]		objectName			Swaption object name
    *  @param [in]		price			    Swaption price
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption implied vol
	*/
	double tryMeLWOSwaptionImpliedVol(const std::string& objectName, const double & price, const LAStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the meLWOSwaptionDelta
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Delta
	*/
	double tryMeLWOSwaptionDelta(const std::string& objectName, const LAStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the meLWOSwaptionGamma
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Gamma
	*/
	double tryMeLWOSwaptionGamma(const std::string& objectName, const LAStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the meLWOSwaptionVega
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Vega
	*/
	double tryMeLWOSwaptionVega(const std::string& objectName, const LAStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the meLWOSwaptionTheta
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Theta
	*/
	double tryMeLWOSwaptionTheta(const std::string& objectName, const LAStringMatrix& valuationSettingsLVB );
}

