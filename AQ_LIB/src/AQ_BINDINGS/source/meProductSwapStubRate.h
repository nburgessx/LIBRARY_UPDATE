/* 
 * @brief			Swig interface to Java for meProductSwapStubRate method
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for meProductSwapStubRate
*  @param [in]		curveIndices		A list of curves to interpolate from
*  @param [in]		curveTenors			A list of tenors corresponding to curve indexes
*  @param [in]		tenorCurveFixings	A list of fixings corresponding to curve indexes
*  @param [in]		swapLVB				A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap stub rate
*/
double meProductSwapStubRate(const std::vector<std::string>& curveIndices,
							const std::vector<std::string>& curveTenors,
							const std::vector<double>& tenorCurveFixings,
							const std::vector<std::vector <std::string> >& swapLVB, 
							bool validateKeys);

/* @brief			return a set of expected keys for swap stub rate label value block
*  @return			expected keys
*/
std::vector<std::string> meProductSwapStubRateLVBKeys();