/*
 * @brief			validation interface for the meProductSwapPV method
 * @Created:		27 April 2016
 * @Author:			Hans Roggeman
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Swap.h"
#include "Variant.h"

using etrading::LabelValueBlock;

namespace validation_api
{
	const std::string tryMeLWOCurveDataCreate (
		const std::string& mdcName,
		const std::string& currency,
		const std::string& tenorString,
		const std::string& swapType = "",
		const LAStringMatrix& swapStringBlock = LAStringMatrix(0),
		const std::string& toTenorString = "",
		const LAStringMatrix& fraStringBlock = LAStringMatrix(0),
		const LAStringMatrix& irFuturesStringBlock = LAStringMatrix(0),
		const std::string& centralBankTypeString = "",
		const LAStringMatrix& centralBankStringBlock = LAStringMatrix(0),
		const LAStringMatrix& fxStringBlock = LAStringMatrix(0),
		const std::string& unitCurrency = "",
		const bool isInvertedFX = false
		);

    etrading::VariantMatrix tryMeLWOCurveDataDisplay( const std::string& mdcName );

}