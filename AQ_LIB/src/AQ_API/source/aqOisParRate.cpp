#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"

#include "aqOisParRate.h"
#include "tryAqOisParRate.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqOisParRate
*  @param [in]		oisSwapLVB		A label value block defining the ois swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid
*  @return			OIS Par Swap Rate
*/
double aqOisParRate(const std::vector<std::vector <std::string> >& oisSwapLVB, 
								bool validateKeys)
{
    AQ_API_START
	double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(oisSwapLVB);
		
	ret = validation::tryAqOisParRate(lvb, validateKeys);
	return ret;
    AQ_API_END
}

/* @brief			return a set of expected keys for ois swap par rate label value block
*  @return			expected keys
*/
std::vector<std::string> aqOisParRateLVBKeys()
{
    AQ_API_START
	std::vector<std::string> ret;
	ret = validation::tryAqOisParRateLVBKeys();
	return ret;
    AQ_API_END

}
