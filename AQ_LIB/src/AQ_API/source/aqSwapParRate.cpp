#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"

#include "aqSwapParRate.h"
#include "tryAqSwapParRate.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqSwapParRate
*  @param [in]		swapLVB			A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Par Swap Rate
*/
double aqSwapParRate(const std::vector<std::vector <std::string> >& swapLVB, 
						bool validateKeys)
{
	AQ_API_START
    double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(swapLVB);
		
	ret = validation::tryAqSwapParRate(lvb, validateKeys);
	return ret;
    AQ_API_END
}

/* @brief			return a set of expected keys for swap par rate label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapParRateLVBKeys()
{
    AQ_API_START
	std::vector<std::string> ret;
	ret = validation::tryAqSwapParRateLVBKeys();
	return ret;
    AQ_API_END
}
