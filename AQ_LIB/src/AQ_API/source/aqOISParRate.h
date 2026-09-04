#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqOISSwapParRate
*  @param [in]		oisSwapLVB		A label value block defining the ois swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to TRUE
*  @return			OIS Par Swap Rate
*/
double aqOISParRate(const std::vector<std::vector <std::string> >& oisSwapLVB, 
								bool validateKeys=true);


/* @brief			return a set of expected keys for ois swap par rate label value block
*  @return			expected keys
*/
std::vector<std::string> aqOISParRateLVBKeys();