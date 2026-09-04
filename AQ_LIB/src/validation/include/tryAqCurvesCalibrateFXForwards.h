#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
    /* @brief			validation method for aqCurvesCalibrateFXForwards
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		fxFwdConv			FX forward conventions
    */
    AQLString tryAqCurvesCalibrateFXForwards( const AQLString& curveCollection,
                                            const AQLString& staticDataTable,
                                            const AQLString& curveIndex,
                                            const AQLStringMatrix& curveConv,
                                            const AQLStringMatrix& fxFwdConv );
}

