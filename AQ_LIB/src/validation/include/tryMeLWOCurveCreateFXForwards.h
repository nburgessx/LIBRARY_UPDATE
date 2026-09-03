#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
    /* @brief			validation method for meLWOCurveCreateFXForwards
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		fxFwdConv			FX forward conventions
    */
    LAString tryMeLWOCurveCreateFXForwards( const std::string& lwoCurveName,
                                            const LAString& curveCollectionInput,
                                            const LAString& staticDataTableInput,
                                            const LAString& curveIndexInput,
                                            const LAStringMatrix& curveConv,
                                            const LAStringMatrix& fxFwdConv );
}

