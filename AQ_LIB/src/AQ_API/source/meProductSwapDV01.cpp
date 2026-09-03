#include "LACoreTemplateType.h"
#include "TypeUtilities.h"

#include "meProductSwapDV01.h"
#include "tryMeProductSwapDV01.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for meProductSwapDV01
*  @param [in]		swapLVB			A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap DV01
*/
double meProductSwapDV01(const std::vector<std::vector <std::string> >& swapLVB, 
						bool validateKeys)
{
	AQ_API_START
    double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(swapLVB);
		
	ret = validation::tryMeProductSwapDV01(lvb, validateKeys);
	return ret;
    AQ_API_END
}

/* @brief			return a set of expected keys for swap DV01 label value block
*  @return			expected keys
*/
std::vector<std::string> meProductSwapDV01LVBKeys()
{
    AQ_API_START
	std::vector<std::string> ret;
	ret = validation::tryMeProductSwapDV01LVBKeys();
	return ret;
    AQ_API_END
}
