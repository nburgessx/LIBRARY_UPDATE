/* 
 * @brief			Swig interface to Java for function mirSetUpSwapCurve
 * @Created:		14 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

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
								const std::vector<std::vector <std::string> >& YldData, 
								const std::vector<std::vector <std::string> >& MData, 
								const std::vector<std::vector <std::string> >& LData, 
								const std::vector<std::vector <std::string> >& LGrid, 
								const std::vector<std::vector <std::string> >& SData, 
								const std::vector<std::vector <std::string> >& SGrid, 
								const std::vector<std::vector <std::string> >& FRAData, 
								const std::vector<std::vector <std::string> >& FRA3MGrid, 
								const std::vector<std::vector <std::string> >& FRA6MGrid, 
								const std::vector<std::vector <std::string> >& FData, 
								const std::vector<std::vector <std::string> >& FGrid, 
								const std::vector<std::vector <std::string> >& AdjData, 
								const std::vector<std::vector <std::string> >& AdjGrid, 
								const std::string& CurveNames,
								const std::string& CurveName_DF2) throw(std::exception) ;
