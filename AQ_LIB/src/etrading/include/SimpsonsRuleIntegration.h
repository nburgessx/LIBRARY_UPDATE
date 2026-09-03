/*
 * @brief			Numerical Integration - Simpsons Rule
 * @Created:		6th May 2018
 * @Author:			Nicholas Burgess
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once
#include "LACoreTemplateType.h"
#include "CoreEnumerations.h"
#include "LAInterpolationBase.h"

namespace etrading
{
    namespace SimpsonsRule
    {
        // IntegrationLimits Struct to Pass Parameters as a Single Unit
        // --------------------------------------------------------------
        // nSteps: Number of abscissae must be even, we manage this in the SimpsonsRuleIntegrand constructor
        struct IntegrationLimits
	    {
		    double  lowerBound;
            double  upperBound;
            size_t  nSteps;
        };


        // Target Function Class to ensure joinDate is specified
        // --------------------------------------------------------------
        class TargetFunction
        {
        public:
            TargetFunction() {};
            virtual ~TargetFunction() {};

            /* Brief: Target Function for Numerical Integration using Simpsons Rule
                @param[in]  terms               The function's X-Axis term values, x
                @param[in]  values              The function's Y-Axis values, y=f(x)
                @param[in]  interpolationType   Interpolation Type: LINEAR_INTERPOLATION or SPLINE_INTERPOLATION
                Cannot use Linear-Spline interpolation with this construct
            */
            TargetFunction( const DoubleVector & terms,
                            const DoubleVector & values,
                            const InterpolationEnum & interpolationType );

            /* Brief: Target Function for Numerical Integration using Simpsons Rule
                @param[in]  terms               The function's X-Axis term values, x
                @param[in]  values              The function's Y-Axis values, y=f(x)
                @param[in]  interpolationType   Interpolation Type: LINEAR_INTERPOLATION, SPLINE_INTERPOLATION or LINEARSPLINE_INTERPOLATION
                @param[in]  joinDate            Join Date for Linear-Interpolation, set to zero to disable
                Supports Linear-Spline interpolation
            */
            TargetFunction( const DoubleVector & terms,
                            const DoubleVector & values,
                            const InterpolationEnum & interpolationType,
                            const double & joinDateAsDouble );

            DoubleVector            terms;
            DoubleVector            values;
            InterpolationEnum       interpolationType;
            double                  joinDateAsDouble;
        };

        // Helper Function to Convert the interpolation method enum to the LAPriceDataInterpolation identifier string method
        LAString toInterpolationMethod( const InterpolationEnum & interpolationType );

    }
    // End of Namespace SimpsonsRule
    // ------------------------------

    
    // Typedef Helpers
    // -------------------------------------------------
    typedef SimpsonsRule::TargetFunction srTargetFunction;
    typedef SimpsonsRule::IntegrationLimits srIntegrationLimits;
    // -------------------------------------------------


    // Class to Numerically integrate a target function based on a set of interpolated data points
    class SimpsonsRuleIntegrand
    {
    public:

        // Constructor / Destructor
        SimpsonsRuleIntegrand() {};
        virtual ~SimpsonsRuleIntegrand() {};

        // Alternative Constructors
        SimpsonsRuleIntegrand( const srTargetFunction & targetFunction ); 
        SimpsonsRuleIntegrand( const DoubleVector & terms, const DoubleVector & values, const InterpolationEnum & interpolationType ); 

        // Copy Constructors
        SimpsonsRuleIntegrand( const SimpsonsRuleIntegrand & rhs );
        
        // Assignment Operator
        SimpsonsRuleIntegrand & operator=( const SimpsonsRuleIntegrand & rhs );

         // Perform Numerical Integration using Simpson's Rule
        double integrate( const srIntegrationLimits & integrationLimits ) const;
        double integrate( const double & lowerBound, const double & upperBound, const size_t & nSteps ) const;
        DoubleVector integrate( const DoubleVector & lowerBounds, const DoubleVector & upperBounds, const size_t & nSteps, const bool optimize = false ) const;

    private:
       
         // Initialize the TargetFunction
        void setUpTargetFunction();

        std::shared_ptr<LAInterpolationBase> targetInterpolationPtr_;
        srTargetFunction targetFunction_;
    };
}

