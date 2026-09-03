#pragma once

#include "LACoreTemplateType.h"

namespace validation
{
    /* @brief			validation method for meCurveCalibrateFXForwards
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		fxFwdConv			FX forward conventions
    */
    LAString tryMeCurveCalibrateFXForwards( const LAString& curveCollection,
                                            const LAString& staticDataTable,
                                            const LAString& curveIndex,
                                            const LAStringMatrix& curveConv,
                                            const LAStringMatrix& fxFwdConv );
}

