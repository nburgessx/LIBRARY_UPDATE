#pragma once

#include "AQLDataInstance.h"

namespace validation
{
    /* @brief			validation interface for the mirGetAnnuity1 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		frequency		Frequency
    *  @param [in]		slidingRule		Sliding rule
    *  @param [in]		calendar		Calendar
    *  @param [in]		firstOddDate	End date of the front stub period
    *  @param [in]		lastOddDate		Start date of the end stub period
    *  @param [in]		paymentDay		PaymentDay
    *  @param [in]		dayCount		Day count convention
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		curveName		Name of the curve set
    *  @return			Annuity
    */
    double tryMirGetAnnuity1( AQLDataInstance* dataInstance,
                              const AQLDate& fromDate,
                              const AQLDate& toDate,
                              const AQLString& curveId,
                              const AQLString& frequency,
                              const AQLString& slidingRule,
                              const AQLString& calendar,
                              const AQLString& firstOddDate,
                              const AQLString& lastOddDate,
                              const AQLString& paymentDay,
                              const AQLString& dayCount,
                              const AQLString& interpolation,
                              const AQLString& curveName );

    /* @brief			validation interface for the mirGetAnnuity2 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		dates			A vector of the payment dates
    *  @param [in]		dayCount		Day count convention
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		curveName		Name of the curve set
    *  @return			Annuity
    */
    double tryMirGetAnnuity2( AQLDataInstance* dataInstance,
                              const AQLString& curveId,
                              const DateVector& dates,
                              const AQLString& dayCount,
                              const AQLString& interpolation,
                              const AQLString& curveName );

}
