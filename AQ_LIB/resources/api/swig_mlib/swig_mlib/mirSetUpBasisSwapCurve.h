/* 
 * @brief			Swig interface to Java for function mirSetUpBasisSwapCurve
 * @Created:		16 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

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
										const std::vector<std::vector <std::string> >& GeneralProps, 
										const std::vector<std::vector <std::string> >& BasisConv,
										const std::vector<std::vector <std::string> >& BasisRates,
										const std::string& CurveNames, 
										const std::vector<std::vector <std::string> >& FwdConv, 
										const std::vector<std::vector <std::string> >& FwdFXs, 
										const std::vector<std::vector <std::string> >& SpotFXs) throw(std::exception);