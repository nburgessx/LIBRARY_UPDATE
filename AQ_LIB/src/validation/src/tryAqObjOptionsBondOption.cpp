#include "tryAqObjOptionsBondOption.h"
#include "BondOptionPricer.h"
#include "BondOption.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "ObjectUtilities.h"
#include "AQObjUtilities.h"
#include "RecordMacros.h"
#include "OptionFactory.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;


namespace validation
{

	/* @brief			validation interface for the aqObjOptionsBondOptionCreate method
	*  @param [in]		objectName					BondOption object name
	*  @param [in]		optionLVB					Option Label Value Block
	*  @param [in]		validateKeys				True to validate the all keys provided are valid. Default to True
	*  @return			BondOption object Handle
	*/
	std::string tryAqObjOptionsBondOptionCreate(const std::string& objectName, const LabelValueBlock& optionLVB, const bool& validateKeys)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(objectName, optionLVB, validateKeys);

		std::shared_ptr<etrading::Option> optionPtr = etrading::createOption(objectName, optionLVB, validateKeys);

		// Register the Option in the AQObj object cache
		etrading::registerToCache< etrading::Option >(optionPtr);

		std::string result = objectName;

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the aqObjOptionsBondOptionDisplay method, which displays the INPUT parameters of the cached option
	*  @param [in]		objectName		BondOption object name
	*  @return			Display of the swaption input parameters
	*/
	AnyTypeMatrix tryAqObjOptionsBondOptionDisplay(const std::string& objectName)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(objectName);

		auto option = etrading::getOption(objectName);
		auto result = option->inputParameters().toAnyTypeMatrix();

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqObjOptionsBondOptionPV method, which calculates the PV of the cached bond option
	*  @param [in]		objectName			BondOption object name
	*  @param [in]		valuationSettings	A LabelValueBlock containing valuation settings such as Bond's valuation date, discountCurve's curveCollection
	*  @param [in]		bondPrice			Bond Spot price at settlement date
	*  @param [in]		repoRate			Bond's repo rate
	*  @param [in]		repoDaycount		Bond's repo daycount
	*  @param [in]		discountRate		Continuously compounded risk free rate
	*  @param [in]		discountDayCount	Day count for the discounting
	*  @return			The Bond Option PV
	*/
	double tryAqObjOptionsBondOptionPV(const std::string& objectName, const AQLStringMatrix& valuationSettings, const double& bondPrice, const double& repoRate, const std::string& repoDaycount, const double& discountRate, const std::string& discountDayCount)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( objectName, valuationSettings, bondPrice, repoRate, repoDaycount, discountRate, discountDayCount);

		const auto& option = etrading::getOption(objectName);
		const auto& bondOption = std::dynamic_pointer_cast<etrading::BondOption>( option );

		AQ_REQUIRE(bondOption != nullptr, "Could not find specified Bond Option: " + objectName );

		etrading::BondOptionPricer pricer(bondOption, valuationSettings, discountRate, etrading::toDayCountEnum(discountDayCount));

		auto result = pricer.spotOptionPrice(bondPrice, repoRate, etrading::toDayCountEnum(repoDaycount));

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqObjOptionsBondFutureOptionPV method, which calculates the PV of the cached bond future option
	*  @param [in]		objectName			BondOption object name
	*  @param [in]		valuationSettings	A LabelValueBlock containing valuation settings such as Bond's valuation date, discountCurve's curveCollection
	*  @param [in]		bondFuturePrice		Bond forward price at option expiry date
	*  @param [in]		discountRate		Continuously compounded risk free rate
	*  @param [in]		discountDayCount	Day count for the discounting
	*  @return			The Bond Option PV
	*/
	double tryAqObjOptionsBondFutureOptionPV(const std::string& objectName, const AQLStringMatrix& valuationSettings, const double& bondFuturePrice, const double& discountRate, const std::string& discountDayCount)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(objectName, valuationSettings, bondFuturePrice, discountRate, discountDayCount);

		const auto& option = etrading::getOption(objectName);
		const auto& bondOption = std::dynamic_pointer_cast<etrading::BondOption>(option);

		AQ_REQUIRE(bondOption != nullptr, "Could not find specified Bond Option: " + objectName);

		etrading::BondOptionPricer pricer(bondOption, valuationSettings, discountRate, etrading::toDayCountEnum(discountDayCount));

		auto result = pricer.forwardOptionPrice(bondFuturePrice);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqObjOptionsBondFutureOptionGreeks method, to calculate Numerical greeks
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
	AnyTypeMatrix tryAqObjOptionsBondFutureOptionGreeks(const std::string& greekType, const std::string& objectName, const AQLStringMatrix& valuationSettings, const double& bondFuturePrice, const double& discountRate, const std::string& discountDayCount, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump, const double & rhoBump, const bool& showColumnHeaders)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(greekType, objectName, valuationSettings, bondFuturePrice, discountRate, discountDayCount, deltaBump, gammaBump, vegaBump, thetaBump, rhoBump, showColumnHeaders);

		const auto& option = etrading::getOption(objectName);
		const auto& bondOption = std::dynamic_pointer_cast<etrading::BondOption>(option);
		AQ_REQUIRE(bondOption != nullptr, "Could not find specified Bond Option: " + objectName);

		etrading::BondOptionPricer pricer(bondOption, valuationSettings, discountRate, etrading::toDayCountEnum(discountDayCount));

		etrading::NumericalGreekBump greekBump;
		greekBump.deltaBump = deltaBump;
		greekBump.gammaBump = gammaBump;
		greekBump.vegaBump = vegaBump;
		greekBump.thetaBump = thetaBump;
		greekBump.rhoBump = rhoBump;

		const auto greeks = pricer.forwardOptionGreeks(etrading::toGreekTypeEnum(greekType), bondFuturePrice, greekBump);

		const auto result = etrading::viewGreeks(greeks, showColumnHeaders);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqObjOptionsBondOptionGreeks method, to calculate Numerical greeks
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
	AnyTypeMatrix tryAqObjOptionsBondOptionGreeks(const std::string& greekType, const std::string& objectName, const AQLStringMatrix& valuationSettings, const double& bondSpotPrice, const double& repoRate, const std::string& repoDaycount, const double& discountRate, const std::string& discountDayCount, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump, const double & rhoBump, const bool& showColumnHeaders)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(greekType, objectName, valuationSettings, bondSpotPrice, repoRate, repoDaycount, discountRate, discountDayCount, deltaBump, gammaBump, vegaBump, thetaBump, rhoBump, showColumnHeaders);

		const auto& option = etrading::getOption(objectName);
		const auto& bondOption = std::dynamic_pointer_cast<etrading::BondOption>(option);
		AQ_REQUIRE(bondOption != nullptr, "Could not find specified Bond Option: " + objectName);

		etrading::BondOptionPricer pricer(bondOption, valuationSettings, discountRate, etrading::toDayCountEnum(discountDayCount));

		etrading::NumericalGreekBump greekBump;
		greekBump.deltaBump = deltaBump;
		greekBump.gammaBump = gammaBump;
		greekBump.vegaBump = vegaBump;
		greekBump.thetaBump = thetaBump;
		greekBump.rhoBump = rhoBump;

		const auto greeks = pricer.spotOptionGreeks(etrading::toGreekTypeEnum(greekType), bondSpotPrice, repoRate, etrading::toDayCountEnum(repoDaycount), greekBump);

		const auto result = etrading::viewGreeks(greeks, showColumnHeaders);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}


}
