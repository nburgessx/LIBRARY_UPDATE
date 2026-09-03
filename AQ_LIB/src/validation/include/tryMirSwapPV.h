#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
{


    /* @brief			validation interface for the mirSwapPV method
    *  @param [in]		dataInstance			                Pointer to the object pool
    *  @param [in]		notional		                Swap notional
    *  @param [in]		payRec			                Payer swap or Receiver swap
    *  @param [in]		effectDt		                The effective start date of the swap, ie, base date + spot date
    *  @param [in]		maturity		                Swap maturity date or tenor
    *  @param [in]		curveID			                ID of the curve set
    *  @param [in]		fixedRate				        Swap fixed leg rate
    *  @param [in]		fixedFrequency			        Fixed leg frequency
    *  @param [in]		fixedDaycount		            Fixed leg day count convention
    *  @param [in]		fixedBusinessDayAdjustment		Fixed leg roll convention
    *  @param [in]		fixedCalendar		            Fixed leg calendar
    *  @param [in]		fixedFirstStub		            End date of the front stub period on fixed leg
    *  @param [in]		fixedLastStub		            Start date of the end stub period on fixed leg
    *  @param [in]		fixedRollDay		            Fixed leg rolling date
    *  @param [in]		fixedPayLag			            Fixed leg payment date lag
    *  @param [in]		fixedStubType			        Fixed leg Stub type
    *  @param [in]		floatFrequency			        Floating leg frequency
    *  @param [in]		floatDayCount		            Floating leg day count convention
    *  @param [in]		floatBusinessDayAdjustment		Floating leg rolling convention
    *  @param [in]		floatCalendar		            Floating leg calendar
    *  @param [in]		tFistStub		                End date of the front stub period on floating leg
    *  @param [in]		floatLastStub		            Start date of the end stub period on floating leg
    *  @param [in]		floatRollDay		            Floating leg rolling day
    *  @param [in]		floatFixingLag			        Floating leg fixing day lag
    *  @param [in]		tFistFix		                Floating leg first fixing
    *  @param [in]		floatLastFixing		            Floating leg last fixing
    *  @param [in]		floatPayLag			            Floating leg payment date lag
    *  @param [in]		floatStubType			        Floating leg stub type
    *  @param [in]		interpolation	                Interpolation method
    *  @param [in]		forecastCurve	                Forecasting curve
    *  @param [in]		discountCurve	                Discounting curve
    *  @param [in]		interpFwds		                Boolean that decides if direct interpolation on fwd rates is employed
    *  @param [in]		eomRoll			                Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
    *  @param [in]		floatSpread			            Floating leg spread
    */
    double tryMirSwapPV( AQLDataInstance* dataInstance,
                         double notional,
                         const AQLString& payRec,
                         const AQLString& effectDt,
                         const AQLString& maturity,
                         const AQLString& curveID,
                         double fixedRate,
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
                         double floatSpread );
}

