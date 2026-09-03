
#include <memory>
#include <math.h>
#include <boost/lexical_cast.hpp>
#include "InterpolationFactory.h"
#include "SplineInterpolation.h"
#include "ConstrainedSplineInterpolation.h"
#include "MonotoneConvexInterpolation.h"
#include "LinearInterpolation.h"
#include "StepInterpolation.h"
#include "InterpolationParameters.h"
#include "ETradingException.h"
#include "LACoreUtil.h"
// include these if getting the lambda from a property file
//#include "LAStaticData.h"
//#include "LACoreDataService.h"
//#include "LADefinitions.h"
//#include "LAStaticDataManager.h"
//#include "LAStaticData.h"


namespace etrading
{

    Interpolation::Interpolation( const InterpolationEnum interpEnum )
        : HasConstInstance<InterpolationEnum>( interpEnum ),
          extrapolation_( etrading::CONSTANT_EXTRAPOLATION )
    {}; // overwrite the extrapolation seperately if you wish to change it

    InterpolationFactory::InterpolationFactory() {};

    /* static */ std::shared_ptr<Interpolation>
    InterpolationFactory::generateInterpolation(	const std::vector<double>& xs,
            const std::vector<double>& ys,
            const InterpolationEnum interpEnum,
            double lambda,
            const std::shared_ptr<const InterpolationParameters>& interpolationParameters )
    {
        /*
        // MLIBQ does the strange thing of inserting 0.0 and 1.0 and the beginning for its interpolation (even if it is already there)
        // this only makes a difference if the interpolation is Spline, Constrained Spline
        std::vector<double> xHere(1,0.0);
        xHere.insert(xHere.end(), xs.cbegin(), xs.cend());
        std::vector<double> yHere(1,1.0);
        yHere.insert(yHere.end(), ys.cbegin(), ys.cend());
        */

        if( interpEnum == etrading::SPLINE_INTERPOLATION )
        {
            return std::move( std::shared_ptr<SplineInterpolation>( new SplineInterpolation( xs, ys ) ) );    // should already be moved, no need for std::move
        }
        else if( interpEnum == etrading::CONSTRAINED_SPLINE_INTERPOLATION )
        {
            return std::move( std::shared_ptr<ConstrainedSplineInterpolation>( new ConstrainedSplineInterpolation( xs, ys ) ) );    // should already be moved, no need for std::move
        }
        else if( interpEnum == etrading::MONOTONE_CONVEX_INTERPOLATION )
        {
            if( boost::math::isnan( lambda ) )
            {
                //const LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
                //const LAString strLambda = staticData.getStaticData(KEY_MONOTONECONVEX_LAMBDA);
                //lambda = (strLambda == "NO_DATA") ? 0.0 : boost::lexical_cast<double>(std::string(strLambda.getCString()));
                lambda = 0.0;
            }
            return std::move( std::shared_ptr<MonotoneConvexInterpolation>( new MonotoneConvexInterpolation( xs, ys, lambda ) ) );    // should already be moved, no need for std::move
            /*
            auto ptrMonotoneConvexParametes = dynamic_cast<const MonotoneConvexParameters*>(interpolationParameters.get());
            if(ptrMonotoneConvexParametes != nullptr)
            {
            	return std::move( std::shared_ptr<MonotoneConvexInterpolation>( new MonotoneConvexInterpolation( xHere, yHere, *ptrMonotoneConvexParametes ) ) );    // should already be moved, no need for std::move
            } else
            {
            	throw ETradingException("Illegal parameter set provided to MonotoneConvexInterpolation");
            }
            */
        }
        else if( interpEnum == etrading::LINEAR_INTERPOLATION )
        {
            return std::move( std::shared_ptr<LinearInterpolation>( new LinearInterpolation( xs, ys ) ) );
        }
        else if( interpEnum == etrading::LEFT_CONTINUOUS_INTERPOLATION )
        {
            return std::move( std::shared_ptr<StepInterpolation>( new StepInterpolation( xs, ys, true ) ) );
        }
        else if( interpEnum == etrading::RIGHT_CONTINUOUS_INTERPOLATION )
        {
            return std::move( std::shared_ptr<StepInterpolation>( new StepInterpolation( xs, ys, false ) ) );
        }
        else
        {
			return {};    // the other interpolation methods do not require extraneous data
        }
    };



}