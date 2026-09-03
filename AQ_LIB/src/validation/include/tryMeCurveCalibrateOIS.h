/*
 * @brief			validation interface for the meCurveCalibrateOIS method
 * @Created:		19 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#pragma once

#include <string>
#include "LACoreTemplateType.h"

namespace validation_api
{


    /* @brief			validation interface for the meCurveCalibrateOIS method
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

    const LAString tryMeCurveCalibrateOIS( const LAString& curveCollection,
                                           const LAString& staticDataTable,
                                           const LAString& curveIndex,
                                           const LAStringMatrix& curveConv,
                                           const LAStringMatrix& oisConv,
                                           const LAStringMatrix& oisRates,
                                           const LAStringMatrix& oisHistoricalRates,
                                           const LAStringMatrix& liborOisBasisConv,
                                           const LAStringMatrix& liborOisBasisRates,
                                           const LAStringMatrix& swapConv,
                                           const LAStringMatrix& swapRates );
}
