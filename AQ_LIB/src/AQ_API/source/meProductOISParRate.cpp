#include "LACoreTemplateType.h"
#include "TypeUtilities.h"

#include "meProductOISParRate.h"
#include "tryMeProductOISParRate.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for meProductOISParRate
*  @param [in]		oisSwapLVB		A label value block defining the ois swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid
*  @return			OIS Par Swap Rate
*/
double meProductOISParRate(const std::vector<std::vector <std::string> >& oisSwapLVB, 
								bool validateKeys)
{
    AQ_API_START
	double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(oisSwapLVB);
		
	ret = validation::tryMeProductOISParRate(lvb, validateKeys);
	return ret;
    AQ_API_END
}

/* @brief			return a set of expected keys for ois swap par rate label value block
*  @return			expected keys
*/
std::vector<std::string> meProductOISParRateLVBKeys()
{
    AQ_API_START
	std::vector<std::string> ret;
	ret = validation::tryMeProductOISParRateLVBKeys();
	return ret;
    AQ_API_END

}
