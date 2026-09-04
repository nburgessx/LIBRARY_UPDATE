#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{

    /* @brief			validation interface for the aqObjectsCurveCreateBasis method
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
    const AQLString tryAqObjectsCurveCreateBasis( const std::string& aqoCurveName,
            const AQLString& curveCollectionInput,
            const AQLString& staticDataTableInput,
            const AQLString& curveIndexInput,
            const AQLStringMatrix& curveConv,
            const AQLStringMatrix& basisConv,
            const AQLStringMatrix& basisRates,
            const AQLStringMatrix& fxFwdConv,
            const AQLStringMatrix& fxFwdRates,
            const AQLStringMatrix& spotFxRates );
}
