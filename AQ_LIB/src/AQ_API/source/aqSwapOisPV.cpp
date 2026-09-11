#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"

#include "aqSwapOisPV.h"
#include "tryAqSwapOisPV.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqSwapOisPV
*  @param [in]		oisSwapLVB		A label value block defining the ois swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid
*  @return			OIS PV
*/
double aqSwapOisPV(const std::vector<std::vector <std::string> >& oisSwapLVB, 
						bool validateKeys)
{
    AQ_API_START
	double ret;
	
	// marshall all inputs		
	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(oisSwapLVB);
		
	ret = validation::tryAqSwapOisPV(lvb, validateKeys);
	return ret;
    AQ_API_END
}

/* @brief			return a set of expected keys for swap pv label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapOisPVLVBKeys()
{
    AQ_API_START
	std::vector<std::string> ret;
	ret = validation::tryAqSwapOisPVLVBKeys();
	return ret;
    AQ_API_END
}
