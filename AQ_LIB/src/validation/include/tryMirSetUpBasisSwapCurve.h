#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
{

    /* @brief			validation interface for the SetUpBasisSwapCurve method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		curveID			The curve collection ID
    *  @param [in]		marketName		Name of the curve constructed by this method
    *  @param [in]		generalProps	General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		basisConv		Basis swap conventions
    *  @param [in]		basisRates		Basis swap market rates
    *  @param [in]		curveNames		Equivalent names of the curve being built
    *  @param [in]		fwdConv			FX forward market convention
    *  @param [in]		fwdFXs			Forward FX rates
    *  @param [in]		spotFXs			Spot FX rates
	*  @param [in]		fraConv			FRA Conventions
	*  @param [in]		fraRates		FRA market quotes
    */
    const AQLString tryMirSetUpBasisSwapCurve( AQLDataInstance* dataInstance,
            const AQLString& curveID,
            const AQLString& marketName,
            const AQLStringMatrix& generalProps,
            const AQLStringMatrix& basisConv,
            const AQLStringMatrix& basisRates,
            const AQLString& curveNames,
            const AQLStringMatrix& fwdConv,
            const AQLStringMatrix& fwdFXs,
            const AQLStringMatrix& spotFXs,
			const AQLStringMatrix& fraConv = AQLStringMatrix(),
			const AQLStringMatrix& fraRates = AQLStringMatrix());
}
