#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqCurvesCalibrateBasis
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		basisConv			Basis swap conventions
*  @param [in]		basisRates			Basis swap market rates
*  @param [in]		fxFwdConv			FX forward market convention
*  @param [in]		fxFwdRates			Forward FX rates
*  @param [in]		spotFxRates			Spot FX rates
*/
const std::string aqCurvesCalibrateBasis(const std::string& curveCollection, 
										const std::string& staticDataTable, 
										const std::string& curveIndex, 
										const SWIG_STRINGMATRIX & curveConv, 
										const SWIG_STRINGMATRIX &basisConv,
										const SWIG_STRINGMATRIX & basisRates,
										const SWIG_STRINGMATRIX & fxFwdConv, 
										const SWIG_STRINGMATRIX & fxFwdRates, 
										const SWIG_STRINGMATRIX & spotFxRates);


/* @brief			swig interface for aqCurvesCalibrateBasis
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		basisConv			Basis swap conventions
*  @param [in]		basisRates			Basis swap market rates
*  @param [in]		fxFwdConv			FX forward market convention
*  @param [in]		fxFwdRates			Forward FX rates
*  @param [in]		spotFxRates			Spot FX rates
*  @param [in]		FRAConv				FRA convention
*  @param [in]		FRARates			FRA market data
*/
const std::string aqCurvesCalibrateBasis(const std::string& curveCollection, 
										const std::string& staticDataTable, 
										const std::string& curveIndex, 
										const SWIG_STRINGMATRIX & curveConv, 
										const SWIG_STRINGMATRIX & basisConv,
										const SWIG_STRINGMATRIX & basisRates,
										const SWIG_STRINGMATRIX & fxFwdConv, 
										const SWIG_STRINGMATRIX & fxFwdRates, 
										const SWIG_STRINGMATRIX & spotFxRates,
										const SWIG_STRINGMATRIX & FRAConv,
										const SWIG_STRINGMATRIX & FRARates);