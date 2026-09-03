/* 
 * @brief			Swig interface to Java for meProductOISPV method
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "LACoreTemplateType.h"
#include "TypeUtilities.h"

#include "meProductOISPV.h"
#include "tryMeProductOISPV.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for meProductOISPV
*  @param [in]		oisSwapLVB		A label value block defining the ois swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid
*  @return			OIS PV
*/
double meProductOISPV(const std::vector<std::vector <std::string> >& oisSwapLVB, 
						bool validateKeys)
{
    MLIB_API_START
	double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(oisSwapLVB);
		
	ret = validation_api::tryMeProductOISPV(lvb, validateKeys);
	return ret;
    MLIB_API_END
}

/* @brief			return a set of expected keys for swap pv label value block
*  @return			expected keys
*/
std::vector<std::string> meProductOISPVLVBKeys()
{
    MLIB_API_START
	std::vector<std::string> ret;
	ret = validation_api::tryMeProductOISPVLVBKeys();
	return ret;
    MLIB_API_END
}
