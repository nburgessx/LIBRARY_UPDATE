
#include <map>
#include <utility>
#include <algorithm>
#include <boost/format.hpp>

#include "MonotoneConvexInterpolation.h"
#include "ETradingException.h"

namespace etrading
{
    //const MonotoneConvexParameters& interpolationParameters

    MonotoneConvexInterpolation::MonotoneConvexInterpolation(
        const std::vector<double>& inXs,
        const std::vector<double>& inYs,
        const double lambda )
        : Interpolation( etrading::MONOTONE_CONVEX_INTERPOLATION ), lambda_( lambda )
    {
        if ( inXs.size() != inYs.size() || inYs.size() <= 1 || inXs.size() == 0 )
        {
            throw ETradingException( ( boost::format( "SplineInterpolation: Illegal sizes => x-vector size (%i) and y-vector size (%i)" ) % inXs.size() % inYs.size() ).str() );
        }

        const unsigned int n = inXs.size();

        //sort input data
        std::map<double, double> mp;
        for ( unsigned int i = 0; i < n; i++ )
        {
            mp.insert( std::make_pair( inXs[i], inYs[i] ) );
        }
        DoubleArray tau( n );
        DoubleArray input( n );
        std::map<double, double>::iterator it = mp.begin();
        while ( it != mp.end() )
        {
            const int pos = distance( mp.begin(), it );
            tau[pos] = it->first;
            input[pos] = it->second;
            it++;
        }
        mp.clear();

        //extend the curve to time 0, for the purpose of calculating forward at time 1
        tau.insert( tau.begin(), 0.0 );
        const double tmp = input.front();
        input.insert( input.begin(), tmp );

        //step 1
        DoubleArray fdiscrete( n + 1, 0.0 );
        for ( unsigned int j = 1; j <= n; j++ )
        {
            fdiscrete[j] = ( tau[j] - tau[j - 1] == 0.0 ) ? 0.0
                           : ( input[j] - input[j - 1] ) / ( tau[j] - tau[j - 1] );
        }

        //step 2
        DoubleArray f( n + 1, 0.0 );
        if ( lambda_ == 0 )
        {
            //set under the unameliorated method
            //numbering refers to Wilmott paper
            //(22)
            for ( unsigned int j = 1; j <= n - 1; j++ )
            {
                f[j] = ( tau[j] - tau[j - 1] ) / ( tau[j + 1] - tau[j - 1] ) * fdiscrete[j + 1]
                       + ( tau[j + 1] - tau[j] ) / ( tau[j + 1] - tau[j - 1] ) * fdiscrete[j];
            }
            //(23)
            f[0] = fdiscrete[1] - 0.5 * ( f[1] - fdiscrete[1] );
            //(24)
            f[n] = fdiscrete[n] - 0.5 * ( f[n - 1] - fdiscrete[n] );
        }
        else
        {
            //set under the ameliorated method
            //numbering refers to AMF paper
            DoubleArray theta_m( n, 0.0 ); //theta_minus
            DoubleArray theta_p( n, 0.0 ); //theta_plus
            DoubleArray fmin1( n, 0.0 );
            DoubleArray fmax1( n, 0.0 );
            DoubleArray fmin2( n, 0.0 );
            DoubleArray fmax2( n, 0.0 );
            //(72) and (73)
            DoubleArray falseTau( tau ); //for the false interval
            falseTau.insert( falseTau.begin(), - tau[1] ); //add beginning interval(-1)
            fdiscrete[0] = fdiscrete[1] - ( tau[1] - tau[0] ) / ( tau[2] - tau[0] ) * ( fdiscrete[2] - fdiscrete[1] );
            falseTau.push_back( 2 * tau[n] - tau[n - 1] ); //add end interval(n+1)
            fdiscrete.push_back( fdiscrete[n] + ( tau[n] - tau[n - 1] ) / ( tau[n] - tau[n - 2] ) * ( fdiscrete[n] - fdiscrete[n - 1] ) );
            //(74) fi
            for ( unsigned int j = 0; j <= n; j++ )
            {
                f[j] = ( falseTau[j + 1] - falseTau[j] ) / ( falseTau[j + 2] - falseTau[j] ) * fdiscrete[j + 1]
                       + ( falseTau[j + 2] - falseTau[j + 1] ) / ( falseTau[j + 2] - falseTau[j] ) * fdiscrete[j];
            }
            //(68) theta_minus
            for ( unsigned int j = 1; j <= n - 1; j++ )
            {
                theta_m[j] = ( falseTau[j + 1] - falseTau[j] ) / ( falseTau[j + 1] - falseTau[j - 1] ) * ( fdiscrete[j] - fdiscrete[j - 1] );
            }
            //(71) theta_plus
            for ( unsigned int j = 1; j <= n - 1; j++ )
            {
                theta_p[j] = ( falseTau[j + 2] - falseTau[j + 1] ) / ( falseTau[j + 3] - falseTau[j + 1] ) * ( fdiscrete[j + 2] - fdiscrete[j + 1] );
            }
            //(67) fmin1, fmax1
            for ( unsigned int j = 1; j <= n - 1; j++ )
            {
                if ( fdiscrete[j - 1] < fdiscrete[j] && fdiscrete[j] <= fdiscrete[j + 1] )
                {
                    fmin1[j] = std::min( fdiscrete[j] + 0.5 * theta_m[j], fdiscrete[j + 1] );
                    fmax1[j] = std::min( fdiscrete[j] + 2.0 * theta_m[j], fdiscrete[j + 1] );
                }
                else if ( fdiscrete[j - 1] < fdiscrete[j] && fdiscrete[j] > fdiscrete[j + 1] )
                {
                    fmin1[j] = std::max( fdiscrete[j] - 0.5 * lambda_ * theta_m[j], fdiscrete[j + 1] );
                    fmax1[j] = fdiscrete[j];
                }
                else if ( fdiscrete[j - 1] >= fdiscrete[j] && fdiscrete[j] <= fdiscrete[j + 1] )
                {
                    fmin1[j] = fdiscrete[j];
                    fmax1[j] = std::min( fdiscrete[j] - 0.5 * lambda_ * theta_m[j], fdiscrete[j + 1] );
                }
                else if ( fdiscrete[j - 1] >= fdiscrete[j] && fdiscrete[j] > fdiscrete[j + 1] )
                {
                    fmin1[j] = std::max( fdiscrete[j] + 2.0 * theta_m[j], fdiscrete[j + 1] );
                    fmax1[j] = std::max( fdiscrete[j] + 0.5 * theta_m[j], fdiscrete[j + 1] );
                }
            }
            //(70) fmin2, fmax2
            for ( unsigned int j = 1; j <= n - 1; j++ )
            {
                if ( fdiscrete[j] < fdiscrete[j + 1] && fdiscrete[j + 1] <= fdiscrete[j + 2] )
                {
                    fmin2[j] = std::max( fdiscrete[j + 1] - 2.0 * theta_p[j], fdiscrete[j] );
                    fmax2[j] = std::max( fdiscrete[j + 1] - 0.5 * theta_p[j], fdiscrete[j] );
                }
                else if ( fdiscrete[j] < fdiscrete[j + 1] && fdiscrete[j + 1] > fdiscrete[j + 2] )
                {
                    fmin2[j] = std::max( fdiscrete[j + 1] + 0.5 * lambda_ * theta_p[j], fdiscrete[j] );
                    fmax2[j] = fdiscrete[j + 1];
                }
                else if ( fdiscrete[j] >= fdiscrete[j + 1] && fdiscrete[j + 1] < fdiscrete[j + 2] )
                {
                    fmin2[j] = fdiscrete[j + 1];
                    fmax2[j] = std::min( fdiscrete[j + 1] + 0.5 * lambda_ * theta_p[j], fdiscrete[j] );
                }
                else if ( fdiscrete[j] >= fdiscrete[j + 1] && fdiscrete[j + 1] >= fdiscrete[j + 2] )
                {
                    fmin2[j] = std::min( fdiscrete[j + 1] - 0.5 * theta_p[j], fdiscrete[j] );
                    fmax2[j] = std::min( fdiscrete[j + 1] - 2.0 * theta_p[j], fdiscrete[j] );
                }
            }

            for ( unsigned int j = 1; j <= n - 1; j++ ) //ameliorate fi
            {
                if ( std::max( fmin1[j], fmin2[j] ) <= std:: min( fmax1[j], fmax2[j] ) ) //overlap case
                {
                    //(75, 76)
                    f[j] = std::max( std::max( fmin1[j], fmin2[j] ), std::min( f[j], std::min( fmax1[j], fmax2[j] ) ) ); //adjust in the common range
                }
                else //not overlap case
                {
                    //(78)
                    f[j] = std::max( std::min( fmax1[j], fmax2[j] ), std::min( f[j], std::max( fmin1[j], fmin2[j] ) ) ); //adjust in the gap
                }
            }
            //(79) ameliorate f0
            if ( std::abs( f[0] - fdiscrete[0] ) > 0.5 * std::abs( f[1] - fdiscrete[0] ) )
            {
                f[0] = fdiscrete[1] - 0.5 * ( f[1] - fdiscrete[0] );
            }
            //(80) ameliorate fn
            if ( std::abs( f[n] - fdiscrete[n] ) > 0.5 * std::abs( f[n - 1] - fdiscrete[n] ) )
            {
                f[n] = fdiscrete[n] - 0.5 * ( f[n - 1] - fdiscrete[n] );
            }

        }   // ELSE LAMBDA_ !=0

        /*
        mpDataProvider = new AQLMonotoneConvexInterpolationDataProvider(tau, input, fdiscrete, f, n + 1);
        AQLMonotoneConvexInterpolationDataProvider(const DoubleArray dataIndex,
        							const DoubleArray dataValue,
        							const DoubleArray dataFdiscrete,
        							const DoubleArray dataF,
        							unsigned int s1)
        							: index(dataIndex), value(dataValue), fdiscrete(dataFdiscrete), f(dataF), size(s1)
        {
        }

        const DoubleArray index;	// Valarray to save the data of the x-axis of the data to be interpolated
        const DoubleArray value;	// Valarray to save the data of the y-axis of the data to be interpolated
        const DoubleArray fdiscrete;// Valarray to save the fdiscrete at index
        const DoubleArray f;		// Valarray to save the f at index
        const unsigned int size;	// initial data numbers
        */
        index_ = inXs;
        value_ = input;
        fdiscrete_ = fdiscrete;
        f_ = f;
        size_ = n;
    };

    MonotoneConvexInterpolation::~MonotoneConvexInterpolation() {};

    double MonotoneConvexInterpolation::interpolate( double x1 ) const
    {
        if ( size_ == 1 )
        {
            return value_[0];
        }

        //numbering refers to Wilmott paper
        int size = size_; // index_.size();
        double ret = 0.0;
        if ( x1 <= 0.0 ) //horizontal extrapolate of differential
        {
            //ret = f[0];
            ret = value_[0] + f_[0] * ( x1 - index_[0] );
        }
        else if ( x1 > index_[size - 1] ) //horizontal extrapolate of differential
        {
            //ret = interpolantGeneral(index[size - 1]) + forward(index[size - 1]) * (x1 - index[size - 1]);
            ret = value_[size - 1] + f_[size - 1] * ( x1 - index_[size - 1] );
        }
        else
        {
            auto it = std::upper_bound( index_.begin(), index_.end(), x1 );
            int i = std::distance( index_.begin(), it ) - ( it == index_.end() ? 2 : 1 );
            double x, g0, g1, G, eta, A;
            //the x in (25)
            x = ( x1 - index_[i] ) / ( index_[i + 1] - index_[i] );
            g0 = f_[i] - fdiscrete_[i + 1];
            g1 = f_[i + 1] - fdiscrete_[i + 1];
            if ( x == 0 || x == 1 )
            {
                G = 0.0;
            }
            else if ( ( g0 < 0 && -0.5 * g0 <= g1 && g1 <= -2 * g0 ) || ( g0 > 0 && -0.5 * g0 >= g1 && g1 >= -2 * g0 ) )
            {
                //zone (i)
                G = g0 * ( x - 2.0 * pow( x, 2 ) + pow( x, 3 ) ) + g1 * ( - pow( x, 2 ) + pow( x, 3 ) );
            }
            else if ( ( g0 < 0 && g1 > -2 * g0 ) || ( g0 > 0 && g1 < -2 * g0 ) )
            {
                //zone (ii)
                //(29)
                eta = ( g1 + 2.0 * g0 ) / ( g1 - g0 );
                //(28)
                if ( x <= eta )
                {
                    G = g0 * x;
                }
                else
                {
                    G = g0 * x + 1.0 / 3.0 * ( g1 - g0 ) * pow( x - eta, 3 ) / pow( 1.0 - eta, 2 );
                }
            }
            else if ( ( g0 > 0 && 0 > g1 && g1 > -0.5 * g0 ) || ( g0 < 0 && 0 < g1 && g1 < -0.5 * g0 ) )
            {
                //zone (iii)
                //(31)
                eta = 3.0 * g1 / ( g1 - g0 );
                //(30)
                if ( x <= eta )
                {
                    G = g1 * x - 1.0 / 3.0 * ( g0 - g1 ) * ( pow( eta - x, 3 ) / pow( eta, 2 ) - eta );
                }
                else
                {
                    G = ( 2.0 / 3.0 * g1 + 1.0 / 3.0 * g0 ) * eta + g1 * ( x - eta );
                }
            }
            else if ( g0 == 0 || g1 == 0 )
            {
                G = 0.0;
            }
            else
            {
                //zone (iv)
                //(33)
                eta = g1 / ( g1 + g0 );
                //(34)
                A = - g0 * g1 / ( g0 + g1 );
                //(32)
                if ( x <= eta )
                {
                    G = A * x - 1.0 / 3.0 * ( g0 - A ) * ( pow( eta - x, 3 ) / pow( eta, 2 ) - eta );
                }
                else
                {
                    G = ( 2.0 / 3.0 * A + 1.0 / 3.0 * g0 ) * eta + A * ( x - eta ) + 1.0 / 3.0 * ( g1 - A ) * pow( x - eta, 3 ) / pow( 1.0 - eta, 2 );
                }
            }
            ret = value_[i] + ( x1 - index_[i] ) * fdiscrete_[i + 1] + ( index_[i + 1] - index_[i] ) * G;
        }
        return ret;

    }

}

/*
        // AQLSplineInterpolation.cpp:   void AQLSplineInterpolation::set(const DoubleArray& index, const DoubleArray& value)  - line 166

        std::vector<double> xs = inXs;
        std::vector<double> ys = inYs;

        lambda_ = ( interpolationParameters.size() < 1 ) ? 0.0 : interpolationParameters.at( 0 );

        const unsigned int n = xs.size();

        //sort input data
        std::map<double, double> mp;
        for ( unsigned int i = 0; i < n; i++ )
        {
            mp.insert( std::make_pair( xs[i], ys[i] ) );
        }

        std::vector<double> tau( n );
        std::vector<double> input( n );
        std::map<double, double>::iterator it = mp.begin();
        while ( it != mp.end() )
        {
            const int pos = distance( mp.begin(), it );
            tau[pos] = it->first;
            input[pos] = it->second;
            it++;
        }
        mp.clear();

        //extend the curve to time 0, for the purpose of calculating forward at time 1
        tau.insert( tau.begin(), 0.0 );
        const double tmp = input.front();
        input.insert( input.begin(), tmp );

        //step 1
        std::vector<double> fdiscrete( n + 1, 0.0 );
        // case INPUTGENERAL://general
        for ( unsigned int j = 1; j <= n; j++ )
        {
            fdiscrete[j] = ( tau[j] - tau[j - 1] == 0.0 ) ? 0.0
                           : ( input[j] - input[j - 1] ) / ( tau[j] - tau[j - 1] );
        }

        //step 2
        std::vector<double> f( n + 1, 0.0 );
        if ( lambda_ == 0 )
        {
            //set under the unameliorated method
            //numbering refers to Wilmott paper
            //(22)
            for ( unsigned int j = 1; j <= n - 1; j++ )
            {
                f[j] = ( tau[j] - tau[j - 1] ) / ( tau[j + 1] - tau[j - 1] ) * fdiscrete[j + 1]
                       + ( tau[j + 1] - tau[j] ) / ( tau[j + 1] - tau[j - 1] ) * fdiscrete[j];
            }
            //(23)
            f[0] = fdiscrete[1] - 0.5 * ( f[1] - fdiscrete[1] );
            //(24)
            f[n] = fdiscrete[n] - 0.5 * ( f[n - 1] - fdiscrete[n] );
        }
        else
        {
            //set under the ameliorated method
            //numbering refers to AMF paper
            std::vector<double> theta_m( n, 0.0 ); //theta_minus
            std::vector<double> theta_p( n, 0.0 ); //theta_plus
            std::vector<double> fmin1( n, 0.0 );
            std::vector<double> fmax1( n, 0.0 );
            std::vector<double> fmin2( n, 0.0 );
            std::vector<double> fmax2( n, 0.0 );
            //(72) and (73)
            std::vector<double> falseTau( tau ); //for the false interval
            falseTau.insert( falseTau.begin(), - tau[1] ); //add beginning interval(-1)
            fdiscrete[0] = fdiscrete[1] - ( tau[1] - tau[0] ) / ( tau[2] - tau[0] ) * ( fdiscrete[2] - fdiscrete[1] );
            falseTau.push_back( 2 * tau[n] - tau[n - 1] ); //add end interval(n+1)
            fdiscrete.push_back( fdiscrete[n] + ( tau[n] - tau[n - 1] ) / ( tau[n] - tau[n - 2] ) * ( fdiscrete[n] - fdiscrete[n - 1] ) );
            //(74) fi
            for ( unsigned int j = 0; j <= n; j++ )
            {
                f[j] = ( falseTau[j + 1] - falseTau[j] ) / ( falseTau[j + 2] - falseTau[j] ) * fdiscrete[j + 1]
                       + ( falseTau[j + 2] - falseTau[j + 1] ) / ( falseTau[j + 2] - falseTau[j] ) * fdiscrete[j];
            }
            //(68) theta_minus
            for ( unsigned int j = 1; j <= n - 1; j++ )
            {
                theta_m[j] = ( falseTau[j + 1] - falseTau[j] ) / ( falseTau[j + 1] - falseTau[j - 1] ) * ( fdiscrete[j] - fdiscrete[j - 1] );
            }
            //(71) theta_plus
            for ( unsigned int j = 1; j <= n - 1; j++ )
            {
                theta_p[j] = ( falseTau[j + 2] - falseTau[j + 1] ) / ( falseTau[j + 3] - falseTau[j + 1] ) * ( fdiscrete[j + 2] - fdiscrete[j + 1] );
            }
            //(67) fmin1, fmax1
            for ( unsigned int j = 1; j <= n - 1; j++ )
            {
                if ( fdiscrete[j - 1] < fdiscrete[j] && fdiscrete[j] <= fdiscrete[j + 1] )
                {
                    fmin1[j] = std::min( fdiscrete[j] + 0.5 * theta_m[j], fdiscrete[j + 1] );
                    fmax1[j] = std::min( fdiscrete[j] + 2.0 * theta_m[j], fdiscrete[j + 1] );
                }
                else if ( fdiscrete[j - 1] < fdiscrete[j] && fdiscrete[j] > fdiscrete[j + 1] )
                {
                    fmin1[j] = std::max( fdiscrete[j] - 0.5 * lambda_ * theta_m[j], fdiscrete[j + 1] );
                    fmax1[j] = fdiscrete[j];
                }
                else if ( fdiscrete[j - 1] >= fdiscrete[j] && fdiscrete[j] <= fdiscrete[j + 1] )
                {
                    fmin1[j] = fdiscrete[j];
                    fmax1[j] = std::min( fdiscrete[j] - 0.5 * lambda_ * theta_m[j], fdiscrete[j + 1] );
                }
                else if ( fdiscrete[j - 1] >= fdiscrete[j] && fdiscrete[j] > fdiscrete[j + 1] )
                {
                    fmin1[j] = std::max( fdiscrete[j] + 2.0 * theta_m[j], fdiscrete[j + 1] );
                    fmax1[j] = std::max( fdiscrete[j] + 0.5 * theta_m[j], fdiscrete[j + 1] );
                }
            }
            //(70) fmin2, fmax2
            for ( unsigned int j = 1; j <= n - 1; j++ )
            {
                if ( fdiscrete[j] < fdiscrete[j + 1] && fdiscrete[j + 1] <= fdiscrete[j + 2] )
                {
                    fmin2[j] = std::max( fdiscrete[j + 1] - 2.0 * theta_p[j], fdiscrete[j] );
                    fmax2[j] = std::max( fdiscrete[j + 1] - 0.5 * theta_p[j], fdiscrete[j] );
                }
                else if ( fdiscrete[j] < fdiscrete[j + 1] && fdiscrete[j + 1] > fdiscrete[j + 2] )
                {
                    fmin2[j] = std::max( fdiscrete[j + 1] + 0.5 * lambda_ * theta_p[j], fdiscrete[j] );
                    fmax2[j] = fdiscrete[j + 1];
                }
                else if ( fdiscrete[j] >= fdiscrete[j + 1] && fdiscrete[j + 1] < fdiscrete[j + 2] )
                {
                    fmin2[j] = fdiscrete[j + 1];
                    fmax2[j] = std::min( fdiscrete[j + 1] + 0.5 * lambda_ * theta_p[j], fdiscrete[j] );
                }
                else if ( fdiscrete[j] >= fdiscrete[j + 1] && fdiscrete[j + 1] >= fdiscrete[j + 2] )
                {
                    fmin2[j] = std::min( fdiscrete[j + 1] - 0.5 * theta_p[j], fdiscrete[j] );
                    fmax2[j] = std::min( fdiscrete[j + 1] - 2.0 * theta_p[j], fdiscrete[j] );
                }
            }
            for ( unsigned int j = 1; j <= n - 1; j++ ) //ameliorate fi
            {
                if ( std::max( fmin1[j], fmin2[j] ) <= std::min( fmax1[j], fmax2[j] ) ) //overlap case
                {
                    //(75, 76)
                    f[j] = std::max( std::max( fmin1[j], fmin2[j] ), std::min( f[j], std::min( fmax1[j], fmax2[j] ) ) ); //adjust in the common range
                }
                else //not overlap case
                {
                    //(78)
                    f[j] = std::max( std::min( fmax1[j], fmax2[j] ), std::min( f[j], std::max( fmin1[j], fmin2[j] ) ) ); //adjust in the gap
                }
            }
            //(79) ameliorate f0
            if ( std::abs( f[0] - fdiscrete[0] ) > 0.5 * std::abs( f[1] - fdiscrete[0] ) )
            {
                f[0] = fdiscrete[1] - 0.5 * ( f[1] - fdiscrete[0] );
            }
            //(80) ameliorate fn
            if ( std::abs( f[n] - fdiscrete[n] ) > 0.5 * std::abs( f[n - 1] - fdiscrete[n] ) )
            {
                f[n] = fdiscrete[n] - 0.5 * ( f[n - 1] - fdiscrete[n] );
            }
        }

        index_ = tau;
        value_ = input;
        fdiscrete_ = fdiscrete;
        f_ = f;
        size_ = n + 1;
        return;

*/