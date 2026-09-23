// aqMathNumerical.h

/*
 * @brief			Swig interface for the aqMathPolynomial*, aqMathPoynomial* and aqMathIntegrate* numerical methods
 *                  Note: "Poynomial" is the pre-existing spelling used by the validation wrapper name - kept verbatim
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqMathPolynomialInterpolation - fit a polynomial of the given degree to (x, y) and evaluate it at a single x
*  @param [in]		xValues		Polynomial time vector
*  @param [in]		yValues		Polynomial value vector
*  @param [in]		degree		Polynomial degree
*  @param [in]		x			Interpolation point
*  @return			The interpolated y value at x
*/
double aqMathPolynomialInterpolation( const std::vector<double>& xValues, const std::vector<double>& yValues, const unsigned int degree, const double x );

/* @brief			swig interface for aqMathPolynomialInterpolations - fit a polynomial of the given degree to (x, y) and evaluate it at multiple x values
*  @return			The interpolated y value(s) at x
*/
std::vector<double> aqMathPolynomialInterpolations( const std::vector<double>& xValues, const std::vector<double>& yValues, const unsigned int degree, const std::vector<double>& x );

/* @brief			swig interface for aqMathPoynomialIntegration - fit a polynomial and integrate it between a single pair of bounds
*  @param [in]		lowerBound	Integration lower bound
*  @param [in]		upperBound	Integration upper bound
*  @return			The integral over the lower- and upper bound
*/
double aqMathPoynomialIntegration( const std::vector<double>& xValues, const std::vector<double>& yValues, const unsigned int degree, const double lowerBound, const double upperBound );

/* @brief			swig interface for aqMathPoynomialIntegrations - fit a polynomial and integrate it over multiple bound pairs
*  @return			The integral(s) over each lower- and upper bound
*/
std::vector<double> aqMathPoynomialIntegrations( const std::vector<double>& xValues, const std::vector<double>& yValues, const unsigned int degree, const std::vector<double>& lowerBounds, const std::vector<double>& upperBounds );

/* @brief			swig interface for aqMathIntegrateUsingTerms - numerically integrate an interpolated (term, value) target function
*  @param [in]		terms				A vector of term year fractions, x-axis
*  @param [in]		values				A vector of values, y-axis where y=f(x)
*  @param [in]		interpolationType	Interpolation type, e.g. LINEAR, SPLINE, LINEAR_SPLINE, LINEAR_MONOTONE_SPLINE
*  @param [in]		joinDateAsDouble	When using mixed hybrid interpolation, a join-date year fraction is required
*  @param [in]		lowerBounds			A vector of lower bounds of the integrand
*  @param [in]		upperBounds			A vector of upper bounds of the integrand
*  @param [in]		nSteps				The number of integration steps to take / the number of abscissae
*  @param [in]		optimize			Optional. Default FALSE. Enable OMP threading
*  @return			The numerically integrated area(s)
*/
std::vector<double> aqMathIntegrateUsingTerms( const std::vector<double>& terms,
                                                const std::vector<double>& values,
                                                const std::string& interpolationType,
                                                const double joinDateAsDouble,
                                                const std::vector<double>& lowerBounds,
                                                const std::vector<double>& upperBounds,
                                                const unsigned int nSteps,
                                                const bool optimize = false );

/* @brief			swig interface for aqMathIntegrate - numerically integrate an interpolated (date, value) target function
*  @param [in]		asOfDate			The start / as-of date for the interpolator - the base date used to convert term dates to term year fractions
*  @param [in]		dates				A vector of dates (YYYYMMDD), x-axis
*  @param [in]		values				A vector of values, y-axis where y=f(x)
*  @param [in]		interpolationType	Interpolation type
*  @param [in]		joinDate			Optional. Join date for a mixed hybrid interpolation scheme. Blank for none
*  @param [in]		lowerBoundDates		A vector of lower bound dates (YYYYMMDD) for the integrand
*  @param [in]		upperBoundDates		A vector of upper bound dates (YYYYMMDD) for the integrand, aligned with lowerBoundDates
*  @param [in]		nSteps				The number of integration steps to take / the number of abscissae
*  @param [in]		optimize			Optional. Default FALSE. Enable OMP threading
*  @return			The numerically integrated area(s)
*/
std::vector<double> aqMathIntegrate( const std::string& asOfDate,
                                      const std::vector<std::string>& dates,
                                      const std::vector<double>& values,
                                      const std::string& interpolationType,
                                      const std::string& joinDate,
                                      const std::vector<std::string>& lowerBoundDates,
                                      const std::vector<std::string>& upperBoundDates,
                                      const unsigned int nSteps,
                                      const bool optimize = false );
