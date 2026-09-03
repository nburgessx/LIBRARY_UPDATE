#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
{

    /* @brief			validation interface for the tryMirGetParRate4 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		effectDt	The effective start date of the swap, ie, base date + spot date
    *  @param [in]		maturity		Swap maturity date or tenor
    *  @param [in]		curveID			ID of the curve set
    *  @param [in]		fixedFrequency			Fixed leg frequency
    *  @param [in]		fixedDaycount		Fixed leg day count convention
    *  @param [in]		fixedBusinessDayAdjustment		Fixed leg roll convention
    *  @param [in]		fixedCalendar		Fixed leg calendar
    *  @param [in]		fixedFirstStub		End date of the front stub period on fixed leg
    *  @param [in]		fixedLastStub		Start date of the end stub period on fixed leg
    *  @param [in]		fixedRollDay		Fixed leg rolling date
    *  @param [in]		fixedPayLag			Fixed leg payment date lag
    *  @param [in]		fixedStubType			Fixed leg Stub type
    *  @param [in]		floatFrequency			Floating leg frequency
    *  @param [in]		floatDayCount		Floating leg day count convention
    *  @param [in]		floatBusinessDayAdjustment		Floating leg rolling convention
    *  @param [in]		floatCalendar		Floating leg calendar
    *  @param [in]		tFistStub		End date of the front stub period on floating leg
    *  @param [in]		floatLastStub		Start date of the end stub period on floating leg
    *  @param [in]		floatRollDay		Floating leg rolling day
    *  @param [in]		floatFixingLag			Floating leg fixing day lag
    *  @param [in]		tFistFix		Floating leg first fixing
    *  @param [in]		floatLastFixing		Floating leg last fixing
    *  @param [in]		floatPayLag			Floating leg payment date lag
    *  @param [in]		floatStubType			Floating leg stub type
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		forecastCurve	Forecasting curve
    *  @param [in]		discountCurve	Discounting curve
    *  @param [in]		interpFwds		Boolean that decides if direct interpolation on fwd rates is employed
    *  @param [in]		eomRoll			Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
    *  @param [in]		floatSpread			Floating leg spread
    *  @param [in]		useFwdData		Use fwd rates directly or derive them from spot rates
    *  @param [in]		floatFixingCalendar		Optional float fixing calendar - needed by the server team to price USD swaps
    */
    double tryMirGetParRate4( AQLDataInstance* dataInstance,
                              const AQLString& effectDt,
                              const AQLString& maturity,
                              const AQLString& curveID,
                              const AQLString& fixedFrequency,
                              const AQLString& fixedDaycount,
                              const AQLString& fixedBusinessDayAdjustment,
                              const AQLString& fixedCalendar,
                              const AQLString& fixedFirstStub,
                              const AQLString& fixedLastStub,
                              const AQLString& fixedRollDay,
                              const AQLString& fixedPayLag,
                              const AQLString& fixedStubType,
                              const AQLString& floatFrequency,
                              const AQLString& floatDayCount,
                              const AQLString& floatBusinessDayAdjustment,
                              const AQLString& floatCalendar,
                              const AQLString& floatFirstStub,
                              const AQLString& floatLastStub,
                              const AQLString& floatRollDay,
                              const AQLString& floatFixingLag,
                              double floatFirstFixing,
                              double floatLastFixing,
                              const AQLString& floatPayLag,
                              const AQLString& floatStubType,
                              const AQLString& interpolation,
                              const AQLString& forecastCurve,
                              const AQLString& discountCurve,
                              bool interpFwds,
                              bool eomRoll,
                              double floatSpread,
                              bool useFwdData,
                              const AQLString& floatFixingCalendar = AQLString("") ); // Optional Parameter: Needed by ther server team to price USD swaps


    /* @brief			validation interface for the tryMirGetParRate3 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		frequency		Frequency
    *  @param [in]		dayCount		Day count convention
    *  @param [in]		slidingRule		Sliding rule
    *  @param [in]		calendar		Calendar
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		forecastCurve	Forecasting curve
    *  @param [in]		discountCurve	Discounting curve
    *  @param [in]		firstOddDate	End date of the front stub period
    *  @param [in]		lastOddDate		Start date of the end stub period
    *  @param [in]		paymentDay		PaymentDay
    *  @param [in]		forecastCurve	Forecasting curve
    *  @param [in]		discountCurve	Discounting curve
    *  @param [in]		interpFwds		Boolean that decides if direct interpolation on fwd rates is employed
    *  @param [in]		eomRoll			Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
    *  @return			par rate
    */
    double tryMirGetParRate3( AQLDataInstance* dataInstance,
                              const AQLString& fromDate,
                              const AQLString& toDate,
                              const AQLString& curveId,
                              const AQLString& frequency,
                              const AQLString& dayCount,
                              const AQLString& slidingRule,
                              const AQLString& calendar,
                              const AQLString& interpolation,
                              const AQLString& firstOddDate,
                              const AQLString& lastOddDate,
                              const AQLString& paymentDay,
                              const AQLString& foreCurveName,
                              const AQLString& dfCurveName,
                              bool interpFwds,
                              bool eomRoll );
}

