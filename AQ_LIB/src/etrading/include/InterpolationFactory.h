

#pragma once

#include <memory>
#include <vector>
#include <numeric>
#include <limits>
#include "CoreEnumerations.h"
#include "HasInstance.h"
#include "Singleton.h"
#include "InterpolationParameters.h"


namespace etrading
{
    struct Interpolation : public HasConstInstance<InterpolationEnum>
    {
        Interpolation( const InterpolationEnum interpEnum );
        virtual ~Interpolation() {};
        virtual double interpolate( double x ) const = 0;
        ExtrapolationTypeEnum extrapolation_;
    };

    struct InterpolationFactory : public etrading::Singleton<InterpolationFactory>
    {
        static
        std::shared_ptr<Interpolation>
        generateInterpolation(	const std::vector<double>& xs,
                                const std::vector<double>& ys,
                                const InterpolationEnum,
                                double lambda = std::numeric_limits<double>::quiet_NaN(),
                                const std::shared_ptr<const InterpolationParameters>& interpolationParameters = {} );
    private:
        friend etrading::Singleton<InterpolationFactory>;
        InterpolationFactory();
    };


};
