/*
 * @brief			validation interface for the Swaption analytics 
 * @Created:		18th July 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "tryMeLWOSwaption.h"
#include "SwaptionPricer.h"
#include "SwaptionTrade.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "ObjectUtilities.h"
#include "LWOUtilities.h"
#include "RecordMacros.h"
#include "OptionFactory.h"
#include "RateProviderExternal.h"
#include "RateProvider.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;


namespace validation_api
{

	/* @brief			validation interface for the meLWOSwaptionCreate method
	*  @param [in]		objectName		Swaption object name
	*  @param [in]		tradeLVB		CapFloor Label Value Block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			Swaption object Handle
	*/
	std::string tryMeLWOSwaptionCreate(const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(objectName, tradeLVB, validateKeys);

		std::shared_ptr<etrading::Option> optionPtr = etrading::createOption(objectName, tradeLVB, validateKeys);

		// Register the Option in the LWO Cache
		etrading::registerToCache< etrading::Option >(optionPtr);

		std::string result = objectName;

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the meLWOSwaptionDisplay method, which displays the INPUT parameters of the cached option
	*  @param [in]		objectName		Swaption object name
	*  @return			Display of the swaption input parameters
	*/
	AnyTypeMatrix tryMeLWOSwaptionDisplay(const std::string& objectName)
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

	/* @brief			validation interface for the meLWOSwaptionPV method, which calculates the PV of the cached swaption
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption PV
	*/
	double tryMeLWOSwaptionPV(const std::string& objectName, const LAStringMatrix& valuationSettingsLVB )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( objectName, valuationSettingsLVB );

		const auto& option = etrading::getOption(objectName);
		const auto& swaption = std::dynamic_pointer_cast<etrading::SwaptionTrade>( option );
		MLIB_REQUIRE( swaption != nullptr, "Swaption " + objectName + " does not exist" );

		etrading::SwaptionPricer pricer( swaption, valuationSettingsLVB );
        auto result = pricer.pv();

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the meLWOSwaptionImplivedVol method
	*  @param [in]		objectName			Swaption object name
    *  @param [in]		price			    Swaption price
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption implied vol
	*/
	double tryMeLWOSwaptionImpliedVol(const std::string& objectName, const double & price, const LAStringMatrix& valuationSettingsLVB )
    {
        VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( objectName, valuationSettingsLVB );

		const auto& option = etrading::getOption(objectName);
		const auto& swaption = std::dynamic_pointer_cast<etrading::SwaptionTrade>( option );
		MLIB_REQUIRE( swaption != nullptr, "Swaption " + objectName + " does not exist" );

		etrading::SwaptionPricer pricer( swaption, valuationSettingsLVB );
        auto result = pricer.impliedVol( price );

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meLWOSwaptionDelta
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Delta
	*/
	double tryMeLWOSwaptionDelta(const std::string& objectName, const LAStringMatrix& valuationSettingsLVB )
    {
        VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( objectName, valuationSettingsLVB );

		const auto& option = etrading::getOption(objectName);
		const auto& swaption = std::dynamic_pointer_cast<etrading::SwaptionTrade>( option );
		MLIB_REQUIRE( swaption != nullptr, "Swaption " + objectName + " does not exist" );

		etrading::SwaptionPricer pricer( swaption, valuationSettingsLVB );
        auto result = pricer.delta();

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meLWOSwaptionGamma
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Gamma
	*/
	double tryMeLWOSwaptionGamma(const std::string& objectName, const LAStringMatrix& valuationSettingsLVB )
    {
        VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( objectName, valuationSettingsLVB );

		const auto& option = etrading::getOption(objectName);
		const auto& swaption = std::dynamic_pointer_cast<etrading::SwaptionTrade>( option );
		MLIB_REQUIRE( swaption != nullptr, "Swaption " + objectName + " does not exist" );

		etrading::SwaptionPricer pricer( swaption, valuationSettingsLVB );
        auto result = pricer.gamma();

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meLWOSwaptionVega
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Vega
	*/
	double tryMeLWOSwaptionVega(const std::string& objectName, const LAStringMatrix& valuationSettingsLVB )
    {
        VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( objectName, valuationSettingsLVB );

		const auto& option = etrading::getOption(objectName);
		const auto& swaption = std::dynamic_pointer_cast<etrading::SwaptionTrade>( option );
		MLIB_REQUIRE( swaption != nullptr, "Swaption " + objectName + " does not exist" );

		etrading::SwaptionPricer pricer( swaption, valuationSettingsLVB );
        auto result = pricer.vega();

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meLWOSwaptionTheta
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Theta
	*/
	double tryMeLWOSwaptionTheta(const std::string& objectName, const LAStringMatrix& valuationSettingsLVB )
    {
        VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( objectName, valuationSettingsLVB );

		const auto& option = etrading::getOption(objectName);
		const auto& swaption = std::dynamic_pointer_cast<etrading::SwaptionTrade>( option );
		MLIB_REQUIRE( swaption != nullptr, "Swaption " + objectName + " does not exist" );

		etrading::SwaptionPricer pricer( swaption, valuationSettingsLVB );
        auto result = pricer.theta();

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
    }


}
