

#pragma once

#include <vector>
#include "InterpolationFactory.h"


namespace etrading
{
    struct MonotoneConvexInterpolation : public Interpolation
    {
        MonotoneConvexInterpolation(	const std::vector<double>& inXs,
                                        const std::vector<double>& inYs,
                                        const double lambda );
        /*
        MonotoneConvexInterpolation(	const std::vector<double>& inXs,
        													const std::vector<double>& inYs,
        													const MonotoneConvexParameters& interpolationParameters );
        													*/
        ~MonotoneConvexInterpolation();
        double interpolate( double x ) const;
    private:
        std::vector<double> index_;	// Valarray to save the data of the x-axis of the data to be interpolated
        std::vector<double> value_;	// Valarray to save the data of the y-axis of the data to be interpolated
        std::vector<double> fdiscrete_;// Valarray to save the fdiscrete at index
        std::vector<double> f_;		// Valarray to save the f at index
        unsigned int size_;	// initial data numbers
        double lambda_;

    };
}