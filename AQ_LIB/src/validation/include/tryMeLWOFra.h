/*
 * @brief			validation interface for the meProductSwapPV method
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Swap.h"

using etrading::LabelValueBlock;

namespace validation_api
{

    /* @brief			validation interface for the meLWOFraCreate method
	*  @param [in]		FraObjectName	Fra object name
	*  @param [in]		FraLVB			Fra label value block with Fras
	*  @param [in]		FraObjectName	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryMeLWOFraCreate(const std::string& FraObjectName, const LabelValueBlock& FraLVB, bool validateKeys=true);


    /* @brief			validation interface for the meLWOFraPV method
	*  @param [in]		FraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		valuationDate	    Valuation Date (AssetSwap's Bond SettleDate)
	*  @return			Fra PV
	*/
	double tryMeLWOFraPV(const std::string& FraObjectName, const LabelValueBlock& valuationSettingsLVB);


    /* @brief			validation interface for the meLWOFraDisplay method, which display the INPUT parameters of the cached swap
	*  @param [in]		FraObjectName		Fra object name
	*  @return			Fra display of the input parameters
	*/
	LAStringMatrix tryMeLWOFraDisplay(const std::string& FraObjectName);

   	/* @brief			validation interface for the meLWOFraDisplayCashflows method 
	*  @param [in]		FraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @param [in] 	    columnList          Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Fra display of Fra output
	*/
	AnyTypeMatrix tryMeLWOFraDisplayCashflows(const std::string& FraObjectName, const LabelValueBlock& valuationSettingsLVB, bool showColumnHeaders=true, 
                                             const std::vector<std::string>& columnList=std::vector<std::string>());

	/* @brief			validation interface for the meLWOFraRate method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @return			Fra rate (strikeRate that make the Fra PV Zero)
	*/
	double tryMeLWOFraRate(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB);

	/* @brief			validation interface for the meLWOFraToFuturePrice method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		meanReversion       The Hull-White 1F Mean Reversion Parameter
	*  @param [in]		volatility          The Hull-White 1F Volatility Parameter
	*  @return			Future price
	*/
	double tryMeLWOFraToFuturePrice(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, const double& meanReversion, const double& volatility);

	/* @brief			validation interface for the meLWOFraToFuturePriceFromConvAdj method
	*  @param [in]		fraObjectName		Fra object name
	*  @param [in]		valuationSettingsLVB A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		convexityAdjustment	The convexity adjustment between FraRate and Future rate
	*  @return			Future price
	*/
	double tryMeLWOFraToFuturePriceFromConvAdj(const std::string& fraObjectName, const LabelValueBlock& valuationSettingsLVB, const double& convexityAdjustment);

}

