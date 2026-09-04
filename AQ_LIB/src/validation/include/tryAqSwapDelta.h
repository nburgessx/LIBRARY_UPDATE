#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{
    /* @brief			validation interface for the aqSwapsDelta method
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
    void tryAqSwapDelta(
        AQLStringVector& pillarNames,
        AQLStringVector& headers,
        DoubleMatrix& deltas,
        AQLDataInstance* dataInstance,
        const std::vector<LabelValueBlock>& dealInfo,
        const AQLStringVector& forecastCurveSet,
        const AQLString& curveCollection,
        double bumpSize,
        const AQLString& bumpMode,
        const AQLString& deltaType,
        bool aggregateRisk );
}

