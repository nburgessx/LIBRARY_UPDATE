/*
 * @brief			validation interface for the meCurveCalibrateFXForwards method
 * @Created:		19 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
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

