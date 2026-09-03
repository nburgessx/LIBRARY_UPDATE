/* 
 * @brief			Swig interface to Java for function mirSetUpFwdFXConstantCurve
 * @Created:		30 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for mirSetUpFwdFXConstantCurve
*  @param [in]		CurveID				The curve collection ID
*  @param [in]		MarketName			Name of the curve constructed by this method
*  @param [in]		GeneralProps		General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		FwdFXConstantConv	FX forward conventions
*  @param [in]		CurveNames			Equivalent names of the curve being built
*/
const std::string mirSetUpFwdFXConstantCurve(const std::string& CurveID, 
											const std::string& MarketName, 
											const std::vector<std::vector <std::string> >& GeneralProps, 
											const std::vector<std::vector <std::string> >& FwdFXConstantConv,
											const std::string& CurveNames) throw(std::exception);