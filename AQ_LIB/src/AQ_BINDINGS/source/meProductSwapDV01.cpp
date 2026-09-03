/* 
 * @brief			Swig interface to Java for meProductSwapDV01 method
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "LACoreTemplateType.h"
#include "TypeUtilities.h"

#include "meProductSwapDV01.h"
#include "tryMeProductSwapDV01.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for meProductSwapDV01
*  @param [in]		swapLVB			A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap DV01
*/
double meProductSwapDV01(const std::vector<std::vector <std::string> >& swapLVB, 
						bool validateKeys)
{
	MLIB_API_START
    double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(swapLVB);
		
	ret = validation_api::tryMeProductSwapDV01(lvb, validateKeys);
	return ret;
    MLIB_API_END
}

/* @brief			return a set of expected keys for swap DV01 label value block
*  @return			expected keys
*/
std::vector<std::string> meProductSwapDV01LVBKeys()
{
    MLIB_API_START
	std::vector<std::string> ret;
	ret = validation_api::tryMeProductSwapDV01LVBKeys();
	return ret;
    MLIB_API_END
}
