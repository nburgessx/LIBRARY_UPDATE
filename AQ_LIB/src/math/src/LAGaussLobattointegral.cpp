/*
*/

#include "LAGaussLobattointegral.h"


const double GaussLobattoIntegral::alpha_ = std::sqrt(2.0/3.0); 
const double GaussLobattoIntegral::beta_  = 1.0/std::sqrt(5.0);
const double GaussLobattoIntegral::x1_    = 0.94288241569547971906; 
const double GaussLobattoIntegral::x2_    = 0.64185334234578130578;
const double GaussLobattoIntegral::x3_    = 0.23638319966214988028;

using namespace std;

Integrator::Integrator(double absoluteAccuracy,
                       size_t maxEvaluations
					  )
:
absoluteAccuracy_(absoluteAccuracy),
maxEvaluations_(maxEvaluations)
{
	if(absoluteAccuracy < numeric_limits<double>::epsilon())
	{
		throw LACoreInvalidData("required tolerance is not allowed! @ Integrator::Integrator", __FILE__, __LINE__);
	}
}

void Integrator::setAbsoluteAccuracy(double accuracy)
{
    absoluteAccuracy_= accuracy;
}

void Integrator::setMaxEvaluations(size_t maxEvaluations)
{
    maxEvaluations_ = maxEvaluations;
}

double Integrator::absoluteAccuracy() const
{
    return absoluteAccuracy_;
}

size_t Integrator::maxEvaluations() const
{
    return maxEvaluations_;
}

double Integrator::absoluteError() const
{
    return absoluteError_;
}

void Integrator::setAbsoluteError(double error) const
{
    absoluteError_ = error;
}

size_t Integrator::numberOfEvaluations() const
{
    return evaluations_;
}

void Integrator::setNumberOfEvaluations(size_t evaluations) const
{
    evaluations_ = evaluations;
}

void Integrator::increaseNumberOfEvaluations(size_t increase) const
{
    evaluations_ += increase;
}

bool Integrator::integrationSuccess() const
{
    return evaluations_ <= maxEvaluations_ && absoluteError_ <= absoluteAccuracy_;
}

//
//--------------------------------------------------------------------------------
//

GaussLobattoIntegral::GaussLobattoIntegral(size_t maxIterations,
                                           double absAccuracy,
                                           double relAccuracy,
                                           bool useConvergenceEstimate)
:
Integrator(absAccuracy, maxIterations),
relAccuracy_(relAccuracy),
useConvergenceEstimate_(useConvergenceEstimate)
{
}
