/* 
 * @brief			Swig interface to Java for meProductSwapDV01 method
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for meProductSwapDV01
*  @param [in]		swapLVB			A label value block defining the swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to TRUE
*  @return			Swap DV01
*/
double meProductSwapDV01(const std::vector<std::vector <std::string> >& swapLVB, 
						bool validateKeys=true);

/* @brief			return a set of expected keys for swap DV01 label value block
*  @return			expected keys
*/
std::vector<std::string> meProductSwapDV01LVBKeys();