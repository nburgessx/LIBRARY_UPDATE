/*
* @brief			Polynomial Interpolation
* @Created:		    December 18 2019
* @Author:			Khalid Shafiq
* @Department:	    Quantitative Strategies, New York
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/

#include "PolynomialInterpolation.h"
#include "ExceptionMacros.h"
#include <string>

namespace etrading
{

	PolynomialInterpolation::PolynomialInterpolation( const std::vector<double>& xValues, const std::vector<double>& yValues, const size_t& degree )
	{
		MLIB_REQUIRE( xValues.size() == yValues.size(), "Invalid Polynomial Data: The number of xValues and yValues must be the same" )
		
		// TODO: Relax this constraint. It appears that the underlying code breaks down for higher order polynomials
		// This class breaks down around degree 12-15, subject to the number of data nodes used.
		MLIB_REQUIRE( degree < 11, "Invalid Polynomial Degree: Currently the Max Polynomial Degree Supported is 10" )

		// Boundary Condition: Max Polynomial Degree is capped at the number of data points minus one.
		// A larger degree would lead to an overdetermined system with many solutions and no unique solution.
		// We make this a silent minimum rather than throwing an error
		polynomialDegree_ = std::min( degree, xValues.size() - 1 );

		Vector1d b( yValues.size(), 1 );
		for ( int r = 0; r < b.rows(); ++r )
		{
			b(r, 0) = yValues[r];
		}

		MatrixXd vm = createVandermondeMatrix( xValues, polynomialDegree_ ); // note the degree is capped (see above note)
		polynomialCoefficients_ = vm.colPivHouseholderQr().solve(b);

		// TODO: Calculate the Total Root Mean Squared Error (RMSE) or Average RMSE on each node, perhaps later is more useful
		fittingError_ = 0.0;
	}

	// Copy Constructor
	PolynomialInterpolation::PolynomialInterpolation( const PolynomialInterpolation& rhs )
	 : polynomialCoefficients_(rhs.polynomialCoefficients_), polynomialDegree_( rhs.polynomialDegree_ ), fittingError_(rhs.fittingError_)
	{
	}

	// Assignment Operator
	PolynomialInterpolation& PolynomialInterpolation::operator=( const PolynomialInterpolation& rhs )
	{
		// Performance 
		if (&rhs == this)
		{
			return *this;
		}

		// For exception safety:
		PolynomialInterpolation temp(rhs);  
		std::swap( polynomialDegree_, temp.polynomialDegree_ );
		std::swap( polynomialCoefficients_, temp.polynomialCoefficients_ );
		std::swap( fittingError_, temp.fittingError_ );
	 
		return *this;
	}

	// Clone
	PolynomialInterpolationPtr PolynomialInterpolation::clone()
	{
		PolynomialInterpolationPtr PolyInterPtr = PolynomialInterpolationPtr( new PolynomialInterpolation(*this) );
		return PolyInterPtr;
	}


	double PolynomialInterpolation::interpolate( const double& x ) const
	{
		const size_t nCoefficients = polynomialCoefficients_.size();
		MLIB_REQUIRE( polynomialDegree_ == nCoefficients - 1, "Invalid Interpolation Polynomial: The number of coefficients and the order of the polynomial are inconsistent" )

		/* We implement Horner's method for evaluating the polynomial.
		*  This approach is more efficient than naively evaluating the polynomial terms
		*  and reduces the number of multiplications required.
		*  https://en.wikipedia.org/wiki/Horner%27s_method
		*
		*  if y = d + cx + bx^2 + ax^3
		*  then we can write:
		*     y = d + x( c + x( b + x( a ) ) )
		*/

		// Initialize y with the final coefficient. This is the coefficient of x^n
		double y = polynomialCoefficients_( nCoefficients-1, 0 );

		// Now loop over the remaining coefficients, if any
		// Note this is a count-down loop. So we need to use an int for the loop counter.
		for ( int coefficientIdx = nCoefficients-2; coefficientIdx >=0; coefficientIdx-- )
		{
			y = y * x + polynomialCoefficients_( coefficientIdx, 0 );
		}
		return y;
	}

	std::vector<double> PolynomialInterpolation::interpolate( const std::vector<double>& x ) const
	{
		// Reserve Size for Performance
		const size_t nInterpolants = x.size();
		std::vector<double> y_values( nInterpolants );
		
		for ( size_t i = 0; i < nInterpolants; ++i )
		{
			y_values[i] = interpolate( x[i] );
		}
		return y_values;
	}

	double PolynomialInterpolation::integrate( const double & lowerBound, const double & upperBound ) const
	{
		const size_t nCoefficients = polynomialCoefficients_.size();
		MLIB_REQUIRE( polynomialDegree_ == nCoefficients - 1, "Invalid Interpolation Polynomial: The number of coefficients and the order of the polynomial are inconsistent" )

		/* We implement Horner's method for evaluating the derivative.
		*  This approach is more efficient than naively evaluating the polynomial terms
		*  and reduces the number of multiplications required.
		*  https://en.wikipedia.org/wiki/Horner%27s_method
		*
		*  if y   = d + cx + bx^2 + ax^3
		*  then:
		*  Int(y) = dx + (c/2)x^2 + (b/3)x^3 + (a/4)x^4
		*         = x( d + x( c/2 + x( b/3 + x( a/4 ) ) ) )
		*/

		// Initialize the sums with (final coefficient * x / ( degree + 1 )). This is the coefficient of x^(n+1)
		double degree = (double)polynomialDegree_;
		double coefficient = polynomialCoefficients_[ nCoefficients - 1 ];

		double upperSum = coefficient / ( degree + 1 );
		double lowerSum = coefficient / ( degree + 1 );
		
		/* Now loop over the remaining coefficients, if any.
		 * Note: This is a count-down loop. So we need to use an int for the loop counter.
		 */
		for (int coefficientIdx = nCoefficients - 2; coefficientIdx >= 0; coefficientIdx--, degree--)
		{
			coefficient = polynomialCoefficients_[ coefficientIdx ];
			upperSum = upperSum * upperBound + coefficient / degree;
			lowerSum = lowerSum * lowerBound + coefficient / degree;
		}
		upperSum *= upperBound;
		lowerSum *= lowerBound;

		return  upperSum - lowerSum;  
	}

	std::vector<double> PolynomialInterpolation::integrate(const std::vector<double> & lowerBounds, const std::vector<double> & upperBounds) const
	{
		const size_t nIntegrands = lowerBounds.size();
		MLIB_REQUIRE( nIntegrands == upperBounds.size(), "Invalid Integration Bounds: Number of lowerBounds and upperBounds must match.");
		
		// Reserve Size for Performance
		std::vector<double> integrands( nIntegrands );
	 
		for( size_t i = 0; i < nIntegrands; ++i )
		{
			integrands[i] = integrate( lowerBounds[i], upperBounds[i] );
		}
		return integrands;
	}

	// Instantaneous Differentiation
	double PolynomialInterpolation::differentiate( const double & x ) const
	{  
		const size_t nCoefficients = polynomialCoefficients_.size();
		MLIB_REQUIRE( polynomialDegree_ == nCoefficients - 1, "Invalid Interpolation Polynomial: The number of coefficients and the order of the polynomial are inconsistent" )

		/* We implement Horner's method for evaluating the derivative.
		*  This approach is more efficient than naively evaluating the polynomial terms
		*  and reduces the number of multiplications required.
		*  https://en.wikipedia.org/wiki/Horner%27s_method
		*
		*  if y   = d + cx + bx^2 + ax^3
		*  then:
		*  dydx   = 0 + c + 2bx + 3ax^2
		*         =     c + x( 2b + x( 3a ) )
		*/

		// Initialize dydx with the (final coefficient * degree). This is the coefficient of x^(n-1)
		double degree = (double) polynomialDegree_;
		double dydx = polynomialCoefficients_( nCoefficients - 1, 0 ) * degree;

		/* Now loop over the remaining coefficients, if any.
		 * Note: This is a count-down loop. So we need to use an int for the loop counter.
		 * Note: We do not include the final case where coefficientIdx == 0, since the constant term
		 * disappears in the differential.
		 */
		degree--;
		for ( int coefficientIdx = nCoefficients - 2; coefficientIdx > 0; coefficientIdx--, degree-- )
		{
			dydx = dydx * x + polynomialCoefficients_(coefficientIdx, 0) * degree;
		}
		return dydx;  
	}

	// Instantaneous Differentiation
	std::vector<double> PolynomialInterpolation::differentiate( const std::vector<double> & x ) const
	{
		size_t nPoints = x.size();

		// Reserve Size for Performance
		std::vector<double> dydx( nPoints );
	 
		for( size_t i = 0; i < nPoints; ++i )
		{
			dydx[i] = differentiate( x[i] );
		}
		return dydx;
	}

	// Discrete Differentiation over a range of X values
	double PolynomialInterpolation::differentiate( const double& fromXPoint, const double& toXPoint ) const
	{
		MLIB_REQUIRE( MLIB_IS_GREATER_THAN( toXPoint, fromXPoint ), "Invalid Differentiation Interval: The toXPoint must be greater than the fromXPoint" )
		
		const double fromYPoint	= interpolate( fromXPoint );
		const double toYPoint	= interpolate( toXPoint );

		const double dy			= toYPoint - fromYPoint;
		const double dx  		= toXPoint - fromXPoint;
				
		return dy/dx;
	}

	// Discrete Differentiation over a range of X values
	std::vector<double> PolynomialInterpolation::differentiate( const std::vector<double>& fromXPoints, const std::vector<double>& toXPoints ) const
	{
		const size_t nXPoints = fromXPoints.size();
		MLIB_REQUIRE( nXPoints == toXPoints.size(), "Invalid Differentiation Intervals: The number of fromXPoints and toXPoints must match")
		
		std::vector<double> results(nXPoints, 0.0);
		for( size_t i = 0; i < nXPoints; ++i )
		{
			results[i] = differentiate( fromXPoints[i], toXPoints[i] );
		}
		return results;
	}

	MatrixXd PolynomialInterpolation::createVandermondeMatrix( const std::vector<double>& xValues, const size_t degree )
	{  
		MatrixXd vm( xValues.size(), degree + 1 );
		for ( int r = 0; r < vm.rows(); ++r )
		{
			for ( int c = 0; c < vm.cols(); ++c )
			{
				vm(r, c) = std::pow( xValues[r], c );
			}
		}
		return vm;
	}
}
