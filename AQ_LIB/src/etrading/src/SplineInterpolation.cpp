


#include "SplineInterpolation.h"
#include "ETradingException.h"

namespace etrading
{
    SplineInterpolation::SplineInterpolation( const std::vector<double>& inXs, const std::vector<double>& inYs )
        : Interpolation( etrading::SPLINE_INTERPOLATION )
    {
        std::vector<double> xs = inXs;
        std::vector<double> ys = inYs;

        // AQLSplineInterpolation.cpp:   void AQLSplineInterpolation::set(const DoubleArray& index, const DoubleArray& value)  - line 166
        if ( xs.size() != ys.size() || ys.size() <= 1 || xs.size() == 0 )
        {
            throw ETradingException( ( boost::format( "SplineInterpolation: Illegal sizes => x-vector size (%i) and y-vector size (%i)" ) % xs.size() % ys.size() ).str() );
        }

        unsigned int i, k;
        double p, qn, sig, tmpn;
        unsigned int size = xs.size();
        std::vector<double> tmp( 1 );
        std::vector<double> secDev( 1 );
        tmp.resize( size );
        secDev.resize( size );

        secDev[0] = tmp[0] = 0.0;

        //See Discription Numerical Recipes in C.
        for ( i = 2; i <= size - 1; i++ )
        {
            sig = ( xs[i - 1] - xs[i - 1 - 1] ) / ( xs[i + 1 - 1] - xs[i - 1 - 1] );
            p = sig * secDev[i - 1 - 1] + 2.0;
            secDev[i - 1] = ( sig - 1.0 ) / p;
            tmp[i - 1] = ( ys[i + 1 - 1] - ys[i - 1] ) / ( xs[i + 1 - 1] - xs[i - 1] ) - ( ys[i - 1] - ys[i - 1 - 1] ) / ( xs[i - 1] - xs[i - 1 - 1] );
            tmp[i - 1] = ( 6.0 * tmp[i - 1] / ( xs[i + 1 - 1] - xs[i - 1 - 1] ) - sig * tmp[i - 1 - 1] ) / p;
        }

        qn = tmpn = 0.0;

        secDev[size - 1] = ( tmpn - qn * tmp[size - 1 - 1] ) / ( qn * secDev[size - 1 - 1] + 1.0 );

        for ( k = size - 1; k >= 1; k-- )
        {
            secDev[k - 1] = secDev[k - 1] * secDev[k + 1 - 1] + tmp[k - 1];
        }

        index_ = xs;
        value_ = ys,
        coeff_ = secDev;
        size_ = size;
    };

    SplineInterpolation::~SplineInterpolation() {};

    double SplineInterpolation::interpolate( double x1 ) const
    {
        if ( size_ == 1 )
        {
            return value_[0];
        }

        int klo, khi, k;
        double h, b, a;

        klo = 1;
        khi = size_;

        while( khi - klo > 1 )
        {
            k = ( khi + klo ) >> 1;
            if ( index_[k - 1] > x1 )
            {
                khi = k;
            }
            else
            {
                klo = k;
            }
        }

        h = index_[khi - 1] - index_[klo - 1];

        if ( h == 0.0 )
        {
            throw ETradingException( ( boost::format( "SplineInterpolation::interpolate: Invalid Index data is thrown to spline to set method at MMSplineinterpolation (x1 : %f)" ) % x1 ).str() );
        }

        a = ( index_[khi - 1] - x1 ) / h;
        b = ( x1 - index_[klo - 1] ) / h;

        return a * value_[klo - 1] + b * value_[khi - 1] + ( ( a * a * a - a ) * coeff_[klo - 1] + ( b * b * b - b ) * coeff_[khi - 1] ) * ( h * h ) / 6.0;
    };
}