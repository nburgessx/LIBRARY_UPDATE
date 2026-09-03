/*
 * @brief			validation interface for the meCurveCalibrateBasis method
 * @Created:		19 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
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
    const LAString tryMeCurveCalibrateBasis( const LAString& curveCollection,
            const LAString& staticDataTable,
            const LAString& curveIndex,
            const LAStringMatrix& curveConv,
            const LAStringMatrix& basisConv,
            const LAStringMatrix& basisRates,
            const LAStringMatrix& fxFwdConv,
            const LAStringMatrix& fxFwdRates,
            const LAStringMatrix& spotFxRates,
			const LAStringMatrix& fraConv = LAStringMatrix(),
			const LAStringMatrix& fraRates = LAStringMatrix(),
			const LAStringMatrix& liborConv = LAStringMatrix(),
			const LAStringMatrix& liborRates = LAStringMatrix());

}
