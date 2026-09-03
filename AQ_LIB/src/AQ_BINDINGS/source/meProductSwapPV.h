/* 
 * @brief			Swig interface to Java for tryMeProductSwapPV method
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for meProductSwapPV
*  @param [in]		swapLVB			A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap PV
*/
double meProductSwapPV(const std::vector<std::vector <std::string> >& swapLVB, 
						bool validateKeys=true);

/* @brief			return a set of expected keys for swap pv label value block
*  @return			expected keys
*/
std::vector<std::string> tryMeProductSwapPVLVBKeys();