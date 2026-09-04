#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"

#include "aqSwapsStubRate.h"
#include "tryAqSwapsStubRate.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqSwapsStubRate
*  @param [in]		curveIndices		A list of curves to interpolate from
*  @param [in]		curveTenors			A list of tenors corresponding to curve indexes
*  @param [in]		tenorCurveFixings	A list of fixings corresponding to curve indexes
*  @param [in]		swapLVB				A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap stub rate
*/
double aqSwapsStubRate(const std::vector<std::string>& curveIndices,
							const std::vector<std::string>& curveTenors,
							const std::vector<double>& tenorCurveFixings,
							const std::vector<std::vector <std::string> >& swapLVB, 
							bool validateKeys)
{
	AQ_API_START
    double ret;
	
	// marshall all inputs		
	AQLStringVector tmp_curveIndices;
	swig::buildStringVector(tmp_curveIndices, curveIndices);

	AQLStringVector tmp_curveTenors;
	swig::buildStringVector(tmp_curveTenors, curveTenors);

	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(swapLVB);
		
	ret = validation::tryAqSwapsStubRate( lvb, tmp_curveIndices, tmp_curveTenors, tenorCurveFixings, validateKeys);

	return ret;
    AQ_API_END
}

/* @brief			return a set of expected keys for swap stub rate label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapsStubRateLVBKeys()
{
	AQ_API_START
    std::vector<std::string> ret;
	ret = validation::tryAqSwapsStubRateLVBKeys();
	return ret;
    AQ_API_END
}
