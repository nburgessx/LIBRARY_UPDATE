#pragma once

#include "LACoreTemplateType.h"

namespace validation
{

    /* @brief			validation interface for the meLWOCurveCreateBasis method
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		basisConv			Basis swap conventions
    *  @param [in]		basisRates			Basis swap market rates
    *  @param [in]		fxFwdConv			FX forward market convention
    *  @param [in]		fxFwdRates			Forward FX rates
    *  @param [in]		spotFxRates			Spot FX rates
    */
    const LAString tryMeLWOCurveCreateBasis( const std::string& lwoCurveName,
            const LAString& curveCollectionInput,
            const LAString& staticDataTableInput,
            const LAString& curveIndexInput,
            const LAStringMatrix& curveConv,
            const LAStringMatrix& basisConv,
            const LAStringMatrix& basisRates,
            const LAStringMatrix& fxFwdConv,
            const LAStringMatrix& fxFwdRates,
            const LAStringMatrix& spotFxRates );
}
