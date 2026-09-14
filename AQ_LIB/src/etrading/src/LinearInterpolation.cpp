
#include <utility>
#include <algorithm>

#include "LinearInterpolation.h"
#include "ETradingException.h"
#include "ContainerUtilities.h"

namespace etrading
{

    LinearInterpolation::LinearInterpolation( const std::vector<double>& inXs, const std::vector<double>& inYs )
        : xs_( inXs ), ys_( inYs ), Interpolation( etrading::LINEAR_INTERPOLATION )
    {};

    LinearInterpolation::~LinearInterpolation() {};

    double LinearInterpolation::interpolate( double x ) const
    {
        auto it = std::lower_bound( xs_.cbegin(), xs_.cend(), x );
        if( it == xs_.cbegin() )
        {
            // the first x is larger than or equal to the x we are looking to interpolate
            if( extrapolation_ == etrading::CONSTANT_EXTRAPOLATION || x == xs_[ 0 ] )
            {
                return ys_[ 0 ];
            }
            else if( extrapolation_ == etrading::LINEAR_EXTRAPOLATION )
            {
                double slope = ( ys_[1] - ys_[0] ) / ( xs_[1] - xs_[0] );
                double distance = ( x - xs_[0] );
                return  slope * distance + ys_[0];
            }
        }
        else if( it == xs_.cend() )
        {
            // all the xs are smaller than the x we are looking for
            if( extrapolation_ == etrading::CONSTANT_EXTRAPOLATION )
            {
                return ys_[ ys_.size() - 1 ];
            }
            else if( extrapolation_ == etrading::LINEAR_EXTRAPOLATION )
            {
                double slope = ( ys_[ys_.size() - 1] - ys_[ys_.size() - 2] ) / ( xs_[xs_.size() - 1] - xs_[xs_.size() - 2] );
                double distance = ( x - xs_[xs_.size() - 1] );
                return  slope * distance + ys_[ys_.size() - 1];
            }
        }
        else
        {
            auto idxOfFirstLargerOrEqual = getIndexOf( *it, xs_ );
            if ( idxOfFirstLargerOrEqual  < 0 )
            {
                AQ_THROW( "Linear Interpolation: Extrapolation below the Interpolation Range is Not Supported" );
            }
            else
            {
                double slope = ( ys_[idxOfFirstLargerOrEqual] - ys_[idxOfFirstLargerOrEqual - 1] ) / ( xs_[idxOfFirstLargerOrEqual] - xs_[idxOfFirstLargerOrEqual - 1] );
                double distance = ( x - xs_[idxOfFirstLargerOrEqual] );
                return  slope * distance + ys_[idxOfFirstLargerOrEqual];
            }
        }
        // Should never reach here
		AQ_THROW( "Linear Interpolation: Unable to evaluate the interpolation point." );
    };

}