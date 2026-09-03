#include "Interpolation2DUtilities.h"

namespace etrading
{

    double interpolate( double x, const std::vector<double>&  xs, const std::vector<double>& ys,
                        const InterpolationEnum interpolationEnum,
                        const ExtrapolationTypeEnum extrapolationEnum )
    {
        auto ptrInterpolation =  InterpolationFactory::getInstance().generateInterpolation( xs, ys, interpolationEnum );
        ptrInterpolation.get()->extrapolation_ = extrapolationEnum; // TODO: consider moving this to the CTOR to be able to create std::unique_ptr<const Interpolation>
        return ptrInterpolation->interpolate( x );
    };

}

