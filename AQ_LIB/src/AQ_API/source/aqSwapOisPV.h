#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqOISSwapPV
*  @param [in]		oisSwapLVB		A label value block defining the ois swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to TRUE
*  @return			OIS PV
*/
double aqSwapOisPV(const std::vector<std::vector <std::string> >& oisSwapLVB, 
						bool validateKeys=true);


/* @brief			return a set of expected keys for swap pv label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapOisPVLVBKeys();