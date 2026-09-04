//
// @File: StatisticsUtilities.cpp
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.

#include <limits>

#include "StatisticsUtilities.h"
#include "boost/random.hpp"


namespace etrading
{

    int sampleInteger( int min, int max )
    {
        if( max < min )
        {
            std::swap( min, max );
        }
        boost::random::uniform_int_distribution<int> uniformGenerator( min, max ); // guaranteed unbiased
        return uniformGenerator( statisticsHelpers::mersenneTwister );
    };

    double rUniform( double a, double b )
    {
        if( b < a )
        {
            std::swap( a, b );
        }
        boost::random::uniform_real_distribution<> dist( a, b );
        return dist( statisticsHelpers::mersenneTwister );
    };

    const bool doubleEquals( const double a, const double b )
    {
        return ( std::abs( a - b ) < 1E-10 ); // std::numeric_limits<double>::epsilon()
    };

}