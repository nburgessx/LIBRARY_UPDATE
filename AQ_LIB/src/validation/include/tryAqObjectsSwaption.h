#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"
#include "RateProviderExternal.h"


using etrading::LabelValueBlock; 

namespace validation
{

	/* @brief			validation interface for the aqObjectsSwaptionCreate method
	*  @param [in]		objectName		Swaption object name
	*  @param [in]		tradeLVB		CapFloor Label Value Block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			Swaption object Handle
	*/
	std::string tryAqObjectsSwaptionCreate( const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys=true );

    /* @brief			validation interface for the aqObjectsSwaptionDisplay method, which displays the INPUT parameters of the cached option
	*  @param [in]		objectName		Swaption object name
	*  @return			Display of the swaption input parameters
	*/
	AnyTypeMatrix tryAqObjectsSwaptionDisplay( const std::string& objectName );

	/* @brief			validation interface for the aqObjectsSwaptionPV method, which calculates the PV of the cached swaption
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption PV
	*/
	double tryAqObjectsSwaptionPV(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqObjectsSwaptionImplivedVol method
	*  @param [in]		objectName			Swaption object name
    *  @param [in]		price			    Swaption price
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption implied vol
	*/
	double tryAqObjectsSwaptionImpliedVol(const std::string& objectName, const double & price, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqObjectsSwaptionDelta
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Delta
	*/
	double tryAqObjectsSwaptionDelta(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqObjectsSwaptionGamma
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Gamma
	*/
	double tryAqObjectsSwaptionGamma(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqObjectsSwaptionVega
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Vega
	*/
	double tryAqObjectsSwaptionVega(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqObjectsSwaptionTheta
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Theta
	*/
	double tryAqObjectsSwaptionTheta(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );
}

