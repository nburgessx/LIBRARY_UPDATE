#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"

#include "aqSwapStubRate.h"
#include "tryAqSwapStubRate.h"
#include "DateUtilities.h"          // etrading::toYYYYMMDDFromDate - date to string converter
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqSwapStubRate
*  @param [in]		curveIndices		A list of curves to interpolate from
*  @param [in]		curveTenors			A list of tenors corresponding to curve indexes
*  @param [in]		tenorCurveFixings	A list of fixings corresponding to curve indexes
*  @param [in]		swapLVB				A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap stub rate
*/
double aqSwapStubRate(const std::vector<std::string>& curveIndices,
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
		
	ret = validation::tryAqSwapStubRate( lvb, tmp_curveIndices, tmp_curveTenors, tenorCurveFixings, validateKeys);

	return ret;
    AQ_API_END
}

/* @brief			return a set of expected keys for swap stub rate label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapStubRateLVBKeys()
{
	AQ_API_START
    std::vector<std::string> ret;
	ret = validation::tryAqSwapStubRateLVBKeys();
	return ret;
    AQ_API_END
}

/* @brief			swig interface for aqSwapStubFixingDate
*  @param [in]		curveIndices		A list of curves to interpolate from
*  @param [in]		curveTenors			A list of tenors corresponding to curve indexes
*  @param [in]		tenorCurveFixings	A list of fixings corresponding to curve indexes
*  @param [in]		swapLVB				A label value block defining the swap
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap stub fixing date (YYYYMMDD)
*/
std::string aqSwapStubFixingDate( const std::vector<std::string>& curveIndices,
                               const std::vector<std::string>& curveTenors,
                               const std::vector<double>& tenorCurveFixings,
                               const std::vector<std::vector <std::string> >& swapLVB,
                               bool validateKeys )
{
	AQ_API_START

	// marshall all inputs
	AQLStringVector tmp_curveIndices;
	swig::buildStringVector(tmp_curveIndices, curveIndices);

	AQLStringVector tmp_curveTenors;
	swig::buildStringVector(tmp_curveTenors, curveTenors);

	LabelValueBlock lvb = swig::buildSingleLabelValueBlock(swapLVB);

	AQLDate result = validation::tryAqSwapStubFixingDate( lvb, tmp_curveIndices, tmp_curveTenors, tenorCurveFixings, validateKeys);

	return etrading::toYYYYMMDDFromDate( result );
    AQ_API_END
}
