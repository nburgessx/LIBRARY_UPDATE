#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"
#include "RateProviderExternal.h"


using etrading::LabelValueBlock; 

namespace validation
{

	/* @brief			validation interface for the aqObjOptionsSwaptionCreate method
	*  @param [in]		objectName		Swaption object name
	*  @param [in]		tradeLVB		CapFloor Label Value Block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			Swaption object Handle
	*/
	std::string tryAqObjOptionsSwaptionCreate( const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys=true );

    /* @brief			validation interface for the aqObjOptionsSwaptionDisplay method, which displays the INPUT parameters of the cached option
	*  @param [in]		objectName		Swaption object name
	*  @return			Display of the swaption input parameters
	*/
	AnyTypeMatrix tryAqObjOptionsSwaptionDisplay( const std::string& objectName );

	/* @brief			validation interface for the aqObjOptionsSwaptionPV method, which calculates the PV of the cached swaption
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption PV
	*/
	double tryAqObjOptionsSwaptionPV(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqObjOptionsSwaptionImplivedVol method
	*  @param [in]		objectName			Swaption object name
    *  @param [in]		price			    Swaption price
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption implied vol
	*/
	double tryAqObjOptionsSwaptionImpliedVol(const std::string& objectName, const double & price, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqObjOptionsSwaptionDelta
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Delta
	*/
	double tryAqObjOptionsSwaptionDelta(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqObjOptionsSwaptionGamma
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Gamma
	*/
	double tryAqObjOptionsSwaptionGamma(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqObjOptionsSwaptionVega
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Vega
	*/
	double tryAqObjOptionsSwaptionVega(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqObjOptionsSwaptionTheta
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Theta
	*/
	double tryAqObjOptionsSwaptionTheta(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );
}

