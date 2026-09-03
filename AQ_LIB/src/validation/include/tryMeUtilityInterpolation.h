/*
 * @brief			validation interface for meUtilityInterpolation method
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{

    /* @brief			validation interface for meUtilityInterpolation
    *  @param [in]		xValues			X-axis values of the data
    *  @param [in]		yValues			Y-axis values of the data
    *  @param [in]		xPoint			X-axis point
    *  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A joinXValue as a double value used in hybrid interpolation
    *  @return			the Y-axis interpolated value
    */
    double tryMeUtilityInterpolation( const DoubleVector& xValues,
                                      const DoubleVector& yValues,
                                      double xPoint,
                                      const LAString& interpolation,
									  const double joinXValue = 0.0);

}
