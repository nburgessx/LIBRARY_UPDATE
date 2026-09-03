#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
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
    const AQLString tryMirSetUpOISCurve( AQLDataInstance* dataInstance,
                                        const AQLString& curveID,
                                        const AQLString& marketName,
                                        const AQLStringMatrix& generalProps,
                                        const AQLStringMatrix& oisRates,
                                        const AQLStringMatrix& oisConv,
                                        const AQLString& curveNames,
                                        const AQLStringMatrix& oisHistRates,
                                        const AQLStringMatrix& loBasisRates,
                                        const AQLStringMatrix& loBasisConv,
                                        const AQLStringMatrix& swapRates,
                                        const AQLStringMatrix& swapConv );

}

