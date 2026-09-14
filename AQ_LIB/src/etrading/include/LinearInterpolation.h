
#pragma once

#include <vector>
#include "InterpolationFactory.h"
#include "CoreEnumerations.h"


namespace etrading
{
    struct LinearInterpolation : public Interpolation
    {
        LinearInterpolation( const std::vector<double>& inXs, const std::vector<double>& inYs );
        double interpolate( double x ) const;
        ~LinearInterpolation();

    private:
        std::vector<double> xs_;	// Valarray to save the data of the x-axis of the data to be interpolated
        std::vector<double> ys_;	// Valarray to save the data of the y-axis of the data to be interpolated
    };
}