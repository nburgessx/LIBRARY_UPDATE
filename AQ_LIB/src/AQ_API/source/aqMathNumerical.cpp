// aqMathNumerical.cpp

/*
 * @brief			Swig interface for the aqMathPolynomial*, aqMathPoynomial* and aqMathIntegrate* numerical methods
 */

#include "aqMathNumerical.h"
#include "TypeUtilities.h"
#include "ParameterValidation.h"    // etrading::stringToDate
#include "tryAqMathPolynomial.h"
#include "tryAqMathIntegrate.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

double aqMathPolynomialInterpolation( const std::vector<double>& xValues, const std::vector<double>& yValues, const unsigned int degree, const double x )
{
    AQ_API_START

    double result = validation::tryAqMathPolynomialInterpolation( xValues, yValues, degree, x );
    return result;

    AQ_API_END
}

std::vector<double> aqMathPolynomialInterpolations( const std::vector<double>& xValues, const std::vector<double>& yValues, const unsigned int degree, const std::vector<double>& x )
{
    AQ_API_START

    std::vector<double> result = validation::tryAqMathPolynomialInterpolations( xValues, yValues, degree, x );
    return result;

    AQ_API_END
}

double aqMathPoynomialIntegration( const std::vector<double>& xValues, const std::vector<double>& yValues, const unsigned int degree, const double lowerBound, const double upperBound )
{
    AQ_API_START

    double result = validation::tryAqMathPoynomialIntegration( xValues, yValues, degree, lowerBound, upperBound );
    return result;

    AQ_API_END
}

std::vector<double> aqMathPoynomialIntegrations( const std::vector<double>& xValues, const std::vector<double>& yValues, const unsigned int degree, const std::vector<double>& lowerBounds, const std::vector<double>& upperBounds )
{
    AQ_API_START

    std::vector<double> result = validation::tryAqMathPoynomialIntegrations( xValues, yValues, degree, lowerBounds, upperBounds );
    return result;

    AQ_API_END
}

std::vector<double> aqMathIntegrateUsingTerms( const std::vector<double>& terms,
                                                const std::vector<double>& values,
                                                const std::string& interpolationType,
                                                const double joinDateAsDouble,
                                                const std::vector<double>& lowerBounds,
                                                const std::vector<double>& upperBounds,
                                                const unsigned int nSteps,
                                                const bool optimize )
{
    AQ_API_START

    std::vector<double> result = validation::tryAqMathIntegrateUsingTerms(
        terms, values, interpolationType, joinDateAsDouble, lowerBounds, upperBounds, nSteps, optimize );
    return result;

    AQ_API_END
}

std::vector<double> aqMathIntegrate( const std::string& asOfDate,
                                      const std::vector<std::string>& dates,
                                      const std::vector<double>& values,
                                      const std::string& interpolationType,
                                      const std::string& joinDate,
                                      const std::vector<std::string>& lowerBoundDates,
                                      const std::vector<std::string>& upperBoundDates,
                                      const unsigned int nSteps,
                                      const bool optimize )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate asOfDate_( etrading::stringToDate( asOfDate ) );
    AQLDate joinDate_ = joinDate.empty() ? AQLDate() : AQLDate( etrading::stringToDate( joinDate ) );

    DateVector dates_;
    swig::buildDateVector( dates_, dates );

    DateVector lowerBoundDates_;
    swig::buildDateVector( lowerBoundDates_, lowerBoundDates );

    DateVector upperBoundDates_;
    swig::buildDateVector( upperBoundDates_, upperBoundDates );

    // Call Function and Return Result
    std::vector<double> result = validation::tryAqMathIntegrate(
        asOfDate_, dates_, values, interpolationType, joinDate_, lowerBoundDates_, upperBoundDates_, nSteps, optimize );
    return result;

    AQ_API_END
}
