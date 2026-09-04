#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
    /* @brief			validation method for aqObjectsCurveCreateFXForwards
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		fxFwdConv			FX forward conventions
    */
    AQLString tryAqObjectsCurveCreateFXForwards( const std::string& aqoCurveName,
                                            const AQLString& curveCollectionInput,
                                            const AQLString& staticDataTableInput,
                                            const AQLString& curveIndexInput,
                                            const AQLStringMatrix& curveConv,
                                            const AQLStringMatrix& fxFwdConv );
}

