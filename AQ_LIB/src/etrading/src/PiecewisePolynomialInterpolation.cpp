#include "PiecewisePolynomialInterpolation.h"

#include <ExceptionMacros.h>
#include <algorithm>
#include <string>

namespace etrading
{

	PiecewisePolynomialInterpolation::PiecewisePolynomialInterpolation( const std::vector<double>& xValues, const std::vector<double>& yValues, const size_t& degree ) :
		PolynomialInterpolation(xValues, yValues, degree),
		xValues_(xValues)
	{
		AQ_REQUIRE(degree <= 1, "Currently only flat(degree 0) and linear(degree 1) interpolations are supported")

		piecewisePolynomialCoefficients_.resize(xValues.size() - 1, degree + 1);

		switch (degree)
		{
		case 0 :
			for (int i = 0; i < piecewisePolynomialCoefficients_.rows(); i++)
				piecewisePolynomialCoefficients_(i, 0) = yValues[i];
			break;
		case 1:
		default:
			for (int i = 0; i < piecewisePolynomialCoefficients_.rows(); i++)
			{
				piecewisePolynomialCoefficients_(i, 0) = (xValues[i + 1] * yValues[i] - xValues[i] * yValues[i + 1]) / (xValues[i + 1] - xValues[i]);
				piecewisePolynomialCoefficients_(i, 1) = (yValues[i + 1] - yValues[i]) / (xValues[i + 1] - xValues[i]);
			}
			break;
		}
	}

	// Copy Constructor
	PiecewisePolynomialInterpolation::PiecewisePolynomialInterpolation( const PiecewisePolynomialInterpolation& rhs ) :
		PolynomialInterpolation(rhs),
		xValues_( rhs.xValues_),
		piecewisePolynomialCoefficients_( rhs.piecewisePolynomialCoefficients_)
	{}

	// Assignment Operator
	PiecewisePolynomialInterpolation& PiecewisePolynomialInterpolation::operator=( const PiecewisePolynomialInterpolation& rhs )
	{
		// Performance 
		if (&rhs == this)
		{
			return *this;
		}
		
		PolynomialInterpolation::operator=(rhs);
		piecewisePolynomialCoefficients_ = rhs.piecewisePolynomialCoefficients_;
		xValues_ = rhs.xValues_;
			 
		return *this;
	}

	// Clone
	PolynomialInterpolationPtr PiecewisePolynomialInterpolation::clone()
	{
		PolynomialInterpolationPtr PolyInterPtr = PolynomialInterpolationPtr( new PiecewisePolynomialInterpolation(*this) );
		return PolyInterPtr;
	}

	double PiecewisePolynomialInterpolation::interpolate( const double& x ) const
	{
		auto it = std::upper_bound(xValues_.begin(), xValues_.end(), x);

		if (it == xValues_.begin() || it == xValues_.end())
			AQ_REQUIRE(false, "Value x out of range" );

		int i = std::distance(it, xValues_.begin()) - 1;
		double y;

		switch (polynomialDegree_)
		{
		case 0:
			y = piecewisePolynomialCoefficients_(i, 0);
			break;
		case 1:
		default:
			y = piecewisePolynomialCoefficients_(i, 0) + piecewisePolynomialCoefficients_(i, 1) * x;
			break;
		}

		return y;
	}

	double PiecewisePolynomialInterpolation::integrate( const double & lowerBound, const double & upperBound ) const
	{  
		auto it = std::upper_bound(xValues_.begin(), xValues_.end(), lowerBound);

		if (it == xValues_.begin() || it == xValues_.end())
			AQ_REQUIRE(false, "Value lowerBound out of range" );

		int m = std::distance(it, xValues_.begin()) - 1;

		it = std::upper_bound(xValues_.begin(), xValues_.end(), upperBound);

		if (it == xValues_.begin() || it == xValues_.end())
			AQ_REQUIRE(false, "Value upperBound out of range" );

		int n = std::distance(it, xValues_.begin()) - 1;

		double integral = 0.0;
		double delta;

		switch (polynomialDegree_)
		{
		case 0:
			for (int i = m + 1; i < n; i++)
				integral += piecewisePolynomialCoefficients_(i, 0) * (xValues_[i + 1] - xValues_[i]);

			integral += piecewisePolynomialCoefficients_(m, 0) * (xValues_[m + 1] - lowerBound);
			integral += piecewisePolynomialCoefficients_(n, 0) * (upperBound - xValues_[n]);
			break;
		case 1:
		default:
			for (int i = m + 1; i < n; i++)
			{
				delta = xValues_[i + 1] - xValues_[i];
				integral += piecewisePolynomialCoefficients_(i, 0) * delta + 0.5 * piecewisePolynomialCoefficients_(i, 1) * delta * delta;
			}

			delta = xValues_[m + 1] - lowerBound;
			integral += piecewisePolynomialCoefficients_(m, 0) * delta + 0.5 * piecewisePolynomialCoefficients_(m, 1) * delta * delta;
			delta = upperBound - xValues_[n];
			integral += piecewisePolynomialCoefficients_(n, 0) * delta + 0.5 * piecewisePolynomialCoefficients_(n, 1) * delta * delta;
			break;
		}

		return integral;
	}

	double PiecewisePolynomialInterpolation::differentiate( const double & x ) const
	{  
		auto it = std::upper_bound(xValues_.begin(), xValues_.end(), x);

		if (it == xValues_.begin() || it == xValues_.end())
			AQ_REQUIRE(false, "Value x out of range" );

		int i = std::distance(it, xValues_.begin()) - 1;
		double dydx;

		switch (polynomialDegree_)
		{
		case 0:
			dydx = 0;
			break;
		case 1:
		default:
			dydx = piecewisePolynomialCoefficients_(i, 1);
			break;
		}

		return dydx;  
	}
	
}
