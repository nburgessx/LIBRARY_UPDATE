#include "VasicekFitting.h"
#include "ExceptionMacros.h"
 
#include <ql/math/optimization/endcriteria.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>

namespace etrading
{

	VasicekFittingCostFunction::VasicekFittingCostFunction (const std::shared_ptr<VasicekModel>& modelHelper, const std::string& curveCollection, const std::string& curveIndex, 
															const DateVector& fixingDates, const DoubleVector& targetValues, const double& rt, const bool& futureValuation)
		: modelHelper_ (modelHelper), curveCollection_(curveCollection), curveIndex_(curveIndex), fixingDates_(fixingDates), targetValues_ (targetValues), rt_(rt), futureValuation_(futureValuation)
	{
        MLIB_REQUIRE( fixingDates.size() == targetValues.size(),  "The number of FixingDates and ForwardRates must be the same" );
	}

	VasicekFittingCostFunction::VasicekFittingCostFunction( const VasicekFittingCostFunction& rhs ) 
		: curveCollection_(rhs.curveCollection_), curveIndex_(rhs.curveIndex_), fixingDates_(rhs.fixingDates_), targetValues_ (rhs.targetValues_)
	{
		if (rhs.modelHelper_ != nullptr)
        {
            modelHelper_ = rhs.modelHelper_->clone();
        }
	}

    VasicekFittingCostFunction& VasicekFittingCostFunction::operator =(const VasicekFittingCostFunction& rhs)
	{
	    // 1. Make a temp copy
        VasicekFittingCostFunction temp( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( modelHelper_,      temp.modelHelper_ );
        std::swap( curveCollection_,      temp.curveCollection_ );
        std::swap( curveIndex_,      temp.curveIndex_ );
        std::swap( fixingDates_,      temp.fixingDates_ );
        std::swap( targetValues_,      temp.targetValues_ );

		return *this;
	}


	DoubleVector VasicekFittingCostFunction::getFittedValues(const QuantLib::Array& x) const
	{
        MLIB_REQUIRE( x.size() == 2,  "Vasicek Fitting parameter must be two: Theta, Sigma" );

		// 1) Set vol as input (changing) paramter
		modelHelper_->setTheta(x[0]);
		modelHelper_->setSigma(x[1]);

		// 2) Get ts and Ts of the forward rates:
		DoubleVector fittedForwardRates = modelHelper_->forwardRatesAnalytical(fixingDates_, curveCollection_, curveIndex_, rt_, futureValuation_);

		return fittedForwardRates;

	}

	// Override method of QuantLib::CostFunction
	// Return total LS error
	QuantLib::Real VasicekFittingCostFunction::value ( const QuantLib::Array & x) const 
	{

		QuantLib::Array diffSqs = values(x);

		QuantLib::Real diffSqTotal = 0.0;

		for(size_t i = 0; i < diffSqs.size(); ++i)
		{
			double diffSq = diffSqs[i];
			diffSqTotal += diffSq;
		}

		return  diffSqTotal;
	}

	// Override method of QuantLib::CostFunction
	// Return arrays of LS errors
	QuantLib::Array VasicekFittingCostFunction::values(const QuantLib::Array& x) const
	{
		DoubleVector fittedValues = getFittedValues(x);

		size_t expectedSize = fittedValues.size();

		QuantLib::Array diffSq (expectedSize);

		double factor = 1.0;

		for(size_t i = 0; i < expectedSize; ++i)
		{
			auto fittedVal = fittedValues[i];
			auto targetVal = targetValues_[i];
			double diff = fittedVal - targetVal;

			double dSq = diff * diff * factor;

			diffSq[i] = dSq;
		}

		return  diffSq;
	}


	VasicekFittingConstraint::Impl::Impl(double maxTheta, double minTheta, double maxSigma, double minSigma) : maxTheta_(maxTheta), minTheta_(minTheta), maxSigma_(maxSigma), minSigma_(minSigma)
    {}
        
    bool VasicekFittingConstraint::Impl::test(const QuantLib::Array& p) const
    {
		double theta = p[0];
		double sigma = p[1];
	
		if(theta < minTheta_ || theta > maxTheta_ )
		{
			return false;
		}

		if(sigma < minSigma_ || sigma > maxSigma_ )
		{
			return false;
		}

		return true;
    }


	//Fit Vasicek model to the Intitial Forward Curve, by chaning sigma parameter
	VasicekFittingResult calibrateVasicekToForwardCurve(const double& alpha, const double& theta, const double& sigma, const std::string& curveCollection, const std::string& curveIndex, const DateVector& fixingDates, const DoubleVector& targetForwardRates, const double& rt, const bool& futureValuation)
	{
		//Set up Cost Function
		const std::shared_ptr<VasicekModel>& modelHelper = std::make_shared<VasicekModel>( VasicekModel(alpha, theta, sigma));

		VasicekFittingCostFunction VasicekCostFunction(modelHelper, curveCollection, curveIndex, fixingDates, targetForwardRates, rt, futureValuation);

		//Set up Constraint

		double maxTheta = 1;
		double minTheta = 0.0;

		double maxSigma = 1;
		double minSigma = 0.0;

		VasicekFittingConstraint VasicekConstraint(maxTheta, minTheta, maxSigma, minSigma);
		   
		const double lmEpsilon = 1e-10;

		// parameters specific for Levenberg-Marquardt
		double levenbergMarquardtEpsfcn = lmEpsilon;      
		double levenbergMarquardtXtol   = lmEpsilon;	    
		double levenbergMarquardtGtol   = lmEpsilon;      

		QuantLib::LevenbergMarquardt solver (levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol);

		unsigned int maxIteration                = static_cast<unsigned int>(1000); 
		unsigned int maxStationaryStateIteration = static_cast<unsigned int>(100);
		double rootEpsilon         = lmEpsilon; 
		double functionEpsilon     = lmEpsilon; 
		double gradientNormEpsilon = lmEpsilon;

		QuantLib::EndCriteria VasicekEndCriteria(maxIteration, 
											maxStationaryStateIteration,
											rootEpsilon, 
											functionEpsilon, 
											gradientNormEpsilon);

		// Set the variables to change in the Solver
		QuantLib::Array x(2);
		x[0] = theta;
		x[1] = sigma;

		QuantLib::Problem VasicekProblem (VasicekCostFunction, VasicekConstraint, x);

		QuantLib::EndCriteria::Type result = solver.minimize(VasicekProblem, VasicekEndCriteria);

		VasicekFittingResult fittingResult;
		
		fittingResult.theta = modelHelper->getTheta();
		fittingResult.sigma = modelHelper->getSigma();
		fittingResult.lse = VasicekProblem.functionValue();
		fittingResult.iteration = VasicekProblem.functionEvaluation();
		fittingResult.gradient = VasicekProblem.gradientNormValue();

		return fittingResult;

	}


}
