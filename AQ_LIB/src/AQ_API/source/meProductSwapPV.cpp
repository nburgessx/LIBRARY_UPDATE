#include "LACoreTemplateType.h"
#include "TypeUtilities.h"

#include "meProductSwapPV.h"
#include "tryMeProductSwapPV.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for meProductSwapPV
*  @param [in]		swapLVB			A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap PV
*/
double meProductSwapPV(const std::vector<std::vector <std::string> >& swapLVB, 
						bool validateKeys)
{
    AQ_API_START
	double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(swapLVB);
		
	ret = validation::tryMeProductSwapPV(lvb, validateKeys);

    return ret;
    AQ_API_END

}

/* @brief			return a set of expected keys for swap pv label value block
*  @return			expected keys
*/
std::vector<std::string> tryMeProductSwapPVLVBKeys()
{
    AQ_API_START
	std::vector<std::string> ret;
    ret = validation::tryMeProductSwapPVLVBKeys();
	return ret;
    AQ_API_END
}
