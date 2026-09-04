#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqSwapsPV01
*  @param [in]		swapLVB			A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap PV01
*/
double aqSwapsPV01(const std::vector<std::vector <std::string> >& swapLVB, 
						bool validateKeys=true);

/* @brief			return a set of expected keys for swap pv01 label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapsPV01LVBKeys();