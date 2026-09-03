#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
{


    /* @brief			validation interface for the SetUpOISCurve method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		curveID			The curve collection ID
    *  @param [in]		marketName		Name of the curve constructed by this method
    *  @param [in]		generalProps	General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		oisConv			The OIS curve configuration info
    *  @param [in]		oisRates		Constituent OIS instrument rates
    *  @param [in]		curveNames		Equivalent names of the curve being built
    *  @param [in]		oisHistRates	Historical OIS fixings
    *  @param [in]		loBasisRates	Libor-OIS basis spreads
    *  @param [in]		loBasisConv		Libor-OIS swap conventions
    *  @param [in]		swapRates		Libor swap market rates
    *  @param [in]		swapConv		Libor swap conventions
    */
    const LAString tryMirSetUpOISCurve( LADataInstance* dataInstance,
                                        const LAString& curveID,
                                        const LAString& marketName,
                                        const LAStringMatrix& generalProps,
                                        const LAStringMatrix& oisRates,
                                        const LAStringMatrix& oisConv,
                                        const LAString& curveNames,
                                        const LAStringMatrix& oisHistRates,
                                        const LAStringMatrix& loBasisRates,
                                        const LAStringMatrix& loBasisConv,
                                        const LAStringMatrix& swapRates,
                                        const LAStringMatrix& swapConv );

}

