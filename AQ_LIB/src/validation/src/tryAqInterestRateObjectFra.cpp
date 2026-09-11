#include "tryAqInterestRateObjectFra.h"
#include "tryAqSwapObjectLeg.h"

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
 
	/* @brief			validation interface for the aqInterestRateObjectFraCreate method
	*  @param [in]		fraObjectName	Fra object name
	*  @param [in]		fraLVB			Fra label value block with Fras
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			fraObjectName
	*/
	std::string tryAqInterestRateObjectFraCreate(const std::string& fraObjectName, const LabelValueBlock& fraLVB, bool validateKeys)
	{
		return tryAqSwapObjectLegCreate(fraObjectName, fraLVB, validateKeys);
	}
    
    /* @brief			validation interface for the aqInterestRateObjectFraPV method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @return			Fra PV
	*/
	double tryAqInterestRateObjectFraPV(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB)
	{
		return tryAqSwapObjectLegPV(fraObjectName, valuationSettingsLVB);
	}

    /* @brief			validation interface for the aqInterestRateObjectFraDisplay method, which display the INPUT parameters of the cached swap
	*  @param [in]		fraObjectName		Fra object name
	*  @return			Fra display of the input parameters
	*/
	AQLStringMatrix tryAqInterestRateObjectFraDisplay(const std::string& fraObjectName)
	{
		return tryAqSwapObjectLegDisplay(fraObjectName);
	}

	/* @brief			validation interface for the aqInterestRateObjectFraDisplayCashflows method 
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			columnList      Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Fra display of Fra output
	*/
	AnyTypeMatrix tryAqInterestRateObjectFraDisplayCashflows(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, bool showColumnHeaders, const std::vector<std::string>& columnList)
	{
		return tryAqSwapObjectLegDisplayCashflows(fraObjectName, valuationSettingsLVB, "", showColumnHeaders, columnList);
	}

	/* @brief			validation interface for the aqInterestRateObjectFraRate method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @return			Fra rate (strikeRate that make the Fra PV Zero)
	*/
	double tryAqInterestRateObjectFraRate(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB)
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

	/* @brief			validation interface for the aqInterestRateObjectFraToFuturePrice method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		meanReversion       The Hull-White 1F Mean Reversion Parameter
	*  @param [in]		volatility          The Hull-White 1F Volatility Parameter
	*  @return			Fra rate (strikeRate that make the Fra PV Zero)
	*/
	double tryAqInterestRateObjectFraToFuturePrice(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, const double& meanReversion, const double& volatility)
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

	

	/* @brief			validation interface for the aqInterestRateObjectFraToFuturePriceFromConvAdj method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		convexityAdjustment	The convexity adjustment between FraRate and Future rate
	*  @return			Future price
	*/
	double tryAqInterestRateObjectFraToFuturePriceFromConvAdj(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, const double& convexityAdjustment)
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

