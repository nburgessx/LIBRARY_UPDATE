#include "tryAqObjOptionsCapFloor.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "ObjectUtilities.h"
#include "AQOUtilities.h"
#include "RecordMacros.h"
#include "OptionFactory.h"
#include "RateProviderExternal.h"
#include "RateProvider.h"
#include <omp.h>

using etrading::CreateDataFile;
using etrading::decorateFilename;


namespace validation
{

	/* @brief			validation interface for the aqObjOptionsCapFloorCreate method
	*  @param [in]		objectName	    CapFloor object name
	*  @param [in]		tradeLVB			    CapFloor Label Value Block
	*  @param [in]		validateKeys	    True to validate the all keys provided are valid. Default to True
	*  @return			CapFloor Handle
	*/
	std::string tryAqObjOptionsCapFloorCreate(const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(objectName, tradeLVB, validateKeys);

		std::shared_ptr<etrading::Option> optionPtr = etrading::createCapFloor(objectName, tradeLVB, validateKeys);

		// Register the Option in the AQO object cache
		etrading::registerToCache< etrading::Option >(optionPtr);

		std::string result = objectName;

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqObjOptionsCapFloorDisplay method
	*  @param [in]		objectName		CapFloor object name
	*  @return			CapFloor Input Parameters
	*/
	AnyTypeMatrix tryAqObjOptionsCapFloorDisplay(const std::string& objectName)
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

	/* @brief			validation interface for the aqObjSwapsLegDisplayCashflows method
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		curveCollection Curve collection
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			columnList      Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			CapFloor cashflow display
	*/
	AnyTypeMatrix tryAqObjOptionsCapFloorDisplayCashflows(const std::string& objectName, const std::string& volObjectName, const std::string& curveCollection, const std::string& fixingTableName, const bool& showColumnHeaders, const std::vector<std::string>& columnList)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(objectName, volObjectName, curveCollection, fixingTableName, showColumnHeaders, columnList);

		const auto& option = etrading::getOption(objectName);

		const auto& fixingTable = (fixingTableName.size() != 0) ? etrading::getFixingTable(fixingTableName) : etrading::FixingTablePtr();

		etrading::RateProvider rateProvider(curveCollection, fixingTable);

		const auto& volObject = etrading::getVolatility(volObjectName);

		etrading::CapFloorPricer pricer(option, volObject, std::make_shared<etrading::RateProvider>(rateProvider));

		auto result = pricer.view(showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList));

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqObjOptionsCapFloorPV method
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		curveCollection Curve collection
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @return			CapFloor PV
	*/
	double tryAqObjOptionsCapFloorPV(const std::string& objectName, const std::string& volObjectName, const std::string& curveCollection, const std::string& fixingTableName)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(objectName, volObjectName, curveCollection, fixingTableName);

		const auto& option = etrading::getOption(objectName);

		const auto& fixingTable = (fixingTableName.size() != 0) ? etrading::getFixingTable(fixingTableName) : etrading::FixingTablePtr();

		etrading::RateProvider rateProvider(curveCollection, fixingTable);

		const auto& volObject = etrading::getVolatility(volObjectName);

		etrading::CapFloorPricer pricer(option, volObject, std::make_shared<etrading::RateProvider>(rateProvider));

		auto result = pricer.pv();

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqObjOptionsCapFloorGreeksAnalytical method, to calculate Analytical greeks
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		curveCollection Curve collection
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			CapFloor Greeks' display
	*/
	AnyTypeMatrix tryAqObjOptionsCapFloorGreeksAnalytical(const std::string& objectName, const std::string& volObjectName, const std::string& curveCollection, const std::string& fixingTableName, const bool& showColumnHeaders)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(objectName, volObjectName, curveCollection, fixingTableName, showColumnHeaders);

		const auto& option = etrading::getOption(objectName);

		const auto& fixingTable = (fixingTableName.size() != 0) ? etrading::getFixingTable(fixingTableName) : etrading::FixingTablePtr();

		etrading::RateProvider rateProvider(curveCollection, fixingTable);

		const auto& volObject = etrading::getVolatility(volObjectName);

		etrading::CapFloorPricer pricer(option, volObject, std::make_shared<etrading::RateProvider>(rateProvider));

		// Bump will not be used for analytical greek
		const auto result = pricer.viewGreeks(etrading::ANALYTICAL_GREEK_TYPE, 0.0, 0.0, 0.0, 0.0, showColumnHeaders); //bumpSize = 0.0

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqObjOptionsCapFloorGreeks method, to calculate Numerical greeks
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		curveCollection Curve collection
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @param [in]		deltaBump		Delta bump size
	*  @param [in]		gammaBump		Gamma bump size
	*  @param [in]		vegaBump		Vega bump size
	*  @param [in]		thetaBump		Theta bump size
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			CapFloor Greeks' display
	*/
	AnyTypeMatrix tryAqObjOptionsCapFloorGreeks(const std::string& objectName, const std::string& volObjectName, const std::string& curveCollection, 
												const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump, 
												const std::string& fixingTableName, const bool& showColumnHeaders)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(objectName, volObjectName, curveCollection, deltaBump, gammaBump, vegaBump, thetaBump, fixingTableName, showColumnHeaders);

		const auto& option = etrading::getOption(objectName);

		const auto& fixingTable = (fixingTableName.size() != 0) ? etrading::getFixingTable(fixingTableName) : etrading::FixingTablePtr();

		etrading::RateProvider rateProvider(curveCollection, fixingTable);

		const auto& volObject = etrading::getVolatility(volObjectName);

		etrading::CapFloorPricer pricer(option, volObject, std::make_shared<etrading::RateProvider>(rateProvider));

		const auto result = pricer.viewGreeks(etrading::NUMERICAL_GREEK_TYPE, deltaBump, gammaBump, vegaBump, thetaBump, showColumnHeaders);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	//----- test methods to use external libor rates/discount factors like BB so that we know we are matching their prices-- //

	/* @brief			validation interface for the aqObjSwapsLegDisplayCashflows method
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		rateData		External discountFactors and liborRates
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			columnList      Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			CapFloor cashflow display
	*/
	AnyTypeMatrix tryAqObjOptionsCapFloorDisplayCashflowsFromRates(const std::string& objectName, const std::string& volObjectName, const DoubleMatrix& rateData, const bool& showColumnHeaders, const std::vector<std::string>& columnList)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(objectName, volObjectName, rateData, showColumnHeaders, columnList);

		const auto& option = etrading::getOption(objectName);
		const auto& volObject = etrading::getVolatility(volObjectName);

		AQ_REQUIRE(rateData.size() >= 2, "Invalid dimensions: rateBlock must have at least two columns - discountFactors and liborRates.");

		const auto& dfs = rateData[0];
		const auto& liborRates = rateData[1];

		etrading::RateProviderExternal rateProvider(volObject->asOfDate(), dfs, liborRates);

		etrading::CapFloorPricer pricer(option, volObject, std::make_shared<etrading::RateProviderExternal>(rateProvider));

		auto result = pricer.view(showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList));

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}


	/* @brief			validation interface for the aqObjOptionsCapFloorGreeksAnalyticalFromRates method
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		rateData		External discountFactors and liborRates
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			CapFloor Greeks' display
	*/
	AnyTypeMatrix tryAqObjOptionsCapFloorGreeksAnalyticalFromRates(const std::string& objectName, const std::string& volObjectName, const DoubleMatrix& rateData, const bool& showColumnHeaders)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(objectName, volObjectName, rateData, showColumnHeaders);

		const auto& option = etrading::getOption(objectName);
		const auto& volObject = etrading::getVolatility(volObjectName);

		AQ_REQUIRE(rateData.size() >= 2, "Invalid dimensions: rateBlock must have at least two columns - discountFactors and liborRates.");

		const auto& dfs = rateData[0];
		const auto& liborRates = rateData[1];

		etrading::RateProviderExternal rateProvider(volObject->asOfDate(), dfs, liborRates);

		etrading::CapFloorPricer pricer(option, volObject, std::make_shared<etrading::RateProviderExternal>(rateProvider));

		// Bump will not be used for analytical greek
		const auto result = pricer.viewGreeks(etrading::ANALYTICAL_GREEK_TYPE, 0.0, 0.0, 0.0, 0.0, showColumnHeaders); //bumpSize = 0.0

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqObjOptionsCapFloorGreeksFromRates method
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		rateData		External discountFactors and liborRates
	*  @param [in]		deltaBump		Delta bump size
	*  @param [in]		gammaBump		Gamma bump size
	*  @param [in]		vegaBump		Vega bump size
	*  @param [in]		thetaBump		Theta bump size
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			CapFloor Greeks' display
	*/
	AnyTypeMatrix tryAqObjOptionsCapFloorGreeksFromRates(const std::string& objectName, const std::string& volObjectName, const DoubleMatrix& rateData,
														const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump, 
														const bool& showColumnHeaders)
	{
		VALID_EXCEPTION_START

			// Record Inputs for logs, tests and playback
			RECORD_INPUTS(objectName, volObjectName, rateData, deltaBump, gammaBump, vegaBump, thetaBump, showColumnHeaders);

		const auto& option = etrading::getOption(objectName);
		const auto& volObject = etrading::getVolatility(volObjectName);

		AQ_REQUIRE(rateData.size() >= 2, "Invalid dimensions: rateBlock must have at least two columns - discountFactors and liborRates.");

		const auto& dfs = rateData[0];
		const auto& liborRates = rateData[1];

		etrading::RateProviderExternal rateProvider(volObject->asOfDate(), dfs, liborRates);

		etrading::CapFloorPricer pricer(option, volObject, std::make_shared<etrading::RateProviderExternal>(rateProvider));

		// Bump will not be used for analytical greek
		const auto result = pricer.viewGreeks(etrading::NUMERICAL_GREEK_TYPE, deltaBump, gammaBump, vegaBump, thetaBump, showColumnHeaders);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqObjOptionsCapFloorPVFromRate method
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		rateData		External discountFactors and liborRates
	*  @return			CapFloor PV
	*/
	double tryAqObjOptionsCapFloorPVFromRates(const std::string& objectName, const std::string& volObjectName, const DoubleMatrix& rateData)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(objectName, volObjectName, rateData);

		const auto& option = etrading::getOption(objectName);
		const auto& volObject = etrading::getVolatility(volObjectName);

		AQ_REQUIRE(rateData.size() >= 2, "Invalid dimensions: rateBlock must have at least two columns - discountFactors and liborRates.");

		const auto& dfs = rateData[0];
		const auto& liborRates = rateData[1];

		etrading::RateProviderExternal rateProvider(volObject->asOfDate(), dfs, liborRates);

		etrading::CapFloorPricer pricer(option, volObject, std::make_shared<etrading::RateProviderExternal>(rateProvider));

		auto result = pricer.pv();

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}



}
