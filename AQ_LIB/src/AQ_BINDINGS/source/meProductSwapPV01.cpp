#include "LACoreTemplateType.h"
#include "TypeUtilities.h"

#include "meProductSwapPV01.h"
#include "tryMeProductSwapPV01.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for meProductSwapPV01
*  @param [in]		swapLVB			A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap PV01
*/
double meProductSwapPV01(const std::vector<std::vector <std::string> >& swapLVB, 
						bool validateKeys)
{
    MLIB_API_START
	double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(swapLVB);

	ret = validation_api::tryMeProductSwapPV01(lvb, validateKeys);
	
	return ret;
    MLIB_API_END
}

/* @brief			return a set of expected keys for swap pv01 label value block
*  @return			expected keys
*/
std::vector<std::string> meProductSwapPV01LVBKeys()
{
    MLIB_API_START
	std::vector<std::string> ret;

    ret = validation_api::tryMeProductSwapPV01LVBKeys();
	
	return ret;
    MLIB_API_END
}
