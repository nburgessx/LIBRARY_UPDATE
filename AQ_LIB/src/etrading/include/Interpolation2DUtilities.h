//
// @Description: This is a test program

#pragma once

#include <string>
#include <vector>
#include <algorithm>


#include "CoreEnumerations.h"
#include "ETradingException.h"
#include "ContainerUtilities.h"
#include "InterpolationFactory.h"
#include "ConstrainedSplineInterpolation.h"
#include "SplineInterpolation.h"
#include "MonotoneConvexInterpolation.h"

namespace etrading
{
    // we assume the xs are already sorted in ascending order!!!
    double interpolate( double x, const std::vector<double>&  xs, const std::vector<double>& ys,
                        const InterpolationEnum interpolationEnum = etrading::LINEAR_INTERPOLATION,
                        const ExtrapolationTypeEnum extrapolationEnum = etrading::CONSTANT_EXTRAPOLATION );

}