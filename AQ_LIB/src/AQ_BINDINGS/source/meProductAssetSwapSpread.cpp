/* 
 * @brief			Swig interface to Java for meProductAssetSwapSpread method
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "LACoreTemplateType.h"
#include "TypeUtilities.h"
#include "meProductAssetSwapSpread.h"
#include "tryMeProductAssetSwapSpread.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for meProductAssetSwapSpread
*  @param [in]		bondCleanPrice	Bond Clean Price
*  @param [in]		assetSwapLVB		A label value block defining an asset swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid
*  @return			Asset swap spread
*/
double meProductAssetSwapSpread(double bondCleanPrice,
								const std::vector<std::vector <std::string> >& assetSwapLVB, 
								bool validateKeys)
{
	MLIB_API_START
    double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(assetSwapLVB);
		
	ret = validation_api::tryMeProductAssetSwapSpread(bondCleanPrice, lvb, validateKeys);
	return ret;
    MLIB_API_END

}

/* @brief			return a set of expected keys for asset swap spread label value block
*  @return			expected keys
*/
std::vector<std::string> meProductAssetSwapSpreadLVBKeys()
{
    MLIB_API_START
	std::vector<std::string> ret;
	
	ret = validation_api::tryMeProductAssetSwapSpreadLVBKeys();
	return ret;
    MLIB_API_END
}
