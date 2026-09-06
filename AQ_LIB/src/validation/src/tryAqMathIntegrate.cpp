// tryAqMathIntegrate.cpp

/*
 * @brief			validation interface for Black-Scholes method(s)
 */

#include "tryAqMathIntegrate.h"
#include "CoreEnumerations.h"
#include "SimpsonsRuleIntegration.h"
#include "CurveValidation.h"
#include "CurveStreaming.h"

// Includes for Structured Exception Handling and Input/Output Logging
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"
#include "RecordMacros.h"

// Namespaces for Input/Output Logging
using etrading::CreateDataFile;
using etrading::decorateFilename;


namespace validation
{
    /* @brief			Calculate the Numerical Integral of a Interpolator Target Function
    *  @param [in]		terms			    A vector of terms year fractions, x-axis
    *  @param [in]		values				A vector of values, y-axis where y=f(x)
    *  @param [in]		interpolationType	A string for the interpolation type, can be LinearInterpolation, SplineInterpolation, LinearSplineInterpolation or LinearMonotoneSplineInterpolation
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
                                                      const bool optimize )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( terms, values, interpolationType, joinDateAsDouble, lowerBounds, upperBounds, nSteps );

        // Calculation
        // ------------------------------------------------------------------------------

        // Initialize Target Function and Integrand
        etrading::InterpolationEnum interpolationEnum = etrading::toInterpolationEnum( interpolationType ); // from 'std::string' to 'etrading::InterpolationEnum'
        etrading::SimpsonsRule::TargetFunction target( terms, values, interpolationEnum, joinDateAsDouble );
        etrading::SimpsonsRuleIntegrand integrand( target );

        // Numerically Integrate
        AQ_REQUIRE( lowerBounds.size() == upperBounds.size(), "Invalid Integration Limits - The number of integration lower- and upperBounds is inconsistent" )
        DoubleVector results = integrand.integrate( lowerBounds, upperBounds, nSteps, optimize );
        
        // ------------------------------------------------------------------------------

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( results );

		VALID_EXCEPTION_END
    }

    /* @brief			Calculate the Numerical Integral of a Interpolator Target Function
    *  @param [in]		asOfDate			The startDate / asOfDate for the interpolator. The base date used to convert terms dates to terms year fractions
    *  @param [in]		dates			    A vector of dates, x-axis
    *  @param [in]		values				A vector of values, y-axis where y=f(x)
    *  @param [in]		interpolatioType	A string for the interpolation type, can be LinearInterpolation, SplineInterpolation, LinearSplineInterpolation or LinearMonotoneSplineInterpolation
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
                                            const bool optimize )
    {
        // No Thread Guard here since this has been delegated to the underlying itegration method also in the validation
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD

        // Convert Date Inputs to Terms Year Fractions - needed for underlying math function
        const std::vector<double> terms                 = etrading::convertCurveDatesToTerms( asOfDate, dates );
        const double joinDateAsDouble                   = etrading::convertCurveDateToTerm( asOfDate, joinDate );
        const std::vector<double> lowerBoundsAsDouble   = etrading::convertCurveDatesToTerms( asOfDate, lowerBoundDates );
        const std::vector<double> upperBoundsAsDouble   = etrading::convertCurveDatesToTerms( asOfDate, upperBoundDates );

        // Call the underlying integration method, which manages the thread guard and test recording
        const DoubleVector result = tryAqMathIntegrateUsingTerms( terms, values, interpolationType, joinDateAsDouble, lowerBoundsAsDouble, upperBoundsAsDouble, nSteps, optimize );
        return result;

        VALID_EXCEPTION_END
    }
}
