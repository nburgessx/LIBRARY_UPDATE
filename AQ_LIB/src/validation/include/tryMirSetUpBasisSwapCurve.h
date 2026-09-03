#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
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
    const LAString tryMirSetUpBasisSwapCurve( LADataInstance* dataInstance,
            const LAString& curveID,
            const LAString& marketName,
            const LAStringMatrix& generalProps,
            const LAStringMatrix& basisConv,
            const LAStringMatrix& basisRates,
            const LAString& curveNames,
            const LAStringMatrix& fwdConv,
            const LAStringMatrix& fwdFXs,
            const LAStringMatrix& spotFXs,
			const LAStringMatrix& fraConv = LAStringMatrix(),
			const LAStringMatrix& fraRates = LAStringMatrix());
}
