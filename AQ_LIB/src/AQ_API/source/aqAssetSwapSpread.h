#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqAssetSwapSpread
*  @param [in]		bondCleanPrice	Bond Clean Price
*  @param [in]		assetSwapLVB		A label value block defining an asset swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to TRUE
*  @return			Asset swap spread
*/
double aqAssetSwapSpread(double bondCleanPrice,
								const std::vector<std::vector <std::string> >& assetSwapLVB, 
								bool validateKeys=true);


/* @brief			return a set of expected keys for asset swap spread label value block
*  @return			expected keys
*/
std::vector<std::string> aqAssetSwapSpreadLVBKeys();