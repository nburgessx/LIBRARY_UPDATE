#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{

    /* @brief			validation interface for the meCurveCalibrateBasis method
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		basisConv			Basis swap conventions
    *  @param [in]		basisRates			Basis swap market rates
    *  @param [in]		fxFwdConv			FX forward market convention
    *  @param [in]		fxFwdRates			Forward FX rates
    *  @param [in]		spotFxRates			Spot FX rates
	*  @param [in]		fraConv				FRA conventions
	*  @param [in]		fraRates			FRA market data
	*  @param [in]		liborConv			Libor instrument conventions
	*  @param [in]		liborRates			Libor market data
    */
    const AQLString tryMeCurveCalibrateBasis( const AQLString& curveCollection,
            const AQLString& staticDataTable,
            const AQLString& curveIndex,
            const AQLStringMatrix& curveConv,
            const AQLStringMatrix& basisConv,
            const AQLStringMatrix& basisRates,
            const AQLStringMatrix& fxFwdConv,
            const AQLStringMatrix& fxFwdRates,
            const AQLStringMatrix& spotFxRates,
			const AQLStringMatrix& fraConv = AQLStringMatrix(),
			const AQLStringMatrix& fraRates = AQLStringMatrix(),
			const AQLStringMatrix& liborConv = AQLStringMatrix(),
			const AQLStringMatrix& liborRates = AQLStringMatrix());

}
