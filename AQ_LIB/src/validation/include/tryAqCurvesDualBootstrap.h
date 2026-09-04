#pragma once

#include "AQLCoreTemplateType.h"
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
	const AQLString tryAqCurvesDualBootstrap(const AQLString& curveCollection,
											const AQLString& curveName_db,
											const AQLString& curveName_OIS,
											const AQLString& curveName_swap,
											const AQLString& curveIndex_OIS,
											const AQLString& curveIndex_swap,
											const AQLStringMatrix& commonParams,
											const AQLStringMatrix& generateProp_OIS,
											const AQLStringMatrix& oisRates_OIS,
											const AQLStringMatrix& oisConv_OIS,
											const AQLStringMatrix& histRates_OIS,
											const AQLStringMatrix& lobasisRates_OIS,
											const AQLStringMatrix& lobasisConv_OIS,
											const AQLStringMatrix& swapConv_OIS,
											const AQLStringMatrix& generateProp_swap,
											const AQLStringMatrix& moneyConv_swap,
											const AQLStringMatrix& liborRates_swap,
											const AQLStringMatrix& liborConv_swap,
											const AQLStringMatrix& swapRates_swap,
											const AQLStringMatrix& swapConv_swap,
											const AQLStringMatrix& fra3mRates_swap,
											const AQLStringMatrix& fra6mRates_swap,
											const AQLStringMatrix& fraConv_swap,
											const AQLStringMatrix& futureRates_swap,
											const AQLStringMatrix& futureConv_swap,
											const AQLStringMatrix& tenorBasisConv_swap,
											const AQLStringMatrix& tenorBasisRates_swap);
    
}
