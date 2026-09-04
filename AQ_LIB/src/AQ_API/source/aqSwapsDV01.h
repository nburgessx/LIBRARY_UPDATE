#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqSwapsDV01
*  @param [in]		swapLVB			A label value block defining the swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to TRUE
*  @return			Swap DV01
*/
double aqSwapsDV01(const std::vector<std::vector <std::string> >& swapLVB, 
						bool validateKeys=true);

/* @brief			return a set of expected keys for swap DV01 label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapsDV01LVBKeys();