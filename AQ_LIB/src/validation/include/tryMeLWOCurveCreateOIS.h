#pragma once

#include <string>
#include "LACoreTemplateType.h"

namespace validation
{


    /* @brief			validation interface for the meLWOCurveCreateOIS method
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		oisConv				The OIS curve configuration info
    *  @param [in]		oisRates			Constituent OIS instrument rates
    *  @param [in]		oisHistoricalRates	Historical OIS fixings
    *  @param [in]		liborOisBasisConv	Libor-OIS swap conventions
    *  @param [in]		liborOisBasisRates	Libor-OIS basis spreads
    *  @param [in]		swapConv			Libor swap conventions
    *  @param [in]		swapRates			Libor swap market rates
    */
    const LAString tryMeLWOCurveCreateOIS( const std::string& lwoCurveName,
                                           const LAString& curveCollectionInput,
                                           const LAString& staticDataTableInput,
                                           const LAString& curveIndexInput,
                                           const LAStringMatrix& curveConv,
                                           const LAStringMatrix& oisConv,
                                           const LAStringMatrix& oisRates,
                                           const LAStringMatrix& oisHistoricalRates,
                                           const LAStringMatrix& liborOisBasisConv,
                                           const LAStringMatrix& liborOisBasisRates,
                                           const LAStringMatrix& swapConv,
                                           const LAStringMatrix& swapRates );

}

