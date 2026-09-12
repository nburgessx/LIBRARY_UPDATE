#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Swap.h"

using etrading::LabelValueBlock;

namespace validation
{

    /* @brief			validation interface for the aqIRObjectFraCreate method
	*  @param [in]		FraObjectName	Fra object name
	*  @param [in]		FraLVB			Fra label value block with Fras
	*  @param [in]		FraObjectName	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqIRObjectFraCreate(const std::string& FraObjectName, const LabelValueBlock& FraLVB, bool validateKeys=true);


    /* @brief			validation interface for the aqIRObjectFraPV method
	*  @param [in]		FraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		valuationDate	    Valuation Date (AssetSwap's Bond SettleDate)
	*  @return			Fra PV
	*/
	double tryAqIRObjectFraPV(const std::string& FraObjectName, const LabelValueBlock& valuationSettingsLVB);


    /* @brief			validation interface for the aqIRObjectFraDisplay method, which display the INPUT parameters of the cached swap
	*  @param [in]		FraObjectName		Fra object name
	*  @return			Fra display of the input parameters
	*/
	AQLStringMatrix tryAqIRObjectFraDisplay(const std::string& FraObjectName);

   	/* @brief			validation interface for the aqIRObjectFraDisplayCashflows method 
	*  @param [in]		FraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @param [in] 	    columnList          Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Fra display of Fra output
	*/
	AnyTypeMatrix tryAqIRObjectFraDisplayCashflows(const std::string& FraObjectName, const LabelValueBlock& valuationSettingsLVB, bool showColumnHeaders=true, 
                                             const std::vector<std::string>& columnList=std::vector<std::string>());

	/* @brief			validation interface for the aqIRObjectFraRate method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @return			Fra rate (strikeRate that make the Fra PV Zero)
	*/
	double tryAqIRObjectFraRate(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB);

	/* @brief			validation interface for the aqIRObjectFraToFuturePrice method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		meanReversion       The Hull-White 1F Mean Reversion Parameter
	*  @param [in]		volatility          The Hull-White 1F Volatility Parameter
	*  @return			Future price
	*/
	double tryAqIRObjectFraToFuturePrice(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, const double& meanReversion, const double& volatility);

	/* @brief			validation interface for the aqIRObjectFraToFuturePriceFromConvAdj method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		convexityAdjustment	The convexity adjustment between FraRate and Future rate
	*  @return			Future price
	*/
	double tryAqIRObjectFraToFuturePriceFromConvAdj(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, const double& convexityAdjustment);

}

