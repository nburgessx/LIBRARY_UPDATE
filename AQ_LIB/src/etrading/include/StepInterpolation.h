#pragma once

#include <vector>
#include "InterpolationFactory.h"


namespace etrading
{
    struct StepInterpolation : public Interpolation
    {
        StepInterpolation (	const std::vector<double>& inXs,
                            const std::vector<double>& inYs,
                            bool isLeftContinuous );
        double interpolate( double x ) const;
        ~StepInterpolation ();
        bool isLeftContinuous_;
    private:
        std::vector<double> xs_;	// Valarray to save the data of the x-axis of the data to be interpolated
        std::vector<double> ys_;	// Valarray to save the data of the y-axis of the data to be interpolated
    };

}