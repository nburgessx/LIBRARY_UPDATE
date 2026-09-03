#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
{
    /* @brief			validation method for mirSetUpFwdFXConstantCurve
    *  @param [in]		dataInstance				Pointer to the object pool
    *  @param [in]		curveID				The curve collection ID
    *  @param [in]		marketName			Name of the curve constructed by this method
    *  @param [in]		generalProps		General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		fwdFXConstantConv	FX forward conventions
    *  @param [in]		curveNames			Equivalent names of the curve being built
    */
    AQLString tryMirSetUpFwdFXConstantCurve( AQLDataInstance* dataInstance,
                                            const AQLString& curveID,
                                            const AQLString& marketName,
                                            const AQLStringMatrix& generalProps,
                                            const AQLStringMatrix& fwdFXConstantConv,
                                            const AQLString& curveNames ) ;
}

