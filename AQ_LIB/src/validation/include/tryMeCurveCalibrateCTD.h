#pragma once

#include "LACoreTemplateType.h"

namespace validation
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

