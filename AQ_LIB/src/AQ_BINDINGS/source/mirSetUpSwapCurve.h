/* 
 * @brief			Swig interface to Java for function mirSetUpSwapCurve
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
*  @param [in]		YldData			General yield curve data
*  @param [in]		MData			Money market conventions
*  @param [in]		LData			Libor market conventions
*  @param [in]		LGrid			Libor market data
*  @param [in]		SData			Swap market conventions
*  @param [in]		SGrid			Swap market data
*  @param [in]		FRAData			FRA market conventions
*  @param [in]		FRA3MGrid		3M FRA market data
*  @param [in]		FRA6MGrid		6M FRA market data
*  @param [in]		FData			Futures market conventions
*  @param [in]		FGrid			Futures market data
*  @param [in]		AdjData			Basis swaps market conventions
*  @param [in]		AdjGrid			Basis swaps market data
*  @param [in]		CurveNames		Equivalent names of the curve being built
*  @param [in]		CurveName_DF2	Trade discount curve
*/
const std::string mirSetUpSwapCurve(const std::string& CurveID, 
								const std::string& MarketName, 
								const SWIG_STRINGMATRIX & YldData, 
								const SWIG_STRINGMATRIX & MData, 
								const SWIG_STRINGMATRIX & LData, 
								const SWIG_STRINGMATRIX & LGrid, 
								const SWIG_STRINGMATRIX & SData, 
								const SWIG_STRINGMATRIX & SGrid, 
								const SWIG_STRINGMATRIX & FRAData, 
								const SWIG_STRINGMATRIX & FRA3MGrid, 
								const SWIG_STRINGMATRIX & FRA6MGrid, 
								const SWIG_STRINGMATRIX & FData, 
								const SWIG_STRINGMATRIX & FGrid, 
								const SWIG_STRINGMATRIX & AdjData, 
								const SWIG_STRINGMATRIX & AdjGrid, 
								const std::string& CurveNames,
								const std::string& CurveName_DF2) ;
