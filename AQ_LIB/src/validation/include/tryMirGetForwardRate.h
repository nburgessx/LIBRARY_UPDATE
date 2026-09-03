#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation
{

    /* @brief			validation interface for mrGetForwardRate1
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		fromDateVec	    A vector of from dates
    *  @param [in]		term			Term
    *  @param [in]		curveID			ID of the yield curve
    *  @param [in]		frequency		Frequency of the yield curve
    *  @param [in]		dayCount		Day count convention of the yield curve
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar		Calendar of the yield curve
    *  @param [in]		interpolation	Interpolation method of the yield curve
    *  @param [in]		curveName		Type of the yield curve, default to STD
    *  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
    *  @return			The forward rates based on fromDates and term
    */
    DoubleVector tryMirGetForwardRate1( LADataInstance* dataInstance,
                                        const DateVector& fromDateVec,
                                        double term,
                                        const LAString& curveId,
                                        const LAString& frequency,
                                        const LAString& dayCount,
                                        const LAString& slidingRule,
                                        const LAString& calendar,
                                        const LAString& interpolation,
                                        const LAString& curveName,
                                        bool isFwdInterp = false );

    /* @brief			validation interface for mrGetForwardRate2
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		toDates			A vector of to dates
    *  @param [in]		curveID			ID of the yield curve
    *  @param [in]		frequency		Frequency of the yield curve
    *  @param [in]		dayCount		Day count convention of the yield curve
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar		Calendar of the yield curve
    *  @param [in]		interpolation	Interpolation method of the yield curve
    *  @param [in]		curveName		Type of the yield curve, default to STD
    *  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
    *  @param [in]		useFwdData		Use False to imply forwards from Discount Factors and True to use Forward Data directly. It's default to False.
    *  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryMirGetForwardRate2( LADataInstance* dataInstance,
                                        const DateVector& fromDates,
                                        const DateVector& toDates,
                                        const LAString& curveId,
                                        const LAString& frequency,
                                        const LAString& dayCount,
                                        const LAString& slidingRule,
                                        const LAString& calendar,
                                        const LAString& interpolation,
                                        const LAString& curveName,
                                        bool isFwdInterp,
                                        bool useFwdData );

}

