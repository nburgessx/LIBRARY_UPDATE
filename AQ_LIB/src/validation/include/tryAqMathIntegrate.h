// tryAqMathIntegrate.h

/*
 * @brief			validation interface for the Integration of Interpolators
 */

#pragma once
#include <vector>
#include <string>

// Forward Declaration
class AQLDate;

namespace validation
{
    /* @brief			Calculate the Numerical Integral of a Interpolator Target Function
    *  @param [in]		terms			    A vector of terms year fractions, x-axis
    *  @param [in]		values				A vector of values, y-axis where y=f(x)
    *  @param [in]		interpolatioType	A string for the interpolation type, can be LinearInterpolation, SplineInterpolation, LinearSplineInterpolation or LinearMonotoneSplineInterpolation
    *  @param [in]		joinDateAsDouble    When using mixed hybrid interpolation a join date year fraction is required
    *  @param [in]		lowerBounds         A vector of lower bounds of the integrand
    *  @param [in]		upperBounds         A vector of upper bounds of the integrand
    *  @param [in]		nSteps              The number of integration steps to take / the number of abscissae
    *  @param [in]		optimize            Enable OMP threading: True / False
    *  @return			The numerically intergrated area
    */
    std::vector<double> tryAqMathIntegrateUsingTerms( const std::vector<double> & terms,
                                                      const std::vector<double> & values,
                                                      const std::string & interpolationType,
                                                      const double & joinDateAsDouble,
                                                      const std::vector<double> & lowerBounds,
                                                      const std::vector<double> & upperBounds,
                                                      const unsigned int & nSteps,
                                                      const bool optimize = false );

    /* @brief			Calculate the Numerical Integral of a Interpolator Target Function
    *  @param [in]		asOfDate			The startDate / asOfDate for the interpolator. The base date used to convert terms dates to terms year fractions
    *  @param [in]		dates			    A vector of dates, x-axis
    *  @param [in]		values				A vector of values, y-axis where y=f(x)
    *  @param [in]		interpolationType	A string for the interpolation type, can be LinearInterpolation, SplineInterpolation, LinearSplineInterpolation or LinearMonotoneSplineInterpolation
    *  @param [in]		joinDate            When using mixed hybrid interpolation a join date is required
    *  @param [in]		lowerBoundDates     A vector of lower bounds for the integrand
    *  @param [in]		upperBoundDate s    A vector of upper bounds for the integrand
    *  @param [in]		nSteps              The number of integration steps to take / the number of abscissae
    *  @param [in]		optimize            Enable OMP threading: True / False
    *  @return			The numerically intergrated area
    */
    std::vector<double> tryAqMathIntegrate( const AQLDate & asOfDate,
                                            const std::vector<AQLDate> & dates,
                                            const std::vector<double> & values,
                                            const std::string & interpolationType,
                                            const AQLDate & joinDate,
                                            const std::vector<AQLDate> & lowerBoundDates,
                                            const std::vector<AQLDate> & upperBoundDates,
                                            const unsigned int & nSteps,
                                            const bool optimize = false );
}
