#include "SimpsonsRuleIntegration.h"
#include "ExceptionMacros.h"
#include "LAStepInterpolation.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LALinearSplineInterpolation.h"
#include "LALinearMonotoneSplineInterpolation.h"
#include "LAMonotoneSplineInterpolation.h"
#include <omp.h>

namespace etrading
{
    namespace SimpsonsRule
    {
        /* Brief: Target Function for Numerical Integration using Simpsons Rule
            @param[in]  terms               The function's X-Axis term values, x
            @param[in]  values              The function's Y-Axis values, y=f(x)
            @param[in]  interpolationType   Interpolation Type: LINEAR_INTERPOLATION or SPLINE_INTERPOLATION
            *** Cannot use Linear-Spline interpolation with this construct ***
        */
        TargetFunction::TargetFunction( const DoubleVector & terms, const DoubleVector & values, const InterpolationEnum & interpolationType )
            : terms(terms), values(values), interpolationType(interpolationType), joinDateAsDouble(0.0) // Set join to zero - not needed
        {
            AQ_REQUIRE( terms.size() == values.size(),
                "Invalid Numerical Integration Configuration - Invalid Target Function: The number of terms and values must be the same" )
            
            AQ_REQUIRE( values.size() >= 3,
                "Invalid Numerical Integration Configuration - Invalid Target Function: A minimum of 3 data points are required" )
            
            AQ_REQUIRE( interpolationType != LINEARSPLINE_INTERPOLATION,
                "Invalid Numerical Integration Configuration - Invalid Target Function Set-Up: Linear-Spline specified with no join date" )

            AQ_REQUIRE( interpolationType == LINEAR_INTERPOLATION || interpolationType == SPLINE_INTERPOLATION || interpolationType == LINEARSPLINE_INTERPOLATION || interpolationType == STEP_INTERPOLATION || interpolationType == RIGHT_CONTINUOUS_INTERPOLATION || interpolationType == LEFT_CONTINUOUS_INTERPOLATION,
                "Invalid Numerical Integration Configuration - Invalid Interpolation: We currently support Step, Linear, Spline and Linear-Spline interpolation only" )
        }

        /* Brief: Target Function for Numerical Integration using Simpsons Rule
            @param[in]  terms               The function's X-Axis term values, x
            @param[in]  values              The function's Y-Axis values, y=f(x)
            @param[in]  interpolationType   Interpolation Type: LINEAR_INTERPOLATION, SPLINE_INTERPOLATION or LINEARSPLINE_INTERPOLATION
            @param[in]  joinDate            Join Date for Linear-Interpolation, set to zero to disable
            *** Supports Linear-Spline interpolation ***
        */
        TargetFunction::TargetFunction( const DoubleVector & terms, const DoubleVector & values, const InterpolationEnum & interpolationType, const double & joinDateAsDouble )
            : terms(terms), values(values), interpolationType(interpolationType), joinDateAsDouble(joinDateAsDouble)
        {
            AQ_REQUIRE( terms.size() == values.size(),
                "Invalid Target Function: The number of terms and values must be the same" )
            
            AQ_REQUIRE( values.size() >= 3,
                "Invalid Target Function: A minimum of 3 data points are required" )
            
            AQ_REQUIRE( interpolationType == LINEAR_INTERPOLATION || interpolationType == SPLINE_INTERPOLATION || interpolationType == LINEARSPLINE_INTERPOLATION || interpolationType == STEP_INTERPOLATION,
                "Invalid Target Function Interpolation: We currently support Step, Linear, Spline and Linear-Spline interpolation only" )
        }

        // Helper Function to Convert the interpolation method enum to the LAPriceDataInterpolation identifier string method
        LAString toInterpolationMethod( const InterpolationEnum & interpolationType )
        {
            switch( interpolationType )
            {
                case etrading::STEP_INTERPOLATION:
                    return "STEP";
                case etrading::LINEAR_INTERPOLATION:
                    return "LINEAR";
                case etrading::SPLINE_INTERPOLATION:
                    return "SPLINE";
                case etrading::LINEARSPLINE_INTERPOLATION:
                    return "LINEARSPLINE";
				default:
					AQ_THROW( "Invalid Interpolation Method: Only Step, Linear, Spline and LinearSpline are supported" )
            }   
        }

    }
    // End of SimpsonsRule namespace


    // Simpsons Rule Interpolation Constructor
    SimpsonsRuleIntegrand::SimpsonsRuleIntegrand( const srTargetFunction & targetFunction )
        : targetFunction_( targetFunction )
    {
        // Initialize the Target Interpolation Pointer
        setUpTargetFunction();
    }

    // Simpsons Rule Interpolation Constructor
    SimpsonsRuleIntegrand::SimpsonsRuleIntegrand( const DoubleVector & terms, const DoubleVector & values, const InterpolationEnum & interpolationType )
    {
        // Initialize the Target Interpolation
        targetFunction_ = SimpsonsRule::TargetFunction( terms, values, interpolationType );
        
        // Initialize the Target Interpolation Pointer
        setUpTargetFunction();
    }

    // Copy Constructors
    SimpsonsRuleIntegrand::SimpsonsRuleIntegrand( const SimpsonsRuleIntegrand & rhs )
        : targetFunction_( rhs.targetFunction_ )
    {
    }
        
    // Assignment Operator
    SimpsonsRuleIntegrand & SimpsonsRuleIntegrand::operator=( const SimpsonsRuleIntegrand & rhs )
    {        
        // For Exception Safety
            
        // 1. Make a temp copy
        SimpsonsRuleIntegrand tempCopy( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( targetFunction_, tempCopy.targetFunction_ );
            
        return *this;
    }

    void SimpsonsRuleIntegrand::setUpTargetFunction()
    {
        // Set Interpolation Type
        switch( targetFunction_.interpolationType )
        {
            case STEP_INTERPOLATION:
            {
                targetInterpolationPtr_ = std::make_shared<LAStepInterpolation>();
                break;
            }
            case LINEARSPLINE_INTERPOLATION:
            {
                targetInterpolationPtr_ = std::make_shared<LALinearSplineInterpolation>();
                break;
            }
			case LINEARMONOTONESPLINE_INTERPOLATION:
            {
                targetInterpolationPtr_ = std::make_shared<LALinearMonotoneSplineInterpolation>(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND);
                break;
            }
			case LINEARMONOTONEPARABOLIC_INTERPOLATION:
            {
                targetInterpolationPtr_ = std::make_shared<LALinearMonotoneSplineInterpolation>(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC);
                break;
            }
            case LINEAR_INTERPOLATION:
            {
                targetInterpolationPtr_ = std::make_shared<LALinearInterpolation>();
                break;
            }
            case SPLINE_INTERPOLATION:
            {
                targetInterpolationPtr_ = std::make_shared<LASplineInterpolation>();
                break;
            }
			case MONOTONESPLINE_INTERPOLATION:
            {
                targetInterpolationPtr_ = std::make_shared<LAMonotoneSplineInterpolation>(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND);
                break;
            }
			case MONOTONEPARABOLIC_INTERPOLATION:
            {
                targetInterpolationPtr_ = std::make_shared<LAMonotoneSplineInterpolation>(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC);
                break;
            }
            default:
            {
                AQ_THROW("Invalid Target Function Interpolation Type: Currently only Step, Linear, Spline, MonotoneSpline, LinearSpline, and LinearMonotoneSpline are supported")
            }
        }

        // Set Interpolation Values
        targetInterpolationPtr_->set( targetFunction_.terms, targetFunction_.values );

        // Set Interpolation Join Date
        targetInterpolationPtr_->setJoinDateAsDouble( targetFunction_.joinDateAsDouble );
    }
        
    // Perform Numerical Integration using Simpson's Rule
    double SimpsonsRuleIntegrand::integrate( const srIntegrationLimits & integrationLimits ) const
    {
        AQ_REQUIRE( targetInterpolationPtr_ != nullptr, 
            "Unable to Perform Numerical Integration - Target Function has not been initialized" )

        const double integrandWidth = integrationLimits.upperBound - integrationLimits.lowerBound;
            
        AQ_REQUIRE( integrandWidth >= 0,
            "Invalid Numerical Integration Limits - Integration Upper Bound < Lower Bound" )

        AQ_REQUIRE ( integrationLimits.nSteps > 0,
            "Invalid Numerical Integration Limits - Integration nSteps must be greater than zero" )

        AQ_REQUIRE ( integrationLimits.nSteps <= 10000,
            "Invalid Numerical Integration Limits - Maximum Integration nSteps must be less than 10,000" )

        // The number of abscissae when using Simpson's Rule must be EVEN
        size_t numberOfEvenSteps = integrationLimits.nSteps;
        if ( integrationLimits.nSteps % 2 != 0 )
        {
            // Make number of steps even if odd
            numberOfEvenSteps++;
        }

        // Integrand equals zero if we integrate over a null space
        if ( AQ_IS_EQUAL_ZERO( integrandWidth ) )
        {
            return 0.0;
        }
        
        // Simpson's Rule Formula
        // ---------------------
        //
        // Area = (dx/3) x [ y0 + 4( y1 + y3 + y5 + ... ) + 2( y2 + y4 + y6 + ... ) + yn ]
        //      = ( STEP_SIZE / 3 ) x [ FIRST_Y + 4( ODD_Ys ) + 2( EVEN_Ys ) + LAST_Y ]
        //
        // where x = terms, y = f(x) = values at abscissae, dx = step size
        // note we define stepSizeFactor = dx/3
        //
        // ---------------------

        // Apply Simpsons Rule
        double area = 0.0;

        const double stepSize       = ( integrandWidth / (double)(numberOfEvenSteps) ); // Divide by zero check above
        const double stepSizeFactor = ( stepSize / 3.0 );

        // The abscissa is the evaluation point. This is the time slice where we evaluate the target function
        double thisAbscissa = integrationLimits.lowerBound;

        // Note: We must use "<=" nSteps to capture both lower- and upperBounds
        for (size_t i = 0; i <= numberOfEvenSteps; ++i)
        {
            if ( i == 0 || i == numberOfEvenSteps )
            {
                // Front and End Abscissae
                area += stepSizeFactor * targetInterpolationPtr_->value( thisAbscissa );
            }
            else if ( i % 2 == 0 )
            {
                // Even Abscissae: Multiply by 2.0
                area += stepSizeFactor * 2.0 * targetInterpolationPtr_->value( thisAbscissa );
            }
            else
            {
                // Odd Abscissae: Multiply by 4.0
                area += stepSizeFactor * 4.0 * targetInterpolationPtr_->value( thisAbscissa );
            }

            // Increment the abscissa evaluation point by one step
            thisAbscissa += stepSize;
        }

        return area;
    }

    double SimpsonsRuleIntegrand::integrate( const double & lowerBound, const double & upperBound, const size_t & nSteps ) const
    {
        srIntegrationLimits integrationLimits;
        integrationLimits.lowerBound = lowerBound;
        integrationLimits.upperBound = upperBound;
        integrationLimits.nSteps = nSteps;

        const double area = integrate( integrationLimits );
        return area;
    }

    DoubleVector SimpsonsRuleIntegrand::integrate( const DoubleVector & lowerBounds, const DoubleVector & upperBounds, const size_t & nSteps, const bool optimize ) const
    {
        AQ_REQUIRE( lowerBounds.size() == upperBounds.size(), "Unable to evaluate the integrals provided. The number of lower- and upperbounds are inconsistent" )
        AQ_REQUIRE( upperBounds.size() > 0, "Unable to evaluate the integrals provided. No integral limits have been provided" )

        size_t resultsSize = upperBounds.size();
        DoubleVector results( resultsSize );
        
        // Optimize uses OMP threading
        if ( optimize )
        {
            // Find the maximum number of hardware threads (independent cores) on this machine
            const int nThreads = omp_get_max_threads();

            // Parallelized Calculation
            #pragma omp parallel for num_threads( nThreads )
            for( int i = 0; i < (int)(resultsSize); ++i ) // OMP uses int loops, not size_t
            {
                results[i] = integrate( lowerBounds[i], upperBounds[i], nSteps );
            }
        }
        else
        {
            for( size_t i = 0; i<resultsSize; ++i )
            {
                // Bounds checks performed above to prevent access violation errors
                results[i] = integrate( lowerBounds[i], upperBounds[i], nSteps );
            }
        }

        return results;
    }

}

