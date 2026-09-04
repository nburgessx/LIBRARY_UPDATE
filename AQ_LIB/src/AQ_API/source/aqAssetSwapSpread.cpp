#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "aqAssetSwapSpread.h"
#include "tryAqAssetSwapSpread.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqAssetSwapSpread
*  @param [in]		bondCleanPrice	Bond Clean Price
*  @param [in]		assetSwapLVB		A label value block defining an asset swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid
*  @return			Asset swap spread
*/
double aqAssetSwapSpread(double bondCleanPrice,
								const std::vector<std::vector <std::string> >& assetSwapLVB, 
								bool validateKeys)
{
	AQ_API_START
    double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(assetSwapLVB);
		
	ret = validation::tryAqAssetSwapSpread(bondCleanPrice, lvb, validateKeys);
	return ret;
    AQ_API_END

}

/* @brief			return a set of expected keys for asset swap spread label value block
*  @return			expected keys
*/
std::vector<std::string> aqAssetSwapSpreadLVBKeys()
{
    AQ_API_START
	std::vector<std::string> ret;
	
	ret = validation::tryAqAssetSwapSpreadLVBKeys();
	return ret;
    AQ_API_END
}
