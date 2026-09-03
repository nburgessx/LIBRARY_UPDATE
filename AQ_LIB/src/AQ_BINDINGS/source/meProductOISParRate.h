/* 
 * @brief			Swig interface to Java for meProductOISSwapParRate method
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for meProductOISSwapParRate
*  @param [in]		oisSwapLVB		A label value block defining the ois swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to TRUE
*  @return			OIS Par Swap Rate
*/
double meProductOISParRate(const std::vector<std::vector <std::string> >& oisSwapLVB, 
								bool validateKeys=true);


/* @brief			return a set of expected keys for ois swap par rate label value block
*  @return			expected keys
*/
std::vector<std::string> meProductOISParRateLVBKeys();