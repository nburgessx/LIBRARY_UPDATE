#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{

    /* @brief			validation interface for aqToolsInterpolation
    *  @param [in]		xValues			X-axis values of the data
    *  @param [in]		yValues			Y-axis values of the data
    *  @param [in]		xPoint			X-axis point
    *  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A joinXValue as a double value used in hybrid interpolation
    *  @return			the Y-axis interpolated value
    */
    double tryAqToolsInterpolation( const DoubleVector& xValues,
                                      const DoubleVector& yValues,
                                      double xPoint,
                                      const AQLString& interpolation,
									  const double joinXValue = 0.0);

}
