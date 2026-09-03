#pragma once

#include "LACoreTemplateType.h"
#include <stdarg.h>

namespace validation
{

    /* @brief			validation interface for meCurveCalibrateSwap
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
    const LAString tryMeCurveCalibrateSwap( const LAString& curveCollection,
                                            const LAString& staticDataTable,
                                            const LAString& curveIndex,
                                            const LAStringMatrix& curveConv,
                                            const LAStringMatrix& moneyMarketConv,
                                            const LAStringMatrix& liborConv,
                                            const LAStringMatrix& liborRates,
                                            const LAStringMatrix& swapConv,
                                            const LAStringMatrix& swapRates,
                                            const LAStringMatrix& fraConv,
                                            const LAStringMatrix& fra3mRates,
                                            const LAStringMatrix& fra6mRates,
                                            const LAStringMatrix& futureConv,
                                            const LAStringMatrix& futureRates,
                                            const LAStringMatrix& tenorBasisConv,
                                            const LAStringMatrix& tenorBasisRates );
    
}
