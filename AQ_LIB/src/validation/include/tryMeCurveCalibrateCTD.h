/*
 * @brief			validation interface for the meCurveCalibrateCTD method
 * @Created:		10 July 2017
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
    /* @brief			validation method for meCurveCalibrateCTD
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		curveName			Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		collateralCurves	The group of collateral curves out of which the CTD curve is constructed
    */
    LAString tryMeCurveCalibrateCTD( const LAString& curveCollection,
                                    const LAString& curveName,
                                    const LAString& curveIndexInput,
                                    const LAStringMatrix& curveConv,
                                    const LAStringVector& collateralCurves );
}

