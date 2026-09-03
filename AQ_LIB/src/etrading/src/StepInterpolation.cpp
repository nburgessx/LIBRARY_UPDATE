
#include <utility>
#include <algorithm>
#include <boost/format.hpp>

#include "StepInterpolation.h"
#include "ETradingException.h"
#include "ContainerUtilities.h"
#include "StatisticsUtilities.h"

namespace etrading
{
    StepInterpolation::StepInterpolation(	const std::vector<double>& inXs,
                                            const std::vector<double>& inYs,
                                            bool isLeftContinuous )
        : isLeftContinuous_( isLeftContinuous ), xs_( inXs ), ys_( inYs ),
          Interpolation( isLeftContinuous_ ? etrading::LEFT_CONTINUOUS_INTERPOLATION : etrading::RIGHT_CONTINUOUS_INTERPOLATION )
    {};

    StepInterpolation::~StepInterpolation() {};

    double StepInterpolation::interpolate( double x ) const
    {
        if( xs_.size() == 1 )
        {
            return ys_[ 0 ];
        }

        auto it = std::lower_bound( xs_.cbegin(), xs_.cend(), x );
        if( it == xs_.cbegin() )
        {
            return ys_[ 0 ];
        }
        else if( it == xs_.cend() )
        {
            return ys_[ ys_.size() - 1 ];
        }
        else
        {
            auto idxOfFirstLargerOrEqual = std::distance( xs_.cbegin(), it );
            const bool matchesIdx = doubleEquals( *it, x );
            if ( idxOfFirstLargerOrEqual  < 0 )
            {
                throw ETradingException( "stepInterpolate - Should never reach this line - A" );
            }
            else
            {
                if( isLeftContinuous_ )
                {
                    return  ( !matchesIdx ) ? ys_[idxOfFirstLargerOrEqual - 1] : ys_[idxOfFirstLargerOrEqual];
                }
                else
                {
                    return  ( !matchesIdx ) ? ys_[idxOfFirstLargerOrEqual] : ys_[std::min<int>( idxOfFirstLargerOrEqual + 1, ys_.size() - 1 )];
                }
            }
        }
        throw ETradingException( "stepInterpolate - Should never reach this line -B" );
    }

}
