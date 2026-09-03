/* 
 * @brief			Swig interface to Java for meProductSwapParRate method
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "LACoreTemplateType.h"
#include "TypeUtilities.h"

#include "meProductSwapParRate.h"
#include "tryMeProductSwapParRate.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for meProductSwapParRate
*  @param [in]		swapLVB			A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Par Swap Rate
*/
double meProductSwapParRate(const std::vector<std::vector <std::string> >& swapLVB, 
						bool validateKeys)
{
	MLIB_API_START
    double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(swapLVB);
		
	ret = validation_api::tryMeProductSwapParRate(lvb, validateKeys);
	return ret;
    MLIB_API_END
}

/* @brief			return a set of expected keys for swap par rate label value block
*  @return			expected keys
*/
std::vector<std::string> meProductSwapParRateLVBKeys()
{
    MLIB_API_START
	std::vector<std::string> ret;
	ret = validation_api::tryMeProductSwapParRateLVBKeys();
	return ret;
    MLIB_API_END
}
