#pragma once

#include "AQLCoreTemplateType.h"
#include <stdarg.h>

namespace validation
{

    /* @brief			validation interface for aqCurveCalibrateSwap
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General yield curve data
    *  @param [in]		moneyMarketConv		Money market conventions
    *  @param [in]		liborConv			Libor market conventions
    *  @param [in]		liborRates			Libor market data
    *  @param [in]		swapConv			Swap market conventions
    *  @param [in]		swapRates			Swap market data
    *  @param [in]		fraConv				FRA market conventions
    *  @param [in]		fra3mRates			3M FRA market data
    *  @param [in]		fra6mRates			6M FRA market data
    *  @param [in]		futureConv			Futures market conventions
    *  @param [in]		futureRates			Futures market data
    *  @param [in]		tenorBasisConv	    Tenor Basis market conventions
    *  @param [in]		tenorBasisRates	    Tenor Basis market data
    */
    const AQLString tryAqCurveCalibrateSwap( const AQLString& curveCollection,
                                            const AQLString& staticDataTable,
                                            const AQLString& curveIndex,
                                            const AQLStringMatrix& curveConv,
                                            const AQLStringMatrix& moneyMarketConv,
                                            const AQLStringMatrix& liborConv,
                                            const AQLStringMatrix& liborRates,
                                            const AQLStringMatrix& swapConv,
                                            const AQLStringMatrix& swapRates,
                                            const AQLStringMatrix& fraConv,
                                            const AQLStringMatrix& fra3mRates,
                                            const AQLStringMatrix& fra6mRates,
                                            const AQLStringMatrix& futureConv,
                                            const AQLStringMatrix& futureRates,
                                            const AQLStringMatrix& tenorBasisConv,
                                            const AQLStringMatrix& tenorBasisRates );
    
}
