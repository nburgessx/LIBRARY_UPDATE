#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Swap.h"
#include "Variant.h"

using etrading::LabelValueBlock;

namespace validation
{
	const std::string tryAqCurveObjectDataCreate (
		const std::string& mdcName,
		const std::string& currency,
		const std::string& tenorString,
		const std::string& swapType = "",
		const AQLStringMatrix& swapStringBlock = AQLStringMatrix(0),
		const std::string& toTenorString = "",
		const AQLStringMatrix& fraStringBlock = AQLStringMatrix(0),
		const AQLStringMatrix& irFuturesStringBlock = AQLStringMatrix(0),
		const std::string& centralBankTypeString = "",
		const AQLStringMatrix& centralBankStringBlock = AQLStringMatrix(0),
		const AQLStringMatrix& fxStringBlock = AQLStringMatrix(0),
		const std::string& unitCurrency = "",
		const bool isInvertedFX = false
		);

    etrading::VariantMatrix tryAqCurveObjectDataDisplay( const std::string& mdcName );

}