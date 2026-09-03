/*
 * @brief			Hull White Model
 * @Created:		13th Octt 2017
 * @Author:			Yongyan Zheng
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "VasicekModel.h"
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/constraint.hpp>

namespace etrading
{

	struct VasicekFittingResult
	{
		double theta;
		double sigma;
		double lse; // least squared error
		double iteration; 
		double gradient;
	};


  class VasicekFittingCostFunction : public QuantLib::CostFunction 
   {

	public :

		VasicekFittingCostFunction() {}
		virtual ~VasicekFittingCostFunction(){}

		VasicekFittingCostFunction (const std::shared_ptr<VasicekModel>& modelHelper, const std::string& curveCollection, const std::string& curveIndex, const DateVector& fixingDates, const DoubleVector& targetValues, const double& rt, const bool& futureValuation);

		VasicekFittingCostFunction( const VasicekFittingCostFunction& rhs );
		VasicekFittingCostFunction& operator =(const VasicekFittingCostFunction& rhs);
		
		// The following are override methods of QuantLib::CostFunction

		// Return the sum of the squares of the errors
		virtual QuantLib::Real value ( const QuantLib::Array& x) const;
		// Return the set of erros over each quote
		virtual QuantLib::Array values(const QuantLib::Array& x) const;

	protected:
		virtual DoubleVector getFittedValues(const QuantLib::Array& x) const;


	private :

		std::shared_ptr<VasicekModel> modelHelper_;
		DateVector fixingDates_;
		DoubleVector targetValues_;
		std::string curveCollection_;
		std::string curveIndex_;
		double rt_;
		bool futureValuation_;

	};


	class VasicekFittingConstraint: public QuantLib::Constraint
	{
	public:
		VasicekFittingConstraint(double maxTheta, double minTheta, double maxSigma, double minSigma) 
			: QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>( new VasicekFittingConstraint::Impl(maxTheta, minTheta, maxSigma, minSigma) ) )
	    {}

	private:
		
		class Impl : public QuantLib::Constraint::Impl
	    {
		public:

			Impl(double maxTheta, double minTheta, double maxSigma, double minSigma);
			bool test(const QuantLib::Array& p) const;

	    private:
			double maxTheta_;
			double minTheta_;
			double maxSigma_;
			double minSigma_;
		}; // end of Impl class

	}; // end of VasicekFittingConstraint class

  	//Fit model to the Initial Forward Curve, by chaning theta and sigma parameter
	VasicekFittingResult calibrateVasicekToForwardCurve(const double& alpha, const double& theta, const double& sigma, const std::string& curveCollection, const std::string& curveIndex, const DateVector& fixingDates, const DoubleVector& targetForwardRates, const double& rt, const bool& futureValuation);


}

