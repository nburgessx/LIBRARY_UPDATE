#pragma once

#include <Eigen/Dense>
#include <Eigen/Cholesky>

#include <vector>
#include <memory>
#include <string>

using namespace Eigen;

namespace etrading
{
 
	class PolynomialInterpolation
	{
	public:
  
		typedef Matrix<double, Dynamic, Dynamic> MatrixXd;
		typedef Matrix<double, Dynamic, 1> Vector1d;

		// Constructor / Destructor
		PolynomialInterpolation() {};
		virtual ~PolynomialInterpolation() {};

		// Alternative Constructor
		PolynomialInterpolation( const std::vector<double>& xValues, const std::vector<double>& yValues, const size_t& polynomialDegree );

		// Copy Constructor
		PolynomialInterpolation( const PolynomialInterpolation& rhs );

		// Assignment Operator
		PolynomialInterpolation & operator=( const PolynomialInterpolation& rhs );

		// Clone
		virtual std::shared_ptr<PolynomialInterpolation> clone();

		// Class Accessors
		size_t polynomialDegree() const		{ return polynomialDegree_; }
		double fittingError() const			{ return fittingError_; }

		// Class Methods
		virtual double interpolate( const double& x ) const;
		virtual std::vector<double> interpolate( const std::vector<double>& x ) const;

		virtual double integrate( const double & lowerBound, const double & upperBound ) const;
		virtual std::vector<double> integrate( const std::vector<double> & lowerBounds, const std::vector<double> & upperBounds ) const;
    
		// Instantaneous Differentiation
		virtual double differentiate( const double& x ) const;
		virtual std::vector<double> differentiate( const std::vector<double>& x ) const;

		// Discrete Differentiation over a range of X values
		virtual double differentiate( const double& fromXPoint, const double& toXPoint ) const;
		virtual std::vector<double> differentiate( const std::vector<double>& fromXPoints, const std::vector<double>& toXPoints ) const;

	protected:
	
		size_t   polynomialDegree_;
		Vector1d polynomialCoefficients_;
		double   fittingError_;

	protected:
		
		MatrixXd createVandermondeMatrix( const std::vector<double>& xValues, const size_t degree );
	};
  
	typedef std::shared_ptr<PolynomialInterpolation> PolynomialInterpolationPtr;
}

