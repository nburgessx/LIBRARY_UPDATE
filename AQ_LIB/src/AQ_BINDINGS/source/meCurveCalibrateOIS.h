/* 
 * @brief			Swig interface to Java for function meCurveCalibrateOIS
 * @Created:		03 June 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for meCurveCalibrateOIS
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		oisConv				The OIS curve configuration info
*  @param [in]		oisRates			Constituent OIS instrument rates
*  @param [in]		oisHistoricalRates	Historical OIS fixings
*  @param [in]		liborOisBasisConv	Libor-OIS swap conventions
*  @param [in]		liborOisBasisRates	Libor-OIS basis spreads
*  @param [in]		swapConv			Libor swap conventions
*  @param [in]		swapRates			Libor swap market rates
*/
const std::string meCurveCalibrateOIS(const std::string& curveCollection, 
								const std::string& staticDataTable, 
								const std::string& curveIndex, 
								const SWIG_STRINGMATRIX & curveConv, 
								const SWIG_STRINGMATRIX & oisConv, 
								const SWIG_STRINGMATRIX & oisRates, 
								const SWIG_STRINGMATRIX & oisHistoricalRates, 
								const SWIG_STRINGMATRIX & liborOisBasisConv, 
								const SWIG_STRINGMATRIX & liborOisBasisRates, 
								const SWIG_STRINGMATRIX & swapConv, 
								const SWIG_STRINGMATRIX & swapRates) ;