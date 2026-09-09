#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
    /* @brief			validation method for aqCurveCalibrateCTD
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		curveName			Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		collateralCurves	The group of collateral curves out of which the CTD curve is constructed
    */
    AQLString tryAqCurveCalibrateCTD( const AQLString& curveCollection,
                                    const AQLString& curveName,
                                    const AQLString& curveIndexInput,
                                    const AQLStringMatrix& curveConv,
                                    const AQLStringVector& collateralCurves );
}

