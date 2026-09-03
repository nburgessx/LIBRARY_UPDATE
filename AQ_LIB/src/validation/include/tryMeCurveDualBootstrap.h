#pragma once

#include "LACoreTemplateType.h"
#include <stdarg.h>

namespace validation
{

	/* @brief			validation interface for curve calibration method using dual-bootstrapping
	*  @param [in]		curveCollection		The curve collection ID
	*  @param [in]		curveName_db		Name of the dual-bootstrapping curve object
	*  @param [in]		curveName_OIS		Name of the OIS curve
	*  @param [in]		curveName_swap		Name of the swap curve
	*  @param [in]		curveIndex_OIS		Curve index of OIS curve
	*  @param [in]		curveIndex_swap		Curve index of swap curve
	*  @param [in]		commonParams		Common parameters across all curves
	*  @param [in]		generateProp_OIS	Curve build properties of OIS curve
	*  @param [in]		oisRates_OIS		OIS rates
	*  @param [in]		oisConv_OIS			OIS conventions
	*  @param [in]		histRates_OIS		Historical OIS rates
	*  @param [in]		lobasisRates_OIS	Libor OIS basis rates
	*  @param [in]		lobasisConv_OIS		Libor OIS basis conventions
	*  @param [in]		swapConv_OIS		Swap conventions
	*  @param [in]		generateProp_swap	Curve build properties of swap curve
	*  @param [in]		moneyConv_swap	    Money market conventions
	*  @param [in]		liborRates_swap 	Libor rates
	*  @param [in]		liborConv_swap 		Libor conventions
	*  @param [in]		swapRates_swap 		Swap rates in swap curve
	*  @param [in]		swapConv_swap 		Swap conventions in swap curve
	*  @param [in]		fra3mRates_swap 	3M FRA rates
	*  @param [in]		fra6mRates_swap 	6M FRA rates
	*  @param [in]		fraConv_swap 		FRA conventions
	*  @param [in]		futureRates_swap 	Futures rates
	*  @param [in]		futureConv_swap 	Futures convention
	*  @param [in]		adjustSwapConv_swap 	Tenor Basis market data
	*  @param [in]		adjustSwapRates_swap 	Tenor Basis convention
	*/
	const LAString tryMeCurveDualBootstrap(const LAString& curveCollection,
											const LAString& curveName_db,
											const LAString& curveName_OIS,
											const LAString& curveName_swap,
											const LAString& curveIndex_OIS,
											const LAString& curveIndex_swap,
											const LAStringMatrix& commonParams,
											const LAStringMatrix& generateProp_OIS,
											const LAStringMatrix& oisRates_OIS,
											const LAStringMatrix& oisConv_OIS,
											const LAStringMatrix& histRates_OIS,
											const LAStringMatrix& lobasisRates_OIS,
											const LAStringMatrix& lobasisConv_OIS,
											const LAStringMatrix& swapConv_OIS,
											const LAStringMatrix& generateProp_swap,
											const LAStringMatrix& moneyConv_swap,
											const LAStringMatrix& liborRates_swap,
											const LAStringMatrix& liborConv_swap,
											const LAStringMatrix& swapRates_swap,
											const LAStringMatrix& swapConv_swap,
											const LAStringMatrix& fra3mRates_swap,
											const LAStringMatrix& fra6mRates_swap,
											const LAStringMatrix& fraConv_swap,
											const LAStringMatrix& futureRates_swap,
											const LAStringMatrix& futureConv_swap,
											const LAStringMatrix& tenorBasisConv_swap,
											const LAStringMatrix& tenorBasisRates_swap);
    
}
