#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"

#include "aqSwapsParRate.h"
#include "tryAqSwapsParRate.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqSwapsParRate
*  @param [in]		swapLVB			A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Par Swap Rate
*/
double aqSwapsParRate(const std::vector<std::vector <std::string> >& swapLVB, 
						bool validateKeys)
{
	AQ_API_START
    double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(swapLVB);
		
	ret = validation::tryAqSwapsParRate(lvb, validateKeys);
	return ret;
    AQ_API_END
}

/* @brief			return a set of expected keys for swap par rate label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapsParRateLVBKeys()
{
    AQ_API_START
	std::vector<std::string> ret;
	ret = validation::tryAqSwapsParRateLVBKeys();
	return ret;
    AQ_API_END
}
