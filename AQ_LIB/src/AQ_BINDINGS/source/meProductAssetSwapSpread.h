/* 
 * @brief			Swig interface to Java for meProductAssetSwapSpread method
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for meProductAssetSwapSpread
*  @param [in]		bondCleanPrice	Bond Clean Price
*  @param [in]		assetSwapLVB		A label value block defining an asset swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to TRUE
*  @return			Asset swap spread
*/
double meProductAssetSwapSpread(double bondCleanPrice,
								const std::vector<std::vector <std::string> >& assetSwapLVB, 
								bool validateKeys=true);


/* @brief			return a set of expected keys for asset swap spread label value block
*  @return			expected keys
*/
std::vector<std::string> meProductAssetSwapSpreadLVBKeys();