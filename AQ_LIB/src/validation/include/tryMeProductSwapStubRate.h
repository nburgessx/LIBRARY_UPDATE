/*
 * @brief			validation interface for the meProductSwapStubRate
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation_api
{

    /* @brief			return a set of expected keys for swap stub rate label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductSwapStubRateLVBKeys();

    /* @brief			validation interface for the meProductSwapStubRate method
    *  @param [in]		swapLVB				A label value block defining the swap
	*  @param [in]		curveIndices		A list of curves to interpolate from
    *  @param [in]		curveTenors			A list of tenors corresponding to curve indexes
    *  @param [in]		tenorCurveFixings	A list of fixings corresponding to curve indexes
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
    *  @return			Swap stub rate
    */
    double tryMeProductSwapStubRate( const LabelValueBlock& swapLVB,
									 const LAStringVector& curveIndices = LAStringVector(),
									 const LAStringVector& curveTenors = LAStringVector(),
                                     const DoubleVector& tenorCurveFixings = DoubleVector(),
                                     bool validateKeys = true );

    /* @brief			validation interface for the meProductSwapStubFixingDate method
     *  @param [in]		swapLVB				A label value block defining the swap
	*  @param [in]		curveIndices		A list of curves to interpolate from
    *  @param [in]		curveTenors			A list of tenors corresponding to curve indexes
    *  @param [in]		tenorCurveFixings	A list of fixings corresponding to curve indexes
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
    *  @return			Swap stub rate
    */
    LADate tryMeProductSwapStubFixingDate(  const LabelValueBlock& swapLVB,
											const LAStringVector& curveIndices = LAStringVector(),
											const LAStringVector& curveTenors = LAStringVector(),
											const DoubleVector& tenorCurveFixings = DoubleVector(),
											bool validateKeys = true );

}

