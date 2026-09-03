/*
* @brief			Piecewise Polynomial Interpolation
* @Created:		    February 24 2020
* @Author:			Arthur Wu
* @Department:	    Quantitative Strategies, New York
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/
#pragma once

#include "PolynomialInterpolation.h"

namespace etrading
{
 
	// inheriting from PolynomialInterpolation for the purpose of sharing interface
	class PiecewisePolynomialInterpolation : public PolynomialInterpolation
	{
	public:
  
		typedef Matrix<double, Dynamic, Dynamic> MatrixXd;
		typedef Matrix<double, Dynamic, 1> Vector1d;

		// Constructor / Destructor
		PiecewisePolynomialInterpolation() = default;
		virtual ~PiecewisePolynomialInterpolation() = default;

		// Alternative Constructor
		PiecewisePolynomialInterpolation( const std::vector<double>& xValues, const std::vector<double>& yValues, const size_t& polynomialDegree );

		// Copy Constructor
		PiecewisePolynomialInterpolation( const PiecewisePolynomialInterpolation& rhs );

		// Assignment Operator
		PiecewisePolynomialInterpolation & operator=( const PiecewisePolynomialInterpolation& rhs );

		// Clone
		virtual std::shared_ptr<PolynomialInterpolation> clone();

		// Class Accessors
		const std::vector<double>& xValues() const { return xValues_; }
		const MatrixXd& piecewisePolynomialCoefficients() const	{ return piecewisePolynomialCoefficients_; }

		// Class Methods
		virtual double interpolate( const double& x ) const;
		virtual double integrate( const double & lowerBound, const double & upperBound ) const;
		virtual double differentiate( const double& x ) const;

	protected:
		
		std::vector<double> xValues_;
		MatrixXd piecewisePolynomialCoefficients_;

	};
  
	typedef std::shared_ptr<PiecewisePolynomialInterpolation> PiecewisePolynomialInterpolationPtr;
}

