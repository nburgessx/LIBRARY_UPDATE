/* 
 * @brief			Swig interface to Java for function mirSetUpOISCurve
 * @Created:		14 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for mirSetUpOISCurve
*  @param [in]		CurveID			The curve collection ID
*  @param [in]		MarketName		Name of the curve constructed by this method
*  @param [in]		GeneralProps	General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		OISConv			The OIS curve configuration info
*  @param [in]		OISRates		Constituent OIS instrument rates
*  @param [in]		CurveNames		Equivalent names of the curve being built
*  @param [in]		OISHistRates	Historical OIS fixings
*  @param [in]		LOBasisRates	Libor-OIS basis spreads
*  @param [in]		SwapConv		Libor swap conventions
*  @param [in]		LOBasisConv		Libor-OIS swap conventions
*/
const std::string mirSetUpOISCurve(const std::string& CurveID, 
								const std::string& MarketName, 
								const SWIG_STRINGMATRIX & GeneralProps, 
								const SWIG_STRINGMATRIX & OISConv, 
								const SWIG_STRINGMATRIX & OISRates, 
								const std::string& CurveNames, 
								const SWIG_STRINGMATRIX & OISHistRates, 
								const SWIG_STRINGMATRIX & LOBasisRates, 
								const SWIG_STRINGMATRIX & SwapConv, 
								const SWIG_STRINGMATRIX & SwapRates, 
								const SWIG_STRINGMATRIX & LOBasisConv) ;