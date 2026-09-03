#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
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
    const AQLString tryMirSetUpSwapCurve( AQLDataInstance* dataInstance,
                                         const AQLString& curveID,
                                         const AQLString& marketName,
                                         const AQLStringMatrix& generateProp,
                                         const AQLStringMatrix& moneyConv,
                                         const AQLStringMatrix& liborRates,
                                         const AQLStringMatrix& liborConv,
                                         const AQLStringMatrix& swapRates,
                                         const AQLStringMatrix& swapConv,
                                         const AQLStringMatrix& fra3mRates,
                                         const AQLStringMatrix& fra6mRates,
                                         const AQLStringMatrix& fraConv,
                                         const AQLStringMatrix& futureRates,
                                         const AQLStringMatrix& futureConv,
                                         const AQLStringMatrix& adjustSwapConv,
                                         const AQLStringMatrix& adjustSwapRates,
                                         const AQLString& curveNames,
                                         const AQLString& curveName_DF2 );

}
