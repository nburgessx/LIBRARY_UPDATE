
#pragma once

#include <vector>
#include "InterpolationFactory.h"


namespace etrading
{
    struct SplineInterpolation : public Interpolation
    {
        SplineInterpolation( const std::vector<double>& xs, const std::vector<double>& ys );
        ~SplineInterpolation();
        double interpolate( double x ) const;
    private:
        std::vector<double> index_;	// Valarray to save the data of the x-axis of the data to be interpolated
        std::vector<double> value_;	// Valarray to save the data of the y-axis of the data to be interpolated
        std::vector<double> coeff_;	// Valarray to save the second derivatives of the spline function calculated by the method from the Data Set
        unsigned int size_;	// initial data numbers
    };
}