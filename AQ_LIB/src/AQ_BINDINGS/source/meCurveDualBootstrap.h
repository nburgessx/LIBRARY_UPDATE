/*
* @brief			Swig interface to for curve calibration method using dual-bootstrapping
* @Created:			01 March 2018
* @Author:			Joseph Ye
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			Swig interface to for curve calibration method using dual-bootstrapping
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		curveName_db		Name of the dual-bootstrapping curve object
*  @param [in]		curveName_OIS		Name of the OIS curve
*  @param [in]		curveName_swap		Name of the swap curve
*  @param [in]		curveIndex_OIS		Curve index of OIS curve
*  @param [in]		curveIndex_swap		Curve index of swap curve
*  @param [in]		commonParams		Parameters common to the curves
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
const std::string meCurveDualBootstrap(const std::string& curveCollection,
										const std::string& curveName_db,
										const std::string& curveName_OIS,
										const std::string& curveName_swap,
										const std::string& curveIndex_OIS,
										const std::string& curveIndex_swap,
										const SWIG_STRINGMATRIX & commonParams,
										const SWIG_STRINGMATRIX & generateProp_OIS,
										const SWIG_STRINGMATRIX & oisRates_OIS,
										const SWIG_STRINGMATRIX & oisConv_OIS,
										const SWIG_STRINGMATRIX & histRates_OIS,
										const SWIG_STRINGMATRIX & lobasisRates_OIS,
										const SWIG_STRINGMATRIX & lobasisConv_OIS,
										const SWIG_STRINGMATRIX & swapConv_OIS,
										const SWIG_STRINGMATRIX & generateProp_swap,
										const SWIG_STRINGMATRIX & moneyConv_swap,
										const SWIG_STRINGMATRIX & liborRates_swap,
										const SWIG_STRINGMATRIX & liborConv_swap,
										const SWIG_STRINGMATRIX & swapRates_swap,
										const SWIG_STRINGMATRIX & swapConv_swap,
										const SWIG_STRINGMATRIX & fra3mRates_swap,
										const SWIG_STRINGMATRIX & fra6mRates_swap,
										const SWIG_STRINGMATRIX & fraConv_swap,
										const SWIG_STRINGMATRIX & futureRates_swap,
										const SWIG_STRINGMATRIX & futureConv_swap,
										const SWIG_STRINGMATRIX & tenorBasisConv_swap,
										const SWIG_STRINGMATRIX & tenorBasisRates_swap);
