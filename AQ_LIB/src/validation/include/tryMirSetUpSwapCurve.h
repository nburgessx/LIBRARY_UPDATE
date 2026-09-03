#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include <stdarg.h>

namespace validation
{

    /* @brief			swig interface for mirSetUpOISCurve
    *  @param [in]		dataInstance				Pointer to the object pool
    *  @param [in]		curveID				The curve collection ID
    *  @param [in]		marketName			Name of the curve constructed by this method
    *  @param [in]		generalProps		General yield curve data
    *  @param [in]		moneyConv			Money market conventions
    *  @param [in]		liborRates			Libor market data
    *  @param [in]		liborConv			Libor market conventions
    *  @param [in]		swapConv			Swap market conventions
    *  @param [in]		swapConv			Swap market data
    *  @param [in]		FRAData				FRA market conventions
    *  @param [in]		fra3mRates			3M FRA market data
    *  @param [in]		fra6mRates			6M FRA market data
    *  @param [in]		futureRates			Futures market data
    *  @param [in]		futureConv			Futures market conventions
    *  @param [in]		adjustSwapConv		Basis swaps market conventions
    *  @param [in]		adjustSwapRates		Basis swaps market data
    *  @param [in]		curveNames			Equivalent names of the curve being built
    *  @param [in]		curveName_DF2		Trade discount curve
    */
    const LAString tryMirSetUpSwapCurve( LADataInstance* dataInstance,
                                         const LAString& curveID,
                                         const LAString& marketName,
                                         const LAStringMatrix& generateProp,
                                         const LAStringMatrix& moneyConv,
                                         const LAStringMatrix& liborRates,
                                         const LAStringMatrix& liborConv,
                                         const LAStringMatrix& swapRates,
                                         const LAStringMatrix& swapConv,
                                         const LAStringMatrix& fra3mRates,
                                         const LAStringMatrix& fra6mRates,
                                         const LAStringMatrix& fraConv,
                                         const LAStringMatrix& futureRates,
                                         const LAStringMatrix& futureConv,
                                         const LAStringMatrix& adjustSwapConv,
                                         const LAStringMatrix& adjustSwapRates,
                                         const LAString& curveNames,
                                         const LAString& curveName_DF2 );

}
