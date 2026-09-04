#pragma once

#include <string>
#include "AQLCoreTemplateType.h"

namespace validation
{


    /* @brief			validation interface for the aqObjCurvesCreateOIS method
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
    const AQLString tryAqObjCurvesCreateOIS( const std::string& aqoCurveName,
                                           const AQLString& curveCollectionInput,
                                           const AQLString& staticDataTableInput,
                                           const AQLString& curveIndexInput,
                                           const AQLStringMatrix& curveConv,
                                           const AQLStringMatrix& oisConv,
                                           const AQLStringMatrix& oisRates,
                                           const AQLStringMatrix& oisHistoricalRates,
                                           const AQLStringMatrix& liborOisBasisConv,
                                           const AQLStringMatrix& liborOisBasisRates,
                                           const AQLStringMatrix& swapConv,
                                           const AQLStringMatrix& swapRates );

}

