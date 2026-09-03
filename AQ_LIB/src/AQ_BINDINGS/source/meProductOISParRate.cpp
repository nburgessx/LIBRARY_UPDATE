/* 
 * @brief			Swig interface to Java for meProductOISParRate method
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "LACoreTemplateType.h"
#include "TypeUtilities.h"

#include "meProductOISParRate.h"
#include "tryMeProductOISParRate.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for meProductOISParRate
*  @param [in]		oisSwapLVB		A label value block defining the ois swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid
*  @return			OIS Par Swap Rate
*/
double meProductOISParRate(const std::vector<std::vector <std::string> >& oisSwapLVB, 
								bool validateKeys)
{
    MLIB_API_START
	double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(oisSwapLVB);
		
	ret = validation_api::tryMeProductOISParRate(lvb, validateKeys);
	return ret;
    MLIB_API_END
}

/* @brief			return a set of expected keys for ois swap par rate label value block
*  @return			expected keys
*/
std::vector<std::string> meProductOISParRateLVBKeys()
{
    MLIB_API_START
	std::vector<std::string> ret;
	ret = validation_api::tryMeProductOISParRateLVBKeys();
	return ret;
    MLIB_API_END

}
