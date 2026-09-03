// TestSimpsonsRuleForInterpolants.cpp

/*
 * @brief			Tests to validate the LWO curve calibration for CNH and BRL Curves
 * @Created:		29th May 2018
 * @Author:			Nicholas Burgess
 * @Department:		MHI London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

// Include: Google Test Library
#include <gTest/gTest.h>

#include "Dependency.h"
#include "SimpsonsRuleIntegration.h"

namespace google_test
{
    // Declare Test Fixture Class
    DECLARE_TEST_FIXTURE( TestSimpsonsRuleIntegration )

    // Call Test Fixture Class
    TEST_F( TestSimpsonsRuleIntegration, UNIT_TestNumericalIntegration_PositiveConstantFunction )
    {
        // Target Function: Constant value of 1.0 bounded from 0.0 to 10.0
        DoubleVector terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        DoubleVector values = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
        etrading::InterpolationEnum interpolationType = etrading::LINEAR_INTERPOLATION;
        
        // Initialize Target Function Class
        etrading::SimpsonsRule::TargetFunction target( terms, values, interpolationType );
        
        // Initialize Integration Settings
        etrading::SimpsonsRule::IntegrationLimits integrationLimits;
        integrationLimits.lowerBound  = 0.0;
        integrationLimits.upperBound  = 10.0;
        integrationLimits.nSteps      = 10;

        // Numerically Integrate
        etrading::SimpsonsRuleIntegrand integrand( target );
        const double resultArea = integrand.integrate( integrationLimits );
        const double expectedArea = 10.0; 

        const double tolerance = 1e-4;
        EXPECT_NEAR( resultArea, expectedArea, tolerance );
        // Area = 10 x 1 = 10
    }

    TEST_F( TestSimpsonsRuleIntegration, UNIT_TestNumericalIntegration_NegativeConstantFunction )
    {
        // Target Function: Constant value of -1.0 bounded from 0.0 to 10.0
        DoubleVector terms = { 0, 5, 10 };
        DoubleVector values = { -1, -1, -1 };
        etrading::InterpolationEnum interpolationType = etrading::LINEAR_INTERPOLATION;
        
        // Initialize Target Function Class
        etrading::SimpsonsRule::TargetFunction target( terms, values, interpolationType );
        
        // Initialize Integration Settings
        etrading::SimpsonsRule::IntegrationLimits integrationLimits;
        integrationLimits.lowerBound  = 0.0;
        integrationLimits.upperBound  = 10.0;
        integrationLimits.nSteps      = 10;

        // Numerically Integrate
        etrading::SimpsonsRuleIntegrand integrand( target );
        const double resultArea = integrand.integrate( integrationLimits );
        const double expectedArea = -10.0; 

        const double tolerance = 1e-4;
        EXPECT_NEAR( resultArea, expectedArea, tolerance );
        // Area = -10 x 1 = -10
    }

    // Call Test Fixture Class
    TEST_F( TestSimpsonsRuleIntegration, UNIT_TestNumericalIntegration_TriangularFunction )
    {
        // Target Function: The triangular function y = x bounded between 0 and 10
        DoubleVector terms = { 0, 5, 10 };
        DoubleVector values = { 0, 5, 10 };
        etrading::InterpolationEnum interpolationType = etrading::LINEAR_INTERPOLATION;
        
        // Initialize Target Function Class
        etrading::SimpsonsRule::TargetFunction target( terms, values, interpolationType );
        
        // Initialize Integration Settings
        etrading::SimpsonsRule::IntegrationLimits integrationLimits;
        integrationLimits.lowerBound  = 0.0;
        integrationLimits.upperBound  = 10.0;
        integrationLimits.nSteps      = 10;

        // Numerically Integrate
        etrading::SimpsonsRuleIntegrand integrand( target );
        const double resultArea = integrand.integrate( integrationLimits );
        const double expectedArea = 50.0; 

        const double tolerance = 1e-4;
        EXPECT_NEAR( resultArea, expectedArea, tolerance );
        // Area = 0.5 x 10 x 10 = 50
    }


    // Call Test Fixture Class
    TEST_F( TestSimpsonsRuleIntegration, UNIT_TestNumericalIntegration_StepFunction )
    {
        // Target Function: A piecewise constant or step function
        DoubleVector terms = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        DoubleVector values = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }; 
        etrading::InterpolationEnum interpolationType = etrading::LINEAR_INTERPOLATION;
        
        // Initialize Target Function Class
        etrading::SimpsonsRule::TargetFunction target( terms, values, interpolationType );
        
        // Initialize Integration Settings
        etrading::SimpsonsRule::IntegrationLimits integrationLimits;
        integrationLimits.lowerBound  = 0.0;
        integrationLimits.upperBound  = 10.0;
        integrationLimits.nSteps      = 10;

        // Numerically Integrate
        etrading::SimpsonsRuleIntegrand integrand( target );
        const double resultArea = integrand.integrate( integrationLimits );
        const double expectedArea = 50.0; 

        const double tolerance = 1e-4;
        EXPECT_NEAR( resultArea, expectedArea, tolerance );
        // Area = sum of 1 to 10 = 50
    }

    TEST_F( TestSimpsonsRuleIntegration, UNIT_TestNumericalIntegration_TriangularFunctionWithSignChange )
    {
        // Target Function: A piecewise constant or step function with a sign change
        DoubleVector terms = { -10, 0, 10 };
        DoubleVector values = { -10, 0, 10 };
        etrading::InterpolationEnum interpolationType = etrading::LINEAR_INTERPOLATION;
        
        // Initialize Target Function Class
        etrading::SimpsonsRule::TargetFunction target( terms, values, interpolationType );
        
        // Initialize Integration Settings
        etrading::SimpsonsRule::IntegrationLimits integrationLimits;
        integrationLimits.lowerBound  = -10.0;
        integrationLimits.upperBound  = 10.0;
        integrationLimits.nSteps      = 10;

        // Numerically Integrate
        etrading::SimpsonsRuleIntegrand integrand( target );
        const double resultArea = integrand.integrate( integrationLimits );
        const double expectedArea = 0.0; 

        const double tolerance = 1e-4;
        EXPECT_NEAR( resultArea, expectedArea, tolerance );
        // Area = sum of 1 to 10 = 50
    }

    TEST_F( TestSimpsonsRuleIntegration, UNIT_TestNumericalIntegration_TriangularFunctionWithSignChange_SingleLimit )
    {
        // Target Function: A piecewise constant or step function with a sign change
        DoubleVector terms = { -10, 0, 10 };
        DoubleVector values = { -10, 0, 10 };
        etrading::InterpolationEnum interpolationType = etrading::LINEAR_INTERPOLATION;
        
        // Initialize Target Function Class
        etrading::SimpsonsRule::TargetFunction target( terms, values, interpolationType );
        
        // Initialize Integration Settings
        const double lowerBound  = -10.0;
        const double upperBound  = 10.0;
        const size_t nSteps      = 10;

        // Numerically Integrate
        etrading::SimpsonsRuleIntegrand integrand( target );
        const double resultArea = integrand.integrate( lowerBound, upperBound, nSteps );
        const double expectedArea = 0.0; 

        const double tolerance = 1e-4;
        EXPECT_NEAR( resultArea, expectedArea, tolerance );
        // Area = sum of 1 to 10 = 50
    }

    TEST_F( TestSimpsonsRuleIntegration, UNIT_TestNumericalIntegration_TriangularFunctionWithSignChange_MultipleLimits )
    {
        // Target Function: Linear function y=x with a sign change
        DoubleVector terms = { -10, 0, 10 };
        DoubleVector values = { -10, 0, 10 };
        etrading::InterpolationEnum interpolationType = etrading::LINEAR_INTERPOLATION;
        
        // Initialize Target Function Class
        etrading::SimpsonsRule::TargetFunction target( terms, values, interpolationType );
        
        // Set-Up the Integrand
        etrading::SimpsonsRuleIntegrand integrand( target );

        // Initialize Integration Settings
        const double lowerBound  = -10.0;
        const double upperBound  = 10.0;
        const size_t nSteps      = 10;

        // Numerically Integrate
        const double resultArea = integrand.integrate( lowerBound, upperBound, nSteps );
        const double expectedArea = 0.0; 

        const double tolerance = 1e-4;
        EXPECT_NEAR( resultArea, expectedArea, tolerance );
        // Area = sum of 1 to 10 = 50

        // Test Multiple Limits
        const double actualArea1    = integrand.integrate( -10, -5,  nSteps );
        const double actualArea2    = integrand.integrate( -5,   0,  nSteps );
        const double actualArea3    = integrand.integrate(  0,   5,  nSteps );
        const double actualArea4    = integrand.integrate(  5,   10, nSteps );

        const double expectedArea1  = -37.5;
        const double expectedArea2  = -12.5;
        const double expectedArea3  = 12.5;
        const double expectedArea4  = 37.5;

        EXPECT_NEAR( actualArea1, expectedArea1, tolerance );
        EXPECT_NEAR( actualArea2, expectedArea2, tolerance );
        EXPECT_NEAR( actualArea3, expectedArea3, tolerance );
        EXPECT_NEAR( actualArea4, expectedArea4, tolerance );
    }

    TEST_F( TestSimpsonsRuleIntegration, UNIT_TestNumericalIntegration_TriangularFunctionWithSignChange_VectorLimits )
    {
        // Target Function: Linear function y=x with a sign change
        DoubleVector terms = { -10, 0, 10 };
        DoubleVector values = { -10, 0, 10 };
        etrading::InterpolationEnum interpolationType = etrading::LINEAR_INTERPOLATION;
        
        // Initialize Target Function Class
        etrading::SimpsonsRule::TargetFunction target( terms, values, interpolationType );
        
        // Set-Up the Integrand
        etrading::SimpsonsRuleIntegrand integrand( target );

        // Test a Vector of Integration Limits
        const DoubleVector lowerBounds = { -10, -5, 0, 5 };
        const DoubleVector upperBounds = { -5,   0, 5, 10 };
        const size_t nSteps = 10;

        const DoubleVector actualAreaResults    = integrand.integrate( lowerBounds, upperBounds, nSteps );
        const DoubleVector expectedAreaResults  = { -37.5, -12.5, 12.5, 37.5 };

        ASSERT_EQ( actualAreaResults.size(), expectedAreaResults.size() );

        const double tolerance = 1e-4;
        EXPECT_NEAR( actualAreaResults[0], expectedAreaResults[0], tolerance );
        EXPECT_NEAR( actualAreaResults[1], expectedAreaResults[1], tolerance );
        EXPECT_NEAR( actualAreaResults[2], expectedAreaResults[2], tolerance );
        EXPECT_NEAR( actualAreaResults[3], expectedAreaResults[3], tolerance );
    }

    TEST_F( TestSimpsonsRuleIntegration, UNIT_TestNumericalIntegration_SimplifiedConstructor )
    {
        // Target Function: Linear function y=x with a sign change
        DoubleVector terms = { -10, 0, 10 };
        DoubleVector values = { -10, 0, 10 };
        etrading::InterpolationEnum interpolationType = etrading::LINEAR_INTERPOLATION;
        
        // Set-Up the Integrand Directly without the need for a target function
        etrading::SimpsonsRuleIntegrand integrand( terms, values, interpolationType );

        // Test a Vector of Integration Limits
        const DoubleVector lowerBounds = { -10, -5, 0, 5 };
        const DoubleVector upperBounds = { -5,   0, 5, 10 };
        const size_t nSteps = 10;

        const DoubleVector actualAreaResults    = integrand.integrate( lowerBounds, upperBounds, nSteps );
        const DoubleVector expectedAreaResults  = { -37.5, -12.5, 12.5, 37.5 };

        ASSERT_EQ( actualAreaResults.size(), expectedAreaResults.size() );

        const double tolerance = 1e-4;
        EXPECT_NEAR( actualAreaResults[0], expectedAreaResults[0], tolerance );
        EXPECT_NEAR( actualAreaResults[1], expectedAreaResults[1], tolerance );
        EXPECT_NEAR( actualAreaResults[2], expectedAreaResults[2], tolerance );
        EXPECT_NEAR( actualAreaResults[3], expectedAreaResults[3], tolerance );
    }

    TEST_F( TestSimpsonsRuleIntegration, UNIT_TestNumericalIntegration_100K_Integrals )
    {
        // Target Function: Constant function with a value of 1.0 bounded between -10 and 10
        DoubleVector terms = { -10, 0, 10 };
        DoubleVector values = { 1, 1, 1 };
        etrading::InterpolationEnum interpolationType = etrading::LINEAR_INTERPOLATION;
        
        // Set-Up the Integrand Directly without the need for a target function
        etrading::SimpsonsRuleIntegrand integrand( terms, values, interpolationType );

        // Test a Vector of Integration Limits
        const size_t oneHunderedThousand = 100000;
        
        const DoubleVector lowerBounds( oneHunderedThousand, 0 );               // 100K LowerBounds all set to 0.0
        const DoubleVector upperBounds( oneHunderedThousand, 10 );              // 100K UpperBounds all set to 10.0
        const size_t nSteps = 1000;

        const DoubleVector actualAreaResults = integrand.integrate( lowerBounds, upperBounds, nSteps, false ); // optimize = false i.e. no threading
        const DoubleVector expectedAreaResults( oneHunderedThousand, 10.0 ) ;   // 100K results all with the value of 10

        ASSERT_EQ( actualAreaResults.size(), expectedAreaResults.size() );

        const double tolerance = 1e-4;
        for( size_t i = 0; i < expectedAreaResults.size(); ++i )
        {
            EXPECT_NEAR( actualAreaResults[i], expectedAreaResults[i], tolerance );
        }
    }

    TEST_F( TestSimpsonsRuleIntegration, UNIT_TestNumericalIntegration_100K_Integrals_OMP_Threaded )
    {
        // Target Function: Constant function with a value of 1.0 bounded between -10 and 10
        DoubleVector terms = { -10, 0, 10 };
        DoubleVector values = { 1, 1, 1 };
        etrading::InterpolationEnum interpolationType = etrading::LINEAR_INTERPOLATION;
        
        // Set-Up the Integrand Directly without the need for a target function
        etrading::SimpsonsRuleIntegrand integrand( terms, values, interpolationType );

        // Test a Vector of Integration Limits
        const size_t oneHunderedThousand = 100000;

        const DoubleVector lowerBounds( oneHunderedThousand, 0 );               // 100K LowerBounds all set to 0.0
        const DoubleVector upperBounds( oneHunderedThousand, 10 );              // 100K UpperBounds all set to 10.0
        const size_t nSteps = 1000;

        const DoubleVector actualAreaResults = integrand.integrate( lowerBounds, upperBounds, nSteps, true ); // optimize = true i.e. use OMP Threading
        const DoubleVector expectedAreaResults( oneHunderedThousand, 10.0 ) ;   // 100K results all with the value of 10

        ASSERT_EQ( actualAreaResults.size(), expectedAreaResults.size() );

        const double tolerance = 1e-4;
        for( size_t i = 0; i < expectedAreaResults.size(); ++i )
        {
            EXPECT_NEAR( actualAreaResults[i], expectedAreaResults[i], tolerance );
        }
    }

}