#pragma once

#include "LACoreTemplateType.h"
#include <stdarg.h>

namespace validation_api
{

    /* @brief			validation interface for meLWOCurveCreateSwap
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
    *  @param [in]		convexityAdjConv	Convexity adjustment market conventions
    *  @param [in]		convexityAdjRates	Convexity adjustment market data
    */
    const LAString tryMeLWOCurveCreateSwap( const std::string& lwoCurveName,
                                            const LAString& curveCollectionInput,
                                            const LAString& staticDataTableInput,
                                            const LAString& curveIndexInput,
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
                                            const LAStringMatrix& convexityAdjConv,
                                            const LAStringMatrix& convexityAdjRates );
}
