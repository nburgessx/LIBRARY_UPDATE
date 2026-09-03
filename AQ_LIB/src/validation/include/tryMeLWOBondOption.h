#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock; 

namespace validation
{

	/* @brief			validation interface for the meLWOBondOptionCreate method
	*  @param [in]		objectName					BondOption object name
	*  @param [in]		optionLVB					Option Label Value Block
	*  @param [in]		validateKeys				True to validate the all keys provided are valid. Default to True
	*  @return			BondOption object Handle
	*/
	std::string tryMeLWOBondOptionCreate(const std::string& objectName, const LabelValueBlock& optionLVB, const bool& validateKeys = true);

    /* @brief			validation interface for the meLWOBondOptionDisplay method, which displays the INPUT parameters of the cached option
	*  @param [in]		objectName		BondOption object name
	*  @return			Display of the swaption input parameters
	*/
	AnyTypeMatrix tryMeLWOBondOptionDisplay( const std::string& objectName );

	/* @brief			validation interface for the meLWOBondOptionPV method, which calculates the PV of the cached bond option
	*  @param [in]		objectName			BondOption object name
	*  @param [in]		valuationSettings	A LabelValueBlock containing valuation settings such as Bond's valuation date, discountCurve's curveCollection
	*  @param [in]		bondPrice			Bond Spot price at settlement date
	*  @param [in]		repoRate			Bond's repo rate
	*  @param [in]		repoDaycount		Bond's repo daycount
	*  @param [in]		discountRate		Continuously compounded risk free rate
	*  @param [in]		discountDayCount	Day count for the discounting
	*  @return			The Bond Option PV
	*/
	double tryMeLWOBondOptionPV(const std::string& objectName, const AQLStringMatrix& valuationSettings, const double& bondPrice, const double& repoRate, const std::string& repoDaycount, const double& discountRate, const std::string& discountDayCount);


	/* @brief			validation interface for the meLWOBondFutureOptionPV method, which calculates the PV of the cached bond future option
	*  @param [in]		objectName			BondOption object name
	*  @param [in]		valuationSettings	A LabelValueBlock containing valuation settings such as Bond's valuation date, discountCurve's curveCollection
	*  @param [in]		bondFuturePrice		Bond forward price at option expiry date
	*  @param [in]		discountRate		Continuously compounded risk free rate
	*  @param [in]		discountDayCount	Day count for the discounting
	*  @return			The Bond Option PV
	*/
	double tryMeLWOBondFutureOptionPV(const std::string& objectName, const AQLStringMatrix& valuationSettings, const double& bondFuturePrice, const double& discountRate, const std::string& discountDayCount);

	/* @brief			validation interface for the meLWOBondFutureOptionGreeks method, to calculate Numerical greeks
	*  @param [in]		greekType			Greek type: Analytical or Numerical
	*  @param [in]		objectName			BondOption object name
	*  @param [in]		valuationSettings	A LabelValueBlock containing valuation settings such as Bond's valuation date, discountCurve's curveCollection
	*  @param [in]		bondFuturePrice		Bond forward price at option expiry date
	*  @param [in]		discountRate		Continuously compounded risk free rate
	*  @param [in]		discountDayCount	Day count for the discounting
	*  @param [in]		deltaBump			Delta bump size
	*  @param [in]		gammaBump			Gamma bump size
	*  @param [in]		vegaBump			Vega bump size
	*  @param [in]		thetaBump			Theta bump size
	*  @param [in]		rhoBump				Rho bump size
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			Bond option Greeks' display
	*/
	AnyTypeMatrix tryMeLWOBondFutureOptionGreeks(const std::string& greekType, const std::string& objectName, const AQLStringMatrix& valuationSettings, const double& bondFuturePrice, const double& discountRate, const std::string& discountDayCount, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump, const double & rhoBump, const bool& showColumnHeaders = true);

	/* @brief			validation interface for the meLWOBondOptionGreeks method, to calculate Numerical greeks
	*  @param [in]		greekType			Greek type: Analytical or Numerical
	*  @param [in]		objectName			BondOption object name
	*  @param [in]		valuationSettings	A LabelValueBlock containing valuation settings such as Bond's valuation date, discountCurve's curveCollection
	*  @param [in]		bondFuturePrice		Bond forward price at option expiry date
	*  @param [in]		repoRate			Bond's repo rate
	*  @param [in]		repoDaycount		Bond's repo daycount
	*  @param [in]		discountRate		Continuously compounded risk free rate
	*  @param [in]		discountDayCount	Day count for the discounting
	*  @param [in]		deltaBump			Delta bump size
	*  @param [in]		gammaBump			Gamma bump size
	*  @param [in]		vegaBump			Vega bump size
	*  @param [in]		thetaBump			Theta bump size
	*  @param [in]		rhoBump				Rho bump size
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			Bond option Greeks' display
	*/
	AnyTypeMatrix tryMeLWOBondOptionGreeks(const std::string& greekType, const std::string& objectName, const AQLStringMatrix& valuationSettings, const double& bondSpotPrice, const double& repoRate, const std::string& repoDaycount, const double& discountRate, const std::string& discountDayCount, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump, const double & rhoBump, const bool& showColumnHeaders = true);

}

