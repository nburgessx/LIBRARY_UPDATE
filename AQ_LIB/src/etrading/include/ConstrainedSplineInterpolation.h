#pragma once

#include <memory>
#include <vector>
#include "CoreEnumerations.h"
#include "HasInstance.h"
#include "Singleton.h"
#include "InterpolationFactory.h"


namespace etrading
{
    struct ConstrainedSplineInterpolation : public Interpolation
    {
        ConstrainedSplineInterpolation(	const std::vector<double>& inXs,
                                        const std::vector<double>& inYs );

        double interpolate( double x ) const;
        ~ConstrainedSplineInterpolation();

    private:
        std::vector<double> index_;	// Valarray to conserve x-axis of data
        std::vector<double> value_;	// Valarray to conserve y-axis of data
        std::vector<double> a_, b_, c_, d_;	// Valarray to conserve coefficients of spline function which were calculated from data
        // by the set method. each coefficient has size values
        unsigned int size_;	// the number of initial data
        double slope0_, slope1_;// slope to extrapolate

        static const double D_ZERO; //  = 0.00000001;
        static const double D_UPPER; //  = 10000000000.0;

        double calc_dxx( const double x1, const double x0 ) const;

    };
}