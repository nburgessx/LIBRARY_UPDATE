#include "tryAqObjRatesFra.h"
#include "tryAqObjSwapsLeg.h"

#include "Fra.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"
#include "AQObjUtilities.h"
#include "SettingsValidation.h"

#include <memory>

using etrading::CreateDataFile;

namespace validation
{
 
	/* @brief			validation interface for the aqObjRatesFraCreate method
	*  @param [in]		fraObjectName	Fra object name
	*  @param [in]		fraLVB			Fra label value block with Fras
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			fraObjectName
	*/
	std::string tryAqObjRatesFraCreate(const std::string& fraObjectName, const LabelValueBlock& fraLVB, bool validateKeys)
	{
		return tryAqObjSwapsLegCreate(fraObjectName, fraLVB, validateKeys);
	}
    
    /* @brief			validation interface for the aqObjRatesFraPV method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @return			Fra PV
	*/
	double tryAqObjRatesFraPV(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB)
	{
		return tryAqObjSwapsLegPV(fraObjectName, valuationSettingsLVB);
	}

    /* @brief			validation interface for the aqObjRatesFraDisplay method, which display the INPUT parameters of the cached swap
	*  @param [in]		fraObjectName		Fra object name
	*  @return			Fra display of the input parameters
	*/
	AQLStringMatrix tryAqObjRatesFraDisplay(const std::string& fraObjectName)
	{
		return tryAqObjSwapsLegDisplay(fraObjectName);
	}

	/* @brief			validation interface for the aqObjRatesFraDisplayCashflows method 
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			columnList      Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Fra display of Fra output
	*/
	AnyTypeMatrix tryAqObjRatesFraDisplayCashflows(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, bool showColumnHeaders, const std::vector<std::string>& columnList)
	{
		return tryAqObjSwapsLegDisplayCashflows(fraObjectName, valuationSettingsLVB, "", showColumnHeaders, columnList);
	}

	/* @brief			validation interface for the aqObjRatesFraRate method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @return			Fra rate (strikeRate that make the Fra PV Zero)
	*/
	double tryAqObjRatesFraRate(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(fraObjectName, valuationSettingsLVB);

		auto leg = etrading::getLeg(fraObjectName);

		auto fraObject = std::dynamic_pointer_cast<etrading::Fra>(leg);
		if (fraObject == nullptr)
		{
			throw AQLCoreInvalidData("#Error: The leg is not a Fra type", __FILE__, __LINE__);
		}

		auto result = fraObject->fraRate(valuationSettingsLVB);

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END

	}

	/* @brief			validation interface for the aqObjRatesFraToFuturePrice method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		meanReversion       The Hull-White 1F Mean Reversion Parameter
	*  @param [in]		volatility          The Hull-White 1F Volatility Parameter
	*  @return			Fra rate (strikeRate that make the Fra PV Zero)
	*/
	double tryAqObjRatesFraToFuturePrice(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, const double& meanReversion, const double& volatility)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(fraObjectName, valuationSettingsLVB, meanReversion, volatility);

		auto leg = etrading::getLeg(fraObjectName);

		auto fraObject = std::dynamic_pointer_cast<etrading::Fra>(leg);
		if (fraObject == nullptr)
		{
			throw AQLCoreInvalidData("#Error: The leg is not a Fra type", __FILE__, __LINE__);
		}

		double result = fraObject->fraRateToFuturePrice(valuationSettingsLVB, meanReversion, volatility);

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	

	/* @brief			validation interface for the aqObjRatesFraToFuturePriceFromConvAdj method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		convexityAdjustment	The convexity adjustment between FraRate and Future rate
	*  @return			Future price
	*/
	double tryAqObjRatesFraToFuturePriceFromConvAdj(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, const double& convexityAdjustment)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(fraObjectName, valuationSettingsLVB, convexityAdjustment);

		auto leg = etrading::getLeg(fraObjectName);

		auto fraObject = std::dynamic_pointer_cast<etrading::Fra>(leg);
		if (fraObject == nullptr)
		{
			throw AQLCoreInvalidData("#Error: The leg is not a Fra type", __FILE__, __LINE__);
		}

		double result = fraObject->fraRateToFuturePrice(valuationSettingsLVB, convexityAdjustment);

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END

	}


}

