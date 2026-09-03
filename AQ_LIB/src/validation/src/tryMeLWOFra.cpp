#include "tryMeLWOFra.h"
#include "tryMeLWOLeg.h"

#include "Fra.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"
#include "LWOUtilities.h"
#include "SettingsValidation.h"

#include <memory>

using etrading::CreateDataFile;

namespace validation
{
 
	/* @brief			validation interface for the meLWOFraCreate method
	*  @param [in]		fraObjectName	Fra object name
	*  @param [in]		fraLVB			Fra label value block with Fras
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			fraObjectName
	*/
	std::string tryMeLWOFraCreate(const std::string& fraObjectName, const LabelValueBlock& fraLVB, bool validateKeys)
	{
		return tryMeLWOLegCreate(fraObjectName, fraLVB, validateKeys);
	}
    
    /* @brief			validation interface for the meLWOFraPV method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @return			Fra PV
	*/
	double tryMeLWOFraPV(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB)
	{
		return tryMeLWOLegPV(fraObjectName, valuationSettingsLVB);
	}

    /* @brief			validation interface for the meLWOFraDisplay method, which display the INPUT parameters of the cached swap
	*  @param [in]		fraObjectName		Fra object name
	*  @return			Fra display of the input parameters
	*/
	AQLStringMatrix tryMeLWOFraDisplay(const std::string& fraObjectName)
	{
		return tryMeLWOLegDisplay(fraObjectName);
	}

	/* @brief			validation interface for the meLWOFraDisplayCashflows method 
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			columnList      Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Fra display of Fra output
	*/
	AnyTypeMatrix tryMeLWOFraDisplayCashflows(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, bool showColumnHeaders, const std::vector<std::string>& columnList)
	{
		return tryMeLWOLegDisplayCashflows(fraObjectName, valuationSettingsLVB, "", showColumnHeaders, columnList);
	}

	/* @brief			validation interface for the meLWOFraRate method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @return			Fra rate (strikeRate that make the Fra PV Zero)
	*/
	double tryMeLWOFraRate(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(fraObjectName, valuationSettingsLVB);

		auto leg = etrading::getLeg(fraObjectName);

		auto fraObject = std::dynamic_pointer_cast<etrading::Fra>(leg);
		if (fraObject == nullptr)
		{
			throw AQLCoreInvalidData("#Error: The leg is not a Fra type", __FILE__, __LINE__);
		}

		auto result = fraObject->fraRate(valuationSettingsLVB);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END

	}

	/* @brief			validation interface for the meLWOFraToFuturePrice method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		meanReversion       The Hull-White 1F Mean Reversion Parameter
	*  @param [in]		volatility          The Hull-White 1F Volatility Parameter
	*  @return			Fra rate (strikeRate that make the Fra PV Zero)
	*/
	double tryMeLWOFraToFuturePrice(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, const double& meanReversion, const double& volatility)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(fraObjectName, valuationSettingsLVB, meanReversion, volatility);

		auto leg = etrading::getLeg(fraObjectName);

		auto fraObject = std::dynamic_pointer_cast<etrading::Fra>(leg);
		if (fraObject == nullptr)
		{
			throw AQLCoreInvalidData("#Error: The leg is not a Fra type", __FILE__, __LINE__);
		}

		double result = fraObject->fraRateToFuturePrice(valuationSettingsLVB, meanReversion, volatility);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	

	/* @brief			validation interface for the meLWOFraToFuturePriceFromConvAdj method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		convexityAdjustment	The convexity adjustment between FraRate and Future rate
	*  @return			Future price
	*/
	double tryMeLWOFraToFuturePriceFromConvAdj(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, const double& convexityAdjustment)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(fraObjectName, valuationSettingsLVB, convexityAdjustment);

		auto leg = etrading::getLeg(fraObjectName);

		auto fraObject = std::dynamic_pointer_cast<etrading::Fra>(leg);
		if (fraObject == nullptr)
		{
			throw AQLCoreInvalidData("#Error: The leg is not a Fra type", __FILE__, __LINE__);
		}

		double result = fraObject->fraRateToFuturePrice(valuationSettingsLVB, convexityAdjustment);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END

	}


}

