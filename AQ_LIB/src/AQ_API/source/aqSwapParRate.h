#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqSwapParRate
*  @param [in]		swapLVB			A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Par Swap Rate
*/
double aqSwapParRate(const std::vector<std::vector <std::string> >& swapLVB, 
						bool validateKeys=true);

/* @brief			return a set of expected keys for swap par rate label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapParRateLVBKeys();