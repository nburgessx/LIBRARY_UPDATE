#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for meCurveCalibrateSwap
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General yield curve data
*  @param [in]		moneyMarketConv		Money market conventions
*  @param [in]		liborConv			Libor market conventions
*  @param [in]		liborRates			Libor market data
*  @param [in]		swapConv			Swap market conventions
*  @param [in]		swapRates			Swap market data
*  @param [in]		fraConv				FRA market conventions
*  @param [in]		fra3mRates			3M FRA market data
*  @param [in]		fra6mRates			6M FRA market data
*  @param [in]		futureConv			Futures market conventions
*  @param [in]		futureRates			Futures market data
*  @param [in]		tenorBasisConv	    Tenor Basis market conventions
*  @param [in]		tenorBasisRates	    Tenor Basis market data
*/
const std::string meCurveCalibrateSwap( const std::string& curveCollection, 
								        const std::string& staticDataTable, 
								        const std::string& curveIndex,
								        const SWIG_STRINGMATRIX & curveConv, 
								        const SWIG_STRINGMATRIX & moneyMarketConv,
								        const SWIG_STRINGMATRIX & liborConv,
								        const SWIG_STRINGMATRIX & liborRates,
								        const SWIG_STRINGMATRIX & swapConv,
								        const SWIG_STRINGMATRIX & swapRates,
								        const SWIG_STRINGMATRIX & fraConv,
								        const SWIG_STRINGMATRIX & fra3mRates, 
								        const SWIG_STRINGMATRIX & fra6mRates, 
								        const SWIG_STRINGMATRIX & futureConv,
								        const SWIG_STRINGMATRIX & futureRates,
								        const SWIG_STRINGMATRIX & tenorBasisConv,
								        const SWIG_STRINGMATRIX & tenorBasisRates ) ;
