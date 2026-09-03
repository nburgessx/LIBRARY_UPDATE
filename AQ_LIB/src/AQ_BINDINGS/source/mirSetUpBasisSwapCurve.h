#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for mirSetUpBasisSwapCurve
*  @param [in]		CurveID			The curve collection ID
*  @param [in]		MarketName		Name of the curve constructed by this method
*  @param [in]		GeneralProps	General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		BasisConv		Basis swap conventions
*  @param [in]		BasisRates		Basis swap market rates
*  @param [in]		CurveNames		Equivalent names of the curve being built
*  @param [in]		FwdConv			FX forward market convention
*  @param [in]		FwdFXs			Forward FX rates
*  @param [in]		SpotFXs			Spot FX rates
*/
const std::string mirSetUpBasisSwapCurve(const std::string& CurveID, 
										const std::string& MarketName, 
										const SWIG_STRINGMATRIX & GeneralProps, 
										const SWIG_STRINGMATRIX & BasisConv,
										const SWIG_STRINGMATRIX & BasisRates,
										const std::string& CurveNames, 
										const SWIG_STRINGMATRIX & FwdConv, 
										const SWIG_STRINGMATRIX & FwdFXs, 
										const SWIG_STRINGMATRIX & SpotFXs);


/* @brief			swig interface for mirSetUpBasisSwapCurve
*  @param [in]		CurveID			The curve collection ID
*  @param [in]		MarketName		Name of the curve constructed by this method
*  @param [in]		GeneralProps	General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		BasisConv		Basis swap conventions
*  @param [in]		BasisRates		Basis swap market rates
*  @param [in]		CurveNames		Equivalent names of the curve being built
*  @param [in]		FwdConv			FX forward market convention
*  @param [in]		FwdFXs			Forward FX rates
*  @param [in]		SpotFXs			Spot FX rates
*  @param [in]		FRAConv			FRA convention
*  @param [in]		FRARates		FRA market data
*/
const std::string mirSetUpBasisSwapCurve(const std::string& CurveID, 
										const std::string& MarketName, 
										const SWIG_STRINGMATRIX & GeneralProps, 
										const SWIG_STRINGMATRIX & BasisConv,
										const SWIG_STRINGMATRIX & BasisRates,
										const std::string& CurveNames, 
										const SWIG_STRINGMATRIX & FwdConv, 
										const SWIG_STRINGMATRIX & FwdFXs, 
										const SWIG_STRINGMATRIX & SpotFXs,
										const SWIG_STRINGMATRIX & FRAConv,
										const SWIG_STRINGMATRIX & FRARates);