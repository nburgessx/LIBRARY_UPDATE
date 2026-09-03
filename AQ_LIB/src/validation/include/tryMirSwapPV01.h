/*
 * @brief			validation interface for swap PV01 method(s)
 * @Created:		30 March 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
{

    /* @brief			validation interface for the tryMirSwapPV01 method
    *  @param [in]		dataInstance			                Pointer to the object pool
    *  @param [in]		notional		                Swap notional
    *  @param [in]		payRec			                Payer swap or Receiver swap
    *  @param [in]		effectDt	                    The effective start date of the swap, ie, base date + spot date
    *  @param [in]		maturity		                Swap maturity date or tenor
    *  @param [in]		curveId			                ID of the curve set
    *  @param [in]		fixedFrequency			        Fixed leg frequency
    *  @param [in]		fixedDaycount		            Fixed leg day count convention
    *  @param [in]		fixedBusinessDayAdjustment		Fixed leg roll convention
    *  @param [in]		fixedCalendar		            Fixed leg calendar
    *  @param [in]		fixedFirstStub		            End date of the front stub period on fixed leg
    *  @param [in]		fixedLastStub		            Start date of the end stub period on fixed leg
    *  @param [in]		fixedRollDay		            Fixed leg rolling date
    *  @param [in]		fixedPayLag			            Fixed leg payment date lag
    *  @param [in]		fixedStubType			        Fixed leg Stub type
    *  @param [in]		tSpread			                Floating Leg spread in basis points. This param is not used in the core functions.
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
    *  @param [in]		interpFwds		                Boolean that decides if direct interpolation on fwd rates is employed. This param is not used in the core functions.
    *  @param [in]		eomRoll			                Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
    *  @return			swap pv01
    */
    double tryMirSwapPV01( LADataInstance* dataInstance,
                           double notional,
                           const LAString& payRec,
                           const LAString& effectDt,
                           const LAString& maturity,
                           const LAString& curveId,
                           const LAString& fixedFrequency,
                           const LAString& fixedDaycount,
                           const LAString& fixedBusinessDayAdjustment,
                           const LAString& fixedCalendar,
                           const LAString& fixedFirstStub,
                           const LAString& fixedLastStub,
                           const LAString& fixedRollDay,
                           const LAString& fixedPayLag,
                           const LAString& fixedStubType,
                           double tSpread, // this param is not used
                           const LAString& floatFrequency,
                           const LAString& floatDayCount,
                           const LAString& floatBusinessDayAdjustment,
                           const LAString& floatCalendar,
                           const LAString& floatFirstStub,
                           const LAString& floatLastStub,
                           const LAString& floatRollDay,
                           const LAString& floatFixingLag,
                           double floatFirstFixing,
                           double floatLastFixing,
                           const LAString& floatPayLag,
                           const LAString& floatStubType,
                           const LAString& interpolation,
                           const LAString& forecastCurve,
                           const LAString& discountCurve,
                           bool interpFwds, // this param is not used
                           bool eomRoll );

}

