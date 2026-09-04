#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{

    /* @brief			return a set of expected keys for swap stub rate label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryAqSwapStubRateLVBKeys();

    /* @brief			validation interface for the aqSwapStubRate method
    *  @param [in]		swapLVB				A label value block defining the swap
	*  @param [in]		curveIndices		A list of curves to interpolate from
    *  @param [in]		curveTenors			A list of tenors corresponding to curve indexes
    *  @param [in]		tenorCurveFixings	A list of fixings corresponding to curve indexes
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
    *  @return			Swap stub rate
    */
    double tryAqSwapStubRate( const LabelValueBlock& swapLVB,
									 const AQLStringVector& curveIndices = AQLStringVector(),
									 const AQLStringVector& curveTenors = AQLStringVector(),
                                     const DoubleVector& tenorCurveFixings = DoubleVector(),
                                     bool validateKeys = true );

    /* @brief			validation interface for the aqSwapStubFixingDate method
     *  @param [in]		swapLVB				A label value block defining the swap
	*  @param [in]		curveIndices		A list of curves to interpolate from
    *  @param [in]		curveTenors			A list of tenors corresponding to curve indexes
    *  @param [in]		tenorCurveFixings	A list of fixings corresponding to curve indexes
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
    *  @return			Swap stub rate
    */
    AQLDate tryAqSwapStubFixingDate(  const LabelValueBlock& swapLVB,
											const AQLStringVector& curveIndices = AQLStringVector(),
											const AQLStringVector& curveTenors = AQLStringVector(),
											const DoubleVector& tenorCurveFixings = DoubleVector(),
											bool validateKeys = true );

}

