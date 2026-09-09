#include "tryAqSwaptionObject.h"
#include "SwaptionPricer.h"
#include "SwaptionTrade.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "ObjectUtilities.h"
#include "AQObjUtilities.h"
#include "RecordMacros.h"
#include "OptionFactory.h"
#include "RateProviderExternal.h"
#include "RateProvider.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;


namespace validation
{

	/* @brief			validation interface for the aqSwaptionObjectCreate method
	*  @param [in]		objectName		Swaption object name
	*  @param [in]		tradeLVB		CapFloor Label Value Block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			Swaption object Handle
	*/
	std::string tryAqSwaptionObjectCreate(const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(objectName, tradeLVB, validateKeys);

		std::shared_ptr<etrading::Option> optionPtr = etrading::createOption(objectName, tradeLVB, validateKeys);

		// Register the Option in the AQObj object cache
		etrading::registerToCache< etrading::Option >(optionPtr);

		std::string result = objectName;

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the aqSwaptionObjectDisplay method, which displays the INPUT parameters of the cached option
	*  @param [in]		objectName		Swaption object name
	*  @return			Display of the swaption input parameters
	*/
	AnyTypeMatrix tryAqSwaptionObjectDisplay(const std::string& objectName)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(objectName);

		auto option = etrading::getOption(objectName);
		auto result = option->inputParameters().toAnyTypeMatrix();

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqSwaptionObjectPV method, which calculates the PV of the cached swaption
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption PV
	*/
	double tryAqSwaptionObjectPV(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( objectName, valuationSettingsLVB );

		const auto& option = etrading::getOption(objectName);
		const auto& swaption = std::dynamic_pointer_cast<etrading::SwaptionTrade>( option );
		AQ_REQUIRE( swaption != nullptr, "Swaption " + objectName + " does not exist" );

		etrading::SwaptionPricer pricer( swaption, valuationSettingsLVB );
        auto result = pricer.pv();

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the aqSwaptionObjectImpliedVol method
	*  @param [in]		objectName			Swaption object name
    *  @param [in]		price			    Swaption price
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption implied vol
	*/
	double tryAqSwaptionObjectImpliedVol(const std::string& objectName, const double & price, const AQLStringMatrix& valuationSettingsLVB )
    {
        VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( objectName, valuationSettingsLVB );

		const auto& option = etrading::getOption(objectName);
		const auto& swaption = std::dynamic_pointer_cast<etrading::SwaptionTrade>( option );
		AQ_REQUIRE( swaption != nullptr, "Swaption " + objectName + " does not exist" );

		etrading::SwaptionPricer pricer( swaption, valuationSettingsLVB );
        auto result = pricer.impliedVol( price );

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqSwaptionObjectDelta
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Delta
	*/
	double tryAqSwaptionObjectDelta(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB )
    {
        VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( objectName, valuationSettingsLVB );

		const auto& option = etrading::getOption(objectName);
		const auto& swaption = std::dynamic_pointer_cast<etrading::SwaptionTrade>( option );
		AQ_REQUIRE( swaption != nullptr, "Swaption " + objectName + " does not exist" );

		etrading::SwaptionPricer pricer( swaption, valuationSettingsLVB );
        auto result = pricer.delta();

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqSwaptionObjectGamma
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Gamma
	*/
	double tryAqSwaptionObjectGamma(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB )
    {
        VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( objectName, valuationSettingsLVB );

		const auto& option = etrading::getOption(objectName);
		const auto& swaption = std::dynamic_pointer_cast<etrading::SwaptionTrade>( option );
		AQ_REQUIRE( swaption != nullptr, "Swaption " + objectName + " does not exist" );

		etrading::SwaptionPricer pricer( swaption, valuationSettingsLVB );
        auto result = pricer.gamma();

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqSwaptionObjectVega
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Vega
	*/
	double tryAqSwaptionObjectVega(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB )
    {
        VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( objectName, valuationSettingsLVB );

		const auto& option = etrading::getOption(objectName);
		const auto& swaption = std::dynamic_pointer_cast<etrading::SwaptionTrade>( option );
		AQ_REQUIRE( swaption != nullptr, "Swaption " + objectName + " does not exist" );

		etrading::SwaptionPricer pricer( swaption, valuationSettingsLVB );
        auto result = pricer.vega();

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqSwaptionObjectTheta
	*  @param [in]		objectName			Swaption object name
	*  @param [in]		valuationSettingsLVB	A LabelValueBlock containing valuation settings such as curveCollection, volatilityModel ...
	*  @return			The swaption Theta
	*/
	double tryAqSwaptionObjectTheta(const std::string& objectName, const AQLStringMatrix& valuationSettingsLVB )
    {
        VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( objectName, valuationSettingsLVB );

		const auto& option = etrading::getOption(objectName);
		const auto& swaption = std::dynamic_pointer_cast<etrading::SwaptionTrade>( option );
		AQ_REQUIRE( swaption != nullptr, "Swaption " + objectName + " does not exist" );

		etrading::SwaptionPricer pricer( swaption, valuationSettingsLVB );
        auto result = pricer.theta();

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
    }


}
