//
// @File: ContainerUtilities.h
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of Mizuho International.

#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <boost/format.hpp>


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