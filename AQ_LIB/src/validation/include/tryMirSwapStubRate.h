#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation
{

    /* @brief			validation method for mirSwapStubRate
    *  @param [in]		dataInstance			    Pointer to the object pool
    *  @param [in]		effDt						The effective start date of the swap, ie, base date + spot date
    *  @param [in]		mat							Swap maturity date or tenor
    *  @param [in]		freq						Floating leg frequency
    *  @param [in]		dayCt						Floating leg day count convention
    *  @param [in]		busDayAdj					Floating leg business day adjustment
    *  @param [in]		cal							Floating leg calendar
    *  @param [in]		rollDayString				Floating leg rolling day
    *  @param [in]		fixLag						Floating leg fixing day lag
    *  @param [in]		eomRoll						Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
    *  @param [in]		interpolation				Interpolation method
    *  @param [in]		fistStub					End date of the front stub period on floating leg
    *  @param [in]		lastStub					Start date of the end stub period on floating leg
    *  @param [in]		stub						Floating leg stub type
    *  @param [in]		crvID						ID of the curve set
    *  @param [in]		curveNames					A list of curves to interpolate from
    *  @param [in]		curveTenors					A list of tenors corresponding to curve names
    *  @param [in]		tenorCurveFixings			A list of tenors corresponding to curve names
    *  @param [in]		useCurveName				A curve specifically chosen by user to use
    *  @param [in]		toleranceTenor				Tenor that defines if a nearby curve should be chosen for use
    *  @param [in]		isFwdInterp					Boolean that decides if direct interpolation on fwd rates is employed
    *  @param [in]		useFwdData					Use fwd rates directly or derive them from spot rates
    */
    double tryMirSwapStubRate( LADataInstance* dataInstance,
                               const LAString& effDt,
                               const LAString& mat,
                               const LAString& freq,
                               const LAString& dayCt,
                               const LAString& busDayAdj,
                               const LAString& cal,
                               const LAString& rollDayString,
                               const LAString& fixLag,
                               bool eomRoll,
                               const LAString& interpolation,
                               const LAString& firstStub,
                               const LAString& lastStub,
                               const LAString& stub,
                               const LAString& crvID,
                               const LAStringVector& curveNames,
                               const LAStringVector& curveTenors,
                               const DoubleVector& tenorCurveFixings,
                               const LAString& useCurveName,
                               const LAString& toleranceTenor,
                               bool isFwdInterp,
                               bool useFwdData );

	/* @brief			validation method for mirSwapStubFixingDate
    *  @param [in]		dataInstance			    Pointer to the object pool
    *  @param [in]		effDt						The effective start date of the swap, ie, base date + spot date
    *  @param [in]		mat							Swap maturity date or tenor
    *  @param [in]		freq						Floating leg frequency
    *  @param [in]		dayCt						Floating leg day count convention
    *  @param [in]		busDayAdj					Floating leg business day adjustment
    *  @param [in]		cal							Floating leg calendar
    *  @param [in]		rollDayString				Floating leg rolling day
    *  @param [in]		fixLag						Floating leg fixing day lag
    *  @param [in]		eomRoll						Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
    *  @param [in]		interpolation				Interpolation method
    *  @param [in]		fistStub					End date of the front stub period on floating leg
    *  @param [in]		lastStub					Start date of the end stub period on floating leg
    *  @param [in]		stub						Floating leg stub type
    *  @param [in]		crvID						ID of the curve set
    *  @param [in]		curveNames					A list of curves to interpolate from
    *  @param [in]		curveTenors					A list of tenors corresponding to curve names
    *  @param [in]		tenorCurveFixings			A list of tenors corresponding to curve names
    *  @param [in]		useCurveName				A curve specifically chosen by user to use
    *  @param [in]		toleranceTenor				Tenor that defines if a nearby curve should be chosen for use
    *  @param [in]		isFwdInterp					Boolean that decides if direct interpolation on fwd rates is employed
    *  @param [in]		useFwdData					Use fwd rates directly or derive them from spot rates
    */
    LADate tryMirSwapStubFixingDate( LADataInstance* dataInstance,
									 const LAString& effDt,
									 const LAString& mat,
									 const LAString& freq,
									 const LAString& dayCt,
									 const LAString& busDayAdj,
									 const LAString& cal,
									 const LAString& rollDayString,
									 const LAString& fixLag,
									 bool eomRoll,
									 const LAString& interpolation,
									 const LAString& firstStub,
									 const LAString& lastStub,
									 const LAString& stub,
									 const LAString& crvID,
									 const LAStringVector& curveNames,
									 const LAStringVector& curveTenors,
									 const DoubleVector& tenorCurveFixings,
									 const LAString& useCurveName,
									 const LAString& toleranceTenor,
									 bool isFwdInterp,
									 bool useFwdData );
}

