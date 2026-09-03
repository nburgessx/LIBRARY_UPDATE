#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
{
    /* @brief			validation method for mirSetUpFwdFXConstantCurve
    *  @param [in]		dataInstance				Pointer to the object pool
    *  @param [in]		curveID				The curve collection ID
    *  @param [in]		marketName			Name of the curve constructed by this method
    *  @param [in]		generalProps		General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		fwdFXConstantConv	FX forward conventions
    *  @param [in]		curveNames			Equivalent names of the curve being built
    */
    LAString tryMirSetUpFwdFXConstantCurve( LADataInstance* dataInstance,
                                            const LAString& curveID,
                                            const LAString& marketName,
                                            const LAStringMatrix& generalProps,
                                            const LAStringMatrix& fwdFXConstantConv,
                                            const LAString& curveNames ) ;
}

