#include "NormalDistribution.h"
#include "AQLDist.h"
#include "ExceptionMacros.h"

 // AAD Library
#include <adept.h>

namespace etrading
{
    // Local Boundary Definitions
    // These boundaries should actually be +/- infinity but we chose to use +/- 10 to be consistent with Excel NormsInv function
    // This does not cause a problem, becuase normal variates taken at +/- 10 have already converged to the limit to high precision.
    const double LOWER_BOUND = -10.0;
    const double UPPER_BOUND = 10.0;


    // Calculate the standard normal distribution probability phi, given the standard normal variate x
	// This function is templated on "xdouble", which may be double or adept::adouble.
	// i.e. there is an AAD and regular double version of this function.
	template<typename xdouble>
	xdouble standardNormalDistribution( const xdouble & x )
	{
		const xdouble MM_SQRT2 = 1.41421356237309504880168872420969807856967187537694;
		const xdouble MM_1_SQRTPI = 0.564189583547756286948;

		const xdouble a[5] = {
			1.161110663653770e-002, 3.951404679838207e-001, 2.846603853776254e+001,
			1.887426188426510e+002, 3.209377589138469e+003
		};
		const xdouble b[5] = {
			1.767766952966369e-001, 8.344316438579620e+000, 1.725514762600375e+002,
			1.813893686502485e+003, 8.044716608901563e+003
		};
		const xdouble c[9] = {
			2.15311535474403846e-8, 5.64188496988670089e-1, 8.88314979438837594e00,
			6.61191906371416295e01, 2.98635138197400131e02, 8.81952221241769090e02,
			1.71204761263407058e03, 2.05107837782607147e03, 1.23033935479799725E03
		};
		const xdouble d[9] = {
			1.00000000000000000e00, 1.57449261107098347e01, 1.17693950891312499e02,
			5.37181101862009858e02, 1.62138957456669019e03, 3.29079923573345963e03,
			4.36261909014324716e03, 3.43936767414372164e03, 1.23033935480374942e03
		};
		const xdouble p[6] = {
			1.63153871373020978e-2, 3.05326634961232344e-1, 3.60344899949804439e-1,
			1.25781726111229246e-1, 1.60837851487422766e-2, 6.58749161529837803e-4
		};
		const xdouble q[6] = {
			1.00000000000000000e00, 2.56852019228982242e00, 1.87295284992346047e00,
			5.27905102951428412e-1, 6.05183413124413191e-2, 2.33520497626869185e-3
		};

		xdouble y, z;

		// NOTE: fabs() does not appear to be overloaded in Adept to work with the adouble type.
		// Meanwhile, abs() *is* overloaded, however on Linux/Intel Compiler, this invokes
		// the int version of the function. So we provide a version of abs() which works everywhere:
		y = x > 0. ? x : -1.0 * x;
		if (y <= 0.46875 * MM_SQRT2)
		{
			// evaluate erf() for |x| <= sqrt(2)*0.46875
			z = y * y;
			y = x * ((((a[0] * z + a[1])*z + a[2])*z + a[3])*z + a[4])
				/ ((((b[0] * z + b[1])*z + b[2])*z + b[3])*z + b[4]);
			return 0.5 + y;
		}
		z = exp(-y*y / 2.0) / 2.0;
		if (y <= 4.0 * MM_SQRT2)
		{
			// evaluate erfc() for sqrt(2)*0.46875 <= |x| <= sqrt(2)*4.0
			y = y / MM_SQRT2;
			y = ((((((((c[0] * y + c[1])*y + c[2])*y + c[3])*y + c[4])*y + c[5])*y + c[6])*y + c[7])*y + c[8])
				/ ((((((((d[0] * y + d[1])*y + d[2])*y + d[3])*y + d[4])*y + d[5])*y + d[6])*y + d[7])*y + d[8]);

			y = z * y;
		}
		else
		{
			// evaluate erfc() for |x| > sqrt(2)*4.0
			z = z * MM_SQRT2 / y;
			y = 2 / (y*y);
			y = y * (((((p[0] * y + p[1])*y + p[2])*y + p[3])*y + p[4])*y + p[5])
				/ (((((q[0] * y + q[1])*y + q[2])*y + q[3])*y + q[4])*y + q[5]);
			y = z * (MM_1_SQRTPI - y);
		}
		return (x < 0.0 ? y : 1 - y);
	}
	// Explicit instantiations
	template double standardNormalDistribution<double>( const double & x );
	template adept::adouble standardNormalDistribution<adept::adouble>( const adept::adouble & x );
	
	// Calculate the standard normal distribution probability density function value pdf, given the standard normal variate x
	// This function is templated on "xdouble", which may be double or adept::adouble.
	// i.e. there is an AAD and regular double version of this function.
	template<typename xdouble>
	xdouble standardNormalDistributionPDF( const xdouble & x )
	{
		xdouble result = ONE_OVER_SQRT_TWO_PI * exp(-0.5 * (x*x));
		return result;
	}
	// Explicit instantiations
	template double standardNormalDistributionPDF<double>( const double & x );
	template adept::adouble standardNormalDistributionPDF<adept::adouble>( const adept::adouble & x );



    // Calculate the standard normal variate z,  given the standard normal probability phi
    const double standardNormalDistributionInverse( const double & phi )
    {
        // Note we include epsilon precision to manage double over- and underflow issues
        AQ_REQUIRE( AQ_IS_GREATER_THAN_OR_EQUAL( phi, 0.0 ), "Probability phi cannot be less than zero" );
        AQ_REQUIRE( AQ_IS_LESS_THAN_OR_EQUAL( phi, 1.0 ), "Probability phi cannot be greater than one" );
        
        // Boundary Conditions
        if ( AQ_IS_EQUAL_ZERO( phi ) )
        {
            // Return z = Lower Bound when probability phi = zero
            const double result = LOWER_BOUND;
            return result;
        }

        if ( AQ_IS_EQUAL( phi, 1.0 ) )
        {
            // Return z = Upper Bound when probability phi = zero
            const double result = UPPER_BOUND;
            return result;
        }
            

        const double result = AQLDist::invNormdist( phi );
        return result;
    }
    

    // Calculate a non-standard normal distribution probability phi, given the standard normal variate z and the mean and variance of the normal distribution
    const double normalDistribution( const double & x, const double & mean, const double & variance )
    {
        // Note we include epsilon precision to manage double over- and underflow issues
        AQ_REQUIRE( AQ_IS_GREATER_THAN_ZERO( variance ), "Variance cannot be negative" );

        double z = 0.0;

        if ( AQ_IS_EQUAL_ZERO( variance ) )
        {
            // Boundary Case: Central Limit Theorem Variance adjusted by epsilon
            z = ( x - mean ) / ( variance + AQ_EPSILON );
        }
        else
        {
            // Central Limit Theorem
            z = ( x - mean ) / variance;
        }

        const double result = standardNormalDistribution( z );
        return result;
    }


    // Calculate the non-standard normal distribution probability density function value pdf, given the standard normal variate z and the mean and variance of the normal distribution
    const double normalDistributionPDF( const double & x, const double & mean, const double & variance )
    {
        // Note we include epsilon precision to manage double over- and underflow issues
        AQ_REQUIRE( AQ_IS_GREATER_THAN_ZERO( variance ), "Variance cannot be negative" );

        double z = 0.0;

        if ( AQ_IS_EQUAL_ZERO( variance ) )
        {
            // Boundary Case: Central Limit Theorem Variance adjusted by epsilon
            z = ( x - mean ) / ( variance + AQ_EPSILON );
        }
        else
        {
            // Central Limit Theorem
            z = ( x - mean ) / variance;
        }

        const double result = standardNormalDistributionPDF( z );
        return result;
    }


    // Calculate the non-standard normal variate z,  given the standard normal probability phi  and the mean and variance of the normal distribution
    const double normalDistributionInverse( const double & phi, const double & mean, const double & variance )
    {
        // Note we include epsilon precision to manage double over- and underflow issues
        AQ_REQUIRE( AQ_IS_GREATER_THAN_OR_EQUAL( phi, 0.0 ), "Probability phi cannot be less than zero" );
        AQ_REQUIRE( AQ_IS_LESS_THAN_OR_EQUAL( phi, 1.0 ), "Probability phi cannot be greater than one" );
        AQ_REQUIRE( AQ_IS_GREATER_THAN_ZERO( variance ), "Variance cannot be negative" );

        // Boundary Conditions
        if ( AQ_IS_EQUAL_ZERO( phi ) )
        {
            // Return z = Lower Bound when probability phi = zero
            const double result = LOWER_BOUND;
            return result;
        }

        if ( AQ_IS_EQUAL( phi, 1.0 ) )
        {
            // Return z = Upper Bound when probability phi = zero
            const double result = UPPER_BOUND;
            return result;
        }

        // Standard Normal Inverse, z
        const double z = standardNormalDistributionInverse( phi );

        // Central Limit Theorem
        // z = ( x - mean ) / variance
        // => x = ( z . variance ) + mean
        
        const double x = ( z * variance ) + mean;
        return x;
    }
}