#pragma once

#include "LADataInstance.h"

namespace validation_api
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
    double tryMirGetAnnuity1( LADataInstance* dataInstance,
                              const LADate& fromDate,
                              const LADate& toDate,
                              const LAString& curveId,
                              const LAString& frequency,
                              const LAString& slidingRule,
                              const LAString& calendar,
                              const LAString& firstOddDate,
                              const LAString& lastOddDate,
                              const LAString& paymentDay,
                              const LAString& dayCount,
                              const LAString& interpolation,
                              const LAString& curveName );

    /* @brief			validation interface for the mirGetAnnuity2 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		dates			A vector of the payment dates
    *  @param [in]		dayCount		Day count convention
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		curveName		Name of the curve set
    *  @return			Annuity
    */
    double tryMirGetAnnuity2( LADataInstance* dataInstance,
                              const LAString& curveId,
                              const DateVector& dates,
                              const LAString& dayCount,
                              const LAString& interpolation,
                              const LAString& curveName );

}
