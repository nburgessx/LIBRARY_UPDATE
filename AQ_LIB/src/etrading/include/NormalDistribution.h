#pragma once

namespace etrading
{
    // Calculate the standard normal distribution probability phi, given the standard normal variate x
	// This function is templated on "xdouble", which may be double or adept::adouble.
	// i.e. there is an AAD and regular double version of this function.
	template<typename xdouble>
    xdouble standardNormalDistribution( const xdouble & x );

    // Calculate the standard normal distribution probability density function value pdf, given the standard normal variate x
	// This function is templated on "xdouble", which may be double or adept::adouble.
	// i.e. there is an AAD and regular double version of this function.
	template<typename xdouble>
	xdouble standardNormalDistributionPDF( const xdouble & x );

    // Calculate the standard normal variate z,  given the standard normal probability phi
    const double standardNormalDistributionInverse( const double & phi );
    
	// Calculate the non-standard normal distribution probability phi, given the standard normal variate z and the mean and variance of the normal distribution
	const double normalDistribution(const double & x, const double & mean, const double & variance);

    // Calculate the non-standard normal distribution probability density function value pdf, given the standard normal variate z and the mean and variance of the normal distribution
    const double normalDistributionPDF( const double & x, const double & mean, const double & variance );

    // Calculate the non-standard normal variate z,  given the standard normal probability phi  and the mean and variance of the normal distribution
    const double normalDistributionInverse( const double & phi, const double & mean, const double & variance );
}