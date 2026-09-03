/* 
 * @brief			Swig interface to Java for function meCurveCalibrateFXForwards
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

/* @brief			swig interface for meCurveCalibrateFXForwards
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		fxFwdConv			FX forward conventions
*/
const std::string meCurveCalibrateFXForwards(const std::string& curveCollection, 
											const std::string& staticDataTable, 
											const std::string& curveIndex,
											const SWIG_STRINGMATRIX & curveConv, 
											const SWIG_STRINGMATRIX & fxFwdConv);