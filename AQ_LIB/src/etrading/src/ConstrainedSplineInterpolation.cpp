
#include <cmath>
#include <boost/format.hpp>

#include "ConstrainedSplineInterpolation.h"
#include "ETradingException.h"

namespace etrading
{
    const double ConstrainedSplineInterpolation::D_ZERO = 0.00000001;
    const double ConstrainedSplineInterpolation::D_UPPER = 10000000000.0;

    ConstrainedSplineInterpolation::ConstrainedSplineInterpolation(	const std::vector<double>& inXs, const std::vector<double>& inYs )
        : Interpolation( etrading::CONSTRAINED_SPLINE_INTERPOLATION )
    {
        // AQLSplineInterpolation.cpp:   void AQLSplineInterpolation::set(const DoubleArray& index, const DoubleArray& value)  - line 166
        if ( inXs.size() != inYs.size() || inYs.size() <= 1 || inXs.size() == 0 )
        {
            throw ETradingException( ( boost::format( "SplineInterpolation: Illegal sizes => x-vector size (%i) and y-vector size (%i)" ) % inXs.size() % inYs.size() ).str() );
        }

        std::vector<double> xs = inXs;
        std::vector<double> ys = inYs;

        std::vector<double> gxx;
        std::vector<std::vector<double>> ggxx;	// 1st and 2nd derivative for left and right ends of line
        std::vector<double> a, b, c, d;
        double slope0, slope1;

        size_t i, size, Nmax, Num;

        size = xs.size();
        //Number of lines = points - 1
        Nmax = size - 1;
        Num = 0;

        gxx.resize( size );
        ggxx.resize( size );
        a.resize( size );
        b.resize( size );
        c.resize( size );
        d.resize( size );

        for( i = 0; i < size; i++ )
        {
            ggxx[i].resize( 2 );
        }

        a[0] = b[0] = c[0] = d[0] = 0.0;
        ggxx[0][0] = ggxx[0][1] = 0.0;

        //Calc first derivative (slope) for intermediate points
        for ( i = 0; i <=  Nmax; i++ )
        {
            if ( i == 0 ||  i == Nmax )
            {
                //Set very large slope at ends
                gxx[i] = D_UPPER;
            }
            else if ( ( std::abs( ys[i + 1] - ys[i] ) < D_ZERO ) || ( std::abs( ys[i] - ys[i - 1] ) < D_ZERO ) )
            {
                //Only check for 0 dy. dx assumed NEVER equals 0 !
                gxx[i] = 0.0;
            }
            else if ( std::abs( ( xs[i + 1] - xs[i] ) / ( ys[i + 1] - ys[i] ) + ( xs[i] - xs[i - 1] ) / ( ys[i] - ys[i - 1] ) ) < D_ZERO )
            {
                //Pos PLUS neg slope is 0. Prevent div by zero.
                gxx[i] = 0.0;
            }
            else if ( ( ys[i + 1] - ys[i] ) * ( ys[i] - ys[i - 1] ) < 0 )
            {
                //Pos AND neg slope, assume slope = 0 to prevent overshoot
                gxx[i] = 0;
            }
            else
            {
                //Calculate an average slope for point based on connecting lines
                gxx[i] = 2.0 / (  calc_dxx( xs[i + 1], xs[i] ) / ( ys[i + 1] - ys[i] ) + calc_dxx( xs[i], xs[i - 1] ) / ( ys[i] - ys[i - 1] )  );
            }
        }

        //Reset first derivative (slope) at first and last point

        //First point has 0 2nd derivative
        gxx[0] = 3.0 / 2 * ( ys[1] - ys[0] ) / calc_dxx( xs[1], xs[0] ) - gxx[1] / 2.0 ;
        slope0 = gxx[0];
        //Last point has 0 2nd derivative
        gxx[Nmax] = 3.0 / 2 * ( ys[Nmax] - ys[Nmax - 1] ) / calc_dxx( xs[Nmax], xs[Nmax - 1] ) - gxx[Nmax - 1] / 2.0 ;
        slope1 = gxx[Nmax];

        //Calc second derivative at points
        for ( i = 1; i <=  Nmax  ; i++ )
        {
            ggxx[i][0] = -2.0   * ( gxx[i] + 2 * gxx[i - 1] ) / calc_dxx( xs[i], xs[i - 1] ) + 6.0 * ( ys[i] - ys[i - 1] ) / ( ( calc_dxx( xs[i], xs[i - 1] ) * calc_dxx( xs[i], xs[i - 1] ) ) );
            ggxx[i][1] = 2.0 * ( 2 * gxx[i] + gxx[i - 1] ) / calc_dxx( xs[i], xs[i - 1] ) - 6.0 * ( ys[i] - ys[i - 1] ) / ( ( calc_dxx( xs[i], xs[i - 1] ) * calc_dxx( xs[i], xs[i - 1] ) ) );

            //Calc constants for cubic
            d[i] = 1.0 / 6 * ( ggxx[i][1] - ggxx[i][0] ) / calc_dxx( xs[i], xs[i - 1] );
            c[i] = 1.0 / 2 * ( xs[i] * ggxx[i][0] - xs[i - 1] * ggxx[i][1] ) / calc_dxx( xs[i], xs[i - 1] );
            b[i] = ( ys[i] - ys[i - 1] - c[i] * ( ( xs[i] * xs[i] ) - ( xs[i - 1] * xs[i - 1] ) ) - d[i] * ( ( xs[i] * xs[i] * xs[i] ) - ( xs[i - 1]  * xs[i - 1]  * xs[i - 1] ) ) ) / calc_dxx( xs[i], xs[i - 1] );
            a[i] = ys[i - 1] - b[i] * xs[i - 1] - c[i] * ( xs[i - 1] * xs[i - 1] ) - d[i] * ( xs[i - 1] * xs[i - 1] * xs[i - 1] );

        }

        index_ = xs;
        value_ = ys;
        a_ = a;
        b_ = b;
        c_ = c;
        d_ = d;
        size_ = size;
        slope0_ = slope0;
        slope1_ = slope1;
    };

    ConstrainedSplineInterpolation::~ConstrainedSplineInterpolation() {};

    double ConstrainedSplineInterpolation::interpolate( double x1 ) const
    {
        /*
        std::vector<double> index_;	// Valarray to conserve x-axis of data
        std::vector<double> value_;	// Valarray to conserve y-axis of data
        std::vector<double> a_,b_,c_,d_;	// Valarray to conserve coefficients of spline function which were calculated from data
        														// by the set method. each coefficient has size values
        unsigned int size_;	// the number of initial data
        double slope0_, slope1_;// slope to extrapolate
        */

        if ( size_ == 1 )
        {
            return value_[0];
        }

        double ret;
        size_t Nmax, Num;

        //Number of lines = points - 1
        Nmax = index_.size() - 1;

        //(1a) Find LineNumber or segment. Linear extrapolate if outside range.
        Num = 0;

        if( ( x1 < index_[0] ) || ( x1 > index_[Nmax] ) )
        {
            //X outisde range. Linear interpolate
            //Below min or max?
            if ( x1 < index_[0] )
            {
                ret = value_[0] + ( x1 - index_[0] ) * slope0_;

            }
            else
            {
                ret = value_[Nmax] + ( x1 - index_[Nmax] ) * slope1_;
            }

            return ret;

            /* This is the original constrained splines algorithm to do expolation.  We use a slightly modified algorithm to do the expolation
            with thte a, b, c, d values that we already obtained.
            stdflt B = (y[Num] - y[Num - 1]) / calc_dxx(mpDataProvider->x[Num], mpDataProvider->x[Num - 1]);
            stdflt A = y[Num] - B * mpDataProvider->x[Num];
            o_Y = A + B * x1 ;
            return 0;
            */
        }
        else
        {
            //(1b) Find LineNumber or segment.
            //this->mpDataProvider->x.search(x1, Num);

            for ( size_t i = 1; i <= Nmax; i++ )
            {
                if ( x1 <= index_[i] )
                {
                    Num = i;
                    break;
                }
            }

        }

        ret = a_[Num] + x1 * ( b_[Num]  +  x1  * ( c_[Num] + d_[Num] * x1 ) );

        return ret;

    }

    double ConstrainedSplineInterpolation::calc_dxx( const double x1, const double x0 ) const
    {
        //Calc Xi - Xi-1 to prevent div by zero
        double dxx = x1 - x0;
        if ( std::abs( dxx ) < D_ZERO )
        {
            dxx = D_UPPER;
        }

        return dxx;
    };

};