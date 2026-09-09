#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"
#include "RateProviderExternal.h"


using etrading::LabelValueBlock; 

namespace validation
{

	/* @brief			validation interface for the aqSwaptionObjectCreate method
	*  @param [in]		objectName		Swaption object name
	*  @param [in]		tradeLVB		CapFloor Label Value Block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			Swaption object Handle
	*/
	std::string tryAqSwaptionObjectCreate( const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys=true );

    /* @brief			validation interface for the aqSwaptionObjectDisplay method, which displays the INPUT parameters of the cached option
	*  @param [in]		objectName		Swaption object name
	*  @return			Display of the swaption input parameters
	*/
	AnyTypeMatrix tryAqSwaptionObjectDisplay( const std::string& objectName );

	/* @brief			validation interface for the aqSwaptionObjectPV method, which calculates the PV of the cached swaption
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption PV
	*/
	double tryAqSwaptionObjectPV(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqSwaptionObjectImpliedVol method
	*  @param [in]		objectName			Swaption object name
    *  @param [in]		price			    Swaption price
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption implied vol
	*/
	double tryAqSwaptionObjectImpliedVol(const std::string& objectName, const double & price, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqSwaptionObjectDelta
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Delta
	*/
	double tryAqSwaptionObjectDelta(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqSwaptionObjectGamma
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Gamma
	*/
	double tryAqSwaptionObjectGamma(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqSwaptionObjectVega
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Vega
	*/
	double tryAqSwaptionObjectVega(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );

    /* @brief			validation interface for the aqSwaptionObjectTheta
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Theta
	*/
	double tryAqSwaptionObjectTheta(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB );
}

