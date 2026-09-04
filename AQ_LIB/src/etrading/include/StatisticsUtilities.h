//
// @File: StatisticsUtilities.h
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.

#pragma once

#include <vector>
#include <utility>
#include <math.h> // sqrt
#include <random>

#include <boost/random.hpp>
#include <boost/random/random_device.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/range/numeric.hpp>

// TODO: move to std::accumulate/std::is_arithmetic C++11 VS2015
// TODO: look at doing the calculations for generic containers (not just std::vector) using std::begin/std::end


namespace etrading
{
    namespace statisticsHelpers
    {
        static boost::random_device random_device;     // for the seed engine initialization
        static boost::random::mt19937 mersenneTwister( statisticsHelpers::random_device() );  // https://en.wikipedia.org/wiki/Mersenne_Twister with MT19937 implementation
    }

    // calculates the mean of a vector
    template < typename Z>
    typename std::enable_if< boost::is_arithmetic<Z>::value, double>::type
    calcMean( const std::vector<Z>& inputs )
    {
        double sum = boost::accumulate( inputs, 0.0 );
        return ( sum / inputs.size() );
    }

    // calculates the mean and sample standard deviation of a vector of numbers
    template < typename Z>
    typename std::enable_if< boost::is_arithmetic<Z>::value, std::pair<double, double> >::type
    calcMeanAndSampleStd( const std::vector<Z>& inputs )
    {
        double mean = calcMean<Z>( inputs );
        double tempAcc = 0.0;
        std::for_each ( inputs.cbegin(), inputs.cend(), [&mean,&tempAcc]( const Z & d )
        {
            tempAcc += ( d - mean ) * ( d - mean );
        } );
        double stdev = sqrt( tempAcc / ( inputs.size() - 1 ) );
        return std::make_pair( mean, stdev );
    }

    int sampleInteger( int min, int max );
    double rUniform( double a, double b );

    const bool doubleEquals( const double, const double );
};

