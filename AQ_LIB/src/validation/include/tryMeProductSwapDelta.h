/*
 * @brief			validation interface for the meProductSwapDelta method
 * @Created:		11 April 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation_api
{
    /* @brief			validation interface for the meSwapDelta method
    *  @param [out]		pillarNames		    The final list of pillar names
    *  @param [out]		headers			    Headers of the delta report
    *  @param [out]		deltas			    All the deltas
    *  @param [in]		dataInstance			    Pointer to the object pool
    *  @param [in]		dealInfo		    A vector of label value blocks each defining a swap trade
    *  @param [in]		forecastCurveSet	Forecasting curve Set
    *  @param [in]		CurveCollection	    Curve Collection
    *  @param [in]		discountCurve	    Discounting curve
    *  @param [in]		bumpSize		    Bumping size. Default to 0.01 (basis point).
    *  @param [in]		bumpMode		    Bumping mode. Up, down, or central bumping.
    *  @param [in]		deltaType		    Type of deltas. Flat shift or ladder.
    *  @param [in]		aggregateRisk	    Whether to aggregate risks against the same market instruments from different curves
    */
    void tryMeProductSwapDelta(
        LAStringVector& pillarNames,
        LAStringVector& headers,
        DoubleMatrix& deltas,
        LADataInstance* dataInstance,
        const std::vector<LabelValueBlock>& dealInfo,
        const LAStringVector& forecastCurveSet,
        const LAString& curveCollection,
        double bumpSize,
        const LAString& bumpMode,
        const LAString& deltaType,
        bool aggregateRisk );
}

